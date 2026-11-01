DELETE FROM ai_playerbot_texts WHERE name IN (
                                              'portal_none_known',
                                              'portal_list',
                                              'portal_unknown',
                                              'portal_no_reagents',
                                              'portal_cannot_cast',
                                              'portal_casting'
    );

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
                                                     'portal_none_known',
                                                     'portal_list',
                                                     'portal_unknown',
                                                     'portal_no_reagents',
                                                     'portal_cannot_cast',
                                                     'portal_casting'
    );

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1946, 'portal_none_known', 'I don''t know any portals', 0, 0, '아는 차원문이 없습니다', 'Je ne connais aucun portail', 'Ich kenne keine Portale', '我不会任何传送门', '我不會任何傳送門', 'No conozco ningún portal', 'No conozco ningún portal', 'Я не знаю ни одного портала');

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1947, 'portal_list', 'I can open portals to: %portals', 0, 0, '열 수 있는 차원문: %portals', 'Je peux ouvrir des portails vers : %portals', 'Ich kann Portale öffnen nach: %portals', '我可以开启这些传送门：%portals', '我可以開啟這些傳送門：%portals', 'Puedo abrir portales a: %portals', 'Puedo abrir portales a: %portals', 'Я могу открыть порталы: %portals');

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1948, 'portal_unknown', 'I don''t know a portal to %destination', 0, 0, '%destination 차원문을 모릅니다', 'Je ne connais pas de portail vers %destination', 'Ich kenne kein Portal nach %destination', '我不会通往%destination的传送门', '我不會通往%destination的傳送門', 'No conozco ningún portal a %destination', 'No conozco ningún portal a %destination', 'Я не знаю портала: %destination');

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1949, 'portal_no_reagents', 'I need a Rune of Portals for %portal', 0, 0, '%portal 시전에 차원문의 룬이 필요합니다', 'J''ai besoin d''une Rune des portails pour %portal', 'Ich brauche eine Rune der Portale für %portal', '施放%portal需要传送门符文', '施放%portal需要傳送門符文', 'Necesito una Runa de portales para %portal', 'Necesito una Runa de portales para %portal', 'Для заклинания %portal нужна Руна порталов');

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1950, 'portal_cannot_cast', 'I cannot open %portal right now', 0, 0, '지금은 %portal을(를) 열 수 없습니다', 'Je ne peux pas ouvrir %portal pour le moment', 'Ich kann %portal gerade nicht öffnen', '我现在无法开启%portal', '我現在無法開啟%portal', 'No puedo abrir %portal ahora mismo', 'No puedo abrir %portal en este momento', 'Сейчас я не могу открыть %portal');

INSERT INTO ai_playerbot_texts
(id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1951, 'portal_casting', 'Opening %portal', 0, 0, '%portal 여는 중', 'J''ouvre %portal', 'Öffne %portal', '正在开启%portal', '正在開啟%portal', 'Abriendo %portal', 'Abriendo %portal', 'Открываю %portal');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
                                                              ('portal_none_known', 100),
                                                              ('portal_list', 100),
                                                              ('portal_unknown', 100),
                                                              ('portal_no_reagents', 100),
                                                              ('portal_cannot_cast', 100),
                                                              ('portal_casting', 100);
