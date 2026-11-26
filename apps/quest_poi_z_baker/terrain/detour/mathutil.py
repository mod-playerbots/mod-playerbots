"""Detour math helpers (ports of DetourCommon)."""

from __future__ import annotations

import math
from typing import List, Tuple

Vec3 = Tuple[float, float, float]

EPS_SEG_POLY = 1e-8


def v_dist(a: Vec3, b: Vec3) -> float:
    dx, dy, dz = a[0] - b[0], a[1] - b[1], a[2] - b[2]
    return math.sqrt(dx * dx + dy * dy + dz * dz)


def v_dist_sqr(a: Vec3, b: Vec3) -> float:
    dx, dy, dz = a[0] - b[0], a[1] - b[1], a[2] - b[2]
    return dx * dx + dy * dy + dz * dz


def v_equal(a: Vec3, b: Vec3) -> bool:
    thr = (1.0 / 16384.0) ** 2
    return v_dist_sqr(a, b) < thr


def v_lerp(a: Vec3, b: Vec3, t: float) -> Vec3:
    return (a[0] + (b[0] - a[0]) * t,
            a[1] + (b[1] - a[1]) * t,
            a[2] + (b[2] - a[2]) * t)


def v_mad(a: Vec3, b: Vec3, s: float) -> Vec3:
    return (a[0] + b[0] * s, a[1] + b[1] * s, a[2] + b[2] * s)


def perp2d(u: Vec3, v: Vec3) -> float:
    return u[2] * v[0] - u[0] * v[2]


def tri_area2d(a: Vec3, b: Vec3, c: Vec3) -> float:
    abx = b[0] - a[0]
    abz = b[2] - a[2]
    acx = c[0] - a[0]
    acz = c[2] - a[2]
    return acx * abz - abx * acz


def distance_pt_seg_sqr_2d(pt: Vec3, p: Vec3, q: Vec3) -> Tuple[float, float]:
    pqx = q[0] - p[0]
    pqz = q[2] - p[2]
    dx = pt[0] - p[0]
    dz = pt[2] - p[2]
    d = pqx * pqx + pqz * pqz
    t = pqx * dx + pqz * dz
    if d > 0:
        t /= d
    if t < 0:
        t = 0.0
    elif t > 1:
        t = 1.0
    dx = p[0] + t * pqx - pt[0]
    dz = p[2] + t * pqz - pt[2]
    return dx * dx + dz * dz, t


def point_in_polygon(pt: Vec3, verts: List[Vec3]) -> bool:
    c = False
    n = len(verts)
    j = n - 1
    for i in range(n):
        vi = verts[i]
        vj = verts[j]
        if ((vi[2] > pt[2]) != (vj[2] > pt[2])) and \
                (pt[0] < (vj[0] - vi[0]) * (pt[2] - vi[2]) / (vj[2] - vi[2]) + vi[0]):
            c = not c
        j = i
    return c


def distance_pt_poly_edges_sqr(pt: Vec3, verts: List[Vec3]):
    n = len(verts)
    ed = [0.0] * n
    et = [0.0] * n
    c = False
    j = n - 1
    for i in range(n):
        vi = verts[i]
        vj = verts[j]
        if ((vi[2] > pt[2]) != (vj[2] > pt[2])) and \
                (pt[0] < (vj[0] - vi[0]) * (pt[2] - vi[2]) / (vj[2] - vi[2]) + vi[0]):
            c = not c
        ed[j], et[j] = distance_pt_seg_sqr_2d(pt, vj, vi)
        j = i
    return c, ed, et


def closest_height_point_triangle(p: Vec3, a: Vec3, b: Vec3, c: Vec3):
    eps = 1e-6
    v0 = (c[0] - a[0], c[1] - a[1], c[2] - a[2])
    v1 = (b[0] - a[0], b[1] - a[1], b[2] - a[2])
    v2 = (p[0] - a[0], p[1] - a[1], p[2] - a[2])

    denom = v0[0] * v1[2] - v0[2] * v1[0]
    if abs(denom) < eps:
        return False, 0.0

    u = v1[2] * v2[0] - v1[0] * v2[2]
    v = v0[0] * v2[2] - v0[2] * v2[0]

    if denom < 0:
        denom = -denom
        u = -u
        v = -v

    if u >= 0.0 and v >= 0.0 and (u + v) <= denom:
        h = a[1] + (v0[1] * u + v1[1] * v) / denom
        return True, h
    return False, 0.0


def intersect_segment_poly_2d(p0: Vec3, p1: Vec3, verts: List[Vec3]):
    """Port of dtIntersectSegmentPoly2D.

    Returns (ok, tmin, tmax, seg_min, seg_max).
    """
    tmin = 0.0
    tmax = 1.0
    seg_min = -1
    seg_max = -1

    direction = (p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2])
    n = len(verts)
    j = n - 1
    for i in range(n):
        edge = (verts[i][0] - verts[j][0], verts[i][1] - verts[j][1], verts[i][2] - verts[j][2])
        diff = (p0[0] - verts[j][0], p0[1] - verts[j][1], p0[2] - verts[j][2])
        nn = perp2d(edge, diff)
        d = perp2d(direction, edge)
        if abs(d) < EPS_SEG_POLY:
            if nn < 0:
                return False, tmin, tmax, seg_min, seg_max
            j = i
            continue
        t = nn / d
        if d < 0:
            if t > tmin:
                tmin = t
                seg_min = j
                if tmin > tmax:
                    return False, tmin, tmax, seg_min, seg_max
        else:
            if t < tmax:
                tmax = t
                seg_max = j
                if tmax < tmin:
                    return False, tmin, tmax, seg_min, seg_max
        j = i

    return True, tmin, tmax, seg_min, seg_max


def point_in_poly_2d(px: float, py: float, pz: float, vs: List[Vec3]) -> bool:
    """Point-in-polygon on xz (used by pathfinder helpers)."""
    n = len(vs)
    inside = False
    j = n - 1
    for i in range(n):
        yi, zi = vs[i][1], vs[i][2]
        yj, zj = vs[j][1], vs[j][2]
        if ((zi > pz) != (zj > pz)) and (py < (yj - yi) * (pz - zi) / (zj - zi) + yi):
            inside = not inside
        j = i
    return inside


def dist_pt_seg_sqr(p: Vec3, a: Vec3, b: Vec3) -> float:
    dx, dy, dz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
    lsq = dx * dx + dy * dy + dz * dz
    if lsq < 1e-12:
        return sum((p[i] - a[i]) ** 2 for i in range(3))
    t = max(0.0, min(1.0, sum((p[i] - a[i]) * (b[i] - a[i]) for i in range(3)) / lsq))
    return sum((p[i] - (a[i] + t * (b[i] - a[i]))) ** 2 for i in range(3))
