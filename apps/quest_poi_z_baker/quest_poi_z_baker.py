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

Run a subset (development; --out is required so the committed data/ is safe):

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
import time
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, NamedTuple, Sequence

from db import DEFAULTS, configure, sql
from terrain import coords
from terrain.detour import NavMesh
from terrain.mesh import Bounds, collect_mesh

SCAN_RADIUS = 40.0          # yd around the POI point
CLUSTER_GAP = 8.0           # yd z-gap that splits clusters
NAV_POLY_MIN = 1            # polys needed to call a level navmesh-walkable
MAX_RANKS = 4
MAX_WALK_POINTS = 6
SQL_CHUNK = 200
POLY_CAP = 200000


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


def dist2d(ax: float, ay: float, bx: float, by: float) -> float:
    return math.hypot(ax - bx, ay - by)


def _point_key(point: dict) -> tuple[int, int, int]:
    return int(point["QuestID"]), int(point["PoiIdx"]), int(point["PointIdx"])


def _placeholders(values: Sequence[Any]) -> str:
    return ",".join(["%s"] * len(values))


# Data loading ---------------------------------------------------------------

def load_pois(quest_ids: list[int]) -> list[dict]:
    if not quest_ids:
        return []
    marks = _placeholders(quest_ids)
    return sql(
        "SELECT p.QuestID, p.id AS PoiIdx, p.ObjectiveIndex, p.MapID, "
        "pt.Idx2 AS PointIdx, pt.X, pt.Y "
        "FROM quest_poi p JOIN quest_poi_points pt "
        "ON pt.QuestID = p.QuestID AND pt.Idx1 = p.id "
        f"WHERE p.QuestID IN ({marks}) ORDER BY p.QuestID, p.id, pt.Idx2",
        tuple(quest_ids),
    )


def load_quest_objective_entries(quest_ids: list[int]) -> dict[int, dict]:
    """questId -> {creatureEntries: [...], goEntries: [...], itemIds: [...]}"""
    if not quest_ids:
        return {}
    marks = _placeholders(quest_ids)
    out: dict[int, dict] = {}
    rows = sql(
        "SELECT ID, RequiredNpcOrGo1, RequiredNpcOrGo2, RequiredNpcOrGo3, RequiredNpcOrGo4, "
        "RequiredItemId1, RequiredItemId2, RequiredItemId3, RequiredItemId4, "
        "RequiredItemId5, RequiredItemId6 FROM quest_template "
        f"WHERE ID IN ({marks}) ORDER BY ID",
        tuple(quest_ids),
    )
    for r in rows:
        qid = int(r["ID"])
        cre, go, items = [], [], []
        for i in range(1, 5):
            value = int(r[f"RequiredNpcOrGo{i}"] or 0)
            if value > 0:
                cre.append(value)
            elif value < 0:
                go.append(-value)
        for i in range(1, 7):
            value = int(r[f"RequiredItemId{i}"] or 0)
            if value:
                items.append(value)
        out[qid] = {"cre": cre, "go": go, "items": items}
    return out


def load_dropper_entries(items: set[int]) -> dict[int, set[int]]:
    """itemId -> creature entries that can drop it.

    creature_loot_template.Entry is a loot key, not necessarily the creature
    entry, so resolve it through creature_template.lootid. Reference templates
    can point at other reference templates; the recursive CTE follows the whole
    chain.
    """
    if not items:
        return {}
    item_list = sorted(items)
    marks = _placeholders(item_list)
    query = (
        "WITH RECURSIVE loot (creature_entry, item, ref, depth) AS ("
        "  SELECT ct.entry, CASE WHEN clt.Reference = 0 THEN clt.Item ELSE NULL END, "
        "clt.Reference, 0 "
        "  FROM creature_loot_template clt "
        "  JOIN creature_template ct ON ct.lootid = clt.Entry "
        "  UNION ALL "
        "  SELECT l.creature_entry, CASE WHEN rlt.Reference = 0 THEN rlt.Item ELSE NULL END, "
        "rlt.Reference, l.depth + 1 "
        "  FROM loot l "
        "  JOIN reference_loot_template rlt ON rlt.Entry = ABS(l.ref) "
        "  WHERE l.ref <> 0 AND l.depth < 10"
        ") "
        f"SELECT DISTINCT creature_entry, item FROM loot WHERE item IN ({marks}) "
        "ORDER BY item, creature_entry"
    )
    out: dict[int, set[int]] = defaultdict(set)
    for r in sql(query, tuple(item_list)):
        out[int(r["item"])].add(int(r["creature_entry"]))
    return out


def load_enders(quest_ids: list[int]) -> dict[int, set[int]]:
    """questId -> ender creature entries (turn-in ground truth)."""
    if not quest_ids:
        return {}
    marks = _placeholders(quest_ids)
    out: dict[int, set[int]] = defaultdict(set)
    for r in sql(
        f"SELECT quest, id FROM creature_questender WHERE quest IN ({marks}) ORDER BY quest, id",
        tuple(quest_ids),
    ):
        out[int(r["quest"])].add(int(r["id"]))
    return out


def load_area_triggers(quest_ids: list[int]) -> dict[int, list[dict]]:
    if not quest_ids:
        return {}
    marks = _placeholders(quest_ids)
    out: dict[int, list[dict]] = defaultdict(list)
    for r in sql(
        "SELECT a.quest, t.map, t.x, t.y, t.z FROM areatrigger_involvedrelation a "
        "JOIN areatrigger t ON t.entry = a.id "
        f"WHERE a.quest IN ({marks}) ORDER BY a.quest, t.entry",
        tuple(quest_ids),
    ):
        out[int(r["quest"])].append({"map": int(r["map"]), "x": float(r["x"]),
                                     "y": float(r["y"]), "z": float(r["z"])})
    return out


class MapSpawns(NamedTuple):
    creatures: list
    creature_xs: list
    gameobjects: list
    gameobject_xs: list


def load_spawns_for_map(map_id: int) -> MapSpawns:
    """All creature/GO spawns on the map, x-sorted with prebuilt x arrays."""
    cre = [(float(r["position_x"]), float(r["position_y"]), float(r["position_z"]), int(r["id"]))
           for r in sql(
               "SELECT id, position_x, position_y, position_z FROM creature "
               "WHERE map = %s ORDER BY id", (map_id,))]
    go = [(float(r["position_x"]), float(r["position_y"]), float(r["position_z"]), int(r["id"]))
          for r in sql(
              "SELECT id, position_x, position_y, position_z FROM gameobject "
              "WHERE map = %s ORDER BY id", (map_id,))]
    cre.sort(key=lambda s: s[0])
    go.sort(key=lambda s: s[0])
    return MapSpawns(cre, [s[0] for s in cre], go, [s[0] for s in go])


def spawns_in_radius(xs: list, spawns: list, x: float, y: float, radius: float) -> list:
    """Prebuilt x-array + x-sorted spawns -> spawns within radius (bisect on x)."""
    lo = bisect.bisect_left(xs, x - radius)
    hi = bisect.bisect_right(xs, x + radius)
    return [s for s in spawns[lo:hi] if abs(s[1] - y) <= radius
            and math.hypot(s[0] - x, s[1] - y) <= radius]


def build_truth(objectives: dict[int, dict], droppers: dict[int, set[int]],
                enders: dict[int, set[int]]) -> dict[int, dict]:
    truth: dict[int, dict] = {}
    for qid, o in objectives.items():
        creatures = set(o["cre"])
        creatures.update(e for item in o["items"] for e in droppers.get(item, set()))
        creatures.update(enders.get(qid, set()))
        truth[qid] = {"cre": creatures, "go": set(o["go"])}
    return truth


# Per-point baking -----------------------------------------------------------

def _score_cluster(cluster: dict, nav_clusters: list[dict], spawn_clusters: list[dict],
                   truth_zs: list[float], trigger_zs: list[float],
                   x: float, y: float) -> dict:
    zc = cluster["zmean"]
    nav_hits = [item for c in nav_clusters for item in c["items"]
                if abs(item["cz"] - zc) <= CLUSTER_GAP]
    spawn_hits = [item for c in spawn_clusters for item in c["items"]
                  if abs(c["zmean"] - zc) <= CLUSTER_GAP]
    quest_spawn_hits = any(abs(item["z"] - tz) <= CLUSTER_GAP
                           for item in spawn_hits for tz in truth_zs)
    trigger_hit = any(abs(z - zc) <= CLUSTER_GAP for z in trigger_zs)

    confidence = 0
    evidence = []
    nav_ok = len(nav_hits) >= NAV_POLY_MIN
    if quest_spawn_hits:
        confidence = 90
        evidence.append("quest-spawn")
    if trigger_hit:
        confidence = max(confidence, 85)
        evidence.append("trigger")
    if nav_hits:
        confidence = max(confidence, 45)
        evidence.append(f"navmesh={len(nav_hits)}")
    if spawn_hits:
        confidence = max(confidence, 30)
        evidence.append(f"spawns={len(spawn_hits)}")
    if nav_hits:
        confidence = min(100, confidence + min(8, len(nav_hits)))

    walkable = [
        (round(item["cx"], 1), round(item["cy"], 1), round(item["cz"], 1))
        for item in sorted(nav_hits, key=lambda it: math.hypot(it["cx"] - x, it["cy"] - y))[:MAX_WALK_POINTS]
    ]
    return {
        "z": round(zc, 2),
        "nav_polys": len(nav_hits),
        "spawns": len(spawn_hits),
        "quest_spawn": bool(quest_spawn_hits),
        "trigger": trigger_hit,
        "nav_ok": nav_ok,
        "confidence": int(confidence),
        "evidence": ";".join(evidence) if evidence else "none",
        "walkable": walkable,
    }


def _dedupe_candidates(candidates: list[dict]) -> list[dict]:
    candidates.sort(key=lambda c: (-c["confidence"], -c["nav_polys"]))
    seen_z = set()
    deduped = []
    for candidate in candidates:
        if any(abs(candidate["z"] - z) <= CLUSTER_GAP for z in seen_z):
            continue
        seen_z.add(candidate["z"])
        deduped.append(candidate)
    return deduped[:MAX_RANKS]


def bake_point(x: float, y: float, near_polys: list, truth_zs: list[float],
               trigger_zs: list[float], cre_near: list, go_near: list) -> list[dict]:
    """Ranked candidates for one point from its pre-collected evidence.

    near_polys: [(cx, cy, avgz, wpts)] already within the scan radius.
    cre_near / go_near: spawns already within the scan radius.
    """
    nav_levels = [(avgz, {"cx": cx, "cy": cy, "cz": avgz})
                  for (cx, cy, avgz, _wpts) in near_polys]
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

    candidates = [
        _score_cluster(cluster, nav_clusters, spawn_clusters, truth_zs, trigger_zs, x, y)
        for cluster in final
    ]
    return _dedupe_candidates(candidates)


# Bake pipeline --------------------------------------------------------------

@dataclass
class BakeResult:
    sql_rows: list[str] = field(default_factory=list)
    report: list[list[Any]] = field(default_factory=list)
    json_out: list[dict[str, Any]] = field(default_factory=list)
    pair_keys: set[tuple[int, int]] = field(default_factory=set)
    triple_keys: set[tuple[int, int, int]] = field(default_factory=set)
    quest_keys: set[int] = field(default_factory=set)
    hits: int = 0
    misses: int = 0
    notruth: int = 0
    done: int = 0
    trigger_rows: int = 0
    walkable_rows: int = 0


def _record_candidates(qid: int, poi_idx: int, point_idx: int, map_id: int,
                       candidates: list[dict], truth_zs: list[float],
                       quests: dict[int, str], result: BakeResult) -> None:
    if not candidates:
        return
    best = candidates[0]
    if truth_zs:
        delta = min(abs(best["z"] - tz) for tz in truth_zs)
        if delta <= 6.0:
            result.hits += 1
        else:
            result.misses += 1
        closest = min(truth_zs, key=lambda tz, best_z=best["z"]: abs(tz - best_z))
        result.report.append([qid, poi_idx, point_idx, map_id, f"{best['z']:.1f}",
                              f"{closest:.1f}", f"{delta:.1f}", best["confidence"],
                              best["evidence"], quests.get(qid, "")])
    else:
        result.notruth += 1
        result.report.append([qid, poi_idx, point_idx, map_id, f"{best['z']:.1f}", "", "",
                              best["confidence"], best["evidence"], quests.get(qid, "")])

    for rank, candidate in enumerate(candidates, start=1):
        result.pair_keys.add((qid, poi_idx))
        result.triple_keys.add((qid, poi_idx, point_idx))
        result.quest_keys.add(qid)
        if "trigger" in candidate["evidence"]:
            result.trigger_rows += 1
        if candidate["walkable"]:
            result.walkable_rows += 1
        walkable = ";".join(f"{w[0]},{w[1]},{w[2]}" for w in candidate["walkable"])
        result.sql_rows.append(
            f"({qid},{poi_idx},{point_idx},{map_id},{candidate['z']:.2f},{rank},"
            f"{candidate['confidence']},'{candidate['evidence']}',"
            f"{1 if candidate['nav_ok'] else 0},'{walkable}')")


def _bake_point(point: dict, map_id: int, gpolys: list, gx_arr: list,
                spawns: MapSpawns, truth: dict[int, dict], triggers: dict[int, list],
                quests: dict[int, str], result: BakeResult,
                total_points: int, t0: float) -> None:
    qid = int(point["QuestID"])
    poi_idx = int(point["PoiIdx"])
    point_idx = int(point["PointIdx"])
    px, py = float(point["X"]), float(point["Y"])

    quest_truth = truth.get(qid, {"cre": set(), "go": set()})
    cre_near = spawns_in_radius(spawns.creature_xs, spawns.creatures, px, py, SCAN_RADIUS)
    go_near = spawns_in_radius(spawns.gameobject_xs, spawns.gameobjects, px, py, SCAN_RADIUS)
    truth_zs = [s[2] for s in cre_near if s[3] in quest_truth["cre"]]
    truth_zs += [s[2] for s in go_near if s[3] in quest_truth["go"]]
    trigger_zs = [tr["z"] for tr in triggers.get(qid, [])
                  if tr["map"] == map_id and dist2d(tr["x"], tr["y"], px, py) <= SCAN_RADIUS]

    lo = bisect.bisect_left(gx_arr, px - SCAN_RADIUS)
    hi = bisect.bisect_right(gx_arr, px + SCAN_RADIUS)
    near_polys = [g for g in gpolys[lo:hi] if dist2d(g[0], g[1], px, py) <= SCAN_RADIUS]

    candidates = bake_point(px, py, near_polys, truth_zs, trigger_zs, cre_near, go_near)
    result.done += 1
    if result.done % 250 == 0:
        elapsed = time.time() - t0
        print(f"  {result.done}/{total_points} points, {elapsed:.0f}s elapsed", flush=True)

    result.json_out.append({
        "quest": qid, "poi": poi_idx, "point": point_idx, "map": map_id, "x": px, "y": py,
        "truth_z": sorted({round(z, 1) for z in truth_zs}),
        "candidates": candidates,
    })
    _record_candidates(qid, poi_idx, point_idx, map_id, candidates, truth_zs, quests, result)


def _bake_group(nav: NavMesh, tile_key: tuple, group: list, map_id: int,
                spawns: MapSpawns, truth: dict[int, dict], triggers: dict[int, list],
                quests: dict[int, str], result: BakeResult,
                total_points: int, t0: float) -> None:
    group = sorted(group, key=_point_key)
    gx_lo = min(float(p["X"]) for p in group) - SCAN_RADIUS
    gy_lo = min(float(p["Y"]) for p in group) - SCAN_RADIUS
    gx_hi = max(float(p["X"]) for p in group) + SCAN_RADIUS
    gy_hi = max(float(p["Y"]) for p in group) + SCAN_RADIUS
    try:
        mesh = collect_mesh(nav, Bounds(gx_lo, gy_lo, gx_hi, gy_hi), max_polys=POLY_CAP)
    except Exception as exc:  # polys cap / corrupt tile: skip the group
        print(f"WARN: tile group {tile_key} skipped: {exc}", flush=True)
        return

    gpolys = []
    for wpts, avgz in mesh["polys"]:
        cx = sum(p[0] for p in wpts) / len(wpts)
        cy = sum(p[1] for p in wpts) / len(wpts)
        gpolys.append((cx, cy, avgz, wpts))
    gpolys.sort(key=lambda g: g[0])
    gx_arr = [g[0] for g in gpolys]

    for point in group:
        _bake_point(point, map_id, gpolys, gx_arr, spawns, truth, triggers, quests,
                    result, total_points, t0)


def _bake_map(map_id: int, map_points: list, mmaps: Path, truth: dict[int, dict],
              triggers: dict[int, list], quests: dict[int, str],
              result: BakeResult, total_points: int, t0: float) -> None:
    nav = NavMesh(map_id, mmaps)
    if not nav.has_navmesh:
        print(f"WARN: no navmesh for map {map_id} ({len(map_points)} points skipped)", flush=True)
        return
    spawns = load_spawns_for_map(map_id)
    groups: dict[tuple, list] = defaultdict(list)
    for point in map_points:
        px, py = float(point["X"]), float(point["Y"])
        groups[nav.calc_tile_loc((py, 0.0, px))].append(point)
    print(f"map {map_id}: {len(map_points)} points in {len(groups)} tile groups", flush=True)

    for tile_key, group in sorted(groups.items()):
        _bake_group(nav, tile_key, group, map_id, spawns, truth, triggers, quests,
                    result, total_points, t0)
    del nav


def bake_quests(quests: dict[int, str], pois: list[dict], truth: dict[int, dict],
                triggers: dict[int, list], mmaps: Path, t0: float) -> BakeResult:
    by_map: dict[int, list] = defaultdict(list)
    for point in sorted(pois, key=_point_key):
        by_map[int(point["MapID"])].append(point)

    result = BakeResult()
    total_points = len(pois)
    for map_id in sorted(by_map):
        _bake_map(map_id, by_map[map_id], mmaps, truth, triggers, quests, result,
                  total_points, t0)
    return result


# Outputs --------------------------------------------------------------------

def _build_sql(sql_rows: list[str]) -> str:
    lines = [
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
    for i in range(0, len(sql_rows), SQL_CHUNK):
        lines.append("INSERT INTO `playerbots_quest_poi_z` "
                     "(QuestId, PoiIdx, PointIdx, MapId, Z, `Rank`, Confidence, Evidence, "
                     "NavmeshOk, WalkablePoints) VALUES "
                     + ",".join(sql_rows[i:i + SQL_CHUNK]) + ";")
    return "\n".join(lines) + "\n"


def _bundle_hash() -> str:
    hasher = hashlib.sha256()
    paths = [Path(__file__), Path(__file__).with_name("db.py")]
    paths += sorted((Path(__file__).parent / "terrain").rglob("*.py"))
    for path in paths:
        hasher.update(path.read_bytes())
    return hasher.hexdigest()


def _write_report(outdir: Path, report: list[list[Any]]) -> None:
    report.sort(key=lambda r: (-int(r[7]) if r[7] else 0, r[0]))
    with open(outdir / "quest_poi_z_report.csv", "w", newline="") as f:  # NOSONAR
        writer = csv.writer(f)
        writer.writerow(["quest", "poi_idx", "point_idx", "map", "best_z", "truth_z", "delta",
                         "confidence", "evidence", "tags"])
        writer.writerows(report)


def _write_candidates(outdir: Path, json_out: list[dict[str, Any]]) -> None:
    if json_out and len(json_out) <= 5000:
        (outdir / "quest_poi_z_candidates.json").write_text(  # NOSONAR
            json.dumps(json_out, indent=1))
    elif json_out:
        with open(outdir / "quest_poi_z_candidates.jsonl", "w") as f:  # NOSONAR
            for entry in json_out:
                f.write(json.dumps(entry) + "\n")


def write_outputs(outdir: Path, result: BakeResult, db_name: str, mmaps: Path) -> None:
    _write_report(outdir, result.report)
    _write_candidates(outdir, result.json_out)

    sql_path = outdir / "playerbots_quest_poi_z.sql"  # NOSONAR
    sql_path.write_text(_build_sql(result.sql_rows))  # NOSONAR
    sql_bytes = sql_path.read_bytes()

    manifest = {
        "generated_by": "apps/quest_poi_z_baker/quest_poi_z_baker.py",
        "generated_at_utc": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "bundle_sha256": _bundle_hash(),
        "sql_sha256": hashlib.sha256(sql_bytes).hexdigest(),
        "world_database": db_name,
        "mmaps_path": str(mmaps),
        "rows": len(result.sql_rows),
        "quests": len(result.quest_keys),
        "quest_poi_pairs": len(result.pair_keys),
        "quest_poi_point_triples": len(result.triple_keys),
        "trigger_rows": result.trigger_rows,
        "walkable_rows": result.walkable_rows,
    }
    (outdir / "manifest.json").write_text(  # NOSONAR
        json.dumps(manifest, indent=2) + "\n")


def _print_summary(result: BakeResult, outdir: Path) -> None:
    total_truth = result.hits + result.misses
    baked_points = len(result.json_out) if result.json_out else result.done
    print(f"\nbaked points: {baked_points} (with ground truth: {total_truth})", flush=True)
    if total_truth:
        print(f"rank-1 within 6 yd of spawn truth: "
              f"{result.hits}/{total_truth} ({100 * result.hits / total_truth:.1f}%)")
    print(f"no-ground-truth points (confidence-only): {result.notruth}")
    print(f"outputs in {outdir}")


# CLI ------------------------------------------------------------------------

def parse_args() -> tuple[argparse.Namespace, argparse.ArgumentParser]:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default=None,
                        help="output directory (full bake defaults to apps/quest_poi_z_baker/data; "
                             "required with --quests)")
    parser.add_argument("--quests", default="",
                        help="comma-separated quest ids: bake exactly these (tags: cli)")
    parser.add_argument("--all", action="store_true",
                        help="bake every quest that has POI points (the full bake)")
    parser.add_argument("--mmaps", default="",
                        help="mmaps directory (default: ACORE_DATA_PATH/mmaps)")
    parser.add_argument("--db-host", default=None)
    parser.add_argument("--db-port", type=int, default=None)
    parser.add_argument("--db-user", default=None)
    parser.add_argument("--db-password", default=None)
    parser.add_argument("--db-name", default=None)
    args = parser.parse_args()
    if not args.quests and not args.all:
        parser.error("pass --all for the full bake or --quests 62,76 for a subset")
    return args, parser


def resolve_paths(args: argparse.Namespace,
                  parser: argparse.ArgumentParser) -> tuple[Path, Path, str]:
    if args.out is None:
        if args.quests:
            parser.error("pass --out with --quests so the committed data/ artifact is not overwritten")
        outdir = Path(__file__).resolve().parent / "data"
    else:
        outdir = Path(args.out)  # NOSONAR
    mmaps = Path(args.mmaps) if args.mmaps else coords.get_data_paths()["mmaps"]  # NOSONAR
    return outdir, mmaps, args.db_name or DEFAULTS["database"]


def load_quests(args: argparse.Namespace) -> dict[int, str]:
    if args.quests:
        return {int(q): "cli" for q in args.quests.split(",") if q.strip()}
    return {int(r["QuestID"]): "all"
            for r in sql("SELECT DISTINCT QuestID AS QuestID FROM quest_poi ORDER BY QuestID")}


def load_inputs(quests: dict[int, str]) -> tuple[dict, dict, dict, dict]:
    objectives = load_quest_objective_entries(list(quests))
    droppers = load_dropper_entries({item for o in objectives.values() for item in o["items"]})
    enders = load_enders(list(quests))
    triggers = load_area_triggers(list(quests))
    return objectives, droppers, enders, triggers


def main() -> None:
    args, parser = parse_args()
    configure(host=args.db_host, port=args.db_port, user=args.db_user,
              password=args.db_password, database=args.db_name)

    t0 = time.time()
    outdir, mmaps, db_name = resolve_paths(args, parser)
    outdir.mkdir(parents=True, exist_ok=True)  # NOSONAR

    quests = load_quests(args)
    print(f"quests: {len(quests)}", flush=True)

    objectives, droppers, enders, triggers = load_inputs(quests)
    print(f"loaders done {time.time() - t0:.0f}s", flush=True)

    truth = build_truth(objectives, droppers, enders)
    pois = load_pois(list(quests))
    print(f"POI points: {len(pois)}", flush=True)

    result = bake_quests(quests, pois, truth, triggers, mmaps, t0)
    write_outputs(outdir, result, db_name, mmaps)
    _print_summary(result, outdir)


if __name__ == "__main__":
    main()
