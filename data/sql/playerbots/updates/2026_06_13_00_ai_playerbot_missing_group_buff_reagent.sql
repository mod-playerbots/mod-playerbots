DELETE FROM ai_playerbot_texts_chance
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic',
  'missing_group_buff_reagent'
);

DELETE FROM ai_playerbot_texts
WHERE name IN (
  'rp_missing_reagent_greater_blessing',
  'rp_missing_reagent_gift_of_the_wild',
  'rp_missing_reagent_arcane_brilliance',
  'rp_missing_reagent_generic',
  'missing_group_buff_reagent'
);

INSERT INTO ai_playerbot_texts
  (id, name, text, say_type, reply_type, text_loc1, text_loc2, text_loc3, text_loc4, text_loc5, text_loc6, text_loc7, text_loc8)
VALUES
  (1900, 'missing_group_buff_reagent',
   'I am out of reagents for %group_spell and am casting %base_spell instead.',
   0, 0, '', '', '', '', '', '', '', '');

INSERT INTO ai_playerbot_texts_chance
  (name, probability)
VALUES
  ('missing_group_buff_reagent', 100);
