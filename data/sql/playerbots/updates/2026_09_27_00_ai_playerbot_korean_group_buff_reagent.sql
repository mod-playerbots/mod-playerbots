-- Korean (koKR) text for the group buff reagent notice.
UPDATE `ai_playerbot_texts`
SET `text_loc1` = '%group_spell 시전 재료가 떨어져서 대신 %base_spell을 시전합니다.'
WHERE `id` = 1900 AND `name` = 'missing_group_buff_reagent' AND `text_loc1` = '';
