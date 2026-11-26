"""Terrain reader vendored from acore-data for the quest POI Z baker.

The modules under this package mirror acore-data's ``core/terrain`` readers
(same project, open source) with imports adjusted for this self-contained
tool. See ``README.md`` for the source commit and attribution.
"""

from .coords import get_data_paths
from .detour import NavMesh
from .mesh import Bounds, AreaMapError, collect_mesh

__all__ = [
    "AreaMapError",
    "Bounds",
    "NavMesh",
    "collect_mesh",
    "get_data_paths",
]
