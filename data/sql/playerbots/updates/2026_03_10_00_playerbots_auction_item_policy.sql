-- Playerbots auction item policy table for DB-driven AH rules.

DROP TABLE IF EXISTS `playerbots_auction_item_policy`;
CREATE TABLE IF NOT EXISTS `playerbots_auction_item_policy` (
    `item_id` int unsigned NOT NULL,
    `sellable` tinyint(1) unsigned NOT NULL DEFAULT 1,
    `chance_to_sell` tinyint unsigned NOT NULL DEFAULT 100,
    `min_stack_count` smallint unsigned NOT NULL DEFAULT 0,
    `max_stack_count` smallint unsigned NOT NULL DEFAULT 0,
    `min_bid_pct` smallint unsigned NOT NULL DEFAULT 100,
    `buyout_min_pct` smallint unsigned NOT NULL DEFAULT 110,
    `buyout_max_pct` smallint unsigned NOT NULL DEFAULT 133,
    `undercut_chance` tinyint unsigned NOT NULL DEFAULT 15,
    `market_price_weight_pct` smallint unsigned NOT NULL DEFAULT 75,
    PRIMARY KEY (`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
