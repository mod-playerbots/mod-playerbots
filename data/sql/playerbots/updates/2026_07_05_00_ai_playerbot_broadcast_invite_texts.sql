-- Texts for group/raid invite broadcasts (BroadcastGuildGroupOrRaidInvite)
-- These keys replace the literal sentences previously passed to GetBotText().
DELETE FROM ai_playerbot_texts WHERE name IN ('broadcast_group_invite_any', 'broadcast_group_invite_name', 'broadcast_raid_invite_any', 'broadcast_raid_invite_name');

INSERT INTO `ai_playerbot_texts`
    (`name`, `text`, `say_type`, `reply_type`,
     `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`,
     `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES
    ('broadcast_group_invite_any', 'Hey anyone wanna group up in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_group_invite_any', 'LFM around %zone_name, whisper me', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_group_invite_any', 'Anyone questing in %zone_name? Looking for a group', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_group_invite_name', 'Hey %name, do you want to join my group? I''m heading for %zone_name!', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_group_invite_name', '%name, wanna group up? Heading to %zone_name', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_raid_invite_any', 'Hey anyone want to raid in %zone_name?', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_raid_invite_any', 'Putting a raid together for %zone_name, who''s in?', 0, 0, '', '', '', '', '', '', '', ''),
    ('broadcast_raid_invite_name', 'Hey %name, I''m raiding in %zone_name, want to join?', 0, 0, '', '', '', '', '', '', '', '');
