DELETE FROM ai_playerbot_texts WHERE name IN (
    'class_role_zerevor_mage_tank',
    'class_role_illidan_warlock_tank'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'class_role_zerevor_mage_tank',
    'class_role_illidan_warlock_tank'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1945, 'class_role_zerevor_mage_tank', 'Zerevor mage tank', 0, 0, '제레보르 마법사 탱커', 'tank mage de Zerevor', 'Zerevor-Magiertank', '塞勒沃尔法师坦克', '札瑞佛法師坦克', 'tanque mago de Zerevor', 'tanque mago de Zerevor', 'Танк-маг для Зеревора');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1946, 'class_role_illidan_warlock_tank', 'Illidan warlock tank', 0, 0, '일리단 흑마법사 탱커', 'tank démoniste d''Illidan', 'Illidan-Hexenmeistertank', '伊利丹术士坦克', '伊利丹術士坦克', 'tanque brujo de Illidan', 'tanque brujo de Illidan', 'Танк-чернокнижник для Иллидана');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('class_role_zerevor_mage_tank', 100),
    ('class_role_illidan_warlock_tank', 100);
