"""
Binary parser for AzerothCore .mmap files (MMAP + Detour navmesh format).

Parses MmapTileHeader (56 bytes) and Detour dtMeshHeader (100 bytes) to extract
navmesh metadata: polygon counts, vertex counts, tile bounds, Recast config.
"""

import struct
from pathlib import Path
from typing import Dict, List, NamedTuple, Optional, Tuple

from .coords import mmap_main_filename, mmap_tile_filename


MMAP_MAGIC = 0x4D4D4150  # "MMAP"
MMAP_VERSION = 20


class NavMeshParams(NamedTuple):
    """dtNavMeshParams as stored in the main {map}.mmap file (28 bytes).

    Note: the file has no magic/version header; it is exactly the struct:
    float orig[3]; float tileWidth; float tileHeight; int maxTiles; int maxPolys.
    """
    orig: Tuple[float, float, float]
    tile_width: float
    tile_height: float
    max_tiles: int
    max_polys: int


def parse_navmesh_params(path) -> Optional[NavMeshParams]:
    """Parse the main .mmap navmesh params file (dtNavMeshParams)."""
    try:
        with open(path, "rb") as f:
            data = f.read(28)
    except OSError:
        return None
    if len(data) < 28:
        return None
    ox, oy, oz, tw, th = struct.unpack_from("<5f", data, 0)
    max_tiles, max_polys = struct.unpack_from("<ii", data, 20)
    return NavMeshParams(
        orig=(ox, oy, oz),
        tile_width=tw,
        tile_height=th,
        max_tiles=max_tiles,
        max_polys=max_polys,
    )

# Detour constants
DT_NAVMESH_MAGIC = 0x444E4156  # "DNAV" as little-endian uint32
DT_NAVMESH_VERSION = 7

# Polygon flags
DT_POLYAREA_WATER = 1
DT_POLYAREA_GROUND = 2
DT_POLYAREA_LOWGROUND = 3
DT_POLYAREA_PLATFORM = 4
DT_POLYAREA_GRASS = 5
DT_POLYAREA_DIRT = 6
DT_POLYAREA_ROAD = 7
DT_POLYAREA_WALL = 8
DT_POLYAREA_DOOR = 9
DT_POLYAREA_NONWALKABLE = 10
DT_POLYAREA_JUMP = 11

DT_POLYTYPE_OFFMESH_CONNECTION = 2

# Nav terrain types
NAV_EMPTY = 0x00
NAV_GROUND = 0x01
NAV_MAGMA = 0x02
NAV_SLIME = 0x04
NAV_WATER = 0x08

NAV_TERRAIN_NAMES = {
    NAV_EMPTY: "EMPTY",
    NAV_GROUND: "GROUND",
    NAV_MAGMA: "MAGMA",
    NAV_SLIME: "SLIME",
    NAV_WATER: "WATER",
}

POLY_AREA_NAMES = {
    DT_POLYAREA_WATER: "WATER",
    DT_POLYAREA_GROUND: "GROUND",
    DT_POLYAREA_LOWGROUND: "LOWGROUND",
    DT_POLYAREA_PLATFORM: "PLATFORM",
    DT_POLYAREA_GRASS: "GRASS",
    DT_POLYAREA_DIRT: "DIRT",
    DT_POLYAREA_ROAD: "ROAD",
    DT_POLYAREA_WALL: "WALL",
    DT_POLYAREA_DOOR: "DOOR",
    DT_POLYAREA_NONWALKABLE: "NONWALKABLE",
    DT_POLYAREA_JUMP: "JUMP",
}


class RecastConfig(NamedTuple):
    """Recast navigation mesh generation parameters."""
    walkable_slope_angle: float
    walkable_radius: int
    walkable_height: int
    walkable_climb: int
    vertex_per_map_edge: int
    vertex_per_tile_edge: int
    tiles_per_map_edge: int
    base_unit_dim: float
    cell_size_horizontal: float
    cell_size_vertical: float
    max_simplification_error: float


class MMapTileHeader(NamedTuple):
    """Parsed MmapTileHeader (56 bytes)."""
    magic: int
    dt_version: int
    mmap_version: int
    size: int
    uses_liquids: bool
    recast_config: RecastConfig


class DetourMeshHeader(NamedTuple):
    """Parsed Detour dtMeshHeader (100 bytes)."""
    magic: int
    version: int
    x: int
    y: int
    layer: int
    user_id: int
    poly_count: int
    vert_count: int
    max_link_count: int
    detail_mesh_count: int
    detail_vert_count: int
    detail_tri_count: int
    bv_node_count: int
    off_mesh_con_count: int
    off_mesh_base: int
    walkable_height: float
    walkable_radius: float
    walkable_climb: float
    bmin: Tuple[float, float, float]
    bmax: Tuple[float, float, float]
    bv_quant_factor: float


class MMapTileInfo(NamedTuple):
    """Complete info about an mmap tile."""
    map_id: int
    tile_x: int
    tile_y: int
    mmap_header: Optional[MMapTileHeader]
    detour_header: Optional[DetourMeshHeader]
    file_size: int


class MMapReader:
    """Reads and parses .mmap files."""

    def __init__(self, mmaps_path: Path):
        self.mmaps_path = mmaps_path
        self._cache: Dict[Tuple[int, int, int], MMapTileInfo] = {}

    def get_tile_info(self, map_id: int, tile_x: int, tile_y: int) -> Optional[MMapTileInfo]:
        """Get info for a specific mmap tile."""
        cache_key = (map_id, tile_x, tile_y)
        if cache_key in self._cache:
            return self._cache[cache_key]

        tile_path = self.mmaps_path / mmap_tile_filename(map_id, tile_x, tile_y)
        if not tile_path.exists():
            return None

        info = self._parse_mmtile(tile_path, map_id, tile_x, tile_y)
        if info:
            self._cache[cache_key] = info
        return info

    def get_main_info(self, map_id: int) -> dict:
        """Get info about the main .mmap file (dtNavMeshParams)."""
        main_path = self.mmaps_path / mmap_main_filename(map_id)
        if not main_path.exists():
            return {"exists": False}

        params = parse_navmesh_params(main_path)
        if params is None:
            return {"exists": True, "error": "File too small"}

        return {
            "exists": True,
            "origin": {"x": params.orig[0], "y": params.orig[1], "z": params.orig[2]},
            "tile_width": params.tile_width,
            "tile_height": params.tile_height,
            "max_tiles": params.max_tiles,
            "max_polys": params.max_polys,
            "file_size": main_path.stat().st_size,
        }

    def list_tiles(self, map_id: int) -> List[str]:
        """List all mmap tiles for a map."""
        if not self.mmaps_path.exists():
            return []

        prefix = f"{map_id:03d}"
        tiles = []
        for f in sorted(self.mmaps_path.iterdir()):
            if f.name.startswith(prefix) and f.suffix == ".mmtile":
                tiles.append(f.name)
        return tiles

    def get_tile_stats(self, map_id: int, tile_x: int, tile_y: int) -> Optional[dict]:
        """Get human-readable stats for an mmap tile."""
        info = self.get_tile_info(map_id, tile_x, tile_y)
        if not info or not info.detour_header:
            return None

        dh = info.detour_header
        mh = info.mmap_header

        result = {
            "map_id": map_id,
            "tile": {"x": tile_x, "y": tile_y},
            "file_size": info.file_size,
            "navmesh": {
                "poly_count": dh.poly_count,
                "vert_count": dh.vert_count,
                "detail_mesh_count": dh.detail_mesh_count,
                "detail_vert_count": dh.detail_vert_count,
                "detail_tri_count": dh.detail_tri_count,
                "bv_node_count": dh.bv_node_count,
                "off_mesh_con_count": dh.off_mesh_con_count,
                "bounds": {
                    "min": {"x": dh.bmin[0], "y": dh.bmin[1], "z": dh.bmin[2]},
                    "max": {"x": dh.bmax[0], "y": dh.bmax[1], "z": dh.bmax[2]},
                },
                "walkable": {
                    "height": dh.walkable_height,
                    "radius": dh.walkable_radius,
                    "climb": dh.walkable_climb,
                },
            },
        }

        if mh:
            rc = mh.recast_config
            result["mmap"] = {
                "version": mh.mmap_version,
                "dt_version": mh.dt_version,
                "uses_liquids": mh.uses_liquids,
                "recast_config": {
                    "walkable_slope_angle": rc.walkable_slope_angle,
                    "walkable_radius": rc.walkable_radius,
                    "walkable_height": rc.walkable_height,
                    "walkable_climb": rc.walkable_climb,
                    "cell_size_horizontal": rc.cell_size_horizontal,
                    "cell_size_vertical": rc.cell_size_vertical,
                    "max_simplification_error": rc.max_simplification_error,
                    "vertex_per_tile_edge": rc.vertex_per_tile_edge,
                },
            }

        return result

    def _parse_mmtile(self, path: Path, map_id: int, tile_x: int, tile_y: int) -> Optional[MMapTileInfo]:
        """Parse an .mmtile file."""
        with open(path, "rb") as f:
            data = f.read()

        file_size = path.stat().st_size

        # Parse MmapTileHeader (56 bytes)
        mmap_header = self._parse_mmap_header(data)

        # Parse Detour mesh header (starts at offset 56)
        detour_header = None
        if len(data) > 56:
            detour_header = self._parse_detour_header(data, 56)

        return MMapTileInfo(
            map_id=map_id,
            tile_x=tile_x,
            tile_y=tile_y,
            mmap_header=mmap_header,
            detour_header=detour_header,
            file_size=file_size,
        )

    def _parse_mmap_header(self, data: bytes) -> Optional[MMapTileHeader]:
        """Parse MmapTileHeader (56 bytes)."""
        if len(data) < 56:
            return None

        magic, dt_version, mmap_version, size = struct.unpack_from("<IIII", data, 0)
        if magic != MMAP_MAGIC:
            return None

        uses_liquids = data[16] != 0

        # RecastConfig (36 bytes) at offset 20
        (walkable_slope_angle,
         walkable_radius, walkable_height, walkable_climb,
         vertex_per_map_edge, vertex_per_tile_edge, tiles_per_map_edge,
         base_unit_dim, cell_size_h, cell_size_v, max_simp_error
         ) = struct.unpack_from("<f3Bx3I4f", data, 20)

        rc = RecastConfig(
            walkable_slope_angle=walkable_slope_angle,
            walkable_radius=walkable_radius,
            walkable_height=walkable_height,
            walkable_climb=walkable_climb,
            vertex_per_map_edge=vertex_per_map_edge,
            vertex_per_tile_edge=vertex_per_tile_edge,
            tiles_per_map_edge=tiles_per_map_edge,
            base_unit_dim=base_unit_dim,
            cell_size_horizontal=cell_size_h,
            cell_size_vertical=cell_size_v,
            max_simplification_error=max_simp_error,
        )

        return MMapTileHeader(
            magic=magic,
            dt_version=dt_version,
            mmap_version=mmap_version,
            size=size,
            uses_liquids=uses_liquids,
            recast_config=rc,
        )

    def _parse_detour_header(self, data: bytes, offset: int) -> Optional[DetourMeshHeader]:
        """Parse Detour dtMeshHeader (100 bytes)."""
        if len(data) < offset + 100:
            return None

        magic, version = struct.unpack_from("<II", data, offset)
        if magic != DT_NAVMESH_MAGIC:
            return None

        x, y, layer = struct.unpack_from("<iii", data, offset + 8)
        user_id = struct.unpack_from("<I", data, offset + 20)[0]

        (poly_count, vert_count, max_link_count,
         detail_mesh_count, detail_vert_count, detail_tri_count,
         bv_node_count, off_mesh_con_count, off_mesh_base
         ) = struct.unpack_from("<iiiiiiiii", data, offset + 24)

        (walkable_height, walkable_radius, walkable_climb
         ) = struct.unpack_from("<fff", data, offset + 60)

        bmin = struct.unpack_from("<fff", data, offset + 72)
        bmax = struct.unpack_from("<fff", data, offset + 84)

        bv_quant_factor = struct.unpack_from("<f", data, offset + 96)[0]

        return DetourMeshHeader(
            magic=magic,
            version=version,
            x=x,
            y=y,
            layer=layer,
            user_id=user_id,
            poly_count=poly_count,
            vert_count=vert_count,
            max_link_count=max_link_count,
            detail_mesh_count=detail_mesh_count,
            detail_vert_count=detail_vert_count,
            detail_tri_count=detail_tri_count,
            bv_node_count=bv_node_count,
            off_mesh_con_count=off_mesh_con_count,
            off_mesh_base=off_mesh_base,
            walkable_height=walkable_height,
            walkable_radius=walkable_radius,
            walkable_climb=walkable_climb,
            bmin=bmin,
            bmax=bmax,
            bv_quant_factor=bv_quant_factor,
        )
