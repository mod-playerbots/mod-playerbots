#!/usr/bin/env bash
# =============================================================================
#  AzerothCore client-data extraction for mod-playerbots.
#  Run from anywhere — output lands in $SERVER_DATA_DIR.
#
#  Configure via environment variables (or edit the defaults below):
#    WOW_CLIENT_DATA   3.3.5a client Data/ directory (source MPQs)
#    TOOLS_DIR         server bin dir holding the extractors
#                      (map_extractor, vmap4_extractor, mmaps_generator, ...)
#    SERVER_DATA_DIR   where dbc/maps/vmaps/mmaps are written
#                      (defaults to TOOLS_DIR)
# =============================================================================
set -euo pipefail

# ─── PATHS ──────────────────────────────────────────────────────────────────
WOW_CLIENT_DATA="${WOW_CLIENT_DATA:-$HOME/wow_client_data}"
TOOLS_DIR="${TOOLS_DIR:-$HOME/azerothcore/env/dist/bin}"
SERVER_DATA_DIR="${SERVER_DATA_DIR:-$TOOLS_DIR}"

# ─── TOGGLES ────────────────────────────────────────────────────────────────
EXTRACT_DBC_AND_MAPS=true
EXTRACT_VMAPS=true
EXTRACT_MMAPS=true

MMAP_THREADS=0           # 0 = auto-detect (each thread uses 1-2 GB RAM)
MMAP_SINGLE_MAP=""       # e.g. "489" for Warsong Gulch only

# Copy of the mod-playerbots CORE FORK's src/tools/mmaps_generator/mmaps-config.yaml.
# NOT the stock AzerothCore config: the shipped travel-node graph SQL was
# generated against a mesh built with exactly these values, and a mesh built
# with different values no longer matches the graph. Keep this in sync with
# the repo file.
# A mmaps-config.yaml already present next to the generator takes precedence
# over this embedded copy (see STEP 3 below).
MMAPS_CONFIG_YAML=$(cat <<'YAML_EOF'
mmapsConfig:
  skipLiquid: false
  skipContinents: false
  skipJunkMaps: true
  skipBattlegrounds: false

  # Path to the directory containing navigation data files.
  # This directory should contain the "maps" and "vmaps" folders,
  # and is also where the "mmaps" folder will be created or located.
  dataDir: "./"

  # Off-mesh connections define manual navigation links that are not part of the generated navmesh.
  # They are used to connect two arbitrary points in the world where normal pathfinding cannot reach,
  # such as jumps, ropes, ladders, teleports, elevators, or special scripted movement paths.
  #
  # Format:
  #   mapID tileX,tileY (start_x start_y start_z) (end_x end_y end_z) size
  #
  # Fields:
  #   mapID      - Map identifier where this connection exists.
  #   tileX,tileY- Navmesh tile coordinates the connection belongs to.
  #   start      - World position where the connection begins.
  #   end        - World position where the connection ends.
  #   size       - Effective radius of the connection (agent clearance / usability width).
  offmeshConnections:
    # Make Blades Edge Arena Ropes wider
    - "562 31,20 (6234.474121 256.563721 11.063726) (6230.162598 251.681976 11.199670) 2.1"
    - "562 31,20 (6242.273926 266.697540 11.090456) (6246.688965 272.064819 11.235604) 2.1"

  meshSettings:
    # Here we have global config for recast navigation.
    # It's possible to override these data on map or tile level (see mapsOverrides).

    # Maximum slope angle (in degrees) NPCs can walk on.
    # Surfaces steeper than this will be considered unwalkable.
    walkableSlopeAngle: 60

    # --- Cell Size Calculation ---
    # Many parameters below are defined in "cell units".
    # In RecastDemo, you often work with world units instead of cell units.
    # By default, these cell units are converted to world units using the formula:
    #
    #     cellSize = MMAP::GRID_SIZE / (verticesPerMapEdge - 1)
    #
    # Where:
    #     MMAP::GRID_SIZE = 533.3333f (the size of one map tile in world units)
    #     verticesPerMapEdge = number of vertices along one edge of the full map grid
    #
    # Example:
    #     If verticesPerMapEdge = 2000, then:
    #         cellSize ≈ 533.3333 / (2000 - 1) ≈ 0.2667 world units per cell
    #
    # To convert a value from cell units to world units (e.g., walkableClimb),
    # multiply by cellSize. For example, a walkableClimb of 6 corresponds to:
    #     6 * 0.2667 ≈ 1.6 world units

    # Minimum ceiling height (in cell units) NPCs need to pass under an obstacle.
    # Controls how much vertical clearance is required.
    # To convert to world units, multiply by cellSize (see "Cell Size Calculation").
    walkableHeight: 6

    # Maximum height difference (in cell units) NPCs can step up or down.
    # Higher values allow walking over fences, ledges, or steps.
    # To convert to world units, multiply by cellSize (see "Cell Size Calculation").
    #
    # Vanilla WotLK uses 6, which allows creatures to "jump" over fences.
    # Classic WotLK uses 4, which forces creatures to walk around fences.
    walkableClimb: 4

    # Minimum distance (in cell units) around walkable surfaces.
    # Helps prevent NPCs from clipping into walls and narrow gaps.
    # To convert to world units, multiply by cellSize (see "Cell Size Calculation").
    walkableRadius: 2

    # Number of vertices along one edge of the entire map's navmesh grid.
    # Higher values increase mesh resolution but also CPU/memory usage.
    verticesPerMapEdge: 2000

    # Number of vertices along one edge of each tile chunk.
    # Must divide (verticesPerMapEdge - 1) evenly for seamless tiles.
    # A higher vertex count per tile means fewer total tiles,
    # reducing runtime work to load, unload, and manage tiles.
    verticesPerTileEdge: 80

    # Tolerance for how much a polygon can deviate from the original geometry when simplified.
    # Higher values produce simpler (faster) meshes but can reduce accuracy.
    maxSimplificationError: 0.8

    # You can override any global parameter for a specific map by specifying its map ID.
    # Inside each map override, you can also override parameters per individual tile,
    # identified by a string "tileX,tileY" (coordinates).
    #
    # Overrides cascade: global settings → map overrides → tile overrides.
    # For example:
    #
    # mapsOverrides:
    #   "0":                              # Map ID 0 overrides
    #     walkableRadius: 5               # Override global climb height for entire map 0
    #
    #     tilesOverrides:
    #       "50,70":                      # Tile at coordinates (50,70) on map 0
    #         walkableSlopeAngle: 70      # Override slope angle locally just here
    #         walkableClimb: 4            # Also override climb height for this tile only
    #
    #       "51,71":
    #         walkableClimb: 3            # Override climb height for tile (51,71)
    #
    #       "48,32":
    #         walkableClimb: 1            # Even smaller climb for tile (48,32)
    #
    #   "1":                              # Map ID 1 overrides example
    #     walkableHeight: 8               # Increase clearance for whole map 1
    #
    #     tilesOverrides:
    #       "100,100":
    #         maxSimplificationError: 2.5 # Looser mesh simplification for this tile only
    #
    #       "101,101":
    #         walkableRadius: 1           # Smaller NPC radius here for tight corridors
    #
    # This approach allows very fine-grained control of navigation mesh parameters
    # on a per-map and per-tile basis, optimizing pathfinding quality and performance.
    #
    # All parameters defined globally are eligible for override.
    # Just specify the parameter name and new value in the override section.
    mapsOverrides:
      # maxSimplificationError 1.8 overrides: at the global 0.8 these
      # WMO-heavy tiles exceed Detour's 65535-vertex-per-tile cap and the
      # builder skips them silently, leaving navmesh holes.
      # Keys are "tileY,tileX" matching the mmtile filename digits; the
      # --tile CLI arg takes the reversed pair (file 5712130 = key "21,30"
      # = --tile 30,21).
      "0": # Eastern Kingdoms
        tilesOverrides:
          "52,35": # Deadwind Pass / Karazhan
            maxSimplificationError: 1.8

      "1": # Kalimdor
        tilesOverrides:
          # Teldrassil canopy. walkableClimb 6: the roads step over roots
          # taller than climb 4 allows. steepSlopeAngle 60: the only ground
          # route out of Shadowglen crosses 50-60deg slopes, which default
          # tagging marks NAV_GROUND_STEEP and bots refuse. Without both,
          # the night elf starter zone is unreachable on foot.
          # steepSlopeAngle changes don't alter tile headers; delete the 9
          # .mmtile files (001{11,12,13}{28,29,30}) to force a rebuild.
          "11,28":
            walkableClimb: 6
            steepSlopeAngle: 60
          "11,29":
            walkableClimb: 6
            steepSlopeAngle: 60
          "11,30":
            walkableClimb: 6
            steepSlopeAngle: 60
          "12,28":
            walkableClimb: 6
            steepSlopeAngle: 60
          "12,29":
            walkableClimb: 6
            steepSlopeAngle: 60
          "12,30":
            walkableClimb: 6
            steepSlopeAngle: 60
          "13,28":
            walkableClimb: 6
            steepSlopeAngle: 60
          "13,29":
            walkableClimb: 6
            steepSlopeAngle: 60
          "13,30":
            walkableClimb: 6
            steepSlopeAngle: 60

      "48": # Blackfathom Deeps
        cellSizeVertical: 0.5334 # ch*2 = 0.2667 * 2 ≈ 0.5334. Reduce the chance to have underground levels.

      "509": # Ruins of Ahn'Qiraj
        tilesOverrides:
          "49,29":
            maxSimplificationError: 1.8

      "529": # Arathi Basin
        tilesOverrides:
          "30,29": # Lumber Mill
            # Make sure that Fear will not drop players rom cliff -
            # https://github.com/azerothcore/azerothcore-wotlk/pull/22462#issuecomment-3067024680
            walkableSlopeAngle: 45

      "530": # Outland
        tilesOverrides:
          "32,30": # Dark portal
            walkableSlopeAngle: 45 # https://github.com/chromiecraft/chromiecraft/issues/8404#issuecomment-3476012660
          "35,21": # Shattrath City
            maxSimplificationError: 1.8
          # Blade's Edge Mountains / Zangarmarsh-Coilfang block
          "24,21":
            maxSimplificationError: 1.8
          "24,22":
            maxSimplificationError: 1.8
          "25,19":
            maxSimplificationError: 1.8
          "25,20":
            maxSimplificationError: 1.8
          "25,21":
            maxSimplificationError: 1.8
          "25,22":
            maxSimplificationError: 1.8
          "26,19":
            maxSimplificationError: 1.8
          "26,20":
            maxSimplificationError: 1.8
          "27,19":
            maxSimplificationError: 1.8
          "27,20":
            maxSimplificationError: 1.8
          "27,21":
            maxSimplificationError: 1.8
          "28,21":
            maxSimplificationError: 1.8
          "28,22":
            maxSimplificationError: 1.8
          "29,18":
            maxSimplificationError: 1.8
          "29,19":
            maxSimplificationError: 1.8
          "29,20":
            maxSimplificationError: 1.8
          "29,21":
            maxSimplificationError: 1.8
          "30,19":
            maxSimplificationError: 1.8
          "30,20":
            maxSimplificationError: 1.8

      "532": # Karazhan
        tilesOverrides:
          "52,35":
            maxSimplificationError: 1.8
          "52,36":
            maxSimplificationError: 1.8

      "533": # Naxxramas
        tilesOverrides:
          "26,38":
            maxSimplificationError: 1.8

      "562": # Blade's Edge Arena
        walkableRadius: 0 # This allows walking on the ropes to the pillars
        tilesOverrides:
          "20,31":
            maxSimplificationError: 1.8

      "571": # Northrend
        tilesOverrides:
          "21,30": # Dalaran
            maxSimplificationError: 1.8
          "21,28": # Crystalsong / Icecrown border
            maxSimplificationError: 1.8
          "21,37": # Zul'Drak
            maxSimplificationError: 1.8
          "29,21": # Borean Tundra coast
            maxSimplificationError: 1.8
          # Storm Peaks
          "16,34":
            maxSimplificationError: 1.8
          "16,35":
            maxSimplificationError: 1.8
          "17,34":
            maxSimplificationError: 1.8

  # debugOutput generates debug files in the `meshes` directory for use with RecastDemo.
  # This is useful for inspecting and debugging mmap generation visually.
  #
  # My workflow:
  # 1. Install RecastDemo. I'm building it from the source of this fork: https://github.com/jackpoz/recastnavigation
  # 2. In-game, move your character to the area you want to debug.
  # 3. Type `.mmap loc` in chat. This will output:
  #    - The current tile file name (e.g., `04832.mmtile`)
  #    - The Recast config values used to generate that tile
  # 4. Enable `debugOutput` and regenerate mmaps (preferably just the tile from step 3).
  #    - To regenerate only one tile, delete it from the `mmaps` folder.
  # 5. After generation, you will find debug files in the `meshes` folder, including an OBJ file (e.g., `map0004832.obj`)
  # 6. Copy these debug files to the `Meshes` folder used by RecastDemo.
  #    - RecastDemo expects this folder to be in the same directory as its executable.
  # 7. In RecastDemo:
  #    - Click "Input Mesh" and select the `.obj` file
  #    - Choose "Solo Mesh" in the Sample selector
  # 8. (Optional) Reuse the Recast config values from step 3:
  #    - `cellSizeHorizontal` → "Cell Size"
  #    - `walkableSlopeAngle` → "Max Slope"
  #    - `walkableClimb` → "Max Climb"
  #    - and so on
  # 9. Scroll to the bottom of RecastDemo UI and press "Build" to generate the navigation mesh
  debugOutput: false
YAML_EOF
)

# =============================================================================
# ─── DO NOT EDIT BELOW ──────────────────────────────────────────────────────
# =============================================================================

[ -n "$SERVER_DATA_DIR" ] || { echo "SERVER_DATA_DIR is not set"; exit 1; }
[ -n "$WOW_CLIENT_DATA" ] || { echo "WOW_CLIENT_DATA is not set"; exit 1; }
mkdir -p "$SERVER_DATA_DIR"
cd "$SERVER_DATA_DIR"

# ─── SAFETY: source MPQs are READ-ONLY to this script ──────────────────────
# Resolve both paths to canonical form and refuse to run if the output dir
# is inside the source. Combined with safe_rm() below, this script cannot
# touch any file inside WOW_CLIENT_DATA.
SERVER_DATA_DIR_REAL="$(cd "$SERVER_DATA_DIR" && pwd -P)"
WOW_CLIENT_DATA_REAL="$(cd "$WOW_CLIENT_DATA" && pwd -P 2>/dev/null || echo "$WOW_CLIENT_DATA")"
case "$SERVER_DATA_DIR_REAL/" in
    "$WOW_CLIENT_DATA_REAL"/|"$WOW_CLIENT_DATA_REAL"/*)
        echo "ERROR: SERVER_DATA_DIR ($SERVER_DATA_DIR_REAL) is inside WOW_CLIENT_DATA — refusing." >&2
        exit 1
        ;;
esac

# Refuses to remove anything outside SERVER_DATA_DIR. Resolves the parent
# to absolute path so a symlink inside cwd can't trick us into traversing
# into the source. Use this for every cleanup in this script.
safe_rm() {
    local target="$1"
    local parent_abs base
    parent_abs="$(cd "$(dirname -- "$target")" 2>/dev/null && pwd -P)" || return 0
    base="$(basename -- "$target")"
    local abs="$parent_abs/$base"
    case "$abs/" in
        "$SERVER_DATA_DIR_REAL"/|"$SERVER_DATA_DIR_REAL"/*) ;;
        *)
            echo "REFUSING to rm path outside SERVER_DATA_DIR: $target → $abs" >&2
            exit 1 ;;
    esac
    rm -rf -- "$target"
}

[ "$MMAP_THREADS" -eq 0 ] && MMAP_THREADS=$(nproc 2>/dev/null || echo 4)

echo "Working dir : $(pwd)"
echo "Tools dir   : $TOOLS_DIR"
echo "Threads     : $MMAP_THREADS"
echo "Steps       : maps=$EXTRACT_DBC_AND_MAPS  vmaps=$EXTRACT_VMAPS  mmaps=$EXTRACT_MMAPS"
echo

# ─── Symlink Data/ → MPQ source (only when extracting from client) ──────────
if [ "$EXTRACT_DBC_AND_MAPS" = true ] || [ "$EXTRACT_VMAPS" = true ]; then
    has_mpqs() { find "$1" -maxdepth 1 -iname "*.mpq" -print -quit 2>/dev/null | grep -q .; }

    if has_mpqs "$WOW_CLIENT_DATA"; then
        MPQ_DIR="$WOW_CLIENT_DATA"
    elif has_mpqs "$WOW_CLIENT_DATA/Data"; then
        MPQ_DIR="$WOW_CLIENT_DATA/Data"
    else
        echo "ERROR: no .mpq files in $WOW_CLIENT_DATA" >&2
        exit 1
    fi
    MPQ_DIR="$(cd "$MPQ_DIR" && pwd)"

    # Symlink only — refuse to clobber an existing real directory.
    if [ -e Data ] && [ ! -L Data ]; then
        echo "ERROR: Data/ exists in $(pwd) but is not a symlink" >&2
        exit 1
    fi
    ln -sfn "$MPQ_DIR" Data
    echo "Data/ → $MPQ_DIR"
fi

# ─── STEP 1: DBCs + Maps ────────────────────────────────────────────────────
if [ "$EXTRACT_DBC_AND_MAPS" = true ]; then
    echo
    echo "[1/3] Extracting DBCs + Maps..."
    # Clean slate — map_extractor refuses to run if these dirs already exist.
    safe_rm dbc
    safe_rm maps
    safe_rm Cameras
    # -e 7 = bitfield MAP(1)|DBC(2)|CAMERA(4) — extract everything.
    # The old "-e 2" was DBC-only and skipped maps + cameras entirely.
    "$TOOLS_DIR/map_extractor" -e 7 -f 0
    if [ ! -d maps ] || [ -z "$(ls -A maps 2>/dev/null)" ]; then
        echo "ERROR: map_extractor finished but maps/ is empty — check its output above" >&2
        exit 1
    fi
fi

# ─── STEP 2: VMaps ──────────────────────────────────────────────────────────
if [ "$EXTRACT_VMAPS" = true ]; then
    echo
    echo "[2/3] Extracting VMaps..."
    # Clean slate — vmap4_extractor refuses to run if these dirs already exist.
    safe_rm Buildings
    safe_rm vmaps
    "$TOOLS_DIR/vmap4_extractor" -l -d ./Data
    mkdir -p vmaps
    "$TOOLS_DIR/vmap4_assembler" Buildings vmaps
    safe_rm Buildings
    if [ ! -d vmaps ] || [ -z "$(ls -A vmaps 2>/dev/null)" ]; then
        echo "ERROR: vmap4_assembler finished but vmaps/ is empty — check output above" >&2
        exit 1
    fi
fi

# ─── STEP 3: MMaps ──────────────────────────────────────────────────────────
if [ "$EXTRACT_MMAPS" = true ]; then
    if [ ! -d maps ]; then
        echo "ERROR: maps/ missing in $(pwd) — run with EXTRACT_DBC_AND_MAPS=true once" >&2
        exit 1
    fi
    if [ ! -d vmaps ]; then
        echo "ERROR: vmaps/ missing in $(pwd) — run with EXTRACT_VMAPS=true once" >&2
        exit 1
    fi

    echo
    echo "[3/3] Generating MMaps... (do not interrupt)"
    # Config precedence: an existing mmaps-config.yaml here, then one installed
    # next to the generator, then the embedded copy (synced from the core fork).
    if [ -f mmaps-config.yaml ]; then
        echo "Using existing mmaps-config.yaml in $(pwd)"
    elif [ -f "$TOOLS_DIR/mmaps-config.yaml" ]; then
        cp "$TOOLS_DIR/mmaps-config.yaml" mmaps-config.yaml
        echo "Using mmaps-config.yaml from $TOOLS_DIR"
    else
        printf '%s\n' "$MMAPS_CONFIG_YAML" > mmaps-config.yaml
        echo "No mmaps-config.yaml found — writing embedded copy"
    fi

    # Wipe any existing tiles before regenerating. Mixed tiles from
    # previous runs (different cellSize / verticesPerTileEdge / etc.)
    # would otherwise be silently kept and mixed with new ones,
    # producing a corrupt navmesh. Clean slate every mmap run.
    safe_rm mmaps
    mkdir -p mmaps

    # Workaround: some mmaps_generator builds write a few tiles to /mmaps via
    # an absolute path. We won't create a world-writable root directory here;
    # if your build is affected, create it once yourself before running:
    #     sudo mkdir -p /mmaps && sudo chown "$USER" /mmaps
    # Stale tiles from earlier runs must not leak into this one:
    if [ -d /mmaps ] && compgen -G "/mmaps/*.mmtile" >/dev/null; then
        if [ -w /mmaps ]; then
            rm -f /mmaps/*.mmtile
        else
            echo "ERROR: /mmaps contains stale .mmtile files but is not writable." >&2
            echo "Clean it first (needs privileges): sudo rm -f /mmaps/*.mmtile" >&2
            exit 1
        fi
    fi

    CMD=("$TOOLS_DIR/mmaps_generator" --config mmaps-config.yaml --threads "$MMAP_THREADS")
    [ -n "$MMAP_SINGLE_MAP" ] && CMD+=("$MMAP_SINGLE_MAP")

    START=$(date +%s)
    "${CMD[@]}"
    ELAPSED=$(( $(date +%s) - START ))

    # Fold any strays written to /mmaps back into the local output.
    if [ -d /mmaps ] && compgen -G "/mmaps/*.mmtile" >/dev/null; then
        cp /mmaps/*.mmtile mmaps/
        [ -w /mmaps ] && rm -f /mmaps/*.mmtile
    fi

    echo
    echo "MMap done in $((ELAPSED / 60))m $((ELAPSED % 60))s"
    echo "Tiles: $(ls mmaps/*.mmtile 2>/dev/null | wc -l)"
fi

echo
echo "Done. Restart worldserver to pick up changes."
