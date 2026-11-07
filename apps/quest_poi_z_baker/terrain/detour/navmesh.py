"""Multi-tile Detour navmesh model (links, tile loading, poly queries).

Tile identity uses the Detour header coordinates (``tx, ty``) computed from
the map's ``dtNavMeshParams.orig``; the on-disk file names use the world grid
coordinates ``{map:03d}{gx:02d}{gy:02d}.mmtile``.  Both frames are indexed
once per map by reading every tile header.
"""

from __future__ import annotations

import math
import struct
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from .mathutil import (
    closest_height_point_triangle,
    distance_pt_poly_edges_sqr,
    distance_pt_seg_sqr_2d,
    point_in_polygon,
    v_lerp,
)
from ..detour_parser import (
    DT_OFFMESH_CON_BIDIR,
    DetourOffMeshConnection,
    DetourParser,
    DetourTileData,
)
from ..mmap_reader import NavMeshParams, parse_navmesh_params

DT_EXT_LINK = 0x8000
MMAP_TILE_HEADER_SIZE = 56
DT_NAVMESH_MAGIC = 0x444E4156

# Detour poly ref bit layout used internally (tx: bits 38-43, ty: 32-37).
REF_POLY_MASK = 0xFFFFFFFF


def make_ref(tx: int, ty: int, poly: int) -> int:
    return ((tx & 0x3F) << 38) | ((ty & 0x3F) << 32) | (poly & REF_POLY_MASK)


def ref_tile(ref: int) -> Tuple[int, int]:
    return ((ref >> 38) & 0x3F, (ref >> 32) & 0x3F)


def ref_poly(ref: int) -> int:
    return ref & REF_POLY_MASK


def opposite_tile(side: int) -> int:
    return (side + 4) & 0x7


# dtNavMesh::getNeighbourTilesAt offsets
_NEIGHBOUR_OFFSETS = {
    0: (1, 0), 1: (1, 1), 2: (0, 1), 3: (-1, 1),
    4: (-1, 0), 5: (-1, -1), 6: (0, -1), 7: (1, -1),
}


def _get_slab_coord(va, side: int) -> float:
    if side == 0 or side == 4:
        return va[0]
    if side == 2 or side == 6:
        return va[2]
    return 0.0


def _calc_slab_end_points(va, vb, side: int):
    if side == 0 or side == 4:
        if va[2] < vb[2]:
            return (va[2], va[1]), (vb[2], vb[1])
        return (vb[2], vb[1]), (va[2], va[1])
    if side == 2 or side == 6:
        if va[0] < vb[0]:
            return (va[0], va[1]), (vb[0], vb[1])
        return (vb[0], vb[1]), (va[0], va[1])
    return (0.0, 0.0), (0.0, 0.0)


def _overlap_slabs(amin, amax, bmin, bmax, px: float, py: float) -> bool:
    minx = max(amin[0] + px, bmin[0] + px)
    maxx = min(amax[0] - px, bmax[0] - px)
    if minx > maxx:
        return False
    if (amax[0] - amin[0]) == 0 or (bmax[0] - bmin[0]) == 0:
        return False
    ad = (amax[1] - amin[1]) / (amax[0] - amin[0])
    ak = amin[1] - ad * amin[0]
    bd = (bmax[1] - bmin[1]) / (bmax[0] - bmin[0])
    bk = bmin[1] - bd * bmin[0]
    aminy = ad * minx + ak
    amaxy = ad * maxx + ak
    bminy = bd * minx + bk
    bmaxy = bd * maxx + bk
    dmin = bminy - aminy
    dmax = bmaxy - amaxy
    if dmin * dmax < 0:
        return True
    thr = (py * 2.0) ** 2
    return dmin * dmin <= thr or dmax * dmax <= thr


class Tile:
    __slots__ = ("tx", "ty", "data", "links", "offmesh_by_poly", "_ext_index")

    def __init__(self, tx: int, ty: int, data: DetourTileData):
        self.tx = tx
        self.ty = ty
        self.data = data
        # links[poly_idx] = [(ref, edge, side, bmin, bmax), ...]
        self.links: List[List[Tuple[int, int, int, int, int]]] = []
        self.offmesh_by_poly: Dict[int, DetourOffMeshConnection] = {
            con.poly: con for con in data.off_mesh_cons
        }
        self._ext_index: Optional[Dict[int, List[Tuple[int, int, float]]]] = None

    @property
    def walkable_climb(self) -> float:
        return self.data.walkable_climb

    def ext_edges(self, side: int) -> List[Tuple[int, int, float]]:
        """Index of external edges by side: [(poly_idx, edge_idx, slab_coord)]."""
        if self._ext_index is None:
            index: Dict[int, List[Tuple[int, int, float]]] = {s: [] for s in range(8)}
            data = self.data
            for i, poly in enumerate(data.polygons):
                for j in range(poly.vert_count):
                    nei = poly.neis[j]
                    if not (nei & DT_EXT_LINK):
                        continue
                    d = nei & 0xFF
                    if d >= 8:
                        continue
                    if d == 0 or d == 4:
                        coord = data.vertices[poly.verts[j]][0]
                    elif d == 2 or d == 6:
                        coord = data.vertices[poly.verts[j]][2]
                    else:
                        coord = 0.0
                    index[d].append((i, j, coord))
            self._ext_index = index
        return self._ext_index[side]


class NavMesh:
    """Lazy multi-tile navmesh with server-equivalent link construction."""

    def __init__(self, map_id: int, mmaps_dir):
        self.map_id = map_id
        self.dir = Path(mmaps_dir)
        self.params: Optional[NavMeshParams] = parse_navmesh_params(
            self.dir / f"{map_id:03d}.mmap"
        )
        self.tiles: Dict[Tuple[int, int], Tile] = {}
        self._file_index: Optional[Dict[Tuple[int, int], Path]] = None
        self._parser = DetourParser()

    @property
    def has_navmesh(self) -> bool:
        return self.params is not None

    # ---- tile coordinates / file mapping ----

    def calc_tile_loc(self, pos) -> Tuple[int, int]:
        p = self.params
        tx = math.floor((pos[0] - p.orig[0]) / p.tile_width)
        ty = math.floor((pos[2] - p.orig[2]) / p.tile_height)
        return tx, ty

    def _build_file_index(self) -> Dict[Tuple[int, int], Path]:
        index: Dict[Tuple[int, int], Path] = {}
        for path in self.dir.glob(f"{self.map_id:03d}*.mmtile"):
            try:
                with open(path, "rb") as f:
                    head = f.read(MMAP_TILE_HEADER_SIZE + 100)
            except OSError:
                continue
            if len(head) < MMAP_TILE_HEADER_SIZE + 24:
                continue
            magic, _ver, x, y = struct.unpack_from(
                "<IIii", head, MMAP_TILE_HEADER_SIZE
            )
            if magic != DT_NAVMESH_MAGIC:
                continue
            index[(x, y)] = path
        return index

    def tile_file(self, tx: int, ty: int) -> Optional[Path]:
        if self._file_index is None:
            self._file_index = self._build_file_index()
        return self._file_index.get((tx, ty))

    def tile_at_detour(self, pos) -> Optional[Tile]:
        tx, ty = self.calc_tile_loc(pos)
        return self.tiles.get((tx, ty))

    def _neighbour(self, tx: int, ty: int, side: int) -> Optional[Tile]:
        dx, dy = _NEIGHBOUR_OFFSETS[side]
        return self.tiles.get((tx + dx, ty + dy))

    # ---- loading ----

    def ensure_tile(self, tx: int, ty: int) -> Optional[Tile]:
        key = (tx, ty)
        if key in self.tiles:
            return self.tiles[key]
        path = self.tile_file(tx, ty)
        if path is None:
            return None
        try:
            raw = path.read_bytes()
        except OSError:
            return None
        try:
            data = self._parser.parse(raw[MMAP_TILE_HEADER_SIZE:])
        except (ValueError, struct.error):
            return None
        tile = Tile(tx, ty, data)
        self.tiles[key] = tile
        self.build_links(tile)
        for side in range(8):
            neighbour = self._neighbour(tx, ty, side)
            if neighbour is not None:
                self.build_links(neighbour)
        return tile

    def ensure_tile_at(self, pos) -> Optional[Tile]:
        tx, ty = self.calc_tile_loc(pos)
        return self.ensure_tile(tx, ty)

    # ---- link construction (ports of dtNavMesh connect*) ----

    def build_links(self, tile: Tile) -> None:
        links: List[List[Tuple[int, int, int, int, int]]] = [
            [] for _ in range(len(tile.data.polygons))
        ]
        self._connect_int_links(tile, links)
        self._base_off_mesh_links(tile, links)
        self._end_off_mesh_links(tile, links)
        for side in range(0, 8, 2):
            neighbour = self._neighbour(tile.tx, tile.ty, side)
            if neighbour is not None:
                self._connect_ext_links(tile, neighbour, side, links)
        tile.links = links

    def _connect_int_links(self, tile: Tile, links) -> None:
        data = tile.data
        for i, poly in enumerate(data.polygons):
            if poly.is_offmesh:
                continue
            for j in range(poly.vert_count):
                nei = poly.neis[j]
                if nei == 0 or (nei & DT_EXT_LINK):
                    continue
                idx = nei - 1
                if idx < len(data.polygons):
                    links[i].append((make_ref(tile.tx, tile.ty, idx), j, 0xFF, 0, 0))

    def _base_off_mesh_links(self, tile: Tile, links) -> None:
        data = tile.data
        for con in data.off_mesh_cons:
            if con.poly >= len(data.polygons):
                continue
            extents = (con.radius, data.walkable_climb, con.radius)
            ref, nearest = self.find_nearest_poly_in_tile(tile, con.start, extents)
            if ref is None:
                continue
            if (nearest[0] - con.start[0]) ** 2 + (nearest[2] - con.start[2]) ** 2 > con.radius ** 2:
                continue
            poly = data.polygons[con.poly]
            if poly.vert_count > 0:
                data.vertices[poly.verts[0]] = nearest
            links[con.poly].append((ref, 0, 0xFF, 0, 0))
            land = ref_poly(ref)
            if land < len(links):
                links[land].append((make_ref(tile.tx, tile.ty, con.poly), 0xFF, 0xFF, 0, 0))

    def _end_off_mesh_links(self, tile: Tile, links) -> None:
        data = tile.data
        for con in data.off_mesh_cons:
            if con.poly >= len(data.polygons):
                continue
            end = con.end
            target = self.tile_at_detour(end)
            if target is None:
                continue
            extents = (con.radius, target.data.walkable_climb, con.radius)
            ref, nearest = self.find_nearest_poly_in_tile(target, end, extents)
            if ref is None:
                continue
            if (nearest[0] - end[0]) ** 2 + (nearest[2] - end[2]) ** 2 > con.radius ** 2:
                continue
            poly = data.polygons[con.poly]
            if poly.vert_count > 1:
                data.vertices[poly.verts[1]] = nearest
            same_tile = (target.tx, target.ty) == (tile.tx, tile.ty)
            side = 0xFF if same_tile else self._side_to(tile, target)
            links[con.poly].append((ref, 1, side, 0, 0))
            if con.flags & DT_OFFMESH_CON_BIDIR:
                land = ref_poly(ref)
                back_side = 0xFF if same_tile or side == 0xFF else opposite_tile(side)
                if land < len(links):
                    links[land].append(
                        (make_ref(tile.tx, tile.ty, con.poly), 0xFF, back_side, 0, 0)
                    )

    @staticmethod
    def _side_to(tile: Tile, target: Tile) -> int:
        dx = target.tx - tile.tx
        dy = target.ty - tile.ty
        for side, (ox, oy) in _NEIGHBOUR_OFFSETS.items():
            if (ox, oy) == (dx, dy):
                return side
        return 0xFF

    def _connect_ext_links(self, tile: Tile, target: Tile, side: int, links) -> None:
        data = tile.data
        for i, poly in enumerate(data.polygons):
            if poly.is_offmesh:
                continue
            for j in range(poly.vert_count):
                nei = poly.neis[j]
                if not (nei & DT_EXT_LINK):
                    continue
                dir_ = nei & 0xFF
                if side != -1 and dir_ != side:
                    continue
                va = data.vertices[poly.verts[j]]
                vb = data.vertices[poly.verts[(j + 1) % poly.vert_count]]
                found = self._find_connecting_polys(va, vb, target, opposite_tile(dir_))
                for ref, a0, a1 in found:
                    bmin_b, bmax_b = 0, 0
                    if dir_ in (0, 4):
                        denom = vb[2] - va[2]
                        if denom:
                            tmin = (a0 - va[2]) / denom
                            tmax = (a1 - va[2]) / denom
                            if tmin > tmax:
                                tmin, tmax = tmax, tmin
                            bmin_b = int(max(0.0, min(1.0, tmin)) * 255.0)
                            bmax_b = int(max(0.0, min(1.0, tmax)) * 255.0)
                    elif dir_ in (2, 6):
                        denom = vb[0] - va[0]
                        if denom:
                            tmin = (a0 - va[0]) / denom
                            tmax = (a1 - va[0]) / denom
                            if tmin > tmax:
                                tmin, tmax = tmax, tmin
                            bmin_b = int(max(0.0, min(1.0, tmin)) * 255.0)
                            bmax_b = int(max(0.0, min(1.0, tmax)) * 255.0)
                    links[i].append((ref, j, dir_, bmin_b, bmax_b))

    def _find_connecting_polys(self, va, vb, target: Tile, side: int):
        data = target.data
        amin, amax = _calc_slab_end_points(va, vb, side)
        apos = _get_slab_coord(va, side)
        results = []
        for i, j, bpos in target.ext_edges(side):
            if abs(apos - bpos) > 0.01:
                continue
            poly = data.polygons[i]
            vc = data.vertices[poly.verts[j]]
            vd = data.vertices[poly.verts[(j + 1) % poly.vert_count]]
            bmin, bmax = _calc_slab_end_points(vc, vd, side)
            if not _overlap_slabs(amin, amax, bmin, bmax, 0.01,
                                  data.walkable_climb):
                continue
            if len(results) < 4:
                results.append((
                    make_ref(target.tx, target.ty, i),
                    max(amin[0], bmin[0]),
                    min(amax[0], bmax[0]),
                ))
        return results

    # ---- polygon queries ----

    def poly(self, ref: int):
        tx, ty = ref_tile(ref)
        tile = self.tiles.get((tx, ty))
        if tile is None:
            return None, None
        idx = ref_poly(ref)
        if idx >= len(tile.data.polygons):
            return tile, None
        return tile, tile.data.polygons[idx]

    def poly_vertices(self, ref: int):
        tile, poly = self.poly(ref)
        if poly is None:
            return None, None, None
        verts = [tile.data.vertices[i] for i in poly.verts]
        return tile, poly, verts

    def find_nearest_poly_in_tile(self, tile: Tile, center, half_extents):
        """Port of dtNavMesh::findNearestPolyInTile (brute-force BV equivalent)."""
        bmin = (center[0] - half_extents[0],
                center[1] - half_extents[1],
                center[2] - half_extents[2])
        bmax = (center[0] + half_extents[0],
                center[1] + half_extents[1],
                center[2] + half_extents[2])
        best_ref = None
        best_pt = None
        best_d = math.inf
        for i, poly in enumerate(tile.data.polygons):
            if poly.is_offmesh:
                continue
            pverts = [tile.data.vertices[v] for v in poly.verts]
            if not _poly_aabb_overlaps(pverts, bmin, bmax):
                continue
            ref = make_ref(tile.tx, tile.ty, i)
            closest, pos_over = self.closest_point_on_poly(ref, center)
            diff = (center[0] - closest[0], center[1] - closest[1], center[2] - closest[2])
            if pos_over:
                d = abs(diff[1]) - tile.data.walkable_climb
                d = d * d if d > 0 else 0.0
            else:
                d = diff[0] ** 2 + diff[1] ** 2 + diff[2] ** 2
            if d < best_d:
                best_d = d
                best_ref = ref
                best_pt = closest
        return best_ref, best_pt

    def find_nearest_poly(self, center, half_extents, filt):
        """Port of dtNavMeshQuery::findNearestPoly over loaded/loadable tiles."""
        bmin = (center[0] - half_extents[0],
                center[1] - half_extents[1],
                center[2] - half_extents[2])
        bmax = (center[0] + half_extents[0],
                center[1] + half_extents[1],
                center[2] + half_extents[2])
        minx, miny = self.calc_tile_loc(bmin)
        maxx, maxy = self.calc_tile_loc(bmax)
        best_ref = None
        best_pt = None
        best_d = math.inf
        for ty in range(miny, maxy + 1):
            for tx in range(minx, maxx + 1):
                tile = self.tiles.get((tx, ty))
                if tile is None:
                    tile = self.ensure_tile(tx, ty)
                if tile is None:
                    continue
                for i, poly in enumerate(tile.data.polygons):
                    if poly.is_offmesh:
                        continue
                    if not filt.pass_filter(poly):
                        continue
                    pverts = [tile.data.vertices[v] for v in poly.verts]
                    if not _poly_aabb_overlaps(pverts, bmin, bmax):
                        continue
                    ref = make_ref(tx, ty, i)
                    closest, pos_over = self.closest_point_on_poly(ref, center)
                    diff = (center[0] - closest[0], center[1] - closest[1],
                            center[2] - closest[2])
                    if pos_over:
                        d = abs(diff[1]) - tile.data.walkable_climb
                        d = d * d if d > 0 else 0.0
                    else:
                        d = diff[0] ** 2 + diff[1] ** 2 + diff[2] ** 2
                    if d < best_d:
                        best_d = d
                        best_ref = ref
                        best_pt = closest
        return best_ref, best_pt

    def closest_point_on_poly_boundary(self, ref: int, pos):
        """Port of dtNavMeshQuery::closestPointOnPolyBoundary."""
        tile, poly, verts = self.poly_vertices(ref)
        if poly is None or len(verts) < 3:
            return None
        inside, ed, et = distance_pt_poly_edges_sqr(pos, verts)
        if inside:
            return pos
        dmin = ed[0]
        imin = 0
        for i in range(1, len(verts)):
            if ed[i] < dmin:
                dmin = ed[i]
                imin = i
        va = verts[imin]
        vb = verts[(imin + 1) % len(verts)]
        return v_lerp(va, vb, et[imin])

    def get_poly_height(self, ref: int, pos):
        """Port of dtNavMesh::getPolyHeight. Returns height or None."""
        tile, poly, verts = self.poly_vertices(ref)
        if poly is None:
            return None
        if poly.is_offmesh:
            if len(verts) < 2:
                return None
            d, t = distance_pt_seg_sqr_2d(pos, verts[0], verts[1])
            return verts[0][1] + (verts[1][1] - verts[0][1]) * t
        if not point_in_polygon(pos, verts):
            return None
        mesh = tile.data.detail_meshes[poly_index(poly, tile)]
        vert_base, tri_base, _vert_count, tri_count = mesh
        for j in range(tri_count):
            tri = tile.data.detail_tris[tri_base + j]
            v = []
            for k in range(3):
                if tri[k] < poly.vert_count:
                    v.append(tile.data.vertices[poly.verts[tri[k]]])
                else:
                    v.append(tile.data.detail_verts[
                        vert_base + (tri[k] - poly.vert_count)])
            ok, h = closest_height_point_triangle(pos, v[0], v[1], v[2])
            if ok:
                return h
        # Degenerate triangles/edges: closest point on detail edges.
        closest = self._closest_point_on_detail_edges(tile, poly, pos, False)
        return closest[1]

    def closest_point_on_poly(self, ref: int, pos):
        """Port of dtNavMesh::closestPointOnPoly.

        Returns (closest, pos_over_poly).
        """
        tile, poly, verts = self.poly_vertices(ref)
        if poly is None:
            return pos, False
        if not poly.is_offmesh:
            height = self.get_poly_height(ref, pos)
            if height is not None:
                return (pos[0], height, pos[2]), True
        if poly.is_offmesh and len(verts) >= 2:
            _d, t = distance_pt_seg_sqr_2d(pos, verts[0], verts[1])
            return v_lerp(verts[0], verts[1], t), False
        closest = self._closest_point_on_detail_edges(tile, poly, pos, True)
        return closest, False

    def _closest_point_on_detail_edges(self, tile: Tile, poly, pos, only_boundary: bool):
        (vert_base, tri_base, _vc, tri_count) = tile.data.detail_meshes[
            poly_index(poly, tile)
        ]
        dmin = math.inf
        tmin = 0.0
        pmin = None
        pmax = None
        for i in range(tri_count):
            tri = tile.data.detail_tris[tri_base + i]
            flags = tri[3]
            any_boundary = 0x01 | (0x01 << 2) | (0x01 << 4)
            if only_boundary and (flags & any_boundary) == 0:
                continue
            v = []
            for k in range(3):
                if tri[k] < poly.vert_count:
                    v.append(tile.data.vertices[poly.verts[tri[k]]])
                else:
                    v.append(tile.data.detail_verts[vert_base + (tri[k] - poly.vert_count)])
            for k in range(3):
                j = (k + 2) % 3
                edge_flags = (flags >> (j * 2)) & 3
                if (edge_flags & 0x01) == 0 and (only_boundary or tri[j] < tri[k]):
                    continue
                d, t = distance_pt_seg_sqr_2d(pos, v[j], v[k])
                if d < dmin:
                    dmin = d
                    tmin = t
                    pmin = v[j]
                    pmax = v[k]
        if pmin is None:
            return pos
        return v_lerp(pmin, pmax, tmin)


def poly_index(poly, tile: Tile) -> int:
    """Index of a poly within its tile's poly array."""
    return poly.index


def _poly_aabb_overlaps(verts, bmin, bmax) -> bool:
    if not verts:
        return False
    vmin = list(verts[0])
    vmax = list(verts[0])
    for v in verts[1:]:
        for i in range(3):
            if v[i] < vmin[i]:
                vmin[i] = v[i]
            if v[i] > vmax[i]:
                vmax[i] = v[i]
    for i in range(3):
        if vmax[i] < bmin[i] or vmin[i] > bmax[i]:
            return False
    return True
