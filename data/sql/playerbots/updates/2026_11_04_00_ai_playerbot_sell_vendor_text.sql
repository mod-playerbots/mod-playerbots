-- #########################################################
-- Playerbots - Add the summary of the automatic vendor visit
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts
WHERE name = 'sell_vendor_summary';

DELETE FROM ai_playerbot_texts_chance
WHERE name = 'sell_vendor_summary';

INSERT INTO `ai_playerbot_texts`
(`id`, `name`, `text`, `say_type`, `reply_type`,
 `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
 `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
1946,
'sell_vendor_summary',
'Sold items: %count',
0, 0,
'판매한 아이템: %count',
'Objets vendus : %count',
'Verkaufte Gegenstände: %count',
'已出售物品：%count',
'已出售物品：%count',
'Objetos vendidos: %count',
'Objetos vendidos: %count',
'Продано предметов: %count'
);

INSERT INTO ai_playerbot_texts_chance (name, probability)
VALUES ('sell_vendor_summary', 100);
