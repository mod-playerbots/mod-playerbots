-- #########################################################
-- Playerbots - Add calc command per-stat breakdown text
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts
WHERE name = 'calc_item_breakdown';

DELETE FROM ai_playerbot_texts_chance
WHERE name = 'calc_item_breakdown';

INSERT INTO `ai_playerbot_texts`
(`id`, `name`, `text`, `say_type`, `reply_type`,
 `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
 `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
2014,
'calc_item_breakdown',
'Breakdown: %breakdown (weighted sum %sum)',
0, 0,
'분석: %breakdown (가중 합계 %sum)',
'Détail : %breakdown (somme pondérée %sum)',
'Aufschlüsselung: %breakdown (gewichtete Summe %sum)',
'明细：%breakdown（加权总和 %sum）',
'明細：%breakdown（加權總和 %sum）',
'Desglose: %breakdown (suma ponderada %sum)',
'Desglose: %breakdown (suma ponderada %sum)',
'Разбивка: %breakdown (взвешенная сумма %sum)'
);

INSERT INTO ai_playerbot_texts_chance (name, probability)
VALUES ('calc_item_breakdown', 100);
