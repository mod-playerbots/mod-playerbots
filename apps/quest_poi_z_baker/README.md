# Quest POI Z baker

Generates `playerbots_quest_poi_z`, the offline-baked walkable Z candidates used by
`QuestPoiZStore` for quest destinations and explore/area-trigger approaches.

The runtime store is intentionally dumb: it loads the table once at startup and
selects a candidate. This tool contains the actual reasoning — navmesh levels,
spawn anchors, area triggers, evidence scoring, confidence, rank, and walkable
samples — so the table is reproducible instead of opaque.

## Inputs

- `acore_world` database:
  - `quest_poi`, `quest_poi_points`
  - `quest_template` (NPC/GO and item objectives)
  - `creature`, `gameobject` (spawn anchors)
  - `areatrigger`, `areatrigger_involvedrelation`
  - `creature_questender`
  - `creature_loot_template`, `reference_loot_template` (item-dropper anchors)
- An AzerothCore `mmaps` directory.
- Python 3.10+ and `pymysql`. Use the tool's own venv; the acore-data venv is
  not required:

```bash
python3 -m venv apps/quest_poi_z_baker/.venv
apps/quest_poi_z_baker/.venv/bin/pip install -r apps/quest_poi_z_baker/requirements.txt
```

## Run

Full bake (this is what produces the committed SQL):

```bash
export ACORE_DB_PASSWORD=...
apps/quest_poi_z_baker/.venv/bin/python apps/quest_poi_z_baker/quest_poi_z_baker.py --all \
  --out apps/quest_poi_z_baker/data \
  --mmaps /path/to/mmaps \
  --db-host 127.0.0.1 --db-port 3306 --db-user acore --db-name acore_world
```

Subset (development / debugging; `--out` is required so the committed
`data/` artifact is not overwritten):

```bash
apps/quest_poi_z_baker/.venv/bin/python apps/quest_poi_z_baker/quest_poi_z_baker.py --quests 62,76 \
  --out /tmp/poi-z-bake --mmaps /path/to/mmaps
```

Connection settings can also be supplied with `ACORE_DB_HOST`, `ACORE_DB_PORT`,
`ACORE_DB_USER`, `ACORE_DB_PASSWORD` (or `ACORE_DB_PASS`), and `ACORE_DB_NAME`.
The `mmaps` default comes from `ACORE_DATA_PATH`/`DATA_PATH` (the core's data
directory); pass `--mmaps` explicitly if your layout differs.

## Output

`data/` contains the committed artifact:

- `playerbots_quest_poi_z.sql` — the exact output of the script, including the
  table schema and every candidate row.
- `manifest.json` — source database, mmaps path, generation time, script and
  bundle SHA-256, SQL SHA-256, and row/pair/triple counts.

The diagnostic `quest_poi_z_report.csv` and `quest_poi_z_candidates.jsonl` are
written beside them and are gitignored.

The SQL is generated data. **Do not hand-edit it.** If the algorithm or the
source data changes, rerun the full bake and commit the new SQL + manifest
together. `tests/test_sql.py` verifies the manifest hashes and the row counts,
so an out-of-sync SQL file fails the test suite.

## Algorithm

For every `quest_poi_points` row:

1. Load the navmesh polygons within `SCAN_RADIUS` (40 yd) of the POI point,
   grouped by Detour tile so each tile group's polygons are walked once.
2. Find creature/GO spawns within the same radius. Spawns belonging to the
   quest's own objectives (NPC/GO objectives, item droppers, quest enders) are
   ground truth; other spawns are semantic anchors. Item droppers are resolved
   through `creature_template.lootid`, including nested reference-loot chains.
3. Add quest area triggers whose center is within the radius.
4. Cluster all Z values with an 8 yd gap.
5. Score each cluster and keep up to four ranks.

### Evidence and confidence

| Evidence | Base confidence | Notes |
|---|---:|---|
| `quest-spawn` | 90 | a cluster matches a quest objective/ender spawn |
| `trigger` | 85 | a cluster matches an area-trigger Z |
| `navmesh=N` | 45 | N navmesh polygon centroids in the cluster |
| `spawns=N` | 30 | N non-quest spawn anchors in the cluster |

`navmesh` adds up to 8 points (one per polygon, capped at 8). A cluster with at
least one navmesh polygon is `navmeshOk=1`. Candidates are sorted by
`(-confidence, -nav_polys)` and deduplicated when their Z values are within
8 yd; at most four ranks are emitted per point.

`WalkablePoints` is up to six navmesh polygon centroids in the cluster, sorted by
2D distance to the POI point, formatted `x,y,z` with one decimal and separated
by semicolons. The runtime uses these for area-trigger approaches where the
trigger center is outside the POI outline.

## Runtime use

`QuestPoiZStore` loads `Evidence`, `Confidence`, `Rank`, `NavmeshOk`, and
`WalkablePoints` once at startup. `Lookup(quest, poi, map)` returns the
best-ranked navmesh-reachable candidate; `FindNearestWalkablePoint` returns the
walkable sample nearest a target, filtered to candidates at least as
trustworthy as the selected floor. Explore quests use that sample when it lies
inside the area-trigger radius.

## Import the table

```bash
mysql -u acore -p acore_playerbots < apps/quest_poi_z_baker/data/playerbots_quest_poi_z.sql
```

The SQL creates the table if needed. To replace an existing bake, truncate it
first:

```sql
TRUNCATE TABLE playerbots_quest_poi_z;
```

## Tests

```bash
cd apps/quest_poi_z_baker
.venv/bin/python -m unittest discover -s tests -v
```

The tests do not need a database or `mmaps`; they cover the pure clustering /
scoring functions and verify the committed SQL against the manifest and known
quest-62 rows (16 tests).

## Attribution

`terrain/` is vendored from [acore-data](https://github.com/azerothcore/acore-data)
commit `ffd2a2c` (GPL-2.0), the same project that originally hosted the baker:
`core/terrain/coords.py`, `mmap_reader.py`, `detour_parser.py`, `detour/`, and
the `collect_mesh` portion of `area_map.py`. Imports were made relative and the
SVG renderer was omitted. The baker logic itself is unchanged apart from
self-contained imports, deterministic ordering, and the manifest output.
