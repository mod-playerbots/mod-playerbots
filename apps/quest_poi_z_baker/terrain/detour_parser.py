"""
Pure Python Detour navmesh tile binary parser.

Parses the raw binary data produced by dtCreateNavMeshData (after the
MmapTileHeader).  Layout (all sections 4-byte aligned via dtAlign4):

  1. dtMeshHeader          (100 bytes)
  2. Vertices              float[3] × vertCount
  3. Polygons (dtPoly)     32 bytes × polyCount
  4. Links (dtLink)        16 bytes × maxLinkCount  (zeroed on disk, built on load)
  5. Detail meshes         12 bytes × detailMeshCount
  6. Detail vertices       float[3] × detailVertCount
  7. Detail triangles      uchar[4] × detailTriCount
  8. BV tree (dtBVNode)    12 bytes × bvNodeCount
  9. Off-mesh connections  68 bytes × offMeshConCount

Coordinate convention: Detour Z-up  (y=AC_x, z=AC_y, x=AC_z).
"""

from __future__ import annotations

import struct
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

# ---- Detour constants ----
DT_NAVMESH_MAGIC = 0x444E4156  # "DNAV" little-endian
DT_NAVMESH_VERSION = 7
DT_VERTS_PER_POLYGON = 6
DT_NULL_LINK = 0xFFFFFFFF
DT_POLYTYPE_GROUND = 0x00
DT_POLYTYPE_OFFMESH_CONNECTION = 0x01
DT_POLYFLAGS_WALK = 0x0001
DT_POLYFLAGS_SWIM = 0x0002
DT_POLYFLAGS_FLY = 0x0004
DT_OFFMESH_CON_BIDIR = 0x0002


def _align4(n: int) -> int:
    return (n + 3) & ~3


# ---- Data classes ----

@dataclass
class DetourPoly:
    first_link: int = DT_NULL_LINK
    verts: List[int] = field(default_factory=list)       # vertex indices
    neis: List[int] = field(default_factory=list)        # neighbour polyRefs
    flags: int = 0
    vert_count: int = 0
    area: int = 0
    ptype: int = 0
    index: int = -1                                      # index within the tile

    @property
    def is_offmesh(self) -> bool:
        return self.ptype == DT_POLYTYPE_OFFMESH_CONNECTION


@dataclass
class DetourBVNode:
    bmin: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    bmax: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    i: int = 0


@dataclass
class DetourOffMeshConnection:
    """A dtOffMeshConnection (36 bytes on disk):
    pos[6] (start xyz, end xyz), radius, poly index, link flags, side,
    user id."""
    pos: Tuple[float, float, float, float, float, float] = (0.0,) * 6
    radius: float = 0.0
    poly: int = 0
    flags: int = 0
    side: int = 0
    user_id: int = 0

    @property
    def start(self) -> Tuple[float, float, float]:
        return (self.pos[0], self.pos[1], self.pos[2])

    @property
    def end(self) -> Tuple[float, float, float]:
        return (self.pos[3], self.pos[4], self.pos[5])

    @property
    def bidirectional(self) -> bool:
        return bool(self.flags & DT_OFFMESH_CON_BIDIR)


@dataclass
class DetourTileData:
    # Header
    x: int = 0
    y: int = 0
    layer: int = 0
    bmin: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    bmax: Tuple[float, float, float] = (0.0, 0.0, 0.0)
    walkable_height: float = 0.0
    walkable_radius: float = 0.0
    walkable_climb: float = 0.0
    bv_quant_factor: float = 0.0
    magic: int = 0
    version: int = 0
    user_id: int = 0
    off_mesh_base: int = 0

    # Geometry
    vertices: List[Tuple[float, float, float]] = field(default_factory=list)
    polygons: List[DetourPoly] = field(default_factory=list)
    bv_nodes: List[DetourBVNode] = field(default_factory=list)

    # Detail mesh
    detail_meshes: List[Tuple[int, int, int, int]] = field(default_factory=list)
    detail_verts: List[Tuple[float, float, float]] = field(default_factory=list)
    detail_tris: List[Tuple[int, int, int, int]] = field(default_factory=list)

    # Off-mesh connections
    off_mesh_cons: List[DetourOffMeshConnection] = field(default_factory=list)

    def get_poly_vertices(self, idx: int) -> List[Tuple[float, float, float]]:
        p = self.polygons[idx]
        return [self.vertices[i] for i in p.verts if i < len(self.vertices)]

    def get_poly_center(self, idx: int) -> Tuple[float, float, float]:
        vs = self.get_poly_vertices(idx)
        if not vs:
            return (0.0, 0.0, 0.0)
        return (sum(v[0] for v in vs) / len(vs),
                sum(v[1] for v in vs) / len(vs),
                sum(v[2] for v in vs) / len(vs))


# ---- Parser ----

class DetourParser:
    """Parse raw Detour tile binary data (after MmapTileHeader)."""

    def __init__(self) -> None:
        self._tile: Optional[DetourTileData] = None

    # ---- public ----

    def parse(self, data: bytes) -> DetourTileData:
        if len(data) < 100:
            raise ValueError(f"Too short for dtMeshHeader: {len(data)}")

        tile = DetourTileData()

        # 1. dtMeshHeader  (all int/float, 100 bytes)
        h = struct.unpack_from("<15i10f", data, 0)
        # fields: magic, version, x, y, layer, userId,
        #         polyCount, vertCount, maxLinkCount, detailMeshCount,
        #         detailVertCount, detailTriCount, bvNodeCount, offMeshConCount,
        #         offMeshBase, walkableH, walkableR, walkableClimb,
        #         bmin[3], bmax[3], bvQuantFactor

        magic = h[0]
        if magic != DT_NAVMESH_MAGIC:
            raise ValueError(f"Bad magic 0x{magic:08X}")
        if h[1] != DT_NAVMESH_VERSION:
            raise ValueError(f"Bad version {h[1]}")

        tile.magic, tile.version = magic, h[1]
        tile.x, tile.y, tile.layer = h[2], h[3], h[4]
        tile.user_id = h[5]
        poly_count, vert_count, max_link_count = h[6], h[7], h[8]
        detail_mesh_count = h[9]
        detail_vert_count, detail_tri_count = h[10], h[11]
        bv_node_count, off_mesh_con_count = h[12], h[13]
        off_mesh_base = h[14]
        tile.off_mesh_base = off_mesh_base
        tile.walkable_height = h[15]
        tile.walkable_radius = h[16]
        tile.walkable_climb = h[17]
        tile.bmin = (h[18], h[19], h[20])
        tile.bmax = (h[21], h[22], h[23])
        tile.bv_quant_factor = h[24]

        # Section offsets (cumulative, 4-byte aligned)
        off = _align4(100)                       # header

        # 2. Vertices
        verts_size = _align4(vert_count * 3 * 4)
        for i in range(vert_count):
            v = struct.unpack_from("<fff", data, off + i * 12)
            tile.vertices.append(v)
        off += verts_size

        # 3. Polygons  (dtPoly = 32 bytes each)
        poly_size = _align4(32)
        for i in range(poly_count):
            base = off + i * poly_size
            first_link = struct.unpack_from("<I", data, base)[0]
            verts_raw = struct.unpack_from("<6H", data, base + 4)
            neis_raw = struct.unpack_from("<6H", data, base + 16)
            flags = struct.unpack_from("<H", data, base + 28)[0]
            vert_cnt = data[base + 30]
            area_type = data[base + 31]
            area = area_type & 0x3F
            ptype = (area_type >> 6) & 0x03

            poly = DetourPoly()
            poly.first_link = first_link
            poly.verts = list(verts_raw[:vert_cnt])
            poly.neis = list(neis_raw)
            poly.flags = flags
            poly.vert_count = vert_cnt
            poly.area = area
            poly.ptype = ptype
            poly.index = i
            tile.polygons.append(poly)
        off += poly_count * poly_size

        # 4. Links — zeroed on disk, skip
        links_size = _align4(max_link_count * 16)
        off += links_size

        # 5. Detail meshes  (dtPolyDetail = 12 bytes)
        dm_size = _align4(12)
        for i in range(detail_mesh_count):
            base = off + i * dm_size
            vert_base, tri_base = struct.unpack_from("<II", data, base)
            vert_cnt, tri_cnt = struct.unpack_from("<BB", data, base + 8)
            tile.detail_meshes.append((vert_base, tri_base, vert_cnt, tri_cnt))
        off += detail_mesh_count * dm_size

        # 6. Detail vertices
        dv_size = _align4(detail_vert_count * 3 * 4)
        for i in range(detail_vert_count):
            tile.detail_verts.append(struct.unpack_from("<fff", data, off + i * 12))
        off += dv_size

        # 7. Detail triangles  (4 uchar each)
        for i in range(detail_tri_count):
            t = struct.unpack_from("<4B", data, off + i * 4)
            tile.detail_tris.append(t)
        off += _align4(detail_tri_count * 4)

        # 8. BV tree  (dtBVNode = 16 bytes; quantised coords relative to bmin)
        # dtBVNode: bmin[3] (unsigned short) + bmax[3] (unsigned short) + i (int)
        if bv_node_count > 0:
            qf = tile.bv_quant_factor
            for i in range(bv_node_count):
                base = off + i * 16
                bmin_raw = struct.unpack_from("<3H", data, base)
                bmax_raw = struct.unpack_from("<3H", data, base + 6)
                node_i = struct.unpack_from("<i", data, base + 12)[0]
                # De-quantize and convert to global coords (add tile bmin)
                tile.bv_nodes.append(DetourBVNode(
                    bmin=tuple(tile.bmin[j] + v / qf for j, v in enumerate(bmin_raw)),
                    bmax=tuple(tile.bmin[j] + v / qf for j, v in enumerate(bmax_raw)),
                    i=node_i,
                ))
            off += _align4(bv_node_count * 16)

        # 9. Off-mesh connections (dtOffMeshConnection, 36 bytes each):
        #    pos[6] floats, radius f32, poly u16, flags u8, side u8, userId u32
        for i in range(off_mesh_con_count):
            base = off + i * 36
            pos = struct.unpack_from("<6f", data, base)
            radius = struct.unpack_from("<f", data, base + 24)[0]
            poly = struct.unpack_from("<H", data, base + 28)[0]
            flags = data[base + 30]
            con_side = data[base + 31]
            user_id = struct.unpack_from("<I", data, base + 32)[0]
            tile.off_mesh_cons.append(DetourOffMeshConnection(
                pos=tuple(pos), radius=radius, poly=poly,
                flags=flags, side=con_side, user_id=user_id,
            ))
        off += off_mesh_con_count * _align4(36)

        self._tile = tile
        return tile
