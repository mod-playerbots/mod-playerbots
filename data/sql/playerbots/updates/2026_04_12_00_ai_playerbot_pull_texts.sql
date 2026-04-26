-- #########################################################
-- Playerbots - Add pull command texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN (
    'pull_no_target_error',
    'pull_target_too_far_error',
    'pull_invalid_target_error',
    'pull_action_unavailable_error'
);
DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'pull_no_target_error',
    'pull_target_too_far_error',
    'pull_invalid_target_error',
    'pull_action_unavailable_error'
);

-- pull_no_target_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1755,
    'pull_no_target_error',
    'You have no target',
    0, 0,
    '대상이 없습니다',
    'Vous n''avez pas de cible',
    'Du hast kein Ziel',
    '你没有目标',
    '你沒有目標',
    'No tienes objetivo',
    'No tienes objetivo',
    'У вас нет цели');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pull_no_target_error', 100);

-- pull_target_too_far_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1756,
    'pull_target_too_far_error',
    'The target is too far away',
    0, 0,
    '대상이 너무 멀리 있습니다',
    'La cible est trop loin',
    'Das Ziel ist zu weit entfernt',
    '目标太远了',
    '目標太遠了',
    'El objetivo está demasiado lejos',
    'El objetivo está demasiado lejos',
    'Цель слишком далеко');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pull_target_too_far_error', 100);

-- pull_invalid_target_error
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1757,
    'pull_invalid_target_error',
    'The target can''t be pulled',
    0, 0,
    '해당 대상은 풀링할 수 없습니다',
    'La cible ne peut pas être attirée',
    'Das Ziel kann nicht gepullt werden',
    '该目标无法被拉怪',
    '該目標無法被拉怪',
    'No se puede hacer pull al objetivo',
    'No se puede hacer pull al objetivo',
    'Эту цель нельзя пуллить');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pull_invalid_target_error', 100);

-- pull_action_unavailable_error: %action_name is replaced with the configured pull action
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1758,
    'pull_action_unavailable_error',
    'Can''t perform pull action ''%action_name''',
    0, 0,
    '''%action_name'' 풀 액션을 수행할 수 없습니다',
    'Impossible d''effectuer l''action d''engagement ''%action_name''',
    'Die Pull-Aktion ''%action_name'' kann nicht ausgeführt werden',
    '无法执行拉怪动作“%action_name”',
    '無法執行拉怪動作「%action_name」',
    'No se puede realizar la acción de pull ''%action_name''',
    'No se puede realizar la acción de pull ''%action_name''',
    'Невозможно выполнить действие пула ''%action_name''');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('pull_action_unavailable_error', 100);
