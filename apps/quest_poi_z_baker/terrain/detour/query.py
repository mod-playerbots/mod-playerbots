"""Ports of the dtNavMeshQuery algorithms used by AzerothCore pathfinding.

Includes the stock Detour best-first A* (with an unlimited node pool by
default), the string-pulling straight path, surface movement, raycast and
off-mesh helpers.  Cost/tie semantics mirror the vendored Detour so paths
match the running worldserver as closely as possible.
"""

from __future__ import annotations

import math
from typing import Dict, List, Optional, Tuple

from .mathutil import (
    distance_pt_seg_sqr_2d,
    intersect_segment_poly_2d,
    point_in_polygon,
    tri_area2d,
    v_dist,
    v_equal,
    v_lerp,
    v_mad,
)
from .navmesh import (
    DT_EXT_LINK,
    NavMesh,
    Tile,
    make_ref,
)

# dtStatus flags
DT_FAILURE = 1 << 31
DT_SUCCESS = 1 << 30
DT_IN_PROGRESS = 1 << 29
DT_INVALID_PARAM = 1 << 25
DT_BUFFER_TOO_SMALL = 1 << 24
DT_OUT_OF_NODES = 1 << 23
DT_PARTIAL_RESULT = 1 << 22

# dtNodeFlags
DT_NODE_OPEN = 0x01
DT_NODE_CLOSED = 0x02

# dtStraightPathFlags
DT_STRAIGHTPATH_START = 0x01
DT_STRAIGHTPATH_END = 0x02
DT_STRAIGHTPATH_OFFMESH_CONNECTION = 0x04
DT_STRAIGHTPATH_AREA_CROSSINGS = 0x10
DT_STRAIGHTPATH_ALL_CROSSINGS = 0x20

DT_POLYTYPE_GROUND = 0x00
DT_POLYTYPE_OFFMESH_CONNECTION = 0x01

H_SCALE = 0.999
FAR_FROM_POLY = 50.0
DEFAULT_MAX_NODES = None
DEFAULT_MAX_POINTS = None
DEFAULT_MAX_VISITED = 16


class Node:
    __slots__ = ("pos", "cost", "total", "pidx", "state", "flags", "id",
                 "heap_index")

    def __init__(self) -> None:
        self.pos = (0.0, 0.0, 0.0)
        self.cost = 0.0
        self.total = 0.0
        self.pidx: Optional["Node"] = None
        self.state = 0
        self.flags = 0
        self.id = 0
        self.heap_index = -1


class NodePool:
    """dtNodePool: a poly ref may have multiple nodes with different states."""

    def __init__(self, max_nodes: Optional[int] = None):
        self.max_nodes = max_nodes
        self.nodes: Dict[Tuple[int, int], Node] = {}

    def clear(self) -> None:
        self.nodes.clear()

    def get_node(self, ref: int, state: int = 0) -> Optional[Node]:
        key = (ref, state)
        node = self.nodes.get(key)
        if node is not None:
            return node
        if self.max_nodes is not None and len(self.nodes) >= self.max_nodes:
            return None
        node = Node()
        node.id = ref
        node.state = state
        self.nodes[key] = node
        return node


class NodeQueue:
    """dtNodeQueue binary heap (bubbleUp/trickleDown/modify)."""

    def __init__(self) -> None:
        self.heap: List[Node] = []

    def clear(self) -> None:
        for node in self.heap:
            node.heap_index = -1
        self.heap.clear()

    def empty(self) -> bool:
        return not self.heap

    def push(self, node: Node) -> None:
        self.heap.append(node)
        self._bubble_up(len(self.heap) - 1, node)

    def pop(self) -> Node:
        result = self.heap[0]
        last = self.heap.pop()
        if self.heap:
            self._trickle_down(0, last)
        result.heap_index = -1
        return result

    def modify(self, node: Node) -> None:
        if 0 <= node.heap_index < len(self.heap) and self.heap[node.heap_index] is node:
            self._bubble_up(node.heap_index, node)

    def _bubble_up(self, i: int, node: Node) -> None:
        parent = (i - 1) // 2
        heap = self.heap
        while i > 0 and heap[parent].total > node.total:
            heap[i] = heap[parent]
            heap[i].heap_index = i
            i = parent
            parent = (i - 1) // 2
        heap[i] = node
        node.heap_index = i

    def _trickle_down(self, i: int, node: Node) -> None:
        heap = self.heap
        size = len(heap)
        child = i * 2 + 1
        while child < size:
            if (child + 1) < size and heap[child].total > heap[child + 1].total:
                child += 1
            heap[i] = heap[child]
            heap[i].heap_index = i
            i = child
            child = i * 2 + 1
        self._bubble_up(i, node)


class NavMeshQuery:
    """dtNavMeshQuery port bound to a NavMesh and filter."""

    def __init__(self, navmesh: NavMesh, filt, max_nodes: Optional[int] = None):
        self.nav = navmesh
        self.filter = filt
        self._pool = NodePool(max_nodes)
        self._queue = NodeQueue()

    # ---- ref/link helpers ----

    def _links_for(self, ref: int):
        tile, poly = self.nav.poly(ref)
        if poly is None:
            return []
        self._ensure_needed_tiles(tile, poly)
        return tile.links[poly.index] if poly.index < len(tile.links) else []

    def _ensure_needed_tiles(self, tile: Tile, poly) -> None:
        if poly.is_offmesh:
            if poly.index < len(tile.links) and not tile.links[poly.index]:
                con = tile.offmesh_by_poly.get(poly.index)
                if con is not None:
                    self.nav.ensure_tile_at(con.end)
            return
        needed = set()
        for j in range(poly.vert_count):
            nei = poly.neis[j]
            if nei & DT_EXT_LINK:
                needed.add(nei & 0xFF)
        for side in needed:
            from .navmesh import _NEIGHBOUR_OFFSETS
            dx, dy = _NEIGHBOUR_OFFSETS[side]
            nx, ny = tile.tx + dx, tile.ty + dy
            if (nx, ny) not in self.nav.tiles and self.nav.tile_file(nx, ny) is not None:
                self.nav.ensure_tile(nx, ny)

    def _find_link(self, from_ref: int, to_ref: int):
        tile, poly = self.nav.poly(from_ref)
        if poly is None:
            return None
        links = tile.links[poly.index] if poly.index < len(tile.links) else []
        for link in links:
            if link[0] == to_ref:
                return tile, poly, link
        return None

    def get_portal_points(self, from_ref: int, to_ref: int):
        """Port of dtNavMeshQuery::getPortalPoints. Returns (left, right)."""
        found = self._find_link(from_ref, to_ref)
        if found is None:
            return None
        from_tile, from_poly, link = found
        to_tile, to_poly = self.nav.poly(to_ref)

        if from_poly.is_offmesh:
            for cand in from_tile.links[from_poly.index]:
                if cand[0] == to_ref:
                    vert = from_poly.verts[cand[1]]
                    point = from_tile.data.vertices[vert]
                    return point, point
            return None
        if to_poly is not None and to_poly.is_offmesh:
            for cand in to_tile.links[to_poly.index]:
                if cand[0] == from_ref:
                    vert = to_poly.verts[cand[1]]
                    point = to_tile.data.vertices[vert]
                    return point, point
            return None

        edge = link[1]
        v0 = from_poly.verts[edge]
        v1 = from_poly.verts[(edge + 1) % from_poly.vert_count]
        left = from_tile.data.vertices[v0]
        right = from_tile.data.vertices[v1]
        side = link[2]
        bmin, bmax = link[3], link[4]
        if side != 0xFF and (bmin != 0 or bmax != 255):
            s = 1.0 / 255.0
            left = v_lerp(left, right, bmin * s)
            right = v_lerp(from_tile.data.vertices[v0],
                           from_tile.data.vertices[v1], bmax * s)
        return left, right

    def get_edge_mid_point(self, from_ref: int, to_ref: int):
        pts = self.get_portal_points(from_ref, to_ref)
        if pts is None:
            return None
        left, right = pts
        return ((left[0] + right[0]) * 0.5,
                (left[1] + right[1]) * 0.5,
                (left[2] + right[2]) * 0.5)

    def get_off_mesh_connection_poly_end_points(self, prev_ref: int, poly_ref: int):
        """Port of dtNavMesh::getOffMeshConnectionPolyEndPoints."""
        tile, poly = self.nav.poly(poly_ref)
        if poly is None or not poly.is_offmesh:
            return None
        idx0, idx1 = 0, 1
        for link in tile.links[poly.index]:
            if link[1] == 0:
                if link[0] != prev_ref:
                    idx0, idx1 = 1, 0
                break
        return (tile.data.vertices[poly.verts[idx0]],
                tile.data.vertices[poly.verts[idx1]])

    # ---- nearest poly / heights ----

    def find_nearest_poly(self, center, half_extents):
        return self.nav.find_nearest_poly(center, half_extents, self.filter)

    def closest_point_on_poly(self, ref, pos):
        return self.nav.closest_point_on_poly(ref, pos)

    def closest_point_on_poly_boundary(self, ref, pos):
        return self.nav.closest_point_on_poly_boundary(ref, pos)

    def get_poly_height(self, ref, pos):
        return self.nav.get_poly_height(ref, pos)

    # ---- A* ----

    def find_path(self, start_ref: int, end_ref: int,
                  start_pos, end_pos,
                  max_path: Optional[int] = None):
        """Port of dtNavMeshQuery::findPath. Returns (status, path_refs)."""
        if max_path is None:
            max_path = 1 << 30
        if not start_ref or not end_ref or max_path <= 0:
            return DT_FAILURE | DT_INVALID_PARAM, []
        if start_ref == end_ref:
            return DT_SUCCESS, [start_ref]

        pool = self._pool
        queue = self._queue
        pool.clear()
        queue.clear()

        start_node = pool.get_node(start_ref)
        if start_node is None:
            return DT_FAILURE | DT_OUT_OF_NODES, []
        start_node.pos = start_pos
        start_node.pidx = None
        start_node.cost = 0.0
        start_node.total = v_dist(start_pos, end_pos) * H_SCALE
        start_node.flags = DT_NODE_OPEN
        queue.push(start_node)

        last_best = start_node
        last_best_cost = start_node.total
        out_of_nodes = False

        while not queue.empty():
            best = queue.pop()
            best.flags &= ~DT_NODE_OPEN
            best.flags |= DT_NODE_CLOSED

            if best.id == end_ref:
                last_best = best
                break

            best_ref = best.id
            best_tile, best_poly = self.nav.poly(best_ref)
            if best_poly is None:
                continue
            parent_ref = best.pidx.id if best.pidx is not None else 0
            parent_tile, parent_poly = (None, None)
            if parent_ref:
                parent_tile, parent_poly = self.nav.poly(parent_ref)

            for link in list(self._links_for(best_ref)):
                neighbour_ref = link[0]
                if not neighbour_ref or neighbour_ref == parent_ref:
                    continue
                ntile, npoly = self.nav.poly(neighbour_ref)
                if npoly is None:
                    continue
                if not self.filter.pass_filter(npoly):
                    continue
                cross_side = (link[2] >> 1) if link[2] != 0xFF else 0
                neighbour = pool.get_node(neighbour_ref, cross_side)
                if neighbour is None:
                    out_of_nodes = True
                    continue
                if neighbour.flags == 0:
                    mid = self.get_edge_mid_point(best_ref, neighbour_ref)
                    if mid is not None:
                        neighbour.pos = mid

                if neighbour_ref == end_ref:
                    cur_cost = self.filter.get_cost(best.pos, neighbour.pos, best_poly)
                    end_cost = self.filter.get_cost(neighbour.pos, end_pos, npoly)
                    cost = best.cost + cur_cost + end_cost
                    heuristic = 0.0
                else:
                    cur_cost = self.filter.get_cost(best.pos, neighbour.pos, best_poly)
                    cost = best.cost + cur_cost
                    heuristic = v_dist(neighbour.pos, end_pos) * H_SCALE

                total = cost + heuristic
                if (neighbour.flags & DT_NODE_OPEN) and total >= neighbour.total:
                    continue
                if (neighbour.flags & DT_NODE_CLOSED) and total >= neighbour.total:
                    continue

                neighbour.pidx = best
                neighbour.id = neighbour_ref
                neighbour.flags &= ~DT_NODE_CLOSED
                neighbour.cost = cost
                neighbour.total = total
                if neighbour.flags & DT_NODE_OPEN:
                    queue.modify(neighbour)
                else:
                    neighbour.flags |= DT_NODE_OPEN
                    queue.push(neighbour)

                if heuristic < last_best_cost:
                    last_best_cost = heuristic
                    last_best = neighbour

        status, path = self._path_to_node(last_best, max_path)
        if last_best.id != end_ref:
            status |= DT_PARTIAL_RESULT
        if out_of_nodes:
            status |= DT_OUT_OF_NODES
        return status, path

    def _path_to_node(self, end_node: Node, max_path: int):
        length = 0
        cur = end_node
        while cur is not None:
            length += 1
            cur = cur.pidx
        cur = end_node
        write_count = length
        while write_count > max_path and cur is not None:
            cur = cur.pidx
            write_count -= 1
        path = [0] * write_count
        for i in range(write_count - 1, -1, -1):
            path[i] = cur.id
            cur = cur.pidx
        if length > max_path:
            return DT_SUCCESS | DT_BUFFER_TOO_SMALL, path
        return DT_SUCCESS, path

    # ---- straight path (funnel) ----

    def find_straight_path(self, start_pos, end_pos, path_refs,
                           max_points: Optional[int] = None, options: int = 0):
        """Port of dtNavMeshQuery::findStraightPath.

        Returns (status, points, flags, refs).
        """
        if max_points is None:
            max_points = 1 << 30
        if not path_refs or max_points <= 0:
            return DT_FAILURE | DT_INVALID_PARAM, [], [], []

        points: List[Tuple[float, float, float]] = []
        point_flags: List[int] = []
        point_refs: List[int] = []

        def append_vertex(pos, flags, ref):
            if points and v_equal(points[-1], pos):
                point_flags[-1] = flags
                point_refs[-1] = ref
                return DT_IN_PROGRESS
            points.append(pos)
            point_flags.append(flags)
            point_refs.append(ref)
            if len(points) >= max_points:
                return DT_SUCCESS | DT_BUFFER_TOO_SMALL
            if flags == DT_STRAIGHTPATH_END:
                return DT_SUCCESS
            return DT_IN_PROGRESS

        def append_portals(start_idx, end_idx, end_pos_inner):
            start_pos_inner = points[-1]
            stat = DT_IN_PROGRESS
            for i in range(start_idx, end_idx):
                from_ref = path_refs[i]
                to_ref = path_refs[i + 1]
                portal = self.get_portal_points(from_ref, to_ref)
                if portal is None:
                    break
                left, right = portal
                if options & DT_STRAIGHTPATH_AREA_CROSSINGS:
                    _t, fp = self.nav.poly(from_ref)
                    _t2, tp = self.nav.poly(to_ref)
                    if fp is not None and tp is not None and fp.area == tp.area:
                        continue
                s, t = _intersect_seg_seg_2d(start_pos_inner, end_pos_inner,
                                             left, right)
                if s is not None:
                    pt = v_lerp(left, right, t)
                    stat = append_vertex(pt, 0, path_refs[i + 1])
                    if stat != DT_IN_PROGRESS:
                        return stat
            return stat

        closest_start = self.closest_point_on_poly_boundary(path_refs[0], start_pos)
        if closest_start is None:
            return DT_FAILURE | DT_INVALID_PARAM, [], [], []
        closest_end = self.closest_point_on_poly_boundary(path_refs[-1], end_pos)
        if closest_end is None:
            return DT_FAILURE | DT_INVALID_PARAM, [], [], []

        stat = append_vertex(closest_start, DT_STRAIGHTPATH_START, path_refs[0])
        if stat != DT_IN_PROGRESS:
            return stat, points, point_flags, point_refs

        if len(path_refs) > 1:
            portal_apex = closest_start
            portal_left = portal_apex
            portal_right = portal_apex
            apex_index = 0
            left_index = 0
            right_index = 0
            left_poly_type = 0
            right_poly_type = 0
            left_poly_ref = path_refs[0]
            right_poly_ref = path_refs[0]

            i = 0
            while i < len(path_refs):
                left = right = None
                to_type = DT_POLYTYPE_GROUND
                if i + 1 < len(path_refs):
                    portal = self.get_portal_points(path_refs[i], path_refs[i + 1])
                    if portal is None:
                        closest_end = self.closest_point_on_poly_boundary(
                            path_refs[i], end_pos)
                        if closest_end is None:
                            return DT_FAILURE | DT_INVALID_PARAM, [], [], []
                        if options & (DT_STRAIGHTPATH_AREA_CROSSINGS |
                                      DT_STRAIGHTPATH_ALL_CROSSINGS):
                            append_portals(apex_index, i, closest_end)
                        append_vertex(closest_end, 0, path_refs[i])
                        return (DT_SUCCESS | DT_PARTIAL_RESULT |
                                (DT_BUFFER_TOO_SMALL if len(points) >= max_points else 0),
                                points, point_flags, point_refs)
                    left, right = portal
                    _to_tile, to_poly = self.nav.poly(path_refs[i + 1])
                    to_type = to_poly.ptype if to_poly is not None else DT_POLYTYPE_GROUND
                    if i == 0:
                        d, _t = distance_pt_seg_sqr_2d(portal_apex, left, right)
                        if d < 0.001 ** 2:
                            i += 1
                            continue
                else:
                    left = closest_end
                    right = closest_end
                    to_type = DT_POLYTYPE_GROUND

                if tri_area2d(portal_apex, portal_right, right) <= 0.0:
                    if v_equal(portal_apex, portal_right) or \
                            tri_area2d(portal_apex, portal_left, right) > 0.0:
                        portal_right = right
                        right_poly_ref = path_refs[i + 1] if i + 1 < len(path_refs) else 0
                        right_poly_type = to_type
                        right_index = i
                    else:
                        if options & (DT_STRAIGHTPATH_AREA_CROSSINGS |
                                      DT_STRAIGHTPATH_ALL_CROSSINGS):
                            stat = append_portals(apex_index, left_index, portal_left)
                            if stat != DT_IN_PROGRESS:
                                return stat, points, point_flags, point_refs
                        portal_apex = portal_left
                        apex_index = left_index
                        flags = 0
                        if not left_poly_ref:
                            flags = DT_STRAIGHTPATH_END
                        elif left_poly_type == DT_POLYTYPE_OFFMESH_CONNECTION:
                            flags = DT_STRAIGHTPATH_OFFMESH_CONNECTION
                        stat = append_vertex(portal_apex, flags, left_poly_ref)
                        if stat != DT_IN_PROGRESS:
                            return stat, points, point_flags, point_refs
                        portal_left = portal_apex
                        portal_right = portal_apex
                        left_index = apex_index
                        right_index = apex_index
                        i = apex_index
                        i += 1
                        continue

                if tri_area2d(portal_apex, portal_left, left) >= 0.0:
                    if v_equal(portal_apex, portal_left) or \
                            tri_area2d(portal_apex, portal_right, left) < 0.0:
                        portal_left = left
                        left_poly_ref = path_refs[i + 1] if i + 1 < len(path_refs) else 0
                        left_poly_type = to_type
                        left_index = i
                    else:
                        if options & (DT_STRAIGHTPATH_AREA_CROSSINGS |
                                      DT_STRAIGHTPATH_ALL_CROSSINGS):
                            stat = append_portals(apex_index, right_index, portal_right)
                            if stat != DT_IN_PROGRESS:
                                return stat, points, point_flags, point_refs
                        portal_apex = portal_right
                        apex_index = right_index
                        flags = 0
                        if not right_poly_ref:
                            flags = DT_STRAIGHTPATH_END
                        elif right_poly_type == DT_POLYTYPE_OFFMESH_CONNECTION:
                            flags = DT_STRAIGHTPATH_OFFMESH_CONNECTION
                        stat = append_vertex(portal_apex, flags, right_poly_ref)
                        if stat != DT_IN_PROGRESS:
                            return stat, points, point_flags, point_refs
                        portal_left = portal_apex
                        portal_right = portal_apex
                        left_index = apex_index
                        right_index = apex_index
                        i = apex_index
                        i += 1
                        continue
                i += 1

            if options & (DT_STRAIGHTPATH_AREA_CROSSINGS |
                          DT_STRAIGHTPATH_ALL_CROSSINGS):
                stat = append_portals(apex_index, len(path_refs) - 1, closest_end)
                if stat != DT_IN_PROGRESS:
                    return stat, points, point_flags, point_refs

        append_vertex(closest_end, DT_STRAIGHTPATH_END, 0)
        status = DT_SUCCESS
        if len(points) >= max_points:
            status |= DT_BUFFER_TOO_SMALL
        return status, points, point_flags, point_refs

    # ---- surface movement ----

    def move_along_surface(self, start_ref: int, start_pos, end_pos,
                           max_visited: int = DEFAULT_MAX_VISITED):
        """Port of dtNavMeshQuery::moveAlongSurface.

        Returns (status, result_pos, visited_refs).
        """
        if max_visited <= 0:
            return DT_FAILURE | DT_INVALID_PARAM, start_pos, []
        visited_nodes: Dict[int, Node] = {}
        stack: List[Node] = []
        MAX_STACK = 48

        start_node = Node()
        start_node.id = start_ref
        start_node.flags = DT_NODE_CLOSED
        start_node.pidx = None
        visited_nodes[start_ref] = start_node
        stack.append(start_node)

        best_pos = start_pos
        best_dist = math.inf
        best_node: Optional[Node] = None

        search_pos = v_lerp(start_pos, end_pos, 0.5)
        search_rad_sqr = (v_dist(start_pos, end_pos) / 2.0 + 0.001) ** 2

        while stack:
            cur_node = stack.pop(0)
            cur_ref = cur_node.id
            tile, poly, verts = self.nav.poly_vertices(cur_ref)
            if poly is None:
                continue

            if point_in_polygon(end_pos, verts):
                best_node = cur_node
                best_pos = end_pos
                break

            nverts = poly.vert_count
            j = nverts - 1
            for i in range(nverts):
                neis: List[int] = []
                if poly.neis[j] & DT_EXT_LINK:
                    for link in tile.links[poly.index]:
                        if link[1] == j and link[0]:
                            _nt, n_poly = self.nav.poly(link[0])
                            if n_poly is not None and self.filter.pass_filter(n_poly):
                                if len(neis) < 8:
                                    neis.append(link[0])
                elif poly.neis[j]:
                    idx = poly.neis[j] - 1
                    if idx < len(tile.data.polygons):
                        n_poly = tile.data.polygons[idx]
                        if self.filter.pass_filter(n_poly):
                            neis.append(make_ref(tile.tx, tile.ty, idx))

                if not neis:
                    vj = verts[j]
                    vi = verts[i]
                    dist_sqr, tseg = distance_pt_seg_sqr_2d(end_pos, vj, vi)
                    if dist_sqr < best_dist:
                        best_pos = v_lerp(vj, vi, tseg)
                        best_dist = dist_sqr
                        best_node = cur_node
                else:
                    for nref in neis:
                        nb = visited_nodes.get(nref)
                        if nb is None:
                            nb = Node()
                            nb.id = nref
                            visited_nodes[nref] = nb
                        if nb.flags & DT_NODE_CLOSED:
                            continue
                        vj = verts[j]
                        vi = verts[i]
                        dist_sqr, _tseg = distance_pt_seg_sqr_2d(search_pos, vj, vi)
                        if dist_sqr > search_rad_sqr:
                            continue
                        if len(stack) < MAX_STACK:
                            nb.pidx = cur_node
                            nb.flags |= DT_NODE_CLOSED
                            stack.append(nb)
                j = i

        status = DT_SUCCESS
        visited: List[int] = []
        if best_node is not None:
            prev = None
            node = best_node
            while node is not None:
                nxt = node.pidx
                node.pidx = prev
                prev = node
                node = nxt
            node = prev
            while node is not None:
                visited.append(node.id)
                if len(visited) >= max_visited:
                    status |= DT_BUFFER_TOO_SMALL
                    break
                node = node.pidx
        return status, best_pos, visited

    # ---- raycast ----

    def raycast(self, start_ref: int, start_pos, end_pos,
                max_path: Optional[int] = None):
        """Port of dtNavMeshQuery::raycast.

        Returns (status, t, hit_normal, path_refs).
        """
        if max_path is None:
            max_path = 1 << 30
        t = 0.0
        path: List[int] = []
        hit_normal = (0.0, 0.0, 0.0)

        cur_ref = start_ref
        tile, poly = self.nav.poly(cur_ref)
        if poly is None:
            return DT_FAILURE | DT_INVALID_PARAM, 0.0, hit_normal, []
        cur_pos = start_pos
        direction = (end_pos[0] - start_pos[0],
                     end_pos[1] - start_pos[1],
                     end_pos[2] - start_pos[2])
        status = DT_SUCCESS

        while cur_ref:
            verts = [tile.data.vertices[v] for v in poly.verts]
            ok, tmin, tmax, seg_min, seg_max = intersect_segment_poly_2d(
                start_pos, end_pos, verts)
            if not ok:
                return status, t, hit_normal, path

            if tmax > t:
                t = tmax
            if len(path) < max_path:
                path.append(cur_ref)
            else:
                status |= DT_BUFFER_TOO_SMALL

            if seg_max == -1:
                return status, 3.4028234663852886e38, hit_normal, path

            next_ref = 0
            for link in tile.links[poly.index]:
                if link[1] != seg_max:
                    continue
                _nt, n_poly = self.nav.poly(link[0])
                if n_poly is None or n_poly.is_offmesh:
                    continue
                if not self.filter.pass_filter(n_poly):
                    continue
                if link[2] == 0xFF:
                    next_ref = link[0]
                    break
                if link[3] == 0 and link[4] == 255:
                    next_ref = link[0]
                    break
                side = link[2]
                v0 = poly.verts[link[1]]
                v1 = poly.verts[(link[1] + 1) % poly.vert_count]
                left = tile.data.vertices[v0]
                right = tile.data.vertices[v1]
                if side in (0, 4):
                    s = 1.0 / 255.0
                    lmin = left[2] + (right[2] - left[2]) * (link[3] * s)
                    lmax = left[2] + (right[2] - left[2]) * (link[4] * s)
                    if lmin > lmax:
                        lmin, lmax = lmax, lmin
                    z = start_pos[2] + (end_pos[2] - start_pos[2]) * tmax
                    if lmin <= z <= lmax:
                        next_ref = link[0]
                        break
                elif side in (2, 6):
                    s = 1.0 / 255.0
                    lmin = left[0] + (right[0] - left[0]) * (link[3] * s)
                    lmax = left[0] + (right[0] - left[0]) * (link[4] * s)
                    if lmin > lmax:
                        lmin, lmax = lmax, lmin
                    x = start_pos[0] + (end_pos[0] - start_pos[0]) * tmax
                    if lmin <= x <= lmax:
                        next_ref = link[0]
                        break

            if not next_ref:
                a = seg_max
                b = seg_max + 1 if seg_max + 1 < len(verts) else 0
                va = verts[a]
                vb = verts[b]
                dx = vb[0] - va[0]
                dz = vb[2] - va[2]
                n = math.sqrt(dz * dz + dx * dx)
                if n > 0:
                    hit_normal = (dz / n, 0.0, -dx / n)
                return status, t, hit_normal, path

            cur_ref = next_ref
            tile, poly = self.nav.poly(cur_ref)

        return status, t, hit_normal, path


def _perp2d(u, v) -> float:
    return u[2] * v[0] - u[0] * v[2]


def _intersect_seg_seg_2d(ap, aq, bp, bq):
    """Port of dtIntersectSegSeg2D. Returns (s, t) or (None, None)."""
    u = (aq[0] - ap[0], aq[1] - ap[1], aq[2] - ap[2])
    v = (bq[0] - bp[0], bq[1] - bp[1], bq[2] - bp[2])
    w = (ap[0] - bp[0], ap[1] - bp[1], ap[2] - bp[2])
    d = _perp2d(u, v)
    if abs(d) < 1e-6:
        return None, None
    s = _perp2d(v, w) / d
    if s < 0.0 or s > 1.0:
        return None, None
    t = _perp2d(u, w) / d
    if t < 0.0 or t > 1.0:
        return None, None
    return s, t
