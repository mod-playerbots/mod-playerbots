DELETE FROM ai_playerbot_texts WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'kalecgos_tank_sent_to_spectral_realm',
    'kalecgos_tank_should_enter_spectral_realm',
    'kiljaeden_designated_dragon_orb_user',
    'kiljaeden_no_designated_dragon_orb_user'
);

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1901, 'kalecgos_tank_sent_to_spectral_realm', 'Tank %tank has been sent to the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1902, 'kalecgos_tank_should_enter_spectral_realm', 'Tank %tank should enter the Spectral Realm. The active Kalecgos tank is %current.', 0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1903, 'kiljaeden_designated_dragon_orb_user', '%bot is the first assistant and the designated dragon orb user!', 1, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts
    (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
    (1904, 'kiljaeden_no_designated_dragon_orb_user', 'No bot has been assigned as the designated dragon orb user, and therefore a player must control the dragons. If you would like a bot to use the dragon orbs, please set the assistant flag for a bot.', 1, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
    ('kalecgos_tank_sent_to_spectral_realm', 100),
    ('kalecgos_tank_should_enter_spectral_realm', 100),
    ('kiljaeden_designated_dragon_orb_user', 100),
    ('kiljaeden_no_designated_dragon_orb_user', 100);
