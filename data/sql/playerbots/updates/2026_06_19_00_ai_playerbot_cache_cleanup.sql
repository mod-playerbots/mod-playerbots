-- Flush all caches after RandomItemMgr changes. Tables will be repopulated with
-- the correct entries on the next start.
TRUNCATE TABLE `playerbots_equip_cache`;
TRUNCATE TABLE `playerbots_rnditem_cache`;
TRUNCATE TABLE `playerbots_rarity_cache`;
