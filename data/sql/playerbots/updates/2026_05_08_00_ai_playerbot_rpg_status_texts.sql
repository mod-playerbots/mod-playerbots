-- #########################################################
-- Playerbots - Add rpg debug command texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN (
    'rpg_debug_permission_error',
    'rpg_status_changed',
    'rpg_no_grind_pos_error',
    'rpg_no_camp_pos_error',
    'rpg_no_flight_path_error',
    'rpg_no_quest_error',
    'rpg_invalid_quest_error',
    'rpg_unknown_status_error'
);
DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'rpg_debug_permission_error',
    'rpg_status_changed',
    'rpg_no_grind_pos_error',
    'rpg_no_camp_pos_error',
    'rpg_no_flight_path_error',
    'rpg_no_quest_error',
    'rpg_invalid_quest_error',
    'rpg_unknown_status_error'
);

-- rpg_debug_permission_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1759,
    'rpg_debug_permission_error',
    'Only your master or a GM can change my rpg status.',
    0, 0,
    '주인이나 GM만 제 RPG 상태를 변경할 수 있습니다.',
    'Seul votre maître ou un MJ peut changer mon statut RPG.',
    'Nur dein Meister oder ein GM kann meinen RPG-Status ändern.',
    '只有你的主人或GM才能更改我的RPG状态。',
    '只有你的主人或GM才能更改我的RPG狀態。',
    'Solo tu maestro o un GM puede cambiar mi estado de RPG.',
    'Solo tu maestro o un GM puede cambiar mi estado de RPG.',
    'Только ваш хозяин или GM может изменить мой RPG-статус.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_debug_permission_error', 100);

-- rpg_status_changed: %status is replaced with the new status name (e.g. IDLE, REST, DO_QUEST 1234)
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1760,
    'rpg_status_changed',
    'rpg status -> %status',
    0, 0,
    'RPG 상태 -> %status',
    'statut RPG -> %status',
    'RPG-Status -> %status',
    'RPG状态 -> %status',
    'RPG狀態 -> %status',
    'estado RPG -> %status',
    'estado RPG -> %status',
    'RPG-статус -> %status');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_status_changed', 100);

-- rpg_no_grind_pos_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1761,
    'rpg_no_grind_pos_error',
    'No grind position available.',
    0, 0,
    '사냥 위치를 찾을 수 없습니다.',
    'Aucune position de farm disponible.',
    'Keine Grindposition verfügbar.',
    '没有可用的练级位置。',
    '沒有可用的練級位置。',
    'No hay posición de grindeo disponible.',
    'No hay posición de grindeo disponible.',
    'Нет доступной позиции для гринда.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_no_grind_pos_error', 100);

-- rpg_no_camp_pos_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1762,
    'rpg_no_camp_pos_error',
    'No camp position available.',
    0, 0,
    '캠프 위치를 찾을 수 없습니다.',
    'Aucune position de campement disponible.',
    'Keine Lagerposition verfügbar.',
    '没有可用的营地位置。',
    '沒有可用的營地位置。',
    'No hay posición de campamento disponible.',
    'No hay posición de campamento disponible.',
    'Нет доступной позиции для лагеря.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_no_camp_pos_error', 100);

-- rpg_no_flight_path_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1763,
    'rpg_no_flight_path_error',
    'No flight path available.',
    0, 0,
    '이용 가능한 비행 경로가 없습니다.',
    'Aucun chemin de vol disponible.',
    'Keine Flugroute verfügbar.',
    '没有可用的飞行路径。',
    '沒有可用的飛行路徑。',
    'No hay ruta de vuelo disponible.',
    'No hay ruta de vuelo disponible.',
    'Нет доступного маршрута полёта.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_no_flight_path_error', 100);

-- rpg_no_quest_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1764,
    'rpg_no_quest_error',
    'No quest available; use ''do quest <id>''.',
    0, 0,
    '사용 가능한 퀘스트가 없습니다; ''do quest <id>''를 사용하세요.',
    'Aucune quête disponible ; utilisez ''do quest <id>''.',
    'Keine Quest verfügbar; nutze ''do quest <id>''.',
    '没有可用的任务；请使用 ''do quest <id>''。',
    '沒有可用的任務；請使用 ''do quest <id>''。',
    'No hay misión disponible; usa ''do quest <id>''.',
    'No hay misión disponible; usa ''do quest <id>''.',
    'Нет доступных квестов; используйте ''do quest <id>''.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_no_quest_error', 100);

-- rpg_invalid_quest_error: %quest_id is replaced with the quest id
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1765,
    'rpg_invalid_quest_error',
    'Invalid quest %quest_id',
    0, 0,
    '유효하지 않은 퀘스트 %quest_id',
    'Quête invalide %quest_id',
    'Ungültige Quest %quest_id',
    '无效任务 %quest_id',
    '無效任務 %quest_id',
    'Misión no válida %quest_id',
    'Misión no válida %quest_id',
    'Неверный квест %quest_id');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_invalid_quest_error', 100);

-- rpg_unknown_status_error: option keywords stay in English because they
-- are the actual command tokens parsed by NewRpgInfo::StatusFromString
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1766,
    'rpg_unknown_status_error',
    'Unknown rpg status. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    0, 0,
    '알 수 없는 RPG 상태입니다. 옵션: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    'Statut RPG inconnu. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    'Unbekannter RPG-Status. Optionen: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    '未知的RPG状态。选项: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp。',
    '未知的RPG狀態。選項: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp。',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.',
    'Неизвестный RPG-статус. Опции: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_unknown_status_error', 100);
