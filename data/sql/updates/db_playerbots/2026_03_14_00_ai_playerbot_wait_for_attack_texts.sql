-- #########################################################
-- Playerbots - Add texts for SetWaitForAttackTimeAction
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN ('wait_for_attack_provide_time', 'wait_for_attack_invalid_time', 'wait_for_attack_time_set');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('wait_for_attack_provide_time', 'wait_for_attack_invalid_time', 'wait_for_attack_time_set');

-- ---------------------------------------------------------
-- wait_for_attack_provide_time
-- Please provide a time to set (in seconds)
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1740,
    'wait_for_attack_provide_time',
    'Please provide a time to set (in seconds)',
    0, 0,
    -- koKR
    '설정할 시간을 입력해 주세요 (초 단위)',
    -- frFR
    'Veuillez indiquer un temps à définir (en secondes)',
    -- deDE
    'Bitte gib eine Zeit an (in Sekunden)',
    -- zhCN
    '请提供要设置的时间（以秒为单位）',
    -- zhTW
    '請提供要設定的時間（以秒為單位）',
    -- esES
    'Por favor, indica un tiempo a establecer (en segundos)',
    -- esMX
    'Por favor, indica un tiempo a establecer (en segundos)',
    -- ruRU
    'Пожалуйста, укажите время (в секундах)');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('wait_for_attack_provide_time', 100);

-- ---------------------------------------------------------
-- wait_for_attack_invalid_time
-- Please provide valid time to set (in seconds) between 0 and 99
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1741,
    'wait_for_attack_invalid_time',
    'Please provide valid time to set (in seconds) between 0 and 99',
    0, 0,
    -- koKR
    '0에서 99 사이의 유효한 시간을 입력해 주세요 (초 단위)',
    -- frFR
    'Veuillez indiquer un temps valide (en secondes) entre 0 et 99',
    -- deDE
    'Bitte gib eine gültige Zeit an (in Sekunden) zwischen 0 und 99',
    -- zhCN
    '请提供有效的时间（以秒为单位），范围为 0 到 99',
    -- zhTW
    '請提供有效的時間（以秒為單位），範圍為 0 到 99',
    -- esES
    'Por favor, indica un tiempo válido (en segundos) entre 0 y 99',
    -- esMX
    'Por favor, indica un tiempo válido (en segundos) entre 0 y 99',
    -- ruRU
    'Пожалуйста, укажите допустимое время (в секундах) от 0 до 99');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('wait_for_attack_invalid_time', 100);

-- ---------------------------------------------------------
-- wait_for_attack_time_set
-- Wait for attack time set to %new_time seconds
-- ---------------------------------------------------------
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
	1742,
    'wait_for_attack_time_set',
    'Wait for attack time set to %new_time seconds',
    0, 0,
    -- koKR
    '공격 대기 시간이 %new_time초로 설정되었습니다',
    -- frFR
    'Temps d''attente avant l''attaque défini à %new_time secondes',
    -- deDE
    'Wartezeit vor dem Angriff auf %new_time Sekunden gesetzt',
    -- zhCN
    '等待攻击时间已设置为 %new_time 秒',
    -- zhTW
    '等待攻擊時間已設定為 %new_time 秒',
    -- esES
    'Tiempo de espera para atacar establecido en %new_time segundos',
    -- esMX
    'Tiempo de espera para atacar establecido en %new_time segundos',
    -- ruRU
    'Время ожидания атаки установлено на %new_time секунд');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('wait_for_attack_time_set', 100);
