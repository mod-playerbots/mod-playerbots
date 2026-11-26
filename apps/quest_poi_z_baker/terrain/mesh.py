"""Navmesh polygon collection for the quest POI Z baker.

Extracted from acore-data's ``core/terrain/area_map.py`` (same project, open
source) so the baker is self-contained. Only the mesh query is kept; the SVG
renderer that lives beside it is not needed for baking.
"""

from typing import Any, Dict, List, NamedTuple, Optional, Tuple

DEFAULT_MAX_POLYS = 40000        # polys before refusing (suggest outline)
DEFAULT_Z_BAND = 25.0            # yards around `z` for multi-floor filtering


class AreaMapError(ValueError):
    """Raised for caller-fixable area_map problems (bounds, caps)."""


class Bounds(NamedTuple):
    minx: float
    miny: float
    maxx: float
    maxy: float

    @property
    def span_x(self) -> float:
        return self.maxx - self.minx

    @property
    def span_y(self) -> float:
        return self.maxy - self.miny


def collect_mesh(nav: Any, bounds: Bounds,
                 max_polys: int = DEFAULT_MAX_POLYS,
                 z: Optional[float] = None,
                 z_band: float = DEFAULT_Z_BAND) -> Dict[str, Any]:
    """Gather navmesh polygons intersecting the bounds (world XY).

    Detour vertices are (worldY, worldZ, worldX); they are remapped to world
    (x, y, z) here so callers work purely in world axes. When `z` is given,
    only polys whose average height falls within z +- z_band are kept
    (instances stack floors in XY).

    Only the mmap tiles intersecting the bounds are loaded, using the navmesh's
    own detour-space tile mapping (tile coords are rebased per map - they are
    NOT the filename grid); loading every tile in the map costs seconds per
    call on maps with hundreds of tiles, which is fatal for bulk bakers.
    """
    if nav.params is None:
        raise AreaMapError("no navmesh data for this map")

    # Detour-space point for world (x, y) is (y, 0, x): tx tracks worldY, ty worldX.
    pa = nav.calc_tile_loc((bounds.miny, 0.0, bounds.minx))
    pb = nav.calc_tile_loc((bounds.maxy, 0.0, bounds.maxx))
    tx_lo, tx_hi = sorted((pa[0], pb[0]))
    ty_lo, ty_hi = sorted((pa[1], pb[1]))

    polys: List[Tuple[List[Tuple[float, float, float]], float]] = []
    offmesh: List[Tuple[Tuple[float, float], Tuple[float, float]]] = []
    zmin: Optional[float] = None
    zmax: Optional[float] = None
    tile_count = 0
    poly_total = 0

    for tx in range(tx_lo, tx_hi + 1):
        for ty in range(ty_lo, ty_hi + 1):
            tile = nav.ensure_tile(tx, ty)
            if tile is None:
                continue
            tile_count += 1
            verts = tile.data.vertices
            for poly in tile.data.polygons:
                poly_total += 1
                if poly.is_offmesh:
                    continue
                pv = [verts[i] for i in poly.verts if i < len(verts)]
                if len(pv) < 3:
                    continue
                wpts = [(v[2], v[0], v[1]) for v in pv]
                xs = [p[0] for p in wpts]
                ys = [p[1] for p in wpts]
                if (max(xs) < bounds.minx or min(xs) > bounds.maxx or
                        max(ys) < bounds.miny or min(ys) > bounds.maxy):
                    continue
                avgz = sum(p[2] for p in wpts) / len(wpts)
                if z is not None and abs(avgz - z) > z_band:
                    continue
                polys.append((wpts, avgz))
                zmin = avgz if zmin is None else min(zmin, avgz)
                zmax = avgz if zmax is None else max(zmax, avgz)
            for con in tile.data.off_mesh_cons:
                s, e = con.start, con.end
                spt = (s[2], s[0])
                ept = (e[2], e[0])
                if (max(spt[0], ept[0]) < bounds.minx or
                        min(spt[0], ept[0]) > bounds.maxx or
                        max(spt[1], ept[1]) < bounds.miny or
                        min(spt[1], ept[1]) > bounds.maxy):
                    continue
                if z is not None:
                    con_z = (s[1] + e[1]) / 2.0
                    if abs(con_z - z) > z_band:
                        continue
                offmesh.append((spt, ept))

    if len(polys) > max_polys:
        raise AreaMapError(
            f"area_map: {len(polys)} navmesh polys exceed max_polys="
            f"{max_polys}; shrink the bounds or pass mesh_style='outline'"
        )
    return {
        "polys": polys,
        "offmesh": offmesh,
        "zmin": zmin if zmin is not None else 0.0,
        "zmax": zmax if zmax is not None else 0.0,
        "tile_count": tile_count,
        "poly_total": poly_total,
    }
