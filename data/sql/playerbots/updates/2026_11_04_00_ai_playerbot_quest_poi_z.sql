-- Optional baked walkable Z candidates for quest POIs. The table is populated offline by
-- acore-data/scripts/quest_poi_z_baker.py; its generated rows are not shipped with the module.
CREATE TABLE IF NOT EXISTS `playerbots_quest_poi_z` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `QuestId` INT UNSIGNED NOT NULL,
  `PoiIdx` SMALLINT UNSIGNED NOT NULL,
  `PointIdx` SMALLINT UNSIGNED NOT NULL,
  `MapId` SMALLINT UNSIGNED NOT NULL,
  `Z` FLOAT NOT NULL,
  `Rank` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `Confidence` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `Evidence` VARCHAR(255) NOT NULL DEFAULT '',
  `NavmeshOk` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `WalkablePoints` TEXT NULL,
  PRIMARY KEY (`Id`), KEY `idx_quest` (`QuestId`, `PoiIdx`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
