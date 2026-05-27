-- #########################################################
-- Playerbots - Add PVP / Arena texts for TellPvpAction
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN ('pvp_currency', 'pvp_arena_team', 'pvp_no_arena_team');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('pvp_currency', 'pvp_arena_team', 'pvp_no_arena_team');

-- ---------------------------------------------------------
-- pvp_currency
-- [PVP] Arena points: %arena_points | Honor Points: %honor_points
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1737,
    'pvp_currency',
    '[PVP] Arena points: %arena_points | Honor Points: %honor_points',
    0, 0,
    -- koKR
    '[PVP] 투기장 점수: %arena_points | 명예 점수: %honor_points',
    -- frFR
    '[PVP] Points d''arène : %arena_points | Points d''honneur : %honor_points',
    -- deDE
    '[PVP] Arenapunkte: %arena_points | Ehrenpunkte: %honor_points',
    -- zhCN
    '[PVP] 竞技场点数：%arena_points | 荣誉点数：%honor_points',
    -- zhTW
    '[PVP] 競技場點數：%arena_points | 榮譽點數：%honor_points',
    -- esES
    '[PVP] Puntos de arena: %arena_points | Puntos de honor: %honor_points',
    -- esMX
    '[PVP] Puntos de arena: %arena_points | Puntos de honor: %honor_points',
    -- ruRU
    '[PVP] Очки арены: %arena_points | Очки чести: %honor_points');
    
INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pvp_currency',   100);

-- ---------------------------------------------------------
-- pvp_arena_team
-- [PVP] %bracket: <%team_name> (rating %team_rating)
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1738,
    'pvp_arena_team',
    '[PVP] %bracket: <%team_name> (rating %team_rating)',
    0, 0,
    -- koKR
    '[PVP] %bracket: <%team_name> (평점 %team_rating)',
    -- frFR
    '[PVP] %bracket : <%team_name> (cote %team_rating)',
    -- deDE
    '[PVP] %bracket: <%team_name> (Wertung %team_rating)',
    -- zhCN
    '[PVP] %bracket: <%team_name> (评分 %team_rating)',
    -- zhTW
    '[PVP] %bracket: <%team_name> (評分 %team_rating)',
    -- esES
    '[PVP] %bracket: <%team_name> (índice %team_rating)',
    -- esMX
    '[PVP] %bracket: <%team_name> (índice %team_rating)',
    -- ruRU
    '[PVP] %bracket: <%team_name> (рейтинг %team_rating)');
    
INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pvp_arena_team',   100);

-- ---------------------------------------------------------
-- pvp_no_arena_team
-- [PVP] I have no Arena Team.
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1739,
    'pvp_no_arena_team',
    '[PVP] I have no Arena Team.',
    0, 0,
    -- koKR
    '[PVP] 투기장 팀이 없습니다.',
    -- frFR
    '[PVP] Je n''ai aucune équipe d''arène.',
    -- deDE
    '[PVP] Ich habe kein Arenateam.',
    -- zhCN
    '[PVP] 我没有竞技场战队。',
    -- zhTW
    '[PVP] 我沒有競技場隊伍。',
    -- esES
    '[PVP] No tengo equipo de arena.',
    -- esMX
    '[PVP] No tengo equipo de arena.',
    -- ruRU
    '[PVP] У меня нет команды арены.');
    
INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pvp_no_arena_team',   100);
