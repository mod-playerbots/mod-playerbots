-- #########################################################
-- Playerbots - Add rpg do gather command texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN (
    'rpg_gather_no_profession_error',
    'rpg_gather_strategy_off_warning',
    'rpg_gather_started',
    'rpg_gather_started_no_nodes',
    'rpg_unknown_status_error'
);
DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'rpg_gather_no_profession_error',
    'rpg_gather_strategy_off_warning',
    'rpg_gather_started',
    'rpg_gather_started_no_nodes',
    'rpg_unknown_status_error'
);

-- rpg_gather_no_profession_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1913,
    'rpg_gather_no_profession_error',
    'I have neither Herbalism nor Mining, so I can''t gather.',
    0, 0,
    '저는 약초 채집도 채광도 없어서 채집할 수 없습니다.',
    'Je n''ai ni Herboristerie ni Minage, je ne peux donc pas récolter.',
    'Ich habe weder Kräuterkunde noch Bergbau, also kann ich nicht sammeln.',
    '我既没有草药学也没有采矿，所以无法采集。',
    '我既沒有草藥學也沒有採礦，所以無法採集。',
    'No tengo ni Herboristería ni Minería, así que no puedo recolectar.',
    'No tengo ni Herboristería ni Minería, así que no puedo recolectar.',
    'У меня нет ни травничества, ни горного дела, поэтому я не могу собирать.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_gather_no_profession_error', 100);

-- rpg_gather_strategy_off_warning: 'nc +new rpg' stays in English because
-- it is the actual command the player has to type
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1914,
    'rpg_gather_strategy_off_warning',
    'Note: my ''new rpg'' strategy is off - run ''nc +new rpg'' or I won''t act on this.',
    0, 0,
    '참고: 제 ''new rpg'' 전략이 꺼져 있습니다 - ''nc +new rpg''를 실행하지 않으면 이 명령을 수행하지 않습니다.',
    'Remarque : ma stratégie ''new rpg'' est désactivée - lancez ''nc +new rpg'' ou je n''agirai pas.',
    'Hinweis: Meine ''new rpg''-Strategie ist aus - führe ''nc +new rpg'' aus, sonst reagiere ich nicht darauf.',
    '注意：我的''new rpg''策略已关闭 - 请执行''nc +new rpg''，否则我不会执行此操作。',
    '注意：我的''new rpg''策略已關閉 - 請執行''nc +new rpg''，否則我不會執行此操作。',
    'Nota: mi estrategia ''new rpg'' está desactivada - ejecuta ''nc +new rpg'' o no actuaré.',
    'Nota: mi estrategia ''new rpg'' está desactivada - ejecuta ''nc +new rpg'' o no actuaré.',
    'Примечание: моя стратегия ''new rpg'' выключена - выполните ''nc +new rpg'', иначе я не буду действовать.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_gather_strategy_off_warning', 100);

-- rpg_gather_started
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1915,
    'rpg_gather_started',
    'Starting to gather nodes in this zone.',
    0, 0,
    '이 지역에서 채집을 시작합니다.',
    'Je commence à récolter les gisements de cette zone.',
    'Ich beginne, die Vorkommen in dieser Zone zu sammeln.',
    '开始采集这个区域的资源点。',
    '開始採集這個區域的資源點。',
    'Empiezo a recolectar los nodos de esta zona.',
    'Empiezo a recolectar los nodos de esta zona.',
    'Начинаю собирать ресурсы в этой зоне.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_gather_started', 100);

-- rpg_gather_started_no_nodes
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1916,
    'rpg_gather_started_no_nodes',
    'Starting to gather, but I see no harvestable nodes in this zone right now.',
    0, 0,
    '채집을 시작하지만, 지금 이 지역에는 채집 가능한 자원이 보이지 않습니다.',
    'Je commence à récolter, mais je ne vois aucun gisement exploitable dans cette zone pour le moment.',
    'Ich beginne zu sammeln, sehe aber gerade keine abbaubaren Vorkommen in dieser Zone.',
    '开始采集，但目前在这个区域看不到可采集的资源点。',
    '開始採集，但目前在這個區域看不到可採集的資源點。',
    'Empiezo a recolectar, pero ahora mismo no veo nodos recolectables en esta zona.',
    'Empiezo a recolectar, pero ahora mismo no veo nodos recolectables en esta zona.',
    'Начинаю собирать, но сейчас в этой зоне не вижу доступных ресурсов.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_gather_started_no_nodes', 100);

-- rpg_unknown_status_error: re-inserted with 'do gather' in the option list.
-- Option keywords stay in English because they are the actual command
-- tokens parsed by NewRpgInfo::StatusFromString
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1766,
    'rpg_unknown_status_error',
    'Unknown rpg status. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    0, 0,
    '알 수 없는 RPG 상태입니다. 옵션: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Statut RPG inconnu. Options: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Unbekannter RPG-Status. Optionen: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    '未知的RPG状态。选项: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather。',
    '未知的RPG狀態。選項: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather。',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Estado RPG desconocido. Opciones: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.',
    'Неизвестный RPG-статус. Опции: idle, rest, wander random, wander npc, go grind, go camp, do quest [<id>], travel flight, outdoor pvp, do gather.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('rpg_unknown_status_error', 100);
