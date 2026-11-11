"""Detour query filters, including AzerothCore's slope-aware dtQueryFilterExt."""

from __future__ import annotations

import math
from typing import Tuple

MAX_AREAS = 64

NAV_EMPTY = 0x00
NAV_GROUND = 0x01
NAV_MAGMA = 0x02
NAV_SLIME = 0x04
NAV_WATER = 0x08
NAV_GROUND_STEEP = 0x10


def get_slope_angle(start_x: float, start_y: float, start_z: float,
                    dest_x: float, dest_y: float, dest_z: float) -> float:
    """Port of AC Geometry.h getSlopeAngle."""
    floor_dist = math.sqrt((start_y - dest_y) ** 2 + (start_x - dest_x) ** 2)
    return math.atan2(abs(dest_z - start_z), abs(floor_dist))


class DetourFilter:
    """dtQueryFilter + dtQueryFilterExt (AzerothCore)."""

    __slots__ = ("include_flags", "exclude_flags", "area_cost")

    def __init__(self, include_flags: int = 0, exclude_flags: int = 0):
        self.include_flags = include_flags
        self.exclude_flags = exclude_flags
        self.area_cost = [1.0] * MAX_AREAS

    def set_include_flags(self, flags: int) -> None:
        self.include_flags = flags

    def set_exclude_flags(self, flags: int) -> None:
        self.exclude_flags = flags

    def set_area_cost(self, area: int, cost: float) -> None:
        if 0 <= area < MAX_AREAS:
            self.area_cost[area] = cost

    def pass_filter(self, poly) -> bool:
        return ((poly.flags & self.include_flags) != 0
                and (poly.flags & self.exclude_flags) == 0)

    def get_cost(self, pa: Tuple[float, float, float],
                 pb: Tuple[float, float, float], cur_poly) -> float:
        """dtQueryFilterExt::getCost (pa/pb are Detour-space positions)."""
        start_x, start_y, start_z = pa[2], pa[0], pa[1]
        dest_x, dest_y, dest_z = pb[2], pb[0], pb[1]
        slope_angle = get_slope_angle(start_x, start_y, start_z,
                                      dest_x, dest_y, dest_z)
        slope_deg = slope_angle * 180.0 / math.pi
        cost = 1.0 + (1.0 * (slope_deg / 100.0)) if slope_deg > 0 else 1.0
        dx, dy, dz = pb[0] - pa[0], pb[1] - pa[1], pb[2] - pa[2]
        dist = math.sqrt(dx * dx + dy * dy + dz * dz)
        area = cur_poly.area & 0x3F
        return dist * cost * self.area_cost[area]


def player_filter(headless: bool = False) -> DetourFilter:
    """PathGenerator::CreateFilter for a (non-)headless player."""
    f = DetourFilter()
    if headless:
        f.set_include_flags(NAV_GROUND | NAV_WATER)
        f.set_exclude_flags(NAV_MAGMA | NAV_SLIME | NAV_GROUND_STEEP)
        f.set_area_cost(NAV_WATER, 20.0)
    else:
        f.set_include_flags(NAV_GROUND | NAV_GROUND_STEEP | NAV_WATER | NAV_MAGMA)
    return f


def creature_filter(can_walk: bool = True, can_swim: bool = True) -> DetourFilter:
    """PathGenerator::CreateFilter for a creature."""
    include = 0
    if can_walk:
        include |= NAV_GROUND | NAV_GROUND_STEEP
    if can_swim:
        include |= NAV_WATER | NAV_MAGMA
    return DetourFilter(include_flags=include)
