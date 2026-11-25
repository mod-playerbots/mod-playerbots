DELETE FROM ai_playerbot_texts WHERE name IN ('toc_death_grip_skip');

DELETE FROM ai_playerbot_texts_chance WHERE name IN ('toc_death_grip_skip');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1948, 'toc_death_grip_skip', 'Death Grip ready in %seconds s, skipping my grip', 0, 0, '죽음의 손아귀 %seconds초 후 사용 가능, 이번 당기기는 건너뜁니다', 'Poigne de la mort prête dans %seconds s, je passe mon tour', 'Todesgriff bereit in %seconds s, ich setze meinen Griff aus', '死亡之握%seconds秒后就绪，跳过我的拉人', '死亡之握%seconds秒後就緒，跳過我的拉人', 'Atracción letal lista en %seconds s, me salto mi turno', 'Atracción letal lista en %seconds s, me salto mi turno', 'Хватка смерти будет готова через %seconds с, пропускаю свою очередь');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('toc_death_grip_skip', 100);
