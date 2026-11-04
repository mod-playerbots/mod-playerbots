"""Pure-function tests for the quest POI Z baker (no database or mmaps)."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

TOOL_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOL_DIR))

from quest_poi_z_baker import bake_point, cluster_levels, dist2d, spawns_in_radius  # noqa: E402


class ClusterLevelsTests(unittest.TestCase):
    def test_empty(self) -> None:
        self.assertEqual(cluster_levels([], 8.0), [])

    def test_splits_at_gap(self) -> None:
        clusters = cluster_levels([(1.0, "a"), (2.0, "b"), (20.0, "c")], 8.0)
        self.assertEqual(len(clusters), 2)
        self.assertAlmostEqual(clusters[0]["zmean"], 1.5)
        self.assertEqual(clusters[0]["items"], ["a", "b"])
        self.assertAlmostEqual(clusters[1]["zmean"], 20.0)

    def test_keeps_single_cluster(self) -> None:
        clusters = cluster_levels([(1.0, "a"), (4.0, "b"), (7.0, "c")], 8.0)
        self.assertEqual(len(clusters), 1)
        self.assertAlmostEqual(clusters[0]["zmean"], 4.0)


class SpawnRadiusTests(unittest.TestCase):
    def test_filters_by_radius(self) -> None:
        spawns = [(0.0, 0.0, 10.0, 1), (5.0, 0.0, 11.0, 2), (20.0, 0.0, 12.0, 3)]
        xs = [s[0] for s in spawns]
        got = spawns_in_radius(xs, spawns, 1.0, 0.0, 6.0)
        self.assertEqual([s[3] for s in got], [1, 2])

    def test_empty(self) -> None:
        self.assertEqual(spawns_in_radius([], [], 0.0, 0.0, 10.0), [])

    def test_dist2d(self) -> None:
        self.assertAlmostEqual(dist2d(0.0, 0.0, 3.0, 4.0), 5.0)


class BakePointTests(unittest.TestCase):
    def test_empty(self) -> None:
        self.assertEqual(bake_point(0.0, 0.0, [], [], [], [], []), [])

    def test_ranks_trigger_and_quest_spawn_first(self) -> None:
        near_polys = [
            (0.0, 0.0, 10.0, []),
            (1.0, 0.0, 10.5, []),
            (0.0, 0.0, 30.0, []),
        ]
        candidates = bake_point(
            0.0, 0.0, near_polys, truth_zs=[10.0], trigger_zs=[10.0],
            cre_near=[(0.0, 0.0, 10.0, 123)], go_near=[],
        )
        self.assertTrue(candidates)
        best = candidates[0]
        self.assertGreaterEqual(best["confidence"], 90)
        self.assertTrue(best["quest_spawn"])
        self.assertTrue(best["trigger"])
        self.assertTrue(best["nav_ok"])
        self.assertIn("quest-spawn", best["evidence"])
        self.assertIn("trigger", best["evidence"])
        self.assertIn("navmesh=", best["evidence"])
        self.assertTrue(best["walkable"])
        self.assertLessEqual(len(best["walkable"]), 6)
        self.assertLessEqual(len(candidates), 4)

    def test_navmesh_only_is_lower_confidence(self) -> None:
        candidates = bake_point(
            0.0, 0.0, [(0.0, 0.0, 42.0, [])], truth_zs=[], trigger_zs=[],
            cre_near=[], go_near=[],
        )
        self.assertEqual(len(candidates), 1)
        self.assertTrue(candidates[0]["nav_ok"])
        self.assertGreaterEqual(candidates[0]["confidence"], 45)
        self.assertLess(candidates[0]["confidence"], 90)
        self.assertIn("navmesh=1", candidates[0]["evidence"])


if __name__ == "__main__":
    unittest.main()
