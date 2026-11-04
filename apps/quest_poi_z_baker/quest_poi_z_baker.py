#!/usr/bin/env python3
"""Quest POI Z baker.

Resolves walkable Z candidates for every quest_poi point:

  1. navmesh polys by (x, y) region  -> walkable levels (primary; narrow WMOs included)
  2. creature/GO spawn Zs nearby     -> 1D-clustered semantic anchors (outlier-culled)
  3. quest area triggers             -> exact Z where present

Points are grouped by navmesh tile so the poly set of each tile group is walked
once. Candidates are clustered by Z, evidence-scored, ranked, and emitted as
`playerbots_quest_poi_z` rows (rank <= 4 per point, with walkable sample points
for the runtime attempt-jitter follow-up).

The committed SQL in ``data/playerbots_quest_poi_z.sql`` is the exact output of
this script; ``data/manifest.json`` records the source database, mmaps path,
script/SQL checksums, and row counts.

Run the full bake:

    python3 apps/quest_poi_z_baker/quest_poi_z_baker.py --all \
        --out apps/quest_poi_z_baker/data --mmaps /path/to/mmaps

Run a subset (development):

    python3 apps/quest_poi_z_baker/quest_poi_z_baker.py --quests 62,76 \
        --out /tmp/poi-z-bake --mmaps /path/to/mmaps
"""

from __future__ import annotations

import argparse
import bisect
import csv
import datetime
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path

from db import DEFAULTS, configure, sql
from terrain import coords
from terrain.detour import NavMesh
from terrain.mesh import Bounds, collect_mesh

SCAN_RADIUS = 40.0          # yd around the POI point
CLUSTER_GAP = 8.0           # yd z-gap that splits clusters
NAV_POLY_MIN = 1            # polys needed to call a level navmesh-walkable
MAX_RANKS = 4
MAX_WALK_POINTS = 6

# Geometry helpers -----------------------------------------------------------

def cluster_levels(entries: list[tuple[float, dict]], gap: float) -> list[dict]:
    """1D-cluster (z, payload) pairs by z with a gap threshold. Returns
    [{zmin, zmax, zmean, items: [payload...]}]."""
    if not entries:
        return []
    entries = sorted(entries, key=lambda e: e[0])
    clusters: list[dict] = []
    cur = {"zmin": entries[0][0], "zmax": entries[0][0], "items": [entries[0][1]]}
    prev = entries[0][0]
    for z, payload in entries[1:]:
        if z - prev <= gap:
            cur["items"].append(payload)
            cur["zmax"] = z
        else:
            clusters.append(cur)
            cur = {"zmin": z, "zmax": z, "items": [payload]}
        prev = z
    for c in clusters:
        c["zmean"] = (c["zmin"] + c["zmax"]) / 2.0
    cur["zmean"] = (cur["zmin"] + cur["zmax"]) / 2.0
    clusters.append(cur)
    return clusters


def dist2d(ax, ay, bx, by) -> float:
    return math.hypot(ax - bx, ay - by)


# Data loading ---------------------------------------------------------------

def load_pois(quest_ids: list[int]) -> list[dict]:
    idlist = ",".join(map(str, quest_ids))
    return sql(f"SELECT p.QuestID, p.id AS PoiIdx, p.ObjectiveIndex, p.MapID, "
               f"pt.Idx2 AS PointIdx, pt.X, pt.Y "
               f"FROM quest_poi p JOIN quest_poi_points pt "
               f"ON pt.QuestID = p.QuestID AND pt.Idx1 = p.id "
               f"WHERE p.QuestID IN ({idlist}) ORDER BY p.QuestID, p.id, pt.Idx2")


def load_quest_objective_entries(quest_ids: list[int]) -> dict[int, dict]:
    """questId -> {creatureEntries: [...], goEntries: [...], itemIds: [...]}"""
    idlist = ",".join(map(str, quest_ids))
    out: dict[int, dict] = {}
    for r in sql(f"SELECT ID, RequiredNpcOrGo1, RequiredNpcOrGo2, RequiredNpcOrGo3, RequiredNpcOrGo4, "
                 f"RequiredItemId1, RequiredItemId2, RequiredItemId3, RequiredItemId4, "
                 f"RequiredItemId5, RequiredItemId6 FROM quest_template WHERE ID IN ({idlist}) ORDER BY ID"):
        qid = int(r["ID"])
        cre, go, items = [], [], []
        for i in range(1, 5):
            v = int(r[f"RequiredNpcOrGo{i}"] or 0)
            if v > 0:
                cre.append(v)
            elif v < 0:
                go.append(-v)
        for i in range(1, 7):
            v = int(r[f"RequiredItemId{i}"] or 0)
            if v:
                items.append(v)
        out[qid] = {"cre": cre, "go": go, "items": items}
    return out


def load_dropper_entries(items: set[int]) -> dict[int, set[int]]:
    """itemId -> creature entries that can drop it (direct + reference loot)."""
    if not items:
        return {}
    il = ",".join(map(str, sorted(items)))
    out: dict[int, set[int]] = defaultdict(set)
    for r in sql(f"SELECT Item, Entry FROM creature_loot_template "
                 f"WHERE Reference = 0 AND Item IN ({il}) ORDER BY Item, Entry"):
        out[int(r["Item"])].add(int(r["Entry"]))
    for r in sql(f"SELECT b.Item, a.Entry FROM reference_loot_template b "
                 f"JOIN creature_loot_template a ON a.Reference = b.Entry "
                 f"WHERE b.Item IN ({il}) ORDER BY b.Item, a.Entry"):
        out[int(r["Item"])].add(int(r["Entry"]))
    return out


def load_enders(quest_ids: list[int]) -> dict[int, set[int]]:
    """questId -> ender creature entries (turn-in ground truth)."""
    idlist = ",".join(map(str, quest_ids))
    out: dict[int, set[int]] = defaultdict(set)
    for r in sql(f"SELECT quest, id FROM creature_questender WHERE quest IN ({idlist}) ORDER BY quest, id"):
        out[int(r["quest"])].add(int(r["id"]))
    return out


def load_area_triggers(quest_ids: list[int]) -> dict[int, list[dict]]:
    idlist = ",".join(map(str, quest_ids))
    out: dict[int, list[dict]] = defaultdict(list)
    for r in sql(f"SELECT a.quest, t.map, t.x, t.y, t.z FROM areatrigger_involvedrelation a "
                 f"JOIN areatrigger t ON t.entry = a.id WHERE a.quest IN ({idlist}) ORDER BY a.quest, t.entry"):
        out[int(r["quest"])].append({"map": int(r["map"]), "x": float(r["x"]),
                                     "y": float(r["y"]), "z": float(r["z"])})
    return out


def load_spawns_for_map(map_id: int) -> tuple[list, list, list, list]:
    """All creature/GO spawns on the map, x-sorted, with prebuilt x arrays
    ((spawns, xs) each) so callers bisect without rebuilding."""
    cre = [(float(r["position_x"]), float(r["position_y"]), float(r["position_z"]), int(r["id"]))
           for r in sql(f"SELECT id, position_x, position_y, position_z FROM creature WHERE map = {map_id} ORDER BY id")]
    go = [(float(r["position_x"]), float(r["position_y"]), float(r["position_z"]), int(r["id"]))
          for r in sql(f"SELECT id, position_x, position_y, position_z FROM gameobject WHERE map = {map_id} ORDER BY id")]
    cre.sort(key=lambda s: s[0])
    go.sort(key=lambda s: s[0])
    return cre, [s[0] for s in cre], go, [s[0] for s in go]


def spawns_in_radius(xs: list, spawns: list, x: float, y: float, radius: float) -> list:
    """Prebuilt x-array + x-sorted spawns -> spawns within radius (bisect on x)."""
    lo = bisect.bisect_left(xs, x - radius)
    hi = bisect.bisect_right(xs, x + radius)
    return [s for s in spawns[lo:hi] if abs(s[1] - y) <= radius
            and math.hypot(s[0] - x, s[1] - y) <= radius]


# Per-point baking -----------------------------------------------------------

def bake_point(x: float, y: float, near_polys: list, truth_zs: list[float],
               trigger_zs: list[float], cre_near: list, go_near: list) -> list[dict]:
    """Ranked candidates for one point from its pre-collected evidence.

    near_polys: [(cx, cy, avgz, wpts)] already within the scan radius.
    cre_near / go_near: spawns already within the scan radius.
    """
    nav_levels = [(avgz, {"cx": cx, "cy": cy, "cz": avgz})
                  for (cx, cy, avgz, wpts) in near_polys]
    nav_clusters = cluster_levels(nav_levels, CLUSTER_GAP)

    spawn_pts = [(s[2], {"entry": s[3], "x": s[0], "y": s[1], "z": s[2]})
                 for s in cre_near]
    spawn_pts += [(s[2], {"entry": s[3], "x": s[0], "y": s[1], "z": s[2], "go": True})
                  for s in go_near]
    spawn_clusters = cluster_levels(spawn_pts, CLUSTER_GAP)

    raw = [c["zmean"] for c in nav_clusters if len(c["items"]) >= NAV_POLY_MIN]
    raw += [c["zmean"] for c in spawn_clusters]
    raw += list(trigger_zs)
    final = cluster_levels([(z, None) for z in raw], CLUSTER_GAP)

    candidates = []
    for cl in final:
        zc = cl["zmean"]
        nav_hits = [it for c in nav_clusters for it in c["items"]
                    if abs(it["cz"] - zc) <= CLUSTER_GAP]
        spawn_hits = [it for c in spawn_clusters for it in c["items"]
                      if abs(c["zmean"] - zc) <= CLUSTER_GAP]
        quest_spawn_hits = any(abs(it["z"] - tz) <= CLUSTER_GAP
                               for it in spawn_hits for tz in truth_zs)
        trigger_hit = any(abs(z - zc) <= CLUSTER_GAP for z in trigger_zs)

        conf = 0
        evid = []
        nav_ok = len(nav_hits) >= NAV_POLY_MIN
        if quest_spawn_hits:
            conf = 90
            evid.append("quest-spawn")
        if trigger_hit:
            conf = max(conf, 85)
            evid.append("trigger")
        if nav_hits:
            conf = max(conf, 45)
            evid.append(f"navmesh={len(nav_hits)}")
        if spawn_hits:
            conf = max(conf, 30)
            evid.append(f"spawns={len(spawn_hits)}")
        if nav_hits:
            conf = min(100, conf + min(8, len(nav_hits)))
        candidates.append({
            "z": round(zc, 2),
            "nav_polys": len(nav_hits),
            "spawns": len(spawn_hits),
            "quest_spawn": bool(quest_spawn_hits),
            "trigger": trigger_hit,
            "nav_ok": nav_ok,
            "confidence": int(conf),
            "evidence": ";".join(evid) if evid else "none",
            "walkable": [(round(it["cx"], 1), round(it["cy"], 1), round(it["cz"], 1))
                         for it in sorted(nav_hits, key=lambda it: math.hypot(it["cx"] - x, it["cy"] - y))[:MAX_WALK_POINTS]],
        })

    candidates.sort(key=lambda c: (-c["confidence"], -c["nav_polys"]))
    seen_z = set()
    deduped = []
    for c in candidates:
        if any(abs(c["z"] - z) <= CLUSTER_GAP for z in seen_z):
            continue
        seen_z.add(c["z"])
        deduped.append(c)
    return deduped[:MAX_RANKS]


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(Path(__file__).resolve().parent / "data"),
                    help="output directory (default: apps/quest_poi_z_baker/data)")
    ap.add_argument("--quests", default="",
                    help="comma-separated quest ids: bake exactly these (tags: cli)")
    ap.add_argument("--all", action="store_true",
                    help="bake every quest that has POI points (the full bake)")
    ap.add_argument("--mmaps", default="",
                    help="mmaps directory (default: ACORE_DATA_PATH/mmaps)")
    ap.add_argument("--db-host", default=None)
    ap.add_argument("--db-port", type=int, default=None)
    ap.add_argument("--db-user", default=None)
    ap.add_argument("--db-password", default=None)
    ap.add_argument("--db-name", default=None)
    args = ap.parse_args()

    if not args.quests and not args.all:
        ap.error("pass --all for the full bake or --quests 62,76 for a subset")

    configure(host=args.db_host, port=args.db_port, user=args.db_user,
              password=args.db_password, database=args.db_name)

    import time as _time
    t0 = _time.time()

    outdir = Path(args.out)
    outdir.mkdir(parents=True, exist_ok=True)
    mmaps = Path(args.mmaps) if args.mmaps else coords.get_data_paths()["mmaps"]
    db_name = args.db_name or DEFAULTS["database"]

    if args.quests:
        quests = {int(q): "cli" for q in args.quests.split(",") if q.strip()}
    else:
        quests = {int(r["QuestID"]): "all"
                  for r in sql("SELECT DISTINCT QuestID AS QuestID FROM quest_poi ORDER BY QuestID")}
    print(f"quests: {len(quests)}", flush=True)

    obj = load_quest_objective_entries(list(quests))
    droppers = load_dropper_entries({it for o in obj.values() for it in o["items"]})
    enders = load_enders(list(quests))
    triggers = load_area_triggers(list(quests))
    print(f"loaders done {_time.time() - t0:.0f}s", flush=True)

    truth: dict[int, dict] = {}
    for qid, o in obj.items():
        cre = set(o["cre"]) | set(o["go"]) | {e for it in o["items"] for e in droppers.get(it, set())}
        cre |= enders.get(qid, set())
        truth[qid] = {"cre": cre, "go": set(o["go"])}

    pois = load_pois(list(quests))
    print(f"POI points: {len(pois)}", flush=True)

    by_map: dict[int, list] = defaultdict(list)
    for p in sorted(pois, key=lambda p: (int(p["QuestID"]), int(p["PoiIdx"]), int(p["PointIdx"]))):
        by_map[int(p["MapID"])].append(p)

    report = []
    sql_rows = []
    json_out = []
    hits = misses = notruth = 0
    done = 0
    pair_keys = set()
    triple_keys = set()
    quest_keys = set()
    trigger_rows = walkable_rows = 0

    for map_id in sorted(by_map):
        map_points = by_map[map_id]
        nav = NavMesh(map_id, mmaps)
        if not nav.has_navmesh:
            print(f"WARN: no navmesh for map {map_id} ({len(map_points)} points skipped)", flush=True)
            continue
        cre, cre_xs, go, go_xs = load_spawns_for_map(map_id)

        # group the map's points by navmesh tile so each tile group's polys are
        # walked exactly once
        groups: dict[tuple, list] = defaultdict(list)
        for p in map_points:
            px, py = float(p["X"]), float(p["Y"])
            groups[nav.calc_tile_loc((py, 0.0, px))].append(p)
        print(f"map {map_id}: {len(map_points)} points in {len(groups)} tile groups", flush=True)

        for tile_key, group in sorted(groups.items()):
            group = sorted(group, key=lambda p: (int(p["QuestID"]), int(p["PoiIdx"]), int(p["PointIdx"])))
            gx_lo = min(float(p["X"]) for p in group) - SCAN_RADIUS
            gy_lo = min(float(p["Y"]) for p in group) - SCAN_RADIUS
            gx_hi = max(float(p["X"]) for p in group) + SCAN_RADIUS
            gy_hi = max(float(p["Y"]) for p in group) + SCAN_RADIUS
            try:
                mesh = collect_mesh(nav, Bounds(gx_lo, gy_lo, gx_hi, gy_hi), max_polys=200000)
            except Exception as exc:  # polys cap / corrupt tile: skip the group
                print(f"WARN: tile group {tile_key} skipped: {exc}", flush=True)
                continue

            # polys with centroids, x-sorted for per-point bisect windows
            gpolys = []
            for wpts, avgz in mesh["polys"]:
                cx = sum(p[0] for p in wpts) / len(wpts)
                cy = sum(p[1] for p in wpts) / len(wpts)
                gpolys.append((cx, cy, avgz, wpts))
            gpolys.sort(key=lambda g: g[0])
            gx_arr = [g[0] for g in gpolys]

            for p in group:
                qid = int(p["QuestID"])
                poi_idx = int(p["PoiIdx"])
                point_idx = int(p["PointIdx"])
                px, py = float(p["X"]), float(p["Y"])

                t = truth.get(qid, {"cre": set(), "go": set()})
                cre_near = spawns_in_radius(cre_xs, cre, px, py, SCAN_RADIUS)
                go_near = spawns_in_radius(go_xs, go, px, py, SCAN_RADIUS)
                truth_zs = [s[2] for s in cre_near if s[3] in t["cre"]]
                truth_zs += [s[2] for s in go_near if s[3] in t["go"]]
                trigger_zs = [tr["z"] for tr in triggers.get(qid, [])
                              if tr["map"] == map_id and dist2d(tr["x"], tr["y"], px, py) <= SCAN_RADIUS]

                lo = bisect.bisect_left(gx_arr, px - SCAN_RADIUS)
                hi = bisect.bisect_right(gx_arr, px + SCAN_RADIUS)
                near_polys = [g for g in gpolys[lo:hi]
                              if dist2d(g[0], g[1], px, py) <= SCAN_RADIUS]

                candidates = bake_point(px, py, near_polys, truth_zs, trigger_zs,
                                        cre_near, go_near)

                done += 1
                if done % 250 == 0:
                    el = _time.time() - t0
                    print(f"  {done}/{len(pois)} points, {el:.0f}s elapsed", flush=True)

                entry = {"quest": qid, "poi": poi_idx, "point": point_idx,
                         "map": map_id, "x": px, "y": py,
                         "truth_z": sorted(set(round(z, 1) for z in truth_zs)),
                         "candidates": candidates}
                json_out.append(entry)

                if candidates:
                    best = candidates[0]
                    if truth_zs:
                        delta = min(abs(best["z"] - tz) for tz in truth_zs)
                        if delta <= 6.0:
                            hits += 1
                        else:
                            misses += 1
                        report.append([qid, poi_idx, point_idx, map_id, f"{best['z']:.1f}",
                                       f"{min(truth_zs, key=lambda tz: abs(tz - best['z'])):.1f}",
                                       f"{delta:.1f}", best["confidence"], best["evidence"],
                                       quests.get(qid, "")])
                    else:
                        notruth += 1
                        report.append([qid, poi_idx, point_idx, map_id, f"{best['z']:.1f}", "", "",
                                       best["confidence"], best["evidence"], quests.get(qid, "")])
                    for rank, c in enumerate(candidates, start=1):
                        pair_keys.add((qid, poi_idx))
                        triple_keys.add((qid, poi_idx, point_idx))
                        quest_keys.add(qid)
                        if "trigger" in c["evidence"]:
                            trigger_rows += 1
                        if c["walkable"]:
                            walkable_rows += 1
                        wp = ";".join(f"{w[0]},{w[1]},{w[2]}" for w in c["walkable"])
                        sql_rows.append(
                            f"({qid},{poi_idx},{point_idx},{map_id},{c['z']:.2f},{rank},"
                            f"{c['confidence']},'{c['evidence']}',{1 if c['nav_ok'] else 0},'{wp}')")

        del nav  # free the map's tile cache before the next map

    # outputs ---------------------------------------------------------------
    report.sort(key=lambda r: (-int(r[7]) if r[7] else 0, r[0]))
    with open(outdir / "quest_poi_z_report.csv", "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["quest", "poi_idx", "point_idx", "map", "best_z", "truth_z", "delta",
                    "confidence", "evidence", "tags"])
        w.writerows(report)

    if json_out and len(json_out) <= 5000:
        (outdir / "quest_poi_z_candidates.json").write_text(json.dumps(json_out, indent=1))
    elif json_out:
        # full bake: compact JSONL (one line per point) to keep the file usable
        with open(outdir / "quest_poi_z_candidates.jsonl", "w") as f:
            for entry in json_out:
                f.write(json.dumps(entry) + "\n")

    sql_lines = [
        "-- playerbots_quest_poi_z: baked walkable Z candidates for quest POIs (rank <= 4).",
        "-- Generated by mod-playerbots/apps/quest_poi_z_baker/quest_poi_z_baker.py.",
        "-- Regenerate with: python3 apps/quest_poi_z_baker/quest_poi_z_baker.py "
        "--all --out apps/quest_poi_z_baker/data --mmaps <path>",
        "-- Source database and checksums: apps/quest_poi_z_baker/data/manifest.json",
        "CREATE TABLE IF NOT EXISTS `playerbots_quest_poi_z` (",
        "  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,",
        "  `QuestId` INT UNSIGNED NOT NULL,",
        "  `PoiIdx` SMALLINT UNSIGNED NOT NULL,",
        "  `PointIdx` SMALLINT UNSIGNED NOT NULL,",
        "  `MapId` SMALLINT UNSIGNED NOT NULL,",
        "  `Z` FLOAT NOT NULL,",
        "  `Rank` TINYINT UNSIGNED NOT NULL DEFAULT 1,",
        "  `Confidence` TINYINT UNSIGNED NOT NULL DEFAULT 0,",
        "  `Evidence` VARCHAR(255) NOT NULL DEFAULT '',",
        "  `NavmeshOk` TINYINT UNSIGNED NOT NULL DEFAULT 1,",
        "  `WalkablePoints` TEXT NULL,",
        "  PRIMARY KEY (`Id`), KEY `idx_quest` (`QuestId`, `PoiIdx`)",
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;",
    ]
    if sql_rows:
        for i in range(0, len(sql_rows), 200):
            sql_lines.append("INSERT INTO `playerbots_quest_poi_z` "
                             "(QuestId, PoiIdx, PointIdx, MapId, Z, `Rank`, Confidence, Evidence, "
                             "NavmeshOk, WalkablePoints) VALUES "
                             + ",".join(sql_rows[i:i + 200]) + ";")
    (outdir / "playerbots_quest_poi_z.sql").write_text("\n".join(sql_lines) + "\n")

    sql_path = outdir / "playerbots_quest_poi_z.sql"
    sql_bytes = sql_path.read_bytes()
    script_bytes = Path(__file__).read_bytes()
    bundle_hasher = hashlib.sha256()
    bundle_paths = [Path(__file__), Path(__file__).with_name("db.py")]
    bundle_paths += sorted((Path(__file__).parent / "terrain").rglob("*.py"))
    for bundle_path in bundle_paths:
        bundle_hasher.update(bundle_path.read_bytes())
    manifest = {
        "generated_by": "apps/quest_poi_z_baker/quest_poi_z_baker.py",
        "generated_at_utc": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "script_sha256": hashlib.sha256(script_bytes).hexdigest(),
        "bundle_sha256": bundle_hasher.hexdigest(),
        "sql_sha256": hashlib.sha256(sql_bytes).hexdigest(),
        "world_database": db_name,
        "mmaps_path": str(mmaps),
        "rows": len(sql_rows),
        "quests": len(quest_keys),
        "quest_poi_pairs": len(pair_keys),
        "quest_poi_point_triples": len(triple_keys),
        "trigger_rows": trigger_rows,
        "walkable_rows": walkable_rows,
    }
    (outdir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")

    total_truth = hits + misses
    print(f"\nbaked points: {len(json_out) if json_out else done} (with ground truth: {total_truth})",
          flush=True)
    if total_truth:
        print(f"rank-1 within 6 yd of spawn truth: {hits}/{total_truth} ({100 * hits / total_truth:.1f}%)")
    print(f"no-ground-truth points (confidence-only): {notruth}")
    print(f"outputs in {outdir}")


if __name__ == "__main__":
    main()
