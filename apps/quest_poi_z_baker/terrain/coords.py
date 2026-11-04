"""
Coordinate conversion utilities for AzerothCore terrain data.

Converts between world coordinates (x, y) and grid/tile coordinates
used by .map, .vmap, and .mmap file systems.
"""

import os
from pathlib import Path
from typing import NamedTuple, Tuple


GRID_SIZE = 533.3333  # units per grid cell
MAX_GRIDS = 64        # grids per map edge
MAP_SIZE = GRID_SIZE * MAX_GRIDS  # ~34133.33 units per map edge

# .map tile resolution
MAP_TILE_VERTS_V8 = 128
MAP_TILE_VERTS_V9 = 129


class GridCoord(NamedTuple):
    """Grid coordinate (0-63, 0-63)."""
    x: int
    y: int


class TileCoord(NamedTuple):
    """Tile coordinate for vmap/mmap tiles."""
    x: int
    y: int


def world_to_gridcoord(x: float, y: float) -> GridCoord:
    """Convert world coordinates to grid coordinate (0-63).

    WotLK 3.3.5 world coordinates are center-origin: the map spans
    [-MAP_SIZE/2, +MAP_SIZE/2] and tile (0,0) is at the far +x,+y corner.
    Hence tile = 32 - world / GRID_SIZE (see MapDefines.h SIZE_OF_GRIDS
    and the file naming {map:03d}{tx:02d}{ty:02d}).
    """
    gx = int(32 - x / GRID_SIZE)
    gy = int(32 - y / GRID_SIZE)
    return GridCoord(
        x=max(0, min(MAX_GRIDS - 1, gx)),
        y=max(0, min(MAX_GRIDS - 1, gy)),
    )


def gridcoord_to_world(gx: int, gy: int) -> Tuple[float, float]:
    """Convert grid coordinate to world coordinate (center of grid)."""
    return (
        (32 - gx - 0.5) * GRID_SIZE,
        (32 - gy - 0.5) * GRID_SIZE,
    )


def world_to_map_tile(x: float, y: float) -> Tuple[int, int]:
    """Convert world coordinates to .map tile (gridX, gridY)."""
    return world_to_gridcoord(x, y)


def world_to_vmap_tile(x: float, y: float) -> Tuple[int, int]:
    """Convert world coordinates to vmap tile coordinate."""
    return world_to_gridcoord(x, y)


def world_to_mmap_tile(x: float, y: float) -> Tuple[int, int]:
    """Convert world coordinates to mmap tile coordinate.

    From MapBuilder::getTileBounds:
    tileX = 32 - world_x / GRID_SIZE, tileY = 32 - world_y / GRID_SIZE
    """
    tx = int(32 - x / GRID_SIZE)
    ty = int(32 - y / GRID_SIZE)
    return TileCoord(x=max(0, min(63, tx)), y=max(0, min(63, ty)))


def map_tile_filename(map_id: int, tile_x: int, tile_y: int) -> str:
    """Generate .map tile filename: {mapId:03d}{tileX:02d}{tileY:02d}.map"""
    return f"{map_id:03d}{tile_x:02d}{tile_y:02d}.map"


def vmap_tile_filename(map_id: int, tile_x: int, tile_y: int) -> str:
    """Generate .vmtile filename: {mapId:03d}_{tileX:02d}_{tileY:02d}.vmtile"""
    return f"{map_id:03d}_{tile_x:02d}_{tile_y:02d}.vmtile"


def vmap_tree_filename(map_id: int) -> str:
    """Generate .vmtree filename: {mapId:03d}.vmtree"""
    return f"{map_id:03d}.vmtree"


def mmap_tile_filename(map_id: int, tile_x: int, tile_y: int) -> str:
    """Generate .mmtile filename: {mapId:03d}{tileX:02d}{tileY:02d}.mmtile"""
    return f"{map_id:03d}{tile_x:02d}{tile_y:02d}.mmtile"


def mmap_main_filename(map_id: int) -> str:
    """Generate main .mmap filename: {mapId:03d}.mmap"""
    return f"{map_id:03d}.mmap"


def world_to_local_tile(x: float, y: float, tile_x: int, tile_y: int) -> Tuple[float, float]:
    """Convert world coords to local tile coords (0-1 range within tile).

    Tile (tile_x, tile_y) spans world x in
    [(32 - tile_x - 1) * GRID_SIZE, (32 - tile_x) * GRID_SIZE) —
    centered origin; world x increases toward decreasing tile index.
    """
    grid_x = (32 - tile_x - 1) * GRID_SIZE
    grid_y = (32 - tile_y - 1) * GRID_SIZE
    return ((x - grid_x) / GRID_SIZE, (y - grid_y) / GRID_SIZE)


def local_tile_to_world(local_x: float, local_y: float,
                        tile_x: int, tile_y: int) -> Tuple[float, float]:
    """Convert local tile coordinates back to world coordinates."""
    return (
        (32 - tile_x - 1) * GRID_SIZE + local_x * GRID_SIZE,
        (32 - tile_y - 1) * GRID_SIZE + local_y * GRID_SIZE,
    )


def is_valid_world_coord(x: float, y: float) -> bool:
    """Check if world coordinates are within valid map bounds."""
    return 0 <= x < MAP_SIZE and 0 <= y < MAP_SIZE


def is_valid_gridcoord(gx: int, gy: int) -> bool:
    """Check if grid coordinates are within valid range."""
    return 0 <= gx < MAX_GRIDS and 0 <= gy < MAX_GRIDS


def get_data_paths() -> dict:
    """Get data directory paths from environment or defaults."""
    base = Path(os.environ.get(
        "ACORE_DATA_PATH",
        os.environ.get("DATA_PATH", "/root/azerothcore-wotlk/env/dist/bin"),
    ))
    return {
        "maps": base / "maps",
        "vmaps": base / "vmaps",
        "mmaps": base / "mmaps",
    }
