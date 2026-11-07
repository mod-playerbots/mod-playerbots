DELETE FROM ai_playerbot_texts WHERE name IN (
    'class_role_assistant',
    'class_role_preferred_spec_bot',
    'class_role_bot',
    'class_role_none',
    'class_role_handover',
    'class_role_krosh_mage_tank',
    'class_role_kiggler_moonkin_tank',
    'class_role_leotheras_warlock_tank',
    'class_role_capernian_warlock_tank',
    'class_role_sanguinar_debuff_hunter',
    'class_role_class_mage',
    'class_role_class_balance_druid',
    'class_role_class_warlock',
    'class_role_class_demonology_warlock',
    'class_role_class_hunter'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'class_role_assistant',
    'class_role_preferred_spec_bot',
    'class_role_bot',
    'class_role_none',
    'class_role_handover',
    'class_role_krosh_mage_tank',
    'class_role_kiggler_moonkin_tank',
    'class_role_leotheras_warlock_tank',
    'class_role_capernian_warlock_tank',
    'class_role_sanguinar_debuff_hunter',
    'class_role_class_mage',
    'class_role_class_balance_druid',
    'class_role_class_warlock',
    'class_role_class_demonology_warlock',
    'class_role_class_hunter'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1930, 'class_role_assistant', '%name is the best-geared %class with the assistant flag and is the %role.', 0, 0, '%role: %name (부공격대장 권한이 있는 %class 중 장비가 가장 좋음)', '%name est le %class le mieux équipé avec le statut d''assistant et sera le %role.', '%name ist der am besten ausgerüstete %class mit Assistentenstatus und übernimmt die Rolle %role.', '%name 是装备最好的带有团队助理权限的%class，担任%role。', '%name 是裝備最好的具有團隊助理權限的%class，擔任%role。', '%name es el %class mejor equipado con permiso de ayudante y será el %role.', '%name es el %class mejor equipado con permiso de asistente y será el %role.', '%role: %name (лучшая экипировка среди игроков класса «%class» с правами помощника).');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1931, 'class_role_preferred_spec_bot', '%name is the best-geared %specclass bot and is the %role. Give a %class the assistant flag to choose someone else.', 0, 0, '%role: %name (%specclass 봇 중 장비가 가장 좋음). 다른 사람을 지정하려면 %class에게 부공격대장 권한을 주세요.', '%name est le bot %specclass le mieux équipé et sera le %role. Donnez le statut d''assistant à un %class pour choisir quelqu''un d''autre.', '%name ist der am besten ausgerüstete %specclass-Bot und übernimmt die Rolle %role. Gib einem %class den Assistentenstatus, um jemand anderen zu wählen.', '%name 是装备最好的%specclass机器人，担任%role。如需选择其他人，请给一名%class团队助理权限。', '%name 是裝備最好的%specclass機器人，擔任%role。如需選擇其他人，請給一名%class團隊助理權限。', '%name es el bot %specclass mejor equipado y será el %role. Da permiso de ayudante a un %class para elegir a otra persona.', '%name es el bot %specclass mejor equipado y será el %role. Dale permiso de asistente a un %class para elegir a otra persona.', '%role: %name (лучшая экипировка среди ботов «%specclass»). Чтобы выбрать другого, дайте права помощника игроку класса «%class».');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1932, 'class_role_bot', '%name is the best-geared %class bot and is the %role. Give a %class the assistant flag to choose someone else.', 0, 0, '%role: %name (%class 봇 중 장비가 가장 좋음). 다른 사람을 지정하려면 %class에게 부공격대장 권한을 주세요.', '%name est le bot %class le mieux équipé et sera le %role. Donnez le statut d''assistant à un %class pour choisir quelqu''un d''autre.', '%name ist der am besten ausgerüstete %class-Bot und übernimmt die Rolle %role. Gib einem %class den Assistentenstatus, um jemand anderen zu wählen.', '%name 是装备最好的%class机器人，担任%role。如需选择其他人，请给一名%class团队助理权限。', '%name 是裝備最好的%class機器人，擔任%role。如需選擇其他人，請給一名%class團隊助理權限。', '%name es el bot %class mejor equipado y será el %role. Da permiso de ayudante a un %class para elegir a otra persona.', '%name es el bot %class mejor equipado y será el %role. Dale permiso de asistente a un %class para elegir a otra persona.', '%role: %name (лучшая экипировка среди ботов класса «%class»). Чтобы выбрать другого, дайте права помощника игроку класса «%class».');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1933, 'class_role_none', 'No %class is present to be the %role.', 0, 0, '%role: 맡을 수 있는 %class 없음', 'Aucun %class n''est présent pour être le %role.', 'Kein %class ist anwesend, um die Rolle %role zu übernehmen.', '没有可以担任%role的%class。', '沒有可以擔任%role的%class。', 'No hay ningún %class para ser el %role.', 'No hay ningún %class para ser el %role.', 'Нет игрока класса «%class» для роли «%role».');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1934, 'class_role_handover', '%name takes over as the %role.', 0, 0, '%role: 이제 %name 님이 맡습니다.', '%name prend le relais en tant que %role.', '%name übernimmt jetzt die Rolle %role.', '%name 接替担任%role。', '%name 接替擔任%role。', '%name pasa a ser el %role.', '%name pasa a ser el %role.', '%role: теперь %name.');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1935, 'class_role_krosh_mage_tank', 'Krosh mage tank', 0, 0, '크로쉬 마법사 탱커', 'tank mage de Krosh', 'Krosh-Magiertank', '克洛什法师坦克', '克羅什法師坦克', 'tanque mago de Krosh', 'tanque mago de Krosh', 'Танк-маг для Кроша');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1936, 'class_role_kiggler_moonkin_tank', 'Kiggler moonkin tank', 0, 0, '키글러 달빛야수 탱커', 'tank sélénien de Kiggler', 'Kiggler-Mondkintank', '基戈尔枭兽坦克', '基戈爾梟獸坦克', 'tanque lechúcico de Kiggler', 'tanque lechúcico de Kiggler', 'Танк-лунный совух для Киглера');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1937, 'class_role_leotheras_warlock_tank', 'Leotheras warlock tank', 0, 0, '레오테라스 흑마법사 탱커', 'tank démoniste de Leotheras', 'Leotheras-Hexenmeistertank', '莱欧瑟拉斯术士坦克', '萊歐瑟拉斯術士坦克', 'tanque brujo de Leotheras', 'tanque brujo de Leotheras', 'Танк-чернокнижник для Леотераса');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1938, 'class_role_capernian_warlock_tank', 'Capernian warlock tank', 0, 0, '카퍼니안 흑마법사 탱커', 'tank démoniste de Capernian', 'Capernian-Hexenmeistertank', '卡波妮娅术士坦克', '卡波妮婭術士坦克', 'tanque brujo de Capernian', 'tanque brujo de Capernian', 'Танк-чернокнижник для Каперниан');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1939, 'class_role_sanguinar_debuff_hunter', 'Sanguinar debuff hunter', 0, 0, '생귀나르 약화 효과 사냥꾼', 'chasseur d''affaiblissement de Sanguinar', 'Sanguinar-Schwächungsjäger', '萨古纳尔减益猎人', '薩古納爾減益獵人', 'cazador de perjuicio de Sanguinar', 'cazador de perjuicio de Sanguinar', 'Охотник с ослаблением для Сангвинара');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1940, 'class_role_class_mage', 'Mage', 0, 0, '마법사', 'Mage', 'Magier', '法师', '法師', 'Mago', 'Mago', 'Маг');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1941, 'class_role_class_balance_druid', 'Balance Druid', 0, 0, '조화 드루이드', 'Druide Équilibre', 'Gleichgewichtsdruide', '平衡德鲁伊', '平衡德魯伊', 'Druida Equilibrio', 'Druida Equilibrio', 'Друид (Баланс)');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1942, 'class_role_class_warlock', 'Warlock', 0, 0, '흑마법사', 'Démoniste', 'Hexenmeister', '术士', '術士', 'Brujo', 'Brujo', 'Чернокнижник');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1943, 'class_role_class_demonology_warlock', 'Demonology Warlock', 0, 0, '악마 흑마법사', 'Démoniste Démonologie', 'Dämonologie-Hexenmeister', '恶魔学识术士', '惡魔學識術士', 'Brujo Demonología', 'Brujo Demonología', 'Чернокнижник (Демонология)');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1944, 'class_role_class_hunter', 'Hunter', 0, 0, '사냥꾼', 'Chasseur', 'Jäger', '猎人', '獵人', 'Cazador', 'Cazador', 'Охотник');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('class_role_assistant', 100),
    ('class_role_preferred_spec_bot', 100),
    ('class_role_bot', 100),
    ('class_role_none', 100),
    ('class_role_handover', 100),
    ('class_role_krosh_mage_tank', 100),
    ('class_role_kiggler_moonkin_tank', 100),
    ('class_role_leotheras_warlock_tank', 100),
    ('class_role_capernian_warlock_tank', 100),
    ('class_role_sanguinar_debuff_hunter', 100),
    ('class_role_class_mage', 100),
    ('class_role_class_balance_druid', 100),
    ('class_role_class_warlock', 100),
    ('class_role_class_demonology_warlock', 100),
    ('class_role_class_hunter', 100);
