-- #########################################################
-- Playerbots - Add focus heal command texts
-- Localized for all WotLK locales (koKR, frFR, deDE, zhCN,
-- zhTW, esES, esMX, ruRU)
-- #########################################################

DELETE FROM ai_playerbot_texts WHERE name IN (
    'focus_heal_not_healer',
    'focus_heal_provide_names',
    'focus_heal_no_targets',
    'focus_heal_current_targets',
    'focus_heal_cleared',
    'focus_heal_add_remove_syntax',
    'focus_heal_not_in_group',
    'focus_heal_not_in_group_with',
    'focus_heal_added',
    'focus_heal_removed'
);
DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'focus_heal_not_healer',
    'focus_heal_provide_names',
    'focus_heal_no_targets',
    'focus_heal_current_targets',
    'focus_heal_cleared',
    'focus_heal_add_remove_syntax',
    'focus_heal_not_in_group',
    'focus_heal_not_in_group_with',
    'focus_heal_added',
    'focus_heal_removed'
);

-- focus_heal_not_healer
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1745,
    'focus_heal_not_healer',
    'I''m not a healer or offhealer (please change my strats to heal or offheal)',
    0, 0,
    '저는 힐러나 오프힐러가 아닙니다 (전략을 heal 또는 offheal로 변경해주세요)',
    'Je ne suis pas un soigneur ou un soigneur secondaire (veuillez changer mes strats en heal ou offheal)',
    'Ich bin kein Heiler oder Nebenheiler (bitte ändere meine Strategien auf heal oder offheal)',
    '我不是治疗者或副治疗者（请将我的策略更改为 heal 或 offheal）',
    '我不是治療者或副治療者（請將我的策略更改為 heal 或 offheal）',
    'No soy un sanador ni un sanador secundario (por favor cambia mis estrategias a heal o offheal)',
    'No soy un sanador ni un sanador secundario (por favor cambia mis estrategias a heal o offheal)',
    'Я не лекарь и не побочный лекарь (пожалуйста, измените мои стратегии на heal или offheal)');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_not_healer', 100);

-- focus_heal_provide_names
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1746,
    'focus_heal_provide_names',
    'Please provide one or more player names',
    0, 0,
    '하나 이상의 플레이어 이름을 제공해주세요',
    'Veuillez fournir un ou plusieurs noms de joueurs',
    'Bitte geben Sie einen oder mehrere Spielernamen an',
    '请提供一个或多个玩家名称',
    '請提供一個或多個玩家名稱',
    'Por favor proporciona uno o más nombres de jugadores',
    'Por favor proporciona uno o más nombres de jugadores',
    'Пожалуйста, укажите одно или несколько имён игроков');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_provide_names', 100);

-- focus_heal_no_targets
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1747,
    'focus_heal_no_targets',
    'I don''t have any focus heal targets',
    0, 0,
    '지정된 집중 치유 대상이 없습니다',
    'Je n''ai aucune cible de soin prioritaire',
    'Ich habe keine fokussierten Heilziele',
    '我没有任何集中治疗目标',
    '我沒有任何集中治療目標',
    'No tengo ningún objetivo de sanación prioritario',
    'No tengo ningún objetivo de sanación prioritario',
    'У меня нет целей приоритетного лечения');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_no_targets', 100);

-- focus_heal_current_targets: %targets is replaced with comma-separated player names
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1748,
    'focus_heal_current_targets',
    'My focus heal targets are %targets',
    0, 0,
    '나의 집중 치유 대상: %targets',
    'Mes cibles de soin prioritaire sont %targets',
    'Meine fokussierten Heilziele sind %targets',
    '我的集中治疗目标是 %targets',
    '我的集中治療目標是 %targets',
    'Mis objetivos de sanación prioritarios son %targets',
    'Mis objetivos de sanación prioritarios son %targets',
    'Мои цели приоритетного лечения: %targets');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_current_targets', 100);

-- focus_heal_cleared
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1749,
    'focus_heal_cleared',
    'Removed focus heal targets',
    0, 0,
    '집중 치유 대상을 제거했습니다',
    'Cibles de soin prioritaire supprimées',
    'Fokussierte Heilziele entfernt',
    '已移除集中治疗目标',
    '已移除集中治療目標',
    'Objetivos de sanación prioritarios eliminados',
    'Objetivos de sanación prioritarios eliminados',
    'Цели приоритетного лечения удалены');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_cleared', 100);

-- focus_heal_add_remove_syntax
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1750,
    'focus_heal_add_remove_syntax',
    'Please specify a + for add or - to remove a target',
    0, 0,
    '대상을 추가하려면 +, 제거하려면 -를 지정해주세요',
    'Veuillez spécifier + pour ajouter ou - pour retirer une cible',
    'Bitte geben Sie + zum Hinzufügen oder - zum Entfernen eines Ziels an',
    '请指定 + 添加或 - 移除目标',
    '請指定 + 添加或 - 移除目標',
    'Por favor especifica + para agregar o - para eliminar un objetivo',
    'Por favor especifica + para agregar o - para eliminar un objetivo',
    'Пожалуйста, укажите + для добавления или - для удаления цели');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_add_remove_syntax', 100);

-- focus_heal_not_in_group
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1751,
    'focus_heal_not_in_group',
    'I''m not in a group',
    0, 0,
    '저는 파티에 속해있지 않습니다',
    'Je ne suis pas dans un groupe',
    'Ich bin in keiner Gruppe',
    '我不在队伍中',
    '我不在隊伍中',
    'No estoy en un grupo',
    'No estoy en un grupo',
    'Я не в группе');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_not_in_group', 100);

-- focus_heal_not_in_group_with: %player_name is replaced with the target player's name
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1752,
    'focus_heal_not_in_group_with',
    'I''m not in a group with %player_name',
    0, 0,
    '%player_name 와(과) 같은 파티에 없습니다',
    'Je ne suis pas dans un groupe avec %player_name',
    'Ich bin nicht in einer Gruppe mit %player_name',
    '我与 %player_name 不在同一队伍中',
    '我與 %player_name 不在同一隊伍中',
    'No estoy en un grupo con %player_name',
    'No estoy en un grupo con %player_name',
    'Я не в группе с %player_name');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_not_in_group_with', 100);

-- focus_heal_added: %player_name is replaced with the added player's name
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1753,
    'focus_heal_added',
    'Added %player_name to focus heal targets',
    0, 0,
    '%player_name 을(를) 집중 치유 대상에 추가했습니다',
    '%player_name ajouté aux cibles de soin prioritaire',
    '%player_name zu den fokussierten Heilzielen hinzugefügt',
    '已将 %player_name 添加到集中治疗目标',
    '已將 %player_name 添加到集中治療目標',
    '%player_name agregado a los objetivos de sanación prioritarios',
    '%player_name agregado a los objetivos de sanación prioritarios',
    '%player_name добавлен в цели приоритетного лечения');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_added', 100);

-- focus_heal_removed: %player_name is replaced with the removed player's name
INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES (
    1754,
    'focus_heal_removed',
    'Removed %player_name from focus heal targets',
    0, 0,
    '%player_name 을(를) 집중 치유 대상에서 제거했습니다',
    '%player_name retiré des cibles de soin prioritaire',
    '%player_name aus den fokussierten Heilzielen entfernt',
    '已将 %player_name 从集中治疗目标中移除',
    '已將 %player_name 從集中治療目標中移除',
    '%player_name eliminado de los objetivos de sanación prioritarios',
    '%player_name eliminado de los objetivos de sanación prioritarios',
    '%player_name удалён из целей приоритетного лечения');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('focus_heal_removed', 100);
