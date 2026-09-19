-- Playerbots auction-house price cache with rolling market history.
-- One row per (item, ah_faction). `samples` is a hex-encoded rolling buffer
-- of up to 100 per-query market snapshots (16 bytes each, chronological).

DROP TABLE IF EXISTS `playerbots_auction_price_cache`;
CREATE TABLE IF NOT EXISTS `playerbots_auction_price_cache` (
    `item_entry` int unsigned NOT NULL,
    `ah_faction` tinyint unsigned NOT NULL,
    `updated_at` int unsigned NOT NULL,
    `samples`    varchar(3200) NOT NULL DEFAULT '',
    PRIMARY KEY (`item_entry`, `ah_faction`),
    KEY `idx_updated_at` (`updated_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
