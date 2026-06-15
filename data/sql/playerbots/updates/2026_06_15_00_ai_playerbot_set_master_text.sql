DELETE FROM ai_playerbot_texts WHERE name IN ('set_master_follow');
DELETE FROM ai_playerbot_texts_chance WHERE name IN ('set_master_follow');

INSERT INTO ai_playerbot_texts (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
(1900, 'set_master_follow', 'Now following %name.', 0, 0,
'이제 %name을(를) 따라갑니다.',
'Je suis désormais %name.',
'Ich folge jetzt %name.',
'現在跟隨 %name。',
'现在跟随 %name。',
'Ahora sigo a %name.',
'Ahora sigo a %name.',
'Теперь следую за %name.');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES ('set_master_follow', 100);
