-- Playerbots lazy auction-house price cache.
-- One row per (item, resolved auction-house bucket: 2=Alliance, 6=Horde, 7=Neutral).

DROP TABLE IF EXISTS `playerbots_auction_price_cache`;
CREATE TABLE IF NOT EXISTS `playerbots_auction_price_cache` (
    `item_entry`      int unsigned NOT NULL,
    `ah_faction`      tinyint unsigned NOT NULL,
    `min_unit_buyout` int unsigned NOT NULL DEFAULT 0,
    `avg_unit_buyout` int unsigned NOT NULL DEFAULT 0,
    `sample_count`    smallint unsigned NOT NULL DEFAULT 0,
    `updated_at`      int unsigned NOT NULL,
    PRIMARY KEY (`item_entry`, `ah_faction`),
    KEY `idx_updated_at` (`updated_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
