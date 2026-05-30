DELETE FROM ai_playerbot_texts WHERE name IN (
    'send_mail_disabled'
);

DELETE FROM ai_playerbot_texts_chance WHERE name IN (
    'send_mail_disabled'
);

INSERT INTO ai_playerbot_texts (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8) VALUES
(1899, 'send_mail_disabled', 'I cannot send mail', 0, 0, '우편을 보낼 수 없습니다', 'Je ne peux pas envoyer de courrier', 'Ich kann keine Post senden', '我不能寄送邮件', '我不能寄送郵件', 'No puedo enviar correo', 'No puedo enviar correo', 'Я не могу отправить почту');

INSERT INTO ai_playerbot_texts_chance (name, probability) VALUES
('send_mail_disabled', 100);
