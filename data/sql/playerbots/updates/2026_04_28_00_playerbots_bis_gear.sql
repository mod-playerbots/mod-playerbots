-- BiS gear table for the /p bis party-chat command . Unified across tiers.
-- Lookup keys: (class, tab, slot, faction, auto_gear_score_limit).
-- faction: 0=Both, 1=Alliance, 2=Horde. Faction-specific overrides Both.
-- auto_gear_score_limit: matched exactly against AiPlayerbot.AutoGearScoreLimit.
-- phase: free-text label for readability (e.g. 'Phase 5', 'Pre-Raid').
-- Slot uses AzerothCore EquipmentSlots enum:
-- head=0, neck=1, shoulders=2, chest=4, waist=5, legs=6, feet=7, wrists=8, hands=9,
-- finger1=10, finger2=11, trinket1=12, trinket2=13, back=14, mainhand=15, offhand=16, ranged=17.
-- Druid Bear lives under sentinel tab=10 (resolved at runtime when bot has Tank strategy).

DROP TABLE IF EXISTS `playerbots_bis`;
DROP TABLE IF EXISTS `playerbots_bis_gear`;
CREATE TABLE `playerbots_bis_gear` (
    `class`      TINYINT UNSIGNED NOT NULL,
    `tab`        TINYINT UNSIGNED NOT NULL,
    `slot`       TINYINT UNSIGNED NOT NULL,
    `faction`    TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `auto_gear_score_limit` SMALLINT UNSIGNED NOT NULL,
    `item_id`       INT UNSIGNED NOT NULL,
    `phase`      VARCHAR(32) NOT NULL DEFAULT '',
    `class_name`   VARCHAR(16) NOT NULL,
    `spec_name`    VARCHAR(32) NOT NULL,
    `slot_name`    VARCHAR(16) NOT NULL,
    `faction_name` VARCHAR(8)  NOT NULL DEFAULT 'Both',
    `item_name`    VARCHAR(96) NOT NULL,
    PRIMARY KEY (`class`, `tab`, `slot`, `faction`, `auto_gear_score_limit`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


-- ============================================================
-- Warrior (1)
-- ============================================================
-- Arms (tab 0)
-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 120, 32087, 'Pre-Raid', 'Warrior', 'Arms', 'Head', 'Both', 'Mask of the Deceiver'),
(1, 0, 1, 0, 120, 29349, 'Pre-Raid', 'Warrior', 'Arms', 'Neck', 'Both', 'Adamantine Chain of the Unbroken'),
(1, 0, 2, 0, 120, 33173, 'Pre-Raid', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Ragesteel Shoulders'),
(1, 0, 4, 0, 120, 23522, 'Pre-Raid', 'Warrior', 'Arms', 'Chest', 'Both', 'Ragesteel Breastplate'),
(1, 0, 5, 0, 120, 27985, 'Pre-Raid', 'Warrior', 'Arms', 'Waist', 'Both', 'Deathforge Girdle'),
(1, 0, 6, 0, 120, 30538, 'Pre-Raid', 'Warrior', 'Arms', 'Legs', 'Both', 'Midnight Legguards'),
(1, 0, 7, 0, 120, 25686, 'Pre-Raid', 'Warrior', 'Arms', 'Feet', 'Both', 'Fel Leather Boots'),
(1, 0, 8, 0, 120, 23537, 'Pre-Raid', 'Warrior', 'Arms', 'Wrists', 'Both', 'Black Felsteel Bracers'),
(1, 0, 9, 0, 120, 25685, 'Pre-Raid', 'Warrior', 'Arms', 'Hands', 'Both', 'Fel Leather Gloves'),
(1, 0, 10, 0, 120, 29379, 'Pre-Raid', 'Warrior', 'Arms', 'Finger1', 'Both', 'Ring of Arathi Warlords'),
(1, 0, 11, 0, 120, 30834, 'Pre-Raid', 'Warrior', 'Arms', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 0, 12, 0, 120, 29383, 'Pre-Raid', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 0, 13, 0, 120, 28034, 'Pre-Raid', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Hourglass of the Unraveller'),
(1, 0, 14, 0, 120, 24259, 'Pre-Raid', 'Warrior', 'Arms', 'Back', 'Both', 'Vengeance Wrap'),
(1, 0, 15, 0, 120, 28438, 'Pre-Raid', 'Warrior', 'Arms', 'MainHand', 'Both', 'Dragonmaw'),
(1, 0, 16, 0, 120, 23542, 'Pre-Raid', 'Warrior', 'Arms', 'OffHand', 'Both', 'Fel Edged Battleaxe'),
(1, 0, 17, 0, 120, 30279, 'Pre-Raid', 'Warrior', 'Arms', 'Ranged', 'Both', 'Mama''s Insurance');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 125, 29021, 'Phase 1', 'Warrior', 'Arms', 'Head', 'Both', 'Warbringer Battle-Helm'),
(1, 0, 1, 0, 125, 29349, 'Phase 1', 'Warrior', 'Arms', 'Neck', 'Both', 'Adamantine Chain of the Unbroken'),
(1, 0, 2, 0, 125, 30740, 'Phase 1', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Ripfiend Shoulderplates'),
(1, 0, 4, 0, 125, 29019, 'Phase 1', 'Warrior', 'Arms', 'Chest', 'Both', 'Warbringer Breastplate'),
(1, 0, 5, 0, 125, 28779, 'Phase 1', 'Warrior', 'Arms', 'Waist', 'Both', 'Girdle of the Endless Pit'),
(1, 0, 6, 0, 125, 28741, 'Phase 1', 'Warrior', 'Arms', 'Legs', 'Both', 'Skulker''s Greaves'),
(1, 0, 7, 0, 125, 28608, 'Phase 1', 'Warrior', 'Arms', 'Feet', 'Both', 'Ironstriders of Urgency'),
(1, 0, 8, 0, 125, 28795, 'Phase 1', 'Warrior', 'Arms', 'Wrists', 'Both', 'Bladespire Warbands'),
(1, 0, 9, 0, 125, 28824, 'Phase 1', 'Warrior', 'Arms', 'Hands', 'Both', 'Gauntlets of Martial Perfection'),
(1, 0, 10, 0, 125, 28757, 'Phase 1', 'Warrior', 'Arms', 'Finger1', 'Both', 'Ring of a Thousand Marks'),
(1, 0, 11, 0, 125, 30834, 'Phase 1', 'Warrior', 'Arms', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 0, 12, 0, 125, 29383, 'Phase 1', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 0, 13, 0, 125, 28830, 'Phase 1', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(1, 0, 14, 0, 125, 24259, 'Phase 1', 'Warrior', 'Arms', 'Back', 'Both', 'Vengeance Wrap'),
(1, 0, 15, 0, 125, 28438, 'Phase 1', 'Warrior', 'Arms', 'MainHand', 'Both', 'Dragonmaw'),
(1, 0, 16, 0, 125, 31332, 'Phase 1', 'Warrior', 'Arms', 'OffHand', 'Both', 'Blinkstrike'),
(1, 0, 17, 0, 125, 28772, 'Phase 1', 'Warrior', 'Arms', 'Ranged', 'Both', 'Sunfury Bow of the Phoenix');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 141, 30120, 'Phase 2', 'Warrior', 'Arms', 'Head', 'Both', 'Destroyer Battle-Helm'),
(1, 0, 1, 0, 141, 30022, 'Phase 2', 'Warrior', 'Arms', 'Neck', 'Both', 'Pendant of the Perilous'),
(1, 0, 2, 0, 141, 30122, 'Phase 2', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Destroyer Shoulderblades'),
(1, 0, 4, 0, 141, 30118, 'Phase 2', 'Warrior', 'Arms', 'Chest', 'Both', 'Destroyer Breastplate'),
(1, 0, 5, 0, 141, 30106, 'Phase 2', 'Warrior', 'Arms', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 0, 6, 0, 141, 29995, 'Phase 2', 'Warrior', 'Arms', 'Legs', 'Both', 'Leggings of Murderous Intent'),
(1, 0, 7, 0, 141, 30081, 'Phase 2', 'Warrior', 'Arms', 'Feet', 'Both', 'Warboots of Obliteration'),
(1, 0, 8, 0, 141, 30057, 'Phase 2', 'Warrior', 'Arms', 'Wrists', 'Both', 'Bracers of Eradication'),
(1, 0, 9, 0, 141, 30119, 'Phase 2', 'Warrior', 'Arms', 'Hands', 'Both', 'Destroyer Gauntlets'),
(1, 0, 10, 0, 141, 29997, 'Phase 2', 'Warrior', 'Arms', 'Finger1', 'Both', 'Band of the Ranger-General'),
(1, 0, 11, 0, 141, 30834, 'Phase 2', 'Warrior', 'Arms', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 0, 12, 0, 141, 29383, 'Phase 2', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 0, 13, 0, 141, 28830, 'Phase 2', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(1, 0, 14, 0, 141, 24259, 'Phase 2', 'Warrior', 'Arms', 'Back', 'Both', 'Vengeance Wrap'),
(1, 0, 15, 0, 141, 28439, 'Phase 2', 'Warrior', 'Arms', 'MainHand', 'Both', 'Dragonstrike'),
(1, 0, 16, 0, 141, 30082, 'Phase 2', 'Warrior', 'Arms', 'OffHand', 'Both', 'Talon of Azshara'),
(1, 0, 17, 0, 141, 30105, 'Phase 2', 'Warrior', 'Arms', 'Ranged', 'Both', 'Serpent Spine Longbow');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 156, 32235, 'Phase 3', 'Warrior', 'Arms', 'Head', 'Both', 'Cursed Vision of Sargeras'),
(1, 0, 1, 0, 156, 32591, 'Phase 3', 'Warrior', 'Arms', 'Neck', 'Both', 'Choker of Serrated Blades'),
(1, 0, 2, 0, 156, 30979, 'Phase 3', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Onslaught Shoulderblades'),
(1, 0, 4, 0, 156, 30975, 'Phase 3', 'Warrior', 'Arms', 'Chest', 'Both', 'Onslaught Breastplate'),
(1, 0, 5, 0, 156, 30106, 'Phase 3', 'Warrior', 'Arms', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 0, 6, 0, 156, 32341, 'Phase 3', 'Warrior', 'Arms', 'Legs', 'Both', 'Leggings of Divine Retribution'),
(1, 0, 7, 0, 156, 32345, 'Phase 3', 'Warrior', 'Arms', 'Feet', 'Both', 'Dreadboots of the Legion'),
(1, 0, 8, 0, 156, 30863, 'Phase 3', 'Warrior', 'Arms', 'Wrists', 'Both', 'Deadly Cuffs'),
(1, 0, 9, 0, 156, 32278, 'Phase 3', 'Warrior', 'Arms', 'Hands', 'Both', 'Grips of Silent Justice'),
(1, 0, 10, 0, 156, 32497, 'Phase 3', 'Warrior', 'Arms', 'Finger1', 'Both', 'Stormrage Signet Ring'),
(1, 0, 11, 0, 156, 32335, 'Phase 3', 'Warrior', 'Arms', 'Finger2', 'Both', 'Unstoppable Aggressor''s Ring'),
(1, 0, 12, 0, 156, 28830, 'Phase 3', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Dragonspine Trophy'),
(1, 0, 13, 0, 156, 32505, 'Phase 3', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Madness of the Betrayer'),
(1, 0, 14, 0, 156, 32323, 'Phase 3', 'Warrior', 'Arms', 'Back', 'Both', 'Shadowmoon Destroyer''s Drape'),
(1, 0, 15, 0, 156, 32837, 'Phase 3', 'Warrior', 'Arms', 'MainHand', 'Both', 'Warglaive of Azzinoth'),
(1, 0, 16, 0, 156, 32838, 'Phase 3', 'Warrior', 'Arms', 'OffHand', 'Both', 'Warglaive of Azzinoth'),
(1, 0, 17, 0, 156, 32326, 'Phase 3', 'Warrior', 'Arms', 'Ranged', 'Both', 'Twisted Blades of Zarak');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 164, 32235, 'Phase 4', 'Warrior', 'Arms', 'Head', 'Both', 'Cursed Vision of Sargeras'),
(1, 0, 1, 0, 164, 32591, 'Phase 4', 'Warrior', 'Arms', 'Neck', 'Both', 'Choker of Serrated Blades'),
(1, 0, 2, 0, 164, 30979, 'Phase 4', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Onslaught Shoulderblades'),
(1, 0, 4, 0, 164, 30975, 'Phase 4', 'Warrior', 'Arms', 'Chest', 'Both', 'Onslaught Breastplate'),
(1, 0, 5, 0, 164, 30106, 'Phase 4', 'Warrior', 'Arms', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 0, 6, 0, 164, 32341, 'Phase 4', 'Warrior', 'Arms', 'Legs', 'Both', 'Leggings of Divine Retribution'),
(1, 0, 7, 0, 164, 32345, 'Phase 4', 'Warrior', 'Arms', 'Feet', 'Both', 'Dreadboots of the Legion'),
(1, 0, 8, 0, 164, 30863, 'Phase 4', 'Warrior', 'Arms', 'Wrists', 'Both', 'Deadly Cuffs'),
(1, 0, 9, 0, 164, 32278, 'Phase 4', 'Warrior', 'Arms', 'Hands', 'Both', 'Grips of Silent Justice'),
(1, 0, 10, 0, 164, 33496, 'Phase 4', 'Warrior', 'Arms', 'Finger1', 'Both', 'Signet of Primal Wrath'),
(1, 0, 11, 0, 164, 32497, 'Phase 4', 'Warrior', 'Arms', 'Finger2', 'Both', 'Stormrage Signet Ring'),
(1, 0, 12, 0, 164, 28830, 'Phase 4', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Dragonspine Trophy'),
(1, 0, 13, 0, 164, 32505, 'Phase 4', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Madness of the Betrayer'),
(1, 0, 14, 0, 164, 32323, 'Phase 4', 'Warrior', 'Arms', 'Back', 'Both', 'Shadowmoon Destroyer''s Drape'),
(1, 0, 15, 0, 164, 32837, 'Phase 4', 'Warrior', 'Arms', 'MainHand', 'Both', 'Warglaive of Azzinoth'),
(1, 0, 16, 0, 164, 32838, 'Phase 4', 'Warrior', 'Arms', 'OffHand', 'Both', 'Warglaive of Azzinoth'),
(1, 0, 17, 0, 164, 33474, 'Phase 4', 'Warrior', 'Arms', 'Ranged', 'Both', 'Ancient Amani Longbow');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 200, 41386, 'Pre-Raid', 'Warrior', 'Arms', 'Head', 'Both', 'Spiked Titansteel Helm'),
(1, 0, 1, 0, 200, 42645, 'Pre-Raid', 'Warrior', 'Arms', 'Neck', 'Both', 'Titanium Impact Choker'),
(1, 0, 2, 0, 200, 37627, 'Pre-Raid', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Snake Den Spaulders'),
(1, 0, 4, 0, 200, 37612, 'Pre-Raid', 'Warrior', 'Arms', 'Chest', 'Both', 'Bonegrinder Breastplate'),
(1, 0, 5, 0, 200, 37171, 'Pre-Raid', 'Warrior', 'Arms', 'Waist', 'Both', 'Flame-Bathed Steel Girdle'),
(1, 0, 6, 0, 200, 37193, 'Pre-Raid', 'Warrior', 'Arms', 'Legs', 'Both', 'Staggering Legplates'),
(1, 0, 7, 0, 200, 41391, 'Pre-Raid', 'Warrior', 'Arms', 'Feet', 'Both', 'Spiked Titansteel Treads'),
(1, 0, 8, 0, 200, 37668, 'Pre-Raid', 'Warrior', 'Arms', 'Wrists', 'Both', 'Bands of the Stoneforge'),
(1, 0, 9, 0, 200, 37363, 'Pre-Raid', 'Warrior', 'Arms', 'Hands', 'Both', 'Gauntlets of Dragon Wrath'),
(1, 0, 10, 0, 200, 37642, 'Pre-Raid', 'Warrior', 'Arms', 'Finger1', 'Both', 'Hemorrhaging Circle'),
(1, 0, 12, 0, 200, 40684, 'Pre-Raid', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Mirror of Truth'),
(1, 0, 14, 0, 200, 43566, 'Pre-Raid', 'Warrior', 'Arms', 'Back', 'Both', 'Ice Striker''s Cloak'),
(1, 0, 15, 0, 200, 41257, 'Pre-Raid', 'Warrior', 'Arms', 'MainHand', 'Both', 'Titansteel Destroyer');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 224, 40528, 'Phase 1', 'Warrior', 'Arms', 'Head', 'Both', 'Valorous Dreadnaught Helmet'),
(1, 0, 1, 0, 224, 44664, 'Phase 1', 'Warrior', 'Arms', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(1, 0, 2, 0, 224, 40530, 'Phase 1', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Valorous Dreadnaught Shoulderplates'),
(1, 0, 4, 0, 224, 40525, 'Phase 1', 'Warrior', 'Arms', 'Chest', 'Both', 'Valorous Dreadnaught Battleplate'),
(1, 0, 5, 0, 224, 40317, 'Phase 1', 'Warrior', 'Arms', 'Waist', 'Both', 'Girdle of Razuvious'),
(1, 0, 6, 0, 224, 40318, 'Phase 1', 'Warrior', 'Arms', 'Legs', 'Both', 'Legplates of Double Strikes'),
(1, 0, 7, 0, 224, 40206, 'Phase 1', 'Warrior', 'Arms', 'Feet', 'Both', 'Iron-Spring Jumpers'),
(1, 0, 8, 0, 224, 40733, 'Phase 1', 'Warrior', 'Arms', 'Wrists', 'Both', 'Wristbands of the Sentinel Huntress'),
(1, 0, 9, 0, 224, 40527, 'Phase 1', 'Warrior', 'Arms', 'Hands', 'Both', 'Valorous Dreadnaught Gauntlets'),
(1, 0, 10, 0, 224, 40075, 'Phase 1', 'Warrior', 'Arms', 'Finger1', 'Both', 'Ruthlessness'),
(1, 0, 12, 0, 224, 40256, 'Phase 1', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Grim Toll'),
(1, 0, 14, 0, 224, 40250, 'Phase 1', 'Warrior', 'Arms', 'Back', 'Both', 'Aged Winter Cloak');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 245, 46151, 'Phase 2', 'Warrior', 'Arms', 'Head', 'Both', 'Conqueror''s Siegebreaker Helmet'),
(1, 0, 1, 0, 245, 45459, 'Phase 2', 'Warrior', 'Arms', 'Neck', 'Both', 'Frigid Strength of Hodir'),
(1, 0, 2, 0, 245, 46149, 'Phase 2', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Conqueror''s Siegebreaker Shoulderplates'),
(1, 0, 4, 0, 245, 46146, 'Phase 2', 'Warrior', 'Arms', 'Chest', 'Both', 'Conqueror''s Siegebreaker Battleplate'),
(1, 0, 5, 0, 245, 45241, 'Phase 2', 'Warrior', 'Arms', 'Waist', 'Both', 'Belt of Colossal Rage'),
(1, 0, 6, 0, 245, 45536, 'Phase 2', 'Warrior', 'Arms', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(1, 0, 7, 0, 245, 45599, 'Phase 2', 'Warrior', 'Arms', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(1, 0, 8, 0, 245, 45663, 'Phase 2', 'Warrior', 'Arms', 'Wrists', 'Both', 'Armbands of Bedlam'),
(1, 0, 9, 0, 245, 46148, 'Phase 2', 'Warrior', 'Arms', 'Hands', 'Both', 'Conqueror''s Siegebreaker Gauntlets'),
(1, 0, 10, 0, 245, 45608, 'Phase 2', 'Warrior', 'Arms', 'Finger1', 'Both', 'Brann''s Signet Ring'),
(1, 0, 12, 0, 245, 46038, 'Phase 2', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Dark Matter'),
(1, 0, 14, 0, 245, 46032, 'Phase 2', 'Warrior', 'Arms', 'Back', 'Both', 'Drape of the Faceless General'),
(1, 0, 17, 0, 245, 45296, 'Phase 2', 'Warrior', 'Arms', 'Ranged', 'Both', 'Twirling Blades');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 1, 258, 48383, 'Phase 3', 'Warrior', 'Arms', 'Head', 'Alliance', 'Wrynn''s Helmet of Triumph'),
(1, 0, 0, 2, 258, 48398, 'Phase 3', 'Warrior', 'Arms', 'Head', 'Horde', 'Hellscream''s Helmet of Triumph'),
(1, 0, 1, 1, 258, 47915, 'Phase 3', 'Warrior', 'Arms', 'Neck', 'Alliance', 'Collar of Ceaseless Torment'),
(1, 0, 1, 2, 258, 47988, 'Phase 3', 'Warrior', 'Arms', 'Neck', 'Horde', 'Collar of Unending Torment'),
(1, 0, 2, 1, 258, 48381, 'Phase 3', 'Warrior', 'Arms', 'Shoulders', 'Alliance', 'Wrynn''s Shoulderplates of Triumph'),
(1, 0, 2, 2, 258, 48400, 'Phase 3', 'Warrior', 'Arms', 'Shoulders', 'Horde', 'Hellscream''s Shoulderplates of Triumph'),
(1, 0, 4, 1, 258, 48385, 'Phase 3', 'Warrior', 'Arms', 'Chest', 'Alliance', 'Wrynn''s Battleplate of Triumph'),
(1, 0, 4, 2, 258, 48396, 'Phase 3', 'Warrior', 'Arms', 'Chest', 'Horde', 'Hellscream''s Battleplate of Triumph'),
(1, 0, 5, 1, 258, 47153, 'Phase 3', 'Warrior', 'Arms', 'Waist', 'Alliance', 'Belt of Deathly Dominion'),
(1, 0, 5, 2, 258, 47472, 'Phase 3', 'Warrior', 'Arms', 'Waist', 'Horde', 'Waistguard of Deathly Dominion'),
(1, 0, 6, 1, 258, 48382, 'Phase 3', 'Warrior', 'Arms', 'Legs', 'Alliance', 'Wrynn''s Legplates of Triumph'),
(1, 0, 6, 2, 258, 48399, 'Phase 3', 'Warrior', 'Arms', 'Legs', 'Horde', 'Hellscream''s Legplates of Triumph'),
(1, 0, 7, 1, 258, 47077, 'Phase 3', 'Warrior', 'Arms', 'Feet', 'Alliance', 'Treads of the Icewalker'),
(1, 0, 7, 2, 258, 47445, 'Phase 3', 'Warrior', 'Arms', 'Feet', 'Horde', 'Icewalker Treads'),
(1, 0, 8, 1, 258, 47074, 'Phase 3', 'Warrior', 'Arms', 'Wrists', 'Alliance', 'Bracers of the Untold Massacre'),
(1, 0, 8, 2, 258, 47442, 'Phase 3', 'Warrior', 'Arms', 'Wrists', 'Horde', 'Bracers of the Silent Massacre'),
(1, 0, 9, 1, 258, 47240, 'Phase 3', 'Warrior', 'Arms', 'Hands', 'Alliance', 'Gloves of Bitter Reprisal'),
(1, 0, 9, 2, 258, 47492, 'Phase 3', 'Warrior', 'Arms', 'Hands', 'Horde', 'Gauntlets of Bitter Reprisal'),
(1, 0, 10, 0, 258, 45608, 'Phase 3', 'Warrior', 'Arms', 'Finger1', 'Both', 'Brann''s Signet Ring'),
(1, 0, 12, 0, 258, 46038, 'Phase 3', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Dark Matter'),
(1, 0, 14, 1, 258, 47545, 'Phase 3', 'Warrior', 'Arms', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(1, 0, 14, 2, 258, 47546, 'Phase 3', 'Warrior', 'Arms', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(1, 0, 17, 1, 258, 46995, 'Phase 3', 'Warrior', 'Arms', 'Ranged', 'Alliance', 'Talonstrike'),
(1, 0, 17, 2, 258, 47428, 'Phase 3', 'Warrior', 'Arms', 'Ranged', 'Horde', 'Death''s Head Crossbow');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 264, 51227, 'Phase 4', 'Warrior', 'Arms', 'Head', 'Both', 'Sanctified Ymirjar Lord''s Helmet'),
(1, 0, 1, 0, 264, 50728, 'Phase 4', 'Warrior', 'Arms', 'Neck', 'Both', 'Lana''thel''s Chain of Flagellation'),
(1, 0, 2, 0, 264, 51229, 'Phase 4', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Sanctified Ymirjar Lord''s Shoulderplates'),
(1, 0, 4, 0, 264, 51225, 'Phase 4', 'Warrior', 'Arms', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Battleplate'),
(1, 0, 5, 0, 264, 50707, 'Phase 4', 'Warrior', 'Arms', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(1, 0, 6, 0, 264, 50645, 'Phase 4', 'Warrior', 'Arms', 'Legs', 'Both', 'Leggings of Northern Lights'),
(1, 0, 7, 0, 264, 50639, 'Phase 4', 'Warrior', 'Arms', 'Feet', 'Both', 'Blood-Soaked Saronite Stompers'),
(1, 0, 8, 0, 264, 50670, 'Phase 4', 'Warrior', 'Arms', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(1, 0, 9, 0, 264, 51226, 'Phase 4', 'Warrior', 'Arms', 'Hands', 'Both', 'Sanctified Ymirjar Lord''s Gauntlets'),
(1, 0, 10, 0, 264, 50402, 'Phase 4', 'Warrior', 'Arms', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(1, 0, 12, 0, 264, 50363, 'Phase 4', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(1, 0, 14, 1, 264, 47545, 'Phase 4', 'Warrior', 'Arms', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(1, 0, 14, 2, 264, 47546, 'Phase 4', 'Warrior', 'Arms', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(1, 0, 17, 0, 264, 50733, 'Phase 4', 'Warrior', 'Arms', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 0, 0, 0, 290, 51227, 'Phase 5', 'Warrior', 'Arms', 'Head', 'Both', 'Sanctified Ymirjar Lord''s Helmet'),
(1, 0, 1, 0, 290, 54581, 'Phase 5', 'Warrior', 'Arms', 'Neck', 'Both', 'Penumbra Pendant'),
(1, 0, 2, 0, 290, 51229, 'Phase 5', 'Warrior', 'Arms', 'Shoulders', 'Both', 'Sanctified Ymirjar Lord''s Shoulderplates'),
(1, 0, 4, 0, 290, 51225, 'Phase 5', 'Warrior', 'Arms', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Battleplate'),
(1, 0, 5, 0, 290, 50620, 'Phase 5', 'Warrior', 'Arms', 'Waist', 'Both', 'Coldwraith Links'),
(1, 0, 6, 0, 290, 51228, 'Phase 5', 'Warrior', 'Arms', 'Legs', 'Both', 'Sanctified Ymirjar Lord''s Legplates'),
(1, 0, 7, 0, 290, 54578, 'Phase 5', 'Warrior', 'Arms', 'Feet', 'Both', 'Apocalypse''s Advance'),
(1, 0, 8, 0, 290, 50670, 'Phase 5', 'Warrior', 'Arms', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(1, 0, 9, 0, 290, 50675, 'Phase 5', 'Warrior', 'Arms', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(1, 0, 10, 0, 290, 50657, 'Phase 5', 'Warrior', 'Arms', 'Finger1', 'Both', 'Skeleton Lord''s Circle'),
(1, 0, 11, 0, 290, 52572, 'Phase 5', 'Warrior', 'Arms', 'Finger2', 'Both', 'Ashen Band of Endless Might'),
(1, 0, 12, 0, 290, 50363, 'Phase 5', 'Warrior', 'Arms', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(1, 0, 13, 0, 290, 54590, 'Phase 5', 'Warrior', 'Arms', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(1, 0, 14, 0, 290, 47545, 'Phase 5', 'Warrior', 'Arms', 'Back', 'Both', 'Vereesa''s Dexterity'),
(1, 0, 15, 0, 290, 49623, 'Phase 5', 'Warrior', 'Arms', 'MainHand', 'Both', 'Shadowmourne'),
(1, 0, 17, 0, 290, 50733, 'Phase 5', 'Warrior', 'Arms', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Fury (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Head', 'Both', 'Mask of the Unforgiven'),
(1, 1, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Neck', 'Both', 'Mark of Fordring'),
(1, 1, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(1, 1, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Chest', 'Both', 'Savage Gladiator Chain'),
(1, 1, 5, 0, 66, 13142, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Waist', 'Both', 'Brigam Girdle'),
(1, 1, 6, 0, 66, 14554, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(1, 1, 7, 0, 66, 14616, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Feet', 'Both', 'Bloodmail Boots'),
(1, 1, 8, 0, 66, 12936, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Wrists', 'Both', 'Battleborn Armbraces'),
(1, 1, 9, 0, 66, 14551, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Hands', 'Both', 'Edgemaster''s Handguards'),
(1, 1, 10, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Finger1', 'Both', 'Blackstone Ring'),
(1, 1, 11, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Finger2', 'Both', 'Painweaver Band'),
(1, 1, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(1, 1, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Hand of Justice'),
(1, 1, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Back', 'Both', 'Cape of the Black Baron'),
(1, 1, 15, 0, 66, 12940, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'MainHand', 'Both', 'Dal''Rend''s Sacred Charge'),
(1, 1, 16, 0, 66, 12939, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'OffHand', 'Both', 'Dal''Rend''s Tribal Guardian'),
(1, 1, 17, 0, 66, 12651, 'Phase 1 (Pre-Raid)', 'Warrior', 'Fury', 'Ranged', 'Both', 'Blackcrow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 76, 13404, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Head', 'Both', 'Mask of the Unforgiven'),
(1, 1, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Neck', 'Both', 'Mark of Fordring'),
(1, 1, 2, 0, 76, 12927, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(1, 1, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Chest', 'Both', 'Savage Gladiator Chain'),
(1, 1, 5, 0, 76, 13142, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Waist', 'Both', 'Brigam Girdle'),
(1, 1, 6, 0, 76, 14554, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(1, 1, 7, 0, 76, 14616, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Feet', 'Both', 'Bloodmail Boots'),
(1, 1, 8, 0, 76, 12936, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Wrists', 'Both', 'Battleborn Armbraces'),
(1, 1, 9, 0, 76, 14551, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Hands', 'Both', 'Edgemaster''s Handguards'),
(1, 1, 10, 0, 76, 17713, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Finger1', 'Both', 'Blackstone Ring'),
(1, 1, 11, 0, 76, 13098, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Finger2', 'Both', 'Painweaver Band'),
(1, 1, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(1, 1, 13, 0, 76, 11815, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Hand of Justice'),
(1, 1, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Back', 'Both', 'Cape of the Black Baron'),
(1, 1, 15, 0, 76, 12940, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'MainHand', 'Both', 'Dal''Rend''s Sacred Charge'),
(1, 1, 16, 0, 76, 12939, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'OffHand', 'Both', 'Dal''Rend''s Tribal Guardian'),
(1, 1, 17, 0, 76, 18323, 'Phase 2 (Pre-Raid)', 'Warrior', 'Fury', 'Ranged', 'Both', 'Satyr''s Bow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 78, 12640, 'Phase 2', 'Warrior', 'Fury', 'Head', 'Both', 'Lionheart Helm'),
(1, 1, 1, 0, 78, 18404, 'Phase 2', 'Warrior', 'Fury', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(1, 1, 2, 0, 78, 12927, 'Phase 2', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(1, 1, 4, 0, 78, 11726, 'Phase 2', 'Warrior', 'Fury', 'Chest', 'Both', 'Savage Gladiator Chain'),
(1, 1, 5, 0, 78, 19137, 'Phase 2', 'Warrior', 'Fury', 'Waist', 'Both', 'Onslaught Girdle'),
(1, 1, 6, 0, 78, 14554, 'Phase 2', 'Warrior', 'Fury', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(1, 1, 7, 0, 78, 14616, 'Phase 2', 'Warrior', 'Fury', 'Feet', 'Both', 'Bloodmail Boots'),
(1, 1, 8, 0, 78, 19146, 'Phase 2', 'Warrior', 'Fury', 'Wrists', 'Both', 'Wristguards of Stability'),
(1, 1, 9, 0, 78, 14551, 'Phase 2', 'Warrior', 'Fury', 'Hands', 'Both', 'Edgemaster''s Handguards'),
(1, 1, 10, 0, 78, 17063, 'Phase 2', 'Warrior', 'Fury', 'Finger1', 'Both', 'Band of Accuria'),
(1, 1, 11, 0, 78, 18821, 'Phase 2', 'Warrior', 'Fury', 'Finger2', 'Both', 'Quick Strike Ring'),
(1, 1, 12, 0, 78, 13965, 'Phase 2', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(1, 1, 13, 0, 78, 11815, 'Phase 2', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Hand of Justice'),
(1, 1, 14, 0, 78, 18541, 'Phase 2', 'Warrior', 'Fury', 'Back', 'Both', 'Puissant Cape'),
(1, 1, 15, 0, 78, 17075, 'Phase 2', 'Warrior', 'Fury', 'MainHand', 'Both', 'Vis''kag the Bloodletter'),
(1, 1, 16, 0, 78, 18832, 'Phase 2', 'Warrior', 'Fury', 'OffHand', 'Both', 'Brutality Blade'),
(1, 1, 17, 0, 78, 17069, 'Phase 2', 'Warrior', 'Fury', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 83, 12640, 'Phase 3', 'Warrior', 'Fury', 'Head', 'Both', 'Lionheart Helm'),
(1, 1, 1, 0, 83, 18404, 'Phase 3', 'Warrior', 'Fury', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(1, 1, 2, 0, 83, 19394, 'Phase 3', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Drake Talon Pauldrons'),
(1, 1, 4, 0, 83, 11726, 'Phase 3', 'Warrior', 'Fury', 'Chest', 'Both', 'Savage Gladiator Chain'),
(1, 1, 5, 0, 83, 19137, 'Phase 3', 'Warrior', 'Fury', 'Waist', 'Both', 'Onslaught Girdle'),
(1, 1, 6, 0, 83, 19402, 'Phase 3', 'Warrior', 'Fury', 'Legs', 'Both', 'Legguards of the Fallen Crusader'),
(1, 1, 7, 0, 83, 19387, 'Phase 3', 'Warrior', 'Fury', 'Feet', 'Both', 'Chromatic Boots'),
(1, 1, 8, 0, 83, 19146, 'Phase 3', 'Warrior', 'Fury', 'Wrists', 'Both', 'Wristguards of Stability'),
(1, 1, 9, 0, 83, 14551, 'Phase 3', 'Warrior', 'Fury', 'Hands', 'Both', 'Edgemaster''s Handguards'),
(1, 1, 10, 0, 83, 17063, 'Phase 3', 'Warrior', 'Fury', 'Finger1', 'Both', 'Band of Accuria'),
(1, 1, 11, 0, 83, 19384, 'Phase 3', 'Warrior', 'Fury', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(1, 1, 12, 0, 83, 19406, 'Phase 3', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(1, 1, 13, 0, 83, 11815, 'Phase 3', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Hand of Justice'),
(1, 1, 14, 0, 83, 18541, 'Phase 3', 'Warrior', 'Fury', 'Back', 'Both', 'Puissant Cape'),
(1, 1, 15, 0, 83, 19352, 'Phase 3', 'Warrior', 'Fury', 'MainHand', 'Both', 'Chromatically Tempered Sword'),
(1, 1, 16, 0, 83, 19351, 'Phase 3', 'Warrior', 'Fury', 'OffHand', 'Both', 'Maladath, Runed Blade of the Black Flight'),
(1, 1, 17, 0, 83, 17069, 'Phase 3', 'Warrior', 'Fury', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 88, 12640, 'Phase 5', 'Warrior', 'Fury', 'Head', 'Both', 'Lionheart Helm'),
(1, 1, 1, 0, 88, 18404, 'Phase 5', 'Warrior', 'Fury', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(1, 1, 2, 0, 88, 21330, 'Phase 5', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Conqueror''s Spaulders'),
(1, 1, 4, 0, 88, 21814, 'Phase 5', 'Warrior', 'Fury', 'Chest', 'Both', 'Breastplate of Annihilation'),
(1, 1, 5, 0, 88, 19137, 'Phase 5', 'Warrior', 'Fury', 'Waist', 'Both', 'Onslaught Girdle'),
(1, 1, 6, 0, 88, 22385, 'Phase 5', 'Warrior', 'Fury', 'Legs', 'Both', 'Titanic Leggings'),
(1, 1, 7, 0, 88, 19387, 'Phase 5', 'Warrior', 'Fury', 'Feet', 'Both', 'Chromatic Boots'),
(1, 1, 8, 0, 88, 21618, 'Phase 5', 'Warrior', 'Fury', 'Wrists', 'Both', 'Hive Defiler Wristguards'),
(1, 1, 9, 0, 88, 14551, 'Phase 5', 'Warrior', 'Fury', 'Hands', 'Both', 'Edgemaster''s Handguards'),
(1, 1, 10, 0, 88, 17063, 'Phase 5', 'Warrior', 'Fury', 'Finger1', 'Both', 'Band of Accuria'),
(1, 1, 11, 0, 88, 19384, 'Phase 5', 'Warrior', 'Fury', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(1, 1, 12, 0, 88, 19406, 'Phase 5', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(1, 1, 13, 0, 88, 21670, 'Phase 5', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Badge of the Swarmguard'),
(1, 1, 14, 0, 88, 18541, 'Phase 5', 'Warrior', 'Fury', 'Back', 'Both', 'Puissant Cape'),
(1, 1, 15, 0, 88, 21650, 'Phase 5', 'Warrior', 'Fury', 'MainHand', 'Both', 'Ancient Qiraji Ripper'),
(1, 1, 16, 0, 88, 21650, 'Phase 5', 'Warrior', 'Fury', 'OffHand', 'Both', 'Ancient Qiraji Ripper'),
(1, 1, 17, 0, 88, 17069, 'Phase 5', 'Warrior', 'Fury', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 92, 12640, 'Phase 6', 'Warrior', 'Fury', 'Head', 'Both', 'Lionheart Helm'),
(1, 1, 1, 0, 92, 18404, 'Phase 6', 'Warrior', 'Fury', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(1, 1, 2, 0, 92, 21330, 'Phase 6', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Conqueror''s Spaulders'),
(1, 1, 4, 0, 92, 23000, 'Phase 6', 'Warrior', 'Fury', 'Chest', 'Both', 'Plated Abomination Ribcage'),
(1, 1, 5, 0, 92, 19137, 'Phase 6', 'Warrior', 'Fury', 'Waist', 'Both', 'Onslaught Girdle'),
(1, 1, 6, 0, 92, 22385, 'Phase 6', 'Warrior', 'Fury', 'Legs', 'Both', 'Titanic Leggings'),
(1, 1, 7, 0, 92, 19387, 'Phase 6', 'Warrior', 'Fury', 'Feet', 'Both', 'Chromatic Boots'),
(1, 1, 8, 0, 92, 22936, 'Phase 6', 'Warrior', 'Fury', 'Wrists', 'Both', 'Wristguards of Vengeance'),
(1, 1, 9, 0, 92, 21581, 'Phase 6', 'Warrior', 'Fury', 'Hands', 'Both', 'Gauntlets of Annihilation'),
(1, 1, 11, 0, 92, 19384, 'Phase 6', 'Warrior', 'Fury', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(1, 1, 12, 0, 92, 19406, 'Phase 6', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(1, 1, 13, 0, 92, 21670, 'Phase 6', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Badge of the Swarmguard'),
(1, 1, 14, 0, 92, 23045, 'Phase 6', 'Warrior', 'Fury', 'Back', 'Both', 'Shroud of Dominion'),
(1, 1, 15, 0, 92, 23054, 'Phase 6', 'Warrior', 'Fury', 'MainHand', 'Both', 'Gressil, Dawn of Ruin'),
(1, 1, 16, 0, 92, 23577, 'Phase 6', 'Warrior', 'Fury', 'OffHand', 'Both', 'The Hungering Cold'),
(1, 1, 17, 0, 92, 17069, 'Phase 6', 'Warrior', 'Fury', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 120, 32087, 'Pre-Raid', 'Warrior', 'Fury', 'Head', 'Both', 'Mask of the Deceiver'),
(1, 1, 1, 0, 120, 29349, 'Pre-Raid', 'Warrior', 'Fury', 'Neck', 'Both', 'Adamantine Chain of the Unbroken'),
(1, 1, 2, 0, 120, 33173, 'Pre-Raid', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Ragesteel Shoulders'),
(1, 1, 4, 0, 120, 23522, 'Pre-Raid', 'Warrior', 'Fury', 'Chest', 'Both', 'Ragesteel Breastplate'),
(1, 1, 5, 0, 120, 27985, 'Pre-Raid', 'Warrior', 'Fury', 'Waist', 'Both', 'Deathforge Girdle'),
(1, 1, 6, 0, 120, 30538, 'Pre-Raid', 'Warrior', 'Fury', 'Legs', 'Both', 'Midnight Legguards'),
(1, 1, 7, 0, 120, 25686, 'Pre-Raid', 'Warrior', 'Fury', 'Feet', 'Both', 'Fel Leather Boots'),
(1, 1, 8, 0, 120, 23537, 'Pre-Raid', 'Warrior', 'Fury', 'Wrists', 'Both', 'Black Felsteel Bracers'),
(1, 1, 9, 0, 120, 25685, 'Pre-Raid', 'Warrior', 'Fury', 'Hands', 'Both', 'Fel Leather Gloves'),
(1, 1, 10, 0, 120, 29379, 'Pre-Raid', 'Warrior', 'Fury', 'Finger1', 'Both', 'Ring of Arathi Warlords'),
(1, 1, 11, 0, 120, 30834, 'Pre-Raid', 'Warrior', 'Fury', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 1, 12, 0, 120, 29383, 'Pre-Raid', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 1, 13, 0, 120, 28034, 'Pre-Raid', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Hourglass of the Unraveller'),
(1, 1, 14, 0, 120, 24259, 'Pre-Raid', 'Warrior', 'Fury', 'Back', 'Both', 'Vengeance Wrap'),
(1, 1, 15, 0, 120, 28438, 'Pre-Raid', 'Warrior', 'Fury', 'MainHand', 'Both', 'Dragonmaw'),
(1, 1, 16, 0, 120, 23542, 'Pre-Raid', 'Warrior', 'Fury', 'OffHand', 'Both', 'Fel Edged Battleaxe'),
(1, 1, 17, 0, 120, 30279, 'Pre-Raid', 'Warrior', 'Fury', 'Ranged', 'Both', 'Mama''s Insurance');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 125, 29021, 'Phase 1', 'Warrior', 'Fury', 'Head', 'Both', 'Warbringer Battle-Helm'),
(1, 1, 1, 0, 125, 29349, 'Phase 1', 'Warrior', 'Fury', 'Neck', 'Both', 'Adamantine Chain of the Unbroken'),
(1, 1, 2, 0, 125, 29023, 'Phase 1', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Warbringer Shoulderplates'),
(1, 1, 4, 0, 125, 29019, 'Phase 1', 'Warrior', 'Fury', 'Chest', 'Both', 'Warbringer Breastplate'),
(1, 1, 5, 0, 125, 28779, 'Phase 1', 'Warrior', 'Fury', 'Waist', 'Both', 'Girdle of the Endless Pit'),
(1, 1, 6, 0, 125, 28741, 'Phase 1', 'Warrior', 'Fury', 'Legs', 'Both', 'Skulker''s Greaves'),
(1, 1, 7, 0, 125, 28608, 'Phase 1', 'Warrior', 'Fury', 'Feet', 'Both', 'Ironstriders of Urgency'),
(1, 1, 8, 0, 125, 28795, 'Phase 1', 'Warrior', 'Fury', 'Wrists', 'Both', 'Bladespire Warbands'),
(1, 1, 9, 0, 125, 28824, 'Phase 1', 'Warrior', 'Fury', 'Hands', 'Both', 'Gauntlets of Martial Perfection'),
(1, 1, 10, 0, 125, 28757, 'Phase 1', 'Warrior', 'Fury', 'Finger1', 'Both', 'Ring of a Thousand Marks'),
(1, 1, 11, 0, 125, 30834, 'Phase 1', 'Warrior', 'Fury', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 1, 12, 0, 125, 29383, 'Phase 1', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 1, 13, 0, 125, 28830, 'Phase 1', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(1, 1, 14, 0, 125, 24259, 'Phase 1', 'Warrior', 'Fury', 'Back', 'Both', 'Vengeance Wrap'),
(1, 1, 15, 0, 125, 28438, 'Phase 1', 'Warrior', 'Fury', 'MainHand', 'Both', 'Dragonmaw'),
(1, 1, 16, 0, 125, 31332, 'Phase 1', 'Warrior', 'Fury', 'OffHand', 'Both', 'Blinkstrike'),
(1, 1, 17, 0, 125, 30724, 'Phase 1', 'Warrior', 'Fury', 'Ranged', 'Both', 'Barrel-Blade Longrifle');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 141, 30120, 'Phase 2', 'Warrior', 'Fury', 'Head', 'Both', 'Destroyer Battle-Helm'),
(1, 1, 1, 0, 141, 30022, 'Phase 2', 'Warrior', 'Fury', 'Neck', 'Both', 'Pendant of the Perilous'),
(1, 1, 2, 0, 141, 30122, 'Phase 2', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Destroyer Shoulderblades'),
(1, 1, 4, 0, 141, 30118, 'Phase 2', 'Warrior', 'Fury', 'Chest', 'Both', 'Destroyer Breastplate'),
(1, 1, 5, 0, 141, 30106, 'Phase 2', 'Warrior', 'Fury', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 1, 6, 0, 141, 29995, 'Phase 2', 'Warrior', 'Fury', 'Legs', 'Both', 'Leggings of Murderous Intent'),
(1, 1, 7, 0, 141, 30081, 'Phase 2', 'Warrior', 'Fury', 'Feet', 'Both', 'Warboots of Obliteration'),
(1, 1, 8, 0, 141, 30057, 'Phase 2', 'Warrior', 'Fury', 'Wrists', 'Both', 'Bracers of Eradication'),
(1, 1, 9, 0, 141, 30119, 'Phase 2', 'Warrior', 'Fury', 'Hands', 'Both', 'Destroyer Gauntlets'),
(1, 1, 10, 0, 141, 29997, 'Phase 2', 'Warrior', 'Fury', 'Finger1', 'Both', 'Band of the Ranger-General'),
(1, 1, 11, 0, 141, 28757, 'Phase 2', 'Warrior', 'Fury', 'Finger2', 'Both', 'Ring of a Thousand Marks'),
(1, 1, 12, 0, 141, 29383, 'Phase 2', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(1, 1, 13, 0, 141, 28830, 'Phase 2', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(1, 1, 14, 0, 141, 24259, 'Phase 2', 'Warrior', 'Fury', 'Back', 'Both', 'Vengeance Wrap'),
(1, 1, 15, 0, 141, 28439, 'Phase 2', 'Warrior', 'Fury', 'MainHand', 'Both', 'Dragonstrike'),
(1, 1, 16, 0, 141, 30082, 'Phase 2', 'Warrior', 'Fury', 'OffHand', 'Both', 'Talon of Azshara'),
(1, 1, 17, 0, 141, 30105, 'Phase 2', 'Warrior', 'Fury', 'Ranged', 'Both', 'Serpent Spine Longbow');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 200, 41386, 'Pre-Raid', 'Warrior', 'Fury', 'Head', 'Both', 'Spiked Titansteel Helm'),
(1, 1, 1, 0, 200, 42645, 'Pre-Raid', 'Warrior', 'Fury', 'Neck', 'Both', 'Titanium Impact Choker'),
(1, 1, 2, 0, 200, 37627, 'Pre-Raid', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Snake Den Spaulders'),
(1, 1, 4, 0, 200, 39606, 'Pre-Raid', 'Warrior', 'Fury', 'Chest', 'Both', 'Heroes'' Dreadnaught Battleplate'),
(1, 1, 5, 0, 200, 37826, 'Pre-Raid', 'Warrior', 'Fury', 'Waist', 'Both', 'The General''s Steel Girdle'),
(1, 1, 6, 0, 200, 39607, 'Pre-Raid', 'Warrior', 'Fury', 'Legs', 'Both', 'Heroes'' Dreadnaught Legplates'),
(1, 1, 7, 0, 200, 43402, 'Pre-Raid', 'Warrior', 'Fury', 'Feet', 'Both', 'The Obliterator Greaves'),
(1, 1, 8, 0, 200, 37891, 'Pre-Raid', 'Warrior', 'Fury', 'Wrists', 'Both', 'Cast Iron Shackles'),
(1, 1, 9, 0, 200, 39609, 'Pre-Raid', 'Warrior', 'Fury', 'Hands', 'Both', 'Heroes'' Dreadnaught Gauntlets'),
(1, 1, 10, 0, 200, 37151, 'Pre-Raid', 'Warrior', 'Fury', 'Finger1', 'Both', 'Band of Frosted Thorns'),
(1, 1, 12, 0, 200, 40684, 'Pre-Raid', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Mirror of Truth'),
(1, 1, 14, 0, 200, 43566, 'Pre-Raid', 'Warrior', 'Fury', 'Back', 'Both', 'Ice Striker''s Cloak'),
(1, 1, 15, 0, 200, 41257, 'Pre-Raid', 'Warrior', 'Fury', 'MainHand', 'Both', 'Titansteel Destroyer'),
(1, 1, 17, 0, 200, 43284, 'Pre-Raid', 'Warrior', 'Fury', 'Ranged', 'Both', 'Amanitar Skullbow');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 224, 44006, 'Phase 1', 'Warrior', 'Fury', 'Head', 'Both', 'Obsidian Greathelm'),
(1, 1, 1, 0, 224, 44664, 'Phase 1', 'Warrior', 'Fury', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(1, 1, 2, 0, 224, 40530, 'Phase 1', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Valorous Dreadnaught Shoulderplates'),
(1, 1, 4, 0, 224, 40539, 'Phase 1', 'Warrior', 'Fury', 'Chest', 'Both', 'Chestguard of the Recluse'),
(1, 1, 5, 0, 224, 40205, 'Phase 1', 'Warrior', 'Fury', 'Waist', 'Both', 'Stalk-Skin Belt'),
(1, 1, 6, 0, 224, 40529, 'Phase 1', 'Warrior', 'Fury', 'Legs', 'Both', 'Valorous Dreadnaught Legplates'),
(1, 1, 7, 0, 224, 40591, 'Phase 1', 'Warrior', 'Fury', 'Feet', 'Both', 'Melancholy Sabatons'),
(1, 1, 8, 0, 224, 39765, 'Phase 1', 'Warrior', 'Fury', 'Wrists', 'Both', 'Sinner''s Bindings'),
(1, 1, 9, 0, 224, 40541, 'Phase 1', 'Warrior', 'Fury', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(1, 1, 10, 0, 224, 40717, 'Phase 1', 'Warrior', 'Fury', 'Finger1', 'Both', 'Ring of Invincibility'),
(1, 1, 12, 0, 224, 40256, 'Phase 1', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Grim Toll'),
(1, 1, 14, 0, 224, 40403, 'Phase 1', 'Warrior', 'Fury', 'Back', 'Both', 'Drape of the Deadly Foe'),
(1, 1, 17, 0, 224, 40385, 'Phase 1', 'Warrior', 'Fury', 'Ranged', 'Both', 'Envoy of Mortality');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 245, 46151, 'Phase 2', 'Warrior', 'Fury', 'Head', 'Both', 'Conqueror''s Siegebreaker Helmet'),
(1, 1, 1, 0, 245, 45517, 'Phase 2', 'Warrior', 'Fury', 'Neck', 'Both', 'Pendulum of Infinity'),
(1, 1, 2, 0, 245, 46037, 'Phase 2', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Shoulderplates of the Celestial Watch'),
(1, 1, 4, 0, 245, 46146, 'Phase 2', 'Warrior', 'Fury', 'Chest', 'Both', 'Conqueror''s Siegebreaker Battleplate'),
(1, 1, 5, 0, 245, 46041, 'Phase 2', 'Warrior', 'Fury', 'Waist', 'Both', 'Starfall Girdle'),
(1, 1, 6, 0, 245, 45536, 'Phase 2', 'Warrior', 'Fury', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(1, 1, 7, 0, 245, 45599, 'Phase 2', 'Warrior', 'Fury', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(1, 1, 9, 0, 245, 46148, 'Phase 2', 'Warrior', 'Fury', 'Hands', 'Both', 'Conqueror''s Siegebreaker Gauntlets'),
(1, 1, 10, 0, 245, 45608, 'Phase 2', 'Warrior', 'Fury', 'Finger1', 'Both', 'Brann''s Signet Ring'),
(1, 1, 12, 0, 245, 46038, 'Phase 2', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Dark Matter'),
(1, 1, 14, 0, 245, 46032, 'Phase 2', 'Warrior', 'Fury', 'Back', 'Both', 'Drape of the Faceless General'),
(1, 1, 17, 0, 245, 45296, 'Phase 2', 'Warrior', 'Fury', 'Ranged', 'Both', 'Twirling Blades');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 1, 258, 48383, 'Phase 3', 'Warrior', 'Fury', 'Head', 'Alliance', 'Wrynn''s Helmet of Triumph'),
(1, 1, 0, 2, 258, 48398, 'Phase 3', 'Warrior', 'Fury', 'Head', 'Horde', 'Hellscream''s Helmet of Triumph'),
(1, 1, 1, 1, 258, 47060, 'Phase 3', 'Warrior', 'Fury', 'Neck', 'Alliance', 'Charge of the Demon Lord'),
(1, 1, 1, 2, 258, 47433, 'Phase 3', 'Warrior', 'Fury', 'Neck', 'Horde', 'Charge of the Eredar'),
(1, 1, 2, 1, 258, 48381, 'Phase 3', 'Warrior', 'Fury', 'Shoulders', 'Alliance', 'Wrynn''s Shoulderplates of Triumph'),
(1, 1, 2, 2, 258, 48400, 'Phase 3', 'Warrior', 'Fury', 'Shoulders', 'Horde', 'Hellscream''s Shoulderplates of Triumph'),
(1, 1, 4, 1, 258, 48385, 'Phase 3', 'Warrior', 'Fury', 'Chest', 'Alliance', 'Wrynn''s Battleplate of Triumph'),
(1, 1, 4, 2, 258, 48396, 'Phase 3', 'Warrior', 'Fury', 'Chest', 'Horde', 'Hellscream''s Battleplate of Triumph'),
(1, 1, 5, 1, 258, 47002, 'Phase 3', 'Warrior', 'Fury', 'Waist', 'Alliance', 'Bloodbath Belt'),
(1, 1, 5, 2, 258, 47429, 'Phase 3', 'Warrior', 'Fury', 'Waist', 'Horde', 'Bloodbath Girdle'),
(1, 1, 6, 1, 258, 48382, 'Phase 3', 'Warrior', 'Fury', 'Legs', 'Alliance', 'Wrynn''s Legplates of Triumph'),
(1, 1, 6, 2, 258, 48399, 'Phase 3', 'Warrior', 'Fury', 'Legs', 'Horde', 'Hellscream''s Legplates of Triumph'),
(1, 1, 7, 1, 258, 47154, 'Phase 3', 'Warrior', 'Fury', 'Feet', 'Alliance', 'Greaves of the 7th Legion'),
(1, 1, 7, 2, 258, 47473, 'Phase 3', 'Warrior', 'Fury', 'Feet', 'Horde', 'Greaves of the Saronite Citadel'),
(1, 1, 8, 1, 258, 47074, 'Phase 3', 'Warrior', 'Fury', 'Wrists', 'Alliance', 'Bracers of the Untold Massacre'),
(1, 1, 8, 2, 258, 47442, 'Phase 3', 'Warrior', 'Fury', 'Wrists', 'Horde', 'Bracers of the Silent Massacre'),
(1, 1, 9, 1, 258, 47240, 'Phase 3', 'Warrior', 'Fury', 'Hands', 'Alliance', 'Gloves of Bitter Reprisal'),
(1, 1, 9, 2, 258, 47492, 'Phase 3', 'Warrior', 'Fury', 'Hands', 'Horde', 'Gauntlets of Bitter Reprisal'),
(1, 1, 10, 1, 258, 47075, 'Phase 3', 'Warrior', 'Fury', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(1, 1, 10, 2, 258, 47443, 'Phase 3', 'Warrior', 'Fury', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(1, 1, 12, 1, 258, 47131, 'Phase 3', 'Warrior', 'Fury', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(1, 1, 12, 2, 258, 47464, 'Phase 3', 'Warrior', 'Fury', 'Trinket1', 'Horde', 'Death''s Choice'),
(1, 1, 14, 1, 258, 47545, 'Phase 3', 'Warrior', 'Fury', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(1, 1, 14, 2, 258, 47546, 'Phase 3', 'Warrior', 'Fury', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(1, 1, 17, 1, 258, 46995, 'Phase 3', 'Warrior', 'Fury', 'Ranged', 'Alliance', 'Talonstrike'),
(1, 1, 17, 2, 258, 47428, 'Phase 3', 'Warrior', 'Fury', 'Ranged', 'Horde', 'Death''s Head Crossbow');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 264, 51227, 'Phase 4', 'Warrior', 'Fury', 'Head', 'Both', 'Sanctified Ymirjar Lord''s Helmet'),
(1, 1, 1, 0, 264, 50633, 'Phase 4', 'Warrior', 'Fury', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(1, 1, 2, 0, 264, 51229, 'Phase 4', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Sanctified Ymirjar Lord''s Shoulderplates'),
(1, 1, 4, 0, 264, 51225, 'Phase 4', 'Warrior', 'Fury', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Battleplate'),
(1, 1, 5, 0, 264, 50620, 'Phase 4', 'Warrior', 'Fury', 'Waist', 'Both', 'Coldwraith Links'),
(1, 1, 6, 0, 264, 51228, 'Phase 4', 'Warrior', 'Fury', 'Legs', 'Both', 'Sanctified Ymirjar Lord''s Legplates'),
(1, 1, 7, 0, 264, 50639, 'Phase 4', 'Warrior', 'Fury', 'Feet', 'Both', 'Blood-Soaked Saronite Stompers'),
(1, 1, 8, 0, 264, 50670, 'Phase 4', 'Warrior', 'Fury', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(1, 1, 9, 0, 264, 50675, 'Phase 4', 'Warrior', 'Fury', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(1, 1, 10, 1, 264, 47075, 'Phase 4', 'Warrior', 'Fury', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(1, 1, 10, 2, 264, 47443, 'Phase 4', 'Warrior', 'Fury', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(1, 1, 12, 0, 264, 50363, 'Phase 4', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(1, 1, 14, 0, 264, 50653, 'Phase 4', 'Warrior', 'Fury', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(1, 1, 17, 0, 264, 50733, 'Phase 4', 'Warrior', 'Fury', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 1, 0, 0, 290, 51227, 'Phase 5', 'Warrior', 'Fury', 'Head', 'Both', 'Sanctified Ymirjar Lord''s Helmet'),
(1, 1, 1, 0, 290, 54581, 'Phase 5', 'Warrior', 'Fury', 'Neck', 'Both', 'Penumbra Pendant'),
(1, 1, 2, 0, 290, 51229, 'Phase 5', 'Warrior', 'Fury', 'Shoulders', 'Both', 'Sanctified Ymirjar Lord''s Shoulderplates'),
(1, 1, 4, 0, 290, 51225, 'Phase 5', 'Warrior', 'Fury', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Battleplate'),
(1, 1, 5, 0, 290, 50620, 'Phase 5', 'Warrior', 'Fury', 'Waist', 'Both', 'Coldwraith Links'),
(1, 1, 6, 0, 290, 51228, 'Phase 5', 'Warrior', 'Fury', 'Legs', 'Both', 'Sanctified Ymirjar Lord''s Legplates'),
(1, 1, 7, 0, 290, 54578, 'Phase 5', 'Warrior', 'Fury', 'Feet', 'Both', 'Apocalypse''s Advance'),
(1, 1, 8, 0, 290, 50670, 'Phase 5', 'Warrior', 'Fury', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(1, 1, 9, 0, 290, 50675, 'Phase 5', 'Warrior', 'Fury', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(1, 1, 10, 0, 290, 50657, 'Phase 5', 'Warrior', 'Fury', 'Finger1', 'Both', 'Skeleton Lord''s Circle'),
(1, 1, 11, 0, 290, 52572, 'Phase 5', 'Warrior', 'Fury', 'Finger2', 'Both', 'Ashen Band of Endless Might'),
(1, 1, 12, 0, 290, 50363, 'Phase 5', 'Warrior', 'Fury', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(1, 1, 13, 0, 290, 54590, 'Phase 5', 'Warrior', 'Fury', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(1, 1, 14, 0, 290, 47545, 'Phase 5', 'Warrior', 'Fury', 'Back', 'Both', 'Vereesa''s Dexterity'),
(1, 1, 15, 0, 290, 50603, 'Phase 5', 'Warrior', 'Fury', 'MainHand', 'Both', 'Cryptmaker'),
(1, 1, 16, 0, 290, 50603, 'Phase 5', 'Warrior', 'Fury', 'OffHand', 'Both', 'Cryptmaker'),
(1, 1, 17, 0, 290, 50733, 'Phase 5', 'Warrior', 'Fury', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Protection (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 66, 12952, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Head', 'Both', 'Gyth''s Skull'),
(1, 2, 1, 0, 66, 13091, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Neck', 'Both', 'Medallion of Grand Marshal Morris'),
(1, 2, 2, 0, 66, 14552, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(1, 2, 4, 0, 66, 14624, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(1, 2, 5, 0, 66, 14620, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(1, 2, 6, 0, 66, 11927, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of the Eternal Guardian'),
(1, 2, 7, 0, 66, 14621, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Feet', 'Both', 'Deathbone Sabatons'),
(1, 2, 8, 0, 66, 12550, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Wrists', 'Both', 'Runed Golem Shackles'),
(1, 2, 9, 0, 66, 13072, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Hands', 'Both', 'Stonegrip Gauntlets'),
(1, 2, 10, 0, 66, 11669, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Finger1', 'Both', 'Naglering'),
(1, 2, 11, 0, 66, 10795, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Finger2', 'Both', 'Drakeclaw Band'),
(1, 2, 13, 0, 66, 10779, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Demon''s Blood'),
(1, 2, 14, 0, 66, 13397, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Back', 'Both', 'Stoneskin Gargoyle Cape'),
(1, 2, 15, 0, 66, 15806, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'MainHand', 'Both', 'Mirah''s Song'),
(1, 2, 16, 0, 66, 12602, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'OffHand', 'Both', 'Draconian Deflector'),
(1, 2, 17, 0, 66, 12651, 'Phase 1 (Pre-Raid)', 'Warrior', 'Protection', 'Ranged', 'Both', 'Blackcrow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 76, 12952, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Head', 'Both', 'Gyth''s Skull'),
(1, 2, 1, 0, 76, 13091, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Neck', 'Both', 'Medallion of Grand Marshal Morris'),
(1, 2, 2, 0, 76, 14552, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(1, 2, 4, 0, 76, 14624, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(1, 2, 5, 0, 76, 14620, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(1, 2, 6, 0, 76, 11927, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of the Eternal Guardian'),
(1, 2, 7, 0, 76, 14621, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Feet', 'Both', 'Deathbone Sabatons'),
(1, 2, 8, 0, 76, 18754, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(1, 2, 9, 0, 76, 13072, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Hands', 'Both', 'Stonegrip Gauntlets'),
(1, 2, 10, 0, 76, 11669, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Finger1', 'Both', 'Naglering'),
(1, 2, 11, 0, 76, 10795, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Finger2', 'Both', 'Drakeclaw Band'),
(1, 2, 13, 0, 76, 10779, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Demon''s Blood'),
(1, 2, 14, 0, 76, 18495, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(1, 2, 15, 0, 76, 15806, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'MainHand', 'Both', 'Mirah''s Song'),
(1, 2, 16, 0, 76, 12602, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'OffHand', 'Both', 'Draconian Deflector'),
(1, 2, 17, 0, 76, 18323, 'Phase 2 (Pre-Raid)', 'Warrior', 'Protection', 'Ranged', 'Both', 'Satyr''s Bow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 78, 16963, 'Phase 2', 'Warrior', 'Protection', 'Head', 'Both', 'Helm of Wrath'),
(1, 2, 1, 0, 78, 17065, 'Phase 2', 'Warrior', 'Protection', 'Neck', 'Both', 'Medallion of Steadfast Might'),
(1, 2, 2, 0, 78, 16868, 'Phase 2', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Pauldrons of Might'),
(1, 2, 4, 0, 78, 14624, 'Phase 2', 'Warrior', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(1, 2, 5, 0, 78, 16864, 'Phase 2', 'Warrior', 'Protection', 'Waist', 'Both', 'Belt of Might'),
(1, 2, 6, 0, 78, 16962, 'Phase 2', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of Wrath'),
(1, 2, 7, 0, 78, 16862, 'Phase 2', 'Warrior', 'Protection', 'Feet', 'Both', 'Sabatons of Might'),
(1, 2, 8, 0, 78, 16861, 'Phase 2', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bracers of Might'),
(1, 2, 9, 0, 78, 13072, 'Phase 2', 'Warrior', 'Protection', 'Hands', 'Both', 'Stonegrip Gauntlets'),
(1, 2, 10, 0, 78, 11669, 'Phase 2', 'Warrior', 'Protection', 'Finger1', 'Both', 'Naglering'),
(1, 2, 11, 0, 78, 18879, 'Phase 2', 'Warrior', 'Protection', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(1, 2, 13, 0, 78, 18406, 'Phase 2', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(1, 2, 14, 0, 78, 18495, 'Phase 2', 'Warrior', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(1, 2, 15, 0, 78, 18203, 'Phase 2', 'Warrior', 'Protection', 'MainHand', 'Both', 'Eskhandar''s Right Claw'),
(1, 2, 16, 0, 78, 17066, 'Phase 2', 'Warrior', 'Protection', 'OffHand', 'Both', 'Drillborer Disk'),
(1, 2, 17, 0, 78, 18323, 'Phase 2', 'Warrior', 'Protection', 'Ranged', 'Both', 'Satyr''s Bow');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 83, 16963, 'Phase 3', 'Warrior', 'Protection', 'Head', 'Both', 'Helm of Wrath'),
(1, 2, 1, 0, 83, 19383, 'Phase 3', 'Warrior', 'Protection', 'Neck', 'Both', 'Master Dragonslayer''s Medallion'),
(1, 2, 2, 0, 83, 16961, 'Phase 3', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Pauldrons of Wrath'),
(1, 2, 4, 0, 83, 16966, 'Phase 3', 'Warrior', 'Protection', 'Chest', 'Both', 'Breastplate of Wrath'),
(1, 2, 5, 0, 83, 16960, 'Phase 3', 'Warrior', 'Protection', 'Waist', 'Both', 'Waistband of Wrath'),
(1, 2, 6, 0, 83, 16962, 'Phase 3', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of Wrath'),
(1, 2, 7, 0, 83, 16965, 'Phase 3', 'Warrior', 'Protection', 'Feet', 'Both', 'Sabatons of Wrath'),
(1, 2, 8, 0, 83, 16959, 'Phase 3', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bracelets of Wrath'),
(1, 2, 9, 0, 83, 16964, 'Phase 3', 'Warrior', 'Protection', 'Hands', 'Both', 'Gauntlets of Wrath'),
(1, 2, 10, 0, 83, 11669, 'Phase 3', 'Warrior', 'Protection', 'Finger1', 'Both', 'Naglering'),
(1, 2, 11, 0, 83, 18879, 'Phase 3', 'Warrior', 'Protection', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(1, 2, 12, 0, 83, 19431, 'Phase 3', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(1, 2, 13, 0, 83, 18406, 'Phase 3', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(1, 2, 14, 0, 83, 18495, 'Phase 3', 'Warrior', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(1, 2, 15, 0, 83, 19335, 'Phase 3', 'Warrior', 'Protection', 'MainHand', 'Both', 'Spineshatter'),
(1, 2, 17, 0, 83, 19368, 'Phase 3', 'Warrior', 'Protection', 'Ranged', 'Both', 'Dragonbreath Hand Cannon');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 88, 16963, 'Phase 5', 'Warrior', 'Protection', 'Head', 'Both', 'Helm of Wrath'),
(1, 2, 1, 0, 88, 22732, 'Phase 5', 'Warrior', 'Protection', 'Neck', 'Both', 'Mark of C''Thun'),
(1, 2, 2, 0, 88, 21639, 'Phase 5', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Pauldrons of the Unrelenting'),
(1, 2, 4, 0, 88, 16966, 'Phase 5', 'Warrior', 'Protection', 'Chest', 'Both', 'Breastplate of Wrath'),
(1, 2, 5, 0, 88, 21598, 'Phase 5', 'Warrior', 'Protection', 'Waist', 'Both', 'Royal Qiraji Belt'),
(1, 2, 6, 0, 88, 16962, 'Phase 5', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of Wrath'),
(1, 2, 7, 0, 88, 16965, 'Phase 5', 'Warrior', 'Protection', 'Feet', 'Both', 'Sabatons of Wrath'),
(1, 2, 8, 0, 88, 16959, 'Phase 5', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bracelets of Wrath'),
(1, 2, 9, 0, 88, 21674, 'Phase 5', 'Warrior', 'Protection', 'Hands', 'Both', 'Gauntlets of Steadfast Determination'),
(1, 2, 10, 0, 88, 21601, 'Phase 5', 'Warrior', 'Protection', 'Finger1', 'Both', 'Ring of Emperor Vek''lor'),
(1, 2, 11, 0, 88, 18879, 'Phase 5', 'Warrior', 'Protection', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(1, 2, 12, 0, 88, 19431, 'Phase 5', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(1, 2, 13, 0, 88, 18406, 'Phase 5', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(1, 2, 14, 0, 88, 19888, 'Phase 5', 'Warrior', 'Protection', 'Back', 'Both', 'Overlord''s Embrace'),
(1, 2, 15, 0, 88, 19363, 'Phase 5', 'Warrior', 'Protection', 'MainHand', 'Both', 'Crul''shorukh, Edge of Chaos'),
(1, 2, 16, 0, 88, 21269, 'Phase 5', 'Warrior', 'Protection', 'OffHand', 'Both', 'Blessed Qiraji Bulwark'),
(1, 2, 17, 0, 88, 19368, 'Phase 5', 'Warrior', 'Protection', 'Ranged', 'Both', 'Dragonbreath Hand Cannon');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 92, 22418, 'Phase 6', 'Warrior', 'Protection', 'Head', 'Both', 'Dreadnaught Helmet'),
(1, 2, 1, 0, 92, 22732, 'Phase 6', 'Warrior', 'Protection', 'Neck', 'Both', 'Mark of C''Thun'),
(1, 2, 2, 0, 92, 22419, 'Phase 6', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Dreadnaught Pauldrons'),
(1, 2, 4, 0, 92, 22416, 'Phase 6', 'Warrior', 'Protection', 'Chest', 'Both', 'Dreadnaught Breastplate'),
(1, 2, 5, 0, 92, 22422, 'Phase 6', 'Warrior', 'Protection', 'Waist', 'Both', 'Dreadnaught Waistguard'),
(1, 2, 6, 0, 92, 22417, 'Phase 6', 'Warrior', 'Protection', 'Legs', 'Both', 'Dreadnaught Legplates'),
(1, 2, 7, 0, 92, 22420, 'Phase 6', 'Warrior', 'Protection', 'Feet', 'Both', 'Dreadnaught Sabatons'),
(1, 2, 8, 0, 92, 22423, 'Phase 6', 'Warrior', 'Protection', 'Wrists', 'Both', 'Dreadnaught Bracers'),
(1, 2, 9, 0, 92, 22421, 'Phase 6', 'Warrior', 'Protection', 'Hands', 'Both', 'Dreadnaught Gauntlets'),
(1, 2, 10, 0, 92, 21601, 'Phase 6', 'Warrior', 'Protection', 'Finger1', 'Both', 'Ring of Emperor Vek''lor'),
(1, 2, 11, 0, 92, 23059, 'Phase 6', 'Warrior', 'Protection', 'Finger2', 'Both', 'Ring of the Dreadnaught'),
(1, 2, 12, 0, 92, 19431, 'Phase 6', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(1, 2, 13, 0, 92, 19406, 'Phase 6', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Drake Fang Talisman'),
(1, 2, 14, 0, 92, 22938, 'Phase 6', 'Warrior', 'Protection', 'Back', 'Both', 'Cryptfiend Silk Cloak'),
(1, 2, 15, 0, 92, 23577, 'Phase 6', 'Warrior', 'Protection', 'MainHand', 'Both', 'The Hungering Cold'),
(1, 2, 16, 0, 92, 23043, 'Phase 6', 'Warrior', 'Protection', 'OffHand', 'Both', 'The Face of Death'),
(1, 2, 17, 0, 92, 19368, 'Phase 6', 'Warrior', 'Protection', 'Ranged', 'Both', 'Dragonbreath Hand Cannon');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 120, 32083, 'Pre-Raid', 'Warrior', 'Protection', 'Head', 'Both', 'Faceguard of Determination'),
(1, 2, 1, 0, 120, 29386, 'Pre-Raid', 'Warrior', 'Protection', 'Neck', 'Both', 'Necklace of the Juggernaut'),
(1, 2, 2, 0, 120, 27803, 'Pre-Raid', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Shoulderguards of the Bold'),
(1, 2, 4, 0, 120, 28205, 'Pre-Raid', 'Warrior', 'Protection', 'Chest', 'Both', 'Breastplate of the Bold'),
(1, 2, 5, 0, 120, 31460, 'Pre-Raid', 'Warrior', 'Protection', 'Waist', 'Both', 'Sha''tari Vindicator''s Waistguard'),
(1, 2, 6, 0, 120, 29184, 'Pre-Raid', 'Warrior', 'Protection', 'Legs', 'Both', 'Timewarden''s Leggings'),
(1, 2, 7, 0, 120, 29239, 'Pre-Raid', 'Warrior', 'Protection', 'Feet', 'Both', 'Eaglecrest Warboots'),
(1, 2, 8, 0, 120, 29463, 'Pre-Raid', 'Warrior', 'Protection', 'Wrists', 'Both', 'Amber Bands of the Aggressor'),
(1, 2, 9, 0, 120, 27475, 'Pre-Raid', 'Warrior', 'Protection', 'Hands', 'Both', 'Gauntlets of the Bold'),
(1, 2, 10, 0, 120, 30834, 'Pre-Raid', 'Warrior', 'Protection', 'Finger1', 'Both', 'Shapeshifter''s Signet'),
(1, 2, 12, 0, 120, 29387, 'Pre-Raid', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Gnomeregan Auto-Blocker 600'),
(1, 2, 13, 0, 120, 23836, 'Pre-Raid', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Goblin Rocket Launcher'),
(1, 2, 14, 0, 120, 27804, 'Pre-Raid', 'Warrior', 'Protection', 'Back', 'Both', 'Devilshark Cape'),
(1, 2, 15, 0, 120, 28438, 'Pre-Raid', 'Warrior', 'Protection', 'MainHand', 'Both', 'Dragonmaw'),
(1, 2, 16, 0, 120, 29266, 'Pre-Raid', 'Warrior', 'Protection', 'OffHand', 'Both', 'Azure-Shield of Coldarra'),
(1, 2, 17, 0, 120, 32756, 'Pre-Raid', 'Warrior', 'Protection', 'Ranged', 'Both', 'Gyro-Balanced Khorium Destroyer');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 125, 29011, 'Phase 1', 'Warrior', 'Protection', 'Head', 'Both', 'Warbringer Greathelm'),
(1, 2, 1, 0, 125, 28516, 'Phase 1', 'Warrior', 'Protection', 'Neck', 'Both', 'Barbed Choker of Discipline'),
(1, 2, 2, 0, 125, 29016, 'Phase 1', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Warbringer Shoulderguards'),
(1, 2, 4, 0, 125, 29012, 'Phase 1', 'Warrior', 'Protection', 'Chest', 'Both', 'Warbringer Chestguard'),
(1, 2, 5, 0, 125, 28566, 'Phase 1', 'Warrior', 'Protection', 'Waist', 'Both', 'Crimson Girdle of the Indomitable'),
(1, 2, 6, 0, 125, 28621, 'Phase 1', 'Warrior', 'Protection', 'Legs', 'Both', 'Wrynn Dynasty Greaves'),
(1, 2, 7, 0, 125, 28747, 'Phase 1', 'Warrior', 'Protection', 'Feet', 'Both', 'Battlescar Boots'),
(1, 2, 8, 0, 125, 28502, 'Phase 1', 'Warrior', 'Protection', 'Wrists', 'Both', 'Vambraces of Courage'),
(1, 2, 9, 0, 125, 28518, 'Phase 1', 'Warrior', 'Protection', 'Hands', 'Both', 'Iron Gauntlets of the Maiden'),
(1, 2, 10, 0, 125, 29279, 'Phase 1', 'Warrior', 'Protection', 'Finger1', 'Both', 'Violet Signet of the Great Protector'),
(1, 2, 11, 0, 125, 30834, 'Phase 1', 'Warrior', 'Protection', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(1, 2, 12, 0, 125, 29387, 'Phase 1', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Gnomeregan Auto-Blocker 600'),
(1, 2, 13, 0, 125, 23836, 'Phase 1', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Goblin Rocket Launcher'),
(1, 2, 14, 0, 125, 28660, 'Phase 1', 'Warrior', 'Protection', 'Back', 'Both', 'Gilded Thorium Cloak'),
(1, 2, 15, 0, 125, 28438, 'Phase 1', 'Warrior', 'Protection', 'MainHand', 'Both', 'Dragonmaw'),
(1, 2, 16, 0, 125, 28825, 'Phase 1', 'Warrior', 'Protection', 'OffHand', 'Both', 'Aldori Legacy Defender'),
(1, 2, 17, 0, 125, 30724, 'Phase 1', 'Warrior', 'Protection', 'Ranged', 'Both', 'Barrel-Blade Longrifle');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 141, 30115, 'Phase 2', 'Warrior', 'Protection', 'Head', 'Both', 'Destroyer Greathelm'),
(1, 2, 1, 0, 141, 33066, 'Phase 2', 'Warrior', 'Protection', 'Neck', 'Both', 'Veteran''s Pendant of Triumph'),
(1, 2, 2, 0, 141, 30117, 'Phase 2', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Destroyer Shoulderguards'),
(1, 2, 4, 0, 141, 30113, 'Phase 2', 'Warrior', 'Protection', 'Chest', 'Both', 'Destroyer Chestguard'),
(1, 2, 5, 0, 141, 30106, 'Phase 2', 'Warrior', 'Protection', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 2, 6, 0, 141, 30116, 'Phase 2', 'Warrior', 'Protection', 'Legs', 'Both', 'Destroyer Legguards'),
(1, 2, 7, 0, 141, 32793, 'Phase 2', 'Warrior', 'Protection', 'Feet', 'Both', 'Veteran''s Plate Greaves'),
(1, 2, 8, 0, 141, 32818, 'Phase 2', 'Warrior', 'Protection', 'Wrists', 'Both', 'Veteran''s Plate Bracers'),
(1, 2, 9, 2, 141, 29998, 'Phase 2', 'Warrior', 'Protection', 'Hands', 'Horde', 'Royal Gauntlets of Silvermoon'),
(1, 2, 10, 0, 141, 30834, 'Phase 2', 'Warrior', 'Protection', 'Finger1', 'Both', 'Shapeshifter''s Signet'),
(1, 2, 11, 0, 141, 29283, 'Phase 2', 'Warrior', 'Protection', 'Finger2', 'Both', 'Violet Signet of the Master Assassin'),
(1, 2, 12, 0, 141, 23836, 'Phase 2', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Goblin Rocket Launcher'),
(1, 2, 13, 0, 141, 28121, 'Phase 2', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Icon of Unyielding Courage'),
(1, 2, 14, 0, 141, 29994, 'Phase 2', 'Warrior', 'Protection', 'Back', 'Both', 'Thalassian Wildercloak'),
(1, 2, 15, 0, 141, 28439, 'Phase 2', 'Warrior', 'Protection', 'MainHand', 'Both', 'Dragonstrike'),
(1, 2, 16, 0, 141, 28825, 'Phase 2', 'Warrior', 'Protection', 'OffHand', 'Both', 'Aldori Legacy Defender'),
(1, 2, 17, 0, 141, 32756, 'Phase 2', 'Warrior', 'Protection', 'Ranged', 'Both', 'Gyro-Balanced Khorium Destroyer');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 156, 32521, 'Phase 3', 'Warrior', 'Protection', 'Head', 'Both', 'Faceplate of the Impenetrable'),
(1, 2, 1, 0, 156, 33066, 'Phase 3', 'Warrior', 'Protection', 'Neck', 'Both', 'Veteran''s Pendant of Triumph'),
(1, 2, 2, 0, 156, 33732, 'Phase 3', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Vengeful Gladiator''s Plate Shoulders'),
(1, 2, 4, 0, 156, 33728, 'Phase 3', 'Warrior', 'Protection', 'Chest', 'Both', 'Vengeful Gladiator''s Plate Chestpiece'),
(1, 2, 5, 0, 156, 30106, 'Phase 3', 'Warrior', 'Protection', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(1, 2, 6, 0, 156, 30978, 'Phase 3', 'Warrior', 'Protection', 'Legs', 'Both', 'Onslaught Legguards'),
(1, 2, 7, 0, 156, 33812, 'Phase 3', 'Warrior', 'Protection', 'Feet', 'Both', 'Vindicator''s Plate Greaves'),
(1, 2, 8, 0, 156, 33813, 'Phase 3', 'Warrior', 'Protection', 'Wrists', 'Both', 'Vindicator''s Plate Bracers'),
(1, 2, 10, 0, 156, 30834, 'Phase 3', 'Warrior', 'Protection', 'Finger1', 'Both', 'Shapeshifter''s Signet'),
(1, 2, 11, 0, 156, 33919, 'Phase 3', 'Warrior', 'Protection', 'Finger2', 'Both', 'Vindicator''s Band of Triumph'),
(1, 2, 12, 0, 156, 31858, 'Phase 3', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Darkmoon Card: Vengeance'),
(1, 2, 13, 0, 156, 32501, 'Phase 3', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Shadowmoon Insignia'),
(1, 2, 14, 0, 156, 34010, 'Phase 3', 'Warrior', 'Protection', 'Back', 'Both', 'Pepe''s Shroud of Pacification'),
(1, 2, 15, 0, 156, 32254, 'Phase 3', 'Warrior', 'Protection', 'MainHand', 'Both', 'The Brutalizer'),
(1, 2, 16, 0, 156, 32375, 'Phase 3', 'Warrior', 'Protection', 'OffHand', 'Both', 'Bulwark of Azzinoth'),
(1, 2, 17, 0, 156, 32253, 'Phase 3', 'Warrior', 'Protection', 'Ranged', 'Both', 'Legionkiller');

-- ilvl 164 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 164, 35068, 'Phase 5', 'Warrior', 'Protection', 'Head', 'Both', 'Brutal Gladiator''s Plate Helm'),
(1, 2, 1, 0, 164, 34178, 'Phase 5', 'Warrior', 'Protection', 'Neck', 'Both', 'Collar of the Pit Lord'),
(1, 2, 2, 0, 164, 34388, 'Phase 5', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Pauldrons of Berserking'),
(1, 2, 4, 0, 164, 35066, 'Phase 5', 'Warrior', 'Protection', 'Chest', 'Both', 'Brutal Gladiator''s Plate Chestpiece'),
(1, 2, 5, 0, 164, 34547, 'Phase 5', 'Warrior', 'Protection', 'Waist', 'Both', 'Onslaught Waistguard'),
(1, 2, 6, 0, 164, 34381, 'Phase 5', 'Warrior', 'Protection', 'Legs', 'Both', 'Felstrength Legplates'),
(1, 2, 7, 0, 164, 34568, 'Phase 5', 'Warrior', 'Protection', 'Feet', 'Both', 'Onslaught Boots'),
(1, 2, 8, 0, 164, 34442, 'Phase 5', 'Warrior', 'Protection', 'Wrists', 'Both', 'Onslaught Wristguards'),
(1, 2, 9, 0, 164, 34378, 'Phase 5', 'Warrior', 'Protection', 'Hands', 'Both', 'Hard Khorium Battlefists'),
(1, 2, 10, 0, 164, 35131, 'Phase 5', 'Warrior', 'Protection', 'Finger1', 'Both', 'Guardian''s Band of Triumph'),
(1, 2, 11, 0, 164, 34213, 'Phase 5', 'Warrior', 'Protection', 'Finger2', 'Both', 'Ring of Hardened Resolve'),
(1, 2, 12, 0, 164, 34473, 'Phase 5', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Commendation of Kael''thas'),
(1, 2, 13, 0, 164, 31858, 'Phase 5', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Darkmoon Card: Vengeance'),
(1, 2, 14, 0, 164, 34190, 'Phase 5', 'Warrior', 'Protection', 'Back', 'Both', 'Crimson Paragon''s Cover'),
(1, 2, 15, 0, 164, 34164, 'Phase 5', 'Warrior', 'Protection', 'MainHand', 'Both', 'Dragonscale-Encrusted Longblade'),
(1, 2, 16, 0, 164, 34185, 'Phase 5', 'Warrior', 'Protection', 'OffHand', 'Both', 'Sword Breaker''s Bulwark'),
(1, 2, 17, 0, 164, 32253, 'Phase 5', 'Warrior', 'Protection', 'Ranged', 'Both', 'Legionkiller');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 200, 41387, 'Pre-Raid', 'Warrior', 'Protection', 'Head', 'Both', 'Tempered Titansteel Helm'),
(1, 2, 1, 0, 200, 40679, 'Pre-Raid', 'Warrior', 'Protection', 'Neck', 'Both', 'Chained Military Gorget'),
(1, 2, 2, 0, 200, 34389, 'Pre-Raid', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Spaulders of the Thalassian Defender'),
(1, 2, 4, 0, 200, 39611, 'Pre-Raid', 'Warrior', 'Protection', 'Chest', 'Both', 'Heroes'' Dreadnaught Breastplate'),
(1, 2, 5, 0, 200, 37379, 'Pre-Raid', 'Warrior', 'Protection', 'Waist', 'Both', 'Skadi''s Iron Belt'),
(1, 2, 6, 0, 200, 43500, 'Pre-Raid', 'Warrior', 'Protection', 'Legs', 'Both', 'Bolstered Legplates'),
(1, 2, 7, 0, 200, 44201, 'Pre-Raid', 'Warrior', 'Protection', 'Feet', 'Both', 'Sabatons of Draconic Vigor'),
(1, 2, 8, 0, 200, 37620, 'Pre-Raid', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bracers of the Herald'),
(1, 2, 9, 0, 200, 39622, 'Pre-Raid', 'Warrior', 'Protection', 'Hands', 'Both', 'Heroes'' Dreadnaught Handguards'),
(1, 2, 10, 0, 200, 34213, 'Pre-Raid', 'Warrior', 'Protection', 'Finger1', 'Both', 'Ring of Hardened Resolve'),
(1, 2, 12, 0, 200, 37220, 'Pre-Raid', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Essence of Gossamer'),
(1, 2, 14, 0, 200, 43565, 'Pre-Raid', 'Warrior', 'Protection', 'Back', 'Both', 'Durable Nerubhide Cape'),
(1, 2, 15, 0, 200, 37401, 'Pre-Raid', 'Warrior', 'Protection', 'MainHand', 'Both', 'Red Sword of Courage');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 224, 40546, 'Phase 1', 'Warrior', 'Protection', 'Head', 'Both', 'Valorous Dreadnaught Greathelm'),
(1, 2, 1, 0, 224, 44665, 'Phase 1', 'Warrior', 'Protection', 'Neck', 'Both', 'Nexus War Champion Beads'),
(1, 2, 2, 0, 224, 40548, 'Phase 1', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Valorous Dreadnaught Pauldrons'),
(1, 2, 4, 0, 224, 40544, 'Phase 1', 'Warrior', 'Protection', 'Chest', 'Both', 'Valorous Dreadnaught Breastplate'),
(1, 2, 5, 0, 224, 39759, 'Phase 1', 'Warrior', 'Protection', 'Waist', 'Both', 'Ablative Chitin Girdle'),
(1, 2, 6, 0, 224, 40589, 'Phase 1', 'Warrior', 'Protection', 'Legs', 'Both', 'Legplates of Sovereignty'),
(1, 2, 7, 0, 224, 39717, 'Phase 1', 'Warrior', 'Protection', 'Feet', 'Both', 'Inexorable Sabatons'),
(1, 2, 8, 0, 224, 39764, 'Phase 1', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bindings of the Hapless Prey'),
(1, 2, 9, 0, 224, 40545, 'Phase 1', 'Warrior', 'Protection', 'Hands', 'Both', 'Valorous Dreadnaught Handguards'),
(1, 2, 10, 0, 224, 40718, 'Phase 1', 'Warrior', 'Protection', 'Finger1', 'Both', 'Signet of the Impregnable Fortress'),
(1, 2, 12, 0, 224, 40257, 'Phase 1', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Defender''s Code'),
(1, 2, 14, 0, 224, 40722, 'Phase 1', 'Warrior', 'Protection', 'Back', 'Both', 'Platinum Mesh Cloak'),
(1, 2, 15, 0, 224, 40402, 'Phase 1', 'Warrior', 'Protection', 'MainHand', 'Both', 'Last Laugh');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 245, 46166, 'Phase 2', 'Warrior', 'Protection', 'Head', 'Both', 'Conqueror''s Siegebreaker Greathelm'),
(1, 2, 1, 0, 245, 45485, 'Phase 2', 'Warrior', 'Protection', 'Neck', 'Both', 'Bronze Pendant of the Vanir'),
(1, 2, 2, 0, 245, 46167, 'Phase 2', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Conqueror''s Siegebreaker Pauldrons'),
(1, 2, 4, 0, 245, 46162, 'Phase 2', 'Warrior', 'Protection', 'Chest', 'Both', 'Conqueror''s Siegebreaker Breastplate'),
(1, 2, 5, 0, 245, 45139, 'Phase 2', 'Warrior', 'Protection', 'Waist', 'Both', 'Dragonslayer''s Brace'),
(1, 2, 6, 0, 245, 46169, 'Phase 2', 'Warrior', 'Protection', 'Legs', 'Both', 'Conqueror''s Siegebreaker Legguards'),
(1, 2, 7, 0, 245, 45542, 'Phase 2', 'Warrior', 'Protection', 'Feet', 'Both', 'Greaves of the Stonewarder'),
(1, 2, 8, 0, 245, 45111, 'Phase 2', 'Warrior', 'Protection', 'Wrists', 'Both', 'Mimiron''s Inferno Couplings'),
(1, 2, 9, 0, 245, 45487, 'Phase 2', 'Warrior', 'Protection', 'Hands', 'Both', 'Handguards of Revitalization'),
(1, 2, 10, 0, 245, 45471, 'Phase 2', 'Warrior', 'Protection', 'Finger1', 'Both', 'Fate''s Clutch'),
(1, 2, 12, 0, 245, 45158, 'Phase 2', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Heart of Iron'),
(1, 2, 14, 0, 245, 45496, 'Phase 2', 'Warrior', 'Protection', 'Back', 'Both', 'Titanskin Cloak'),
(1, 2, 17, 0, 245, 45137, 'Phase 2', 'Warrior', 'Protection', 'Ranged', 'Both', 'Veranus'' Bane');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 258, 48433, 'Phase 3', 'Warrior', 'Protection', 'Head', 'Both', 'Greathelm of Triumph'),
(1, 2, 2, 0, 258, 48455, 'Phase 3', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Pauldrons of Triumph'),
(1, 2, 4, 1, 258, 46968, 'Phase 3', 'Warrior', 'Protection', 'Chest', 'Alliance', 'Chestplate of the Towering Monstrosity'),
(1, 2, 4, 2, 258, 47415, 'Phase 3', 'Warrior', 'Protection', 'Chest', 'Horde', 'Hauberk of the Towering Monstrosity'),
(1, 2, 5, 1, 258, 47076, 'Phase 3', 'Warrior', 'Protection', 'Waist', 'Alliance', 'Girdle of Bloodied Scars'),
(1, 2, 5, 2, 258, 47444, 'Phase 3', 'Warrior', 'Protection', 'Waist', 'Horde', 'Belt of Bloodied Scars'),
(1, 2, 6, 0, 258, 48447, 'Phase 3', 'Warrior', 'Protection', 'Legs', 'Both', 'Legguards of Triumph'),
(1, 2, 7, 1, 258, 47952, 'Phase 3', 'Warrior', 'Protection', 'Feet', 'Alliance', 'Sabatons of the Lingering Vortex & Dawnbreaker Greaves'),
(1, 2, 8, 1, 258, 47918, 'Phase 3', 'Warrior', 'Protection', 'Wrists', 'Alliance', 'Dreadscale Armguards & Bracers of the Shieldmaiden'),
(1, 2, 9, 0, 258, 48453, 'Phase 3', 'Warrior', 'Protection', 'Hands', 'Both', 'Handguards of Triumph'),
(1, 2, 10, 1, 258, 45471, 'Phase 3', 'Warrior', 'Protection', 'Finger1', 'Alliance', 'Fate''s Clutch'),
(1, 2, 12, 1, 258, 47216, 'Phase 3', 'Warrior', 'Protection', 'Trinket1', 'Alliance', 'The Black Heart'),
(1, 2, 14, 1, 258, 47549, 'Phase 3', 'Warrior', 'Protection', 'Back', 'Alliance', 'Magni''s Resolution'),
(1, 2, 14, 2, 258, 47550, 'Phase 3', 'Warrior', 'Protection', 'Back', 'Horde', 'Cairne''s Endurance'),
(1, 2, 17, 0, 258, 47660, 'Phase 3', 'Warrior', 'Protection', 'Ranged', 'Both', 'Blades of the Sable Cross');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 264, 50640, 'Phase 4', 'Warrior', 'Protection', 'Head', 'Both', 'Broken Ram Skull Helm'),
(1, 2, 2, 0, 264, 51224, 'Phase 4', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Sanctified Ymirjar Lord''s Pauldrons'),
(1, 2, 4, 0, 264, 51220, 'Phase 4', 'Warrior', 'Protection', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Breastplate'),
(1, 2, 5, 0, 264, 50691, 'Phase 4', 'Warrior', 'Protection', 'Waist', 'Both', 'Belt of Broken Bones'),
(1, 2, 6, 0, 264, 51223, 'Phase 4', 'Warrior', 'Protection', 'Legs', 'Both', 'Sanctified Ymirjar Lord''s Legguards'),
(1, 2, 7, 0, 264, 50625, 'Phase 4', 'Warrior', 'Protection', 'Feet', 'Both', 'Grinning Skull Greatboots'),
(1, 2, 8, 0, 264, 50611, 'Phase 4', 'Warrior', 'Protection', 'Wrists', 'Both', 'Bracers of Dark Reckoning'),
(1, 2, 9, 0, 264, 51222, 'Phase 4', 'Warrior', 'Protection', 'Hands', 'Both', 'Sanctified Ymirjar Lord''s Handguards'),
(1, 2, 10, 0, 264, 50622, 'Phase 4', 'Warrior', 'Protection', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(1, 2, 12, 0, 264, 50364, 'Phase 4', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Sindragosa''s Flawless Fang'),
(1, 2, 14, 0, 264, 50718, 'Phase 4', 'Warrior', 'Protection', 'Back', 'Both', 'Royal Crimson Cloak'),
(1, 2, 17, 0, 264, 51834, 'Phase 4', 'Warrior', 'Protection', 'Ranged', 'Both', 'Dreamhunter''s Carbine');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(1, 2, 0, 0, 290, 50640, 'Phase 5', 'Warrior', 'Protection', 'Head', 'Both', 'Broken Ram Skull Helm'),
(1, 2, 1, 0, 290, 50682, 'Phase 5', 'Warrior', 'Protection', 'Neck', 'Both', 'Bile-Encrusted Medallion'),
(1, 2, 2, 0, 290, 51847, 'Phase 5', 'Warrior', 'Protection', 'Shoulders', 'Both', 'Spaulders of the Blood Princes'),
(1, 2, 4, 0, 290, 51220, 'Phase 5', 'Warrior', 'Protection', 'Chest', 'Both', 'Sanctified Ymirjar Lord''s Breastplate'),
(1, 2, 5, 0, 290, 50691, 'Phase 5', 'Warrior', 'Protection', 'Waist', 'Both', 'Belt of Broken Bones'),
(1, 2, 6, 0, 290, 50612, 'Phase 5', 'Warrior', 'Protection', 'Legs', 'Both', 'Legguards of Lost Hope'),
(1, 2, 7, 0, 290, 54579, 'Phase 5', 'Warrior', 'Protection', 'Feet', 'Both', 'Treads of Impending Resurrection'),
(1, 2, 8, 0, 290, 51901, 'Phase 5', 'Warrior', 'Protection', 'Wrists', 'Both', 'Gargoyle Spit Bracers'),
(1, 2, 9, 0, 290, 51222, 'Phase 5', 'Warrior', 'Protection', 'Hands', 'Both', 'Sanctified Ymirjar Lord''s Handguards'),
(1, 2, 10, 0, 290, 50622, 'Phase 5', 'Warrior', 'Protection', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(1, 2, 11, 0, 290, 50404, 'Phase 5', 'Warrior', 'Protection', 'Finger2', 'Both', 'Ashen Band of Endless Courage'),
(1, 2, 12, 0, 290, 54591, 'Phase 5', 'Warrior', 'Protection', 'Trinket1', 'Both', 'Petrified Twilight Scale'),
(1, 2, 13, 0, 290, 50364, 'Phase 5', 'Warrior', 'Protection', 'Trinket2', 'Both', 'Sindragosa''s Flawless Fang'),
(1, 2, 14, 0, 290, 50466, 'Phase 5', 'Warrior', 'Protection', 'Back', 'Both', 'Sentinel''s Winter Cloak'),
(1, 2, 15, 0, 290, 50738, 'Phase 5', 'Warrior', 'Protection', 'MainHand', 'Both', 'Mithrios, Bronzebeard''s Legacy'),
(1, 2, 16, 0, 290, 50729, 'Phase 5', 'Warrior', 'Protection', 'OffHand', 'Both', 'Icecrown Glacial Wall'),
(1, 2, 17, 0, 290, 51834, 'Phase 5', 'Warrior', 'Protection', 'Ranged', 'Both', 'Dreamhunter''s Carbine');


-- ============================================================
-- Paladin (2)
-- ============================================================
-- Holy (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 66, 12633, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Head', 'Both', 'Whitesoul Helm'),
(2, 0, 1, 0, 66, 18723, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(2, 0, 2, 0, 66, 18720, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Shroud of the Nathrezim'),
(2, 0, 4, 0, 66, 13346, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Chest', 'Both', 'Robes of the Exalted'),
(2, 0, 5, 0, 66, 18702, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Waist', 'Both', 'Belt of the Ordained'),
(2, 0, 6, 0, 66, 11841, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Legs', 'Both', 'Senior Designer''s Pantaloons'),
(2, 0, 7, 0, 66, 13954, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Feet', 'Both', 'Verdant Footpads'),
(2, 0, 8, 0, 66, 13969, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Wrists', 'Both', 'Loomguard Armbraces'),
(2, 0, 9, 0, 66, 10787, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Hands', 'Both', 'Atal''ai Gloves'),
(2, 0, 10, 0, 66, 16058, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Finger1', 'Both', 'Fordring''s Seal'),
(2, 0, 11, 0, 66, 18103, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Finger2', 'Both', 'Band of Rumination'),
(2, 0, 12, 0, 66, 11819, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Second Wind'),
(2, 0, 13, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Briarwood Reed'),
(2, 0, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'Back', 'Both', 'Archivist Cape'),
(2, 0, 15, 0, 66, 11923, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'MainHand', 'Both', 'The Hammer of Grace'),
(2, 0, 16, 0, 66, 11928, 'Phase 1 (Pre-Raid)', 'Paladin', 'Holy', 'OffHand', 'Both', 'Thaurissan''s Royal Scepter');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 76, 18490, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Head', 'Both', 'Insightful Hood'),
(2, 0, 1, 0, 76, 18723, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(2, 0, 2, 0, 76, 18720, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Shroud of the Nathrezim'),
(2, 0, 4, 0, 76, 13346, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Chest', 'Both', 'Robes of the Exalted'),
(2, 0, 5, 0, 76, 18702, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Waist', 'Both', 'Belt of the Ordained'),
(2, 0, 6, 0, 76, 18386, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Legs', 'Both', 'Padre''s Trousers'),
(2, 0, 7, 0, 76, 18507, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of the Full Moon'),
(2, 0, 8, 0, 76, 13969, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Wrists', 'Both', 'Loomguard Armbraces'),
(2, 0, 9, 0, 76, 18527, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Hands', 'Both', 'Harmonious Gauntlets'),
(2, 0, 10, 0, 76, 16058, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Finger1', 'Both', 'Fordring''s Seal'),
(2, 0, 11, 0, 76, 18103, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Finger2', 'Both', 'Band of Rumination'),
(2, 0, 12, 0, 76, 11819, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Second Wind'),
(2, 0, 13, 0, 76, 12930, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Briarwood Reed'),
(2, 0, 14, 0, 76, 18510, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'Back', 'Both', 'Hide of the Wild'),
(2, 0, 15, 0, 76, 11923, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'MainHand', 'Both', 'The Hammer of Grace'),
(2, 0, 16, 0, 76, 18523, 'Phase 2 (Pre-Raid)', 'Paladin', 'Holy', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 78, 18490, 'Phase 2', 'Paladin', 'Holy', 'Head', 'Both', 'Insightful Hood'),
(2, 0, 1, 0, 78, 18723, 'Phase 2', 'Paladin', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(2, 0, 2, 0, 78, 18810, 'Phase 2', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(2, 0, 4, 0, 78, 19145, 'Phase 2', 'Paladin', 'Holy', 'Chest', 'Both', 'Robe of Volatile Power'),
(2, 0, 5, 0, 78, 19162, 'Phase 2', 'Paladin', 'Holy', 'Waist', 'Both', 'Corehound Belt'),
(2, 0, 6, 0, 78, 18875, 'Phase 2', 'Paladin', 'Holy', 'Legs', 'Both', 'Salamander Scale Pants'),
(2, 0, 7, 0, 78, 18507, 'Phase 2', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of the Full Moon'),
(2, 0, 8, 0, 78, 13969, 'Phase 2', 'Paladin', 'Holy', 'Wrists', 'Both', 'Loomguard Armbraces'),
(2, 0, 9, 0, 78, 18527, 'Phase 2', 'Paladin', 'Holy', 'Hands', 'Both', 'Harmonious Gauntlets'),
(2, 0, 10, 0, 78, 19140, 'Phase 2', 'Paladin', 'Holy', 'Finger1', 'Both', 'Cauterizing Band'),
(2, 0, 11, 0, 78, 19140, 'Phase 2', 'Paladin', 'Holy', 'Finger2', 'Both', 'Cauterizing Band'),
(2, 0, 12, 0, 78, 17064, 'Phase 2', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Shard of the Scale'),
(2, 0, 13, 0, 78, 12930, 'Phase 2', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Briarwood Reed'),
(2, 0, 14, 0, 78, 18510, 'Phase 2', 'Paladin', 'Holy', 'Back', 'Both', 'Hide of the Wild'),
(2, 0, 15, 0, 78, 17103, 'Phase 2', 'Paladin', 'Holy', 'MainHand', 'Both', 'Azuresong Mageblade'),
(2, 0, 16, 0, 78, 18523, 'Phase 2', 'Paladin', 'Holy', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 83, 19375, 'Phase 3', 'Paladin', 'Holy', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(2, 0, 1, 0, 83, 18723, 'Phase 3', 'Paladin', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(2, 0, 2, 0, 83, 18810, 'Phase 3', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(2, 0, 4, 0, 83, 19145, 'Phase 3', 'Paladin', 'Holy', 'Chest', 'Both', 'Robe of Volatile Power'),
(2, 0, 5, 0, 83, 19162, 'Phase 3', 'Paladin', 'Holy', 'Waist', 'Both', 'Corehound Belt'),
(2, 0, 6, 0, 83, 19385, 'Phase 3', 'Paladin', 'Holy', 'Legs', 'Both', 'Empowered Leggings'),
(2, 0, 7, 0, 83, 19437, 'Phase 3', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of Pure Thought'),
(2, 0, 8, 0, 83, 13969, 'Phase 3', 'Paladin', 'Holy', 'Wrists', 'Both', 'Loomguard Armbraces'),
(2, 0, 9, 0, 83, 19390, 'Phase 3', 'Paladin', 'Holy', 'Hands', 'Both', 'Taut Dragonhide Gloves'),
(2, 0, 10, 0, 83, 19382, 'Phase 3', 'Paladin', 'Holy', 'Finger1', 'Both', 'Pure Elementium Band'),
(2, 0, 11, 0, 83, 19140, 'Phase 3', 'Paladin', 'Holy', 'Finger2', 'Both', 'Cauterizing Band'),
(2, 0, 12, 0, 83, 17064, 'Phase 3', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Shard of the Scale'),
(2, 0, 13, 0, 83, 19395, 'Phase 3', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(2, 0, 14, 0, 83, 19430, 'Phase 3', 'Paladin', 'Holy', 'Back', 'Both', 'Shroud of Pure Thought'),
(2, 0, 15, 0, 83, 19360, 'Phase 3', 'Paladin', 'Holy', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(2, 0, 16, 0, 83, 19312, 'Phase 3', 'Paladin', 'Holy', 'OffHand', 'Both', 'Lei of the Lifegiver');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 88, 20628, 'Phase 5', 'Paladin', 'Holy', 'Head', 'Both', 'Deviate Growth Cap'),
(2, 0, 1, 0, 88, 21712, 'Phase 5', 'Paladin', 'Holy', 'Neck', 'Both', 'Amulet of the Fallen God'),
(2, 0, 2, 0, 88, 18810, 'Phase 5', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(2, 0, 4, 0, 88, 21663, 'Phase 5', 'Paladin', 'Holy', 'Chest', 'Both', 'Robes of the Guardian Saint'),
(2, 0, 5, 0, 88, 21582, 'Phase 5', 'Paladin', 'Holy', 'Waist', 'Both', 'Grasp of the Old God'),
(2, 0, 6, 0, 88, 21667, 'Phase 5', 'Paladin', 'Holy', 'Legs', 'Both', 'Legplates of Blazing Light'),
(2, 0, 7, 0, 88, 19437, 'Phase 5', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of Pure Thought'),
(2, 0, 8, 0, 88, 21604, 'Phase 5', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bracelets of Royal Redemption'),
(2, 0, 9, 0, 88, 20264, 'Phase 5', 'Paladin', 'Holy', 'Hands', 'Both', 'Peacekeeper Gauntlets'),
(2, 0, 10, 0, 88, 19382, 'Phase 5', 'Paladin', 'Holy', 'Finger1', 'Both', 'Pure Elementium Band'),
(2, 0, 11, 0, 88, 21620, 'Phase 5', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ring of the Martyr'),
(2, 0, 12, 0, 88, 17064, 'Phase 5', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Shard of the Scale'),
(2, 0, 13, 0, 88, 19395, 'Phase 5', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(2, 0, 14, 0, 88, 21583, 'Phase 5', 'Paladin', 'Holy', 'Back', 'Both', 'Cloak of Clarity'),
(2, 0, 15, 0, 88, 21839, 'Phase 5', 'Paladin', 'Holy', 'MainHand', 'Both', 'Scepter of the False Prophet'),
(2, 0, 16, 0, 88, 21666, 'Phase 5', 'Paladin', 'Holy', 'OffHand', 'Both', 'Sartura''s Might'),
(2, 0, 17, 0, 88, 22402, 'Phase 5', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Grace');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 92, 20628, 'Phase 6', 'Paladin', 'Holy', 'Head', 'Both', 'Deviate Growth Cap'),
(2, 0, 1, 0, 92, 23057, 'Phase 6', 'Paladin', 'Holy', 'Neck', 'Both', 'Gem of Trapped Innocents'),
(2, 0, 2, 0, 92, 22429, 'Phase 6', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Redemption Spaulders'),
(2, 0, 4, 0, 92, 22425, 'Phase 6', 'Paladin', 'Holy', 'Chest', 'Both', 'Redemption Tunic'),
(2, 0, 5, 0, 92, 21582, 'Phase 6', 'Paladin', 'Holy', 'Waist', 'Both', 'Grasp of the Old God'),
(2, 0, 6, 0, 92, 22427, 'Phase 6', 'Paladin', 'Holy', 'Legs', 'Both', 'Redemption Legguards'),
(2, 0, 7, 0, 92, 22430, 'Phase 6', 'Paladin', 'Holy', 'Feet', 'Both', 'Redemption Boots'),
(2, 0, 8, 0, 92, 21604, 'Phase 6', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bracelets of Royal Redemption'),
(2, 0, 9, 0, 92, 20264, 'Phase 6', 'Paladin', 'Holy', 'Hands', 'Both', 'Peacekeeper Gauntlets'),
(2, 0, 10, 0, 92, 19382, 'Phase 6', 'Paladin', 'Holy', 'Finger1', 'Both', 'Pure Elementium Band'),
(2, 0, 11, 0, 92, 23066, 'Phase 6', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ring of Redemption'),
(2, 0, 12, 0, 92, 23047, 'Phase 6', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Eye of the Dead'),
(2, 0, 13, 0, 92, 19395, 'Phase 6', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(2, 0, 14, 0, 92, 23050, 'Phase 6', 'Paladin', 'Holy', 'Back', 'Both', 'Cloak of the Necropolis'),
(2, 0, 15, 0, 92, 23056, 'Phase 6', 'Paladin', 'Holy', 'MainHand', 'Both', 'Hammer of the Twisting Nether'),
(2, 0, 16, 0, 92, 23075, 'Phase 6', 'Paladin', 'Holy', 'OffHand', 'Both', 'Death''s Bargain'),
(2, 0, 17, 0, 92, 23006, 'Phase 6', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Light');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 120, 32084, 'Pre-Raid', 'Paladin', 'Holy', 'Head', 'Both', 'Helmet of the Steadfast Champion'),
(2, 0, 1, 0, 120, 29374, 'Pre-Raid', 'Paladin', 'Holy', 'Neck', 'Both', 'Necklace of Eternal Hope'),
(2, 0, 2, 0, 120, 27775, 'Pre-Raid', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Hallowed Pauldrons'),
(2, 0, 4, 0, 120, 28230, 'Pre-Raid', 'Paladin', 'Holy', 'Chest', 'Both', 'Hallowed Garments'),
(2, 0, 5, 0, 120, 24256, 'Pre-Raid', 'Paladin', 'Holy', 'Waist', 'Both', 'Girdle of Ruination'),
(2, 0, 6, 0, 120, 30541, 'Pre-Raid', 'Paladin', 'Holy', 'Legs', 'Both', 'Stormsong Kilt'),
(2, 0, 7, 0, 120, 27411, 'Pre-Raid', 'Paladin', 'Holy', 'Feet', 'Both', 'Slippers of Serenity'),
(2, 0, 8, 0, 120, 23539, 'Pre-Raid', 'Paladin', 'Holy', 'Wrists', 'Both', 'Blessed Bracers'),
(2, 0, 9, 0, 120, 27457, 'Pre-Raid', 'Paladin', 'Holy', 'Hands', 'Both', 'Life Bearer''s Gauntlets'),
(2, 0, 10, 0, 120, 29373, 'Pre-Raid', 'Paladin', 'Holy', 'Finger1', 'Both', 'Band of Halos'),
(2, 0, 10, 2, 120, 29168, 'Pre-Raid', 'Paladin', 'Holy', 'Finger1', 'Horde', 'Ancestral Band'),
(2, 0, 12, 0, 120, 29376, 'Pre-Raid', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(2, 0, 13, 0, 120, 30841, 'Pre-Raid', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Lower City Prayer Book'),
(2, 0, 14, 0, 120, 29354, 'Pre-Raid', 'Paladin', 'Holy', 'Back', 'Both', 'Light-Touched Stole of Altruism'),
(2, 0, 15, 0, 120, 23556, 'Pre-Raid', 'Paladin', 'Holy', 'MainHand', 'Both', 'Hand of Eternity'),
(2, 0, 16, 0, 120, 29267, 'Pre-Raid', 'Paladin', 'Holy', 'OffHand', 'Both', 'Light-Bearer''s Faith Shield'),
(2, 0, 17, 0, 120, 25644, 'Pre-Raid', 'Paladin', 'Holy', 'Ranged', 'Both', 'Blessed Book of Nagrand');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 125, 29061, 'Phase 1', 'Paladin', 'Holy', 'Head', 'Both', 'Justicar Diadem'),
(2, 0, 1, 0, 125, 30726, 'Phase 1', 'Paladin', 'Holy', 'Neck', 'Both', 'Archaic Charm of Presence'),
(2, 0, 2, 0, 125, 29064, 'Phase 1', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Justicar Pauldrons'),
(2, 0, 4, 0, 125, 29062, 'Phase 1', 'Paladin', 'Holy', 'Chest', 'Both', 'Justicar Chestpiece'),
(2, 0, 5, 0, 125, 28799, 'Phase 1', 'Paladin', 'Holy', 'Waist', 'Both', 'Belt of Divine Inspiration'),
(2, 0, 6, 0, 125, 30727, 'Phase 1', 'Paladin', 'Holy', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(2, 0, 7, 0, 125, 30737, 'Phase 1', 'Paladin', 'Holy', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(2, 0, 8, 0, 125, 28512, 'Phase 1', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bracers of Justice'),
(2, 0, 9, 0, 125, 28505, 'Phase 1', 'Paladin', 'Holy', 'Hands', 'Both', 'Gauntlets of Renewed Hope'),
(2, 0, 10, 0, 125, 28790, 'Phase 1', 'Paladin', 'Holy', 'Finger1', 'Both', 'Naaru Lightwarden''s Band'),
(2, 0, 11, 0, 125, 30736, 'Phase 1', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ring of Flowing Light'),
(2, 0, 12, 0, 125, 28590, 'Phase 1', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Ribbon of Sacrifice'),
(2, 0, 13, 0, 125, 29376, 'Phase 1', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Essence of the Martyr'),
(2, 0, 14, 0, 125, 28765, 'Phase 1', 'Paladin', 'Holy', 'Back', 'Both', 'Stainless Cloak of the Pure Hearted'),
(2, 0, 15, 0, 125, 28771, 'Phase 1', 'Paladin', 'Holy', 'MainHand', 'Both', 'Light''s Justice'),
(2, 0, 16, 0, 125, 29458, 'Phase 1', 'Paladin', 'Holy', 'OffHand', 'Both', 'Aegis of the Vindicator'),
(2, 0, 17, 0, 125, 28592, 'Phase 1', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Souls Redeemed');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 141, 30136, 'Phase 2', 'Paladin', 'Holy', 'Head', 'Both', 'Crystalforge Greathelm'),
(2, 0, 1, 0, 141, 30018, 'Phase 2', 'Paladin', 'Holy', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(2, 0, 2, 0, 141, 30138, 'Phase 2', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Crystalforge Pauldrons'),
(2, 0, 4, 0, 141, 30134, 'Phase 2', 'Paladin', 'Holy', 'Chest', 'Both', 'Crystalforge Chestpiece'),
(2, 0, 5, 0, 141, 29965, 'Phase 2', 'Paladin', 'Holy', 'Waist', 'Both', 'Girdle of the Righteous Path'),
(2, 0, 6, 0, 141, 29991, 'Phase 2', 'Paladin', 'Holy', 'Legs', 'Both', 'Sunhawk Leggings'),
(2, 0, 7, 0, 141, 30027, 'Phase 2', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of Courage Unending'),
(2, 0, 8, 0, 141, 30047, 'Phase 2', 'Paladin', 'Holy', 'Wrists', 'Both', 'Blackfathom Warbands'),
(2, 0, 9, 0, 141, 30112, 'Phase 2', 'Paladin', 'Holy', 'Hands', 'Both', 'Glorious Gauntlets of Crestfall'),
(2, 0, 10, 0, 141, 28790, 'Phase 2', 'Paladin', 'Holy', 'Finger1', 'Both', 'Naaru Lightwarden''s Band'),
(2, 0, 11, 0, 141, 29920, 'Phase 2', 'Paladin', 'Holy', 'Finger2', 'Both', 'Phoenix-Ring of Rebirth'),
(2, 0, 12, 0, 141, 29376, 'Phase 2', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(2, 0, 13, 0, 141, 28590, 'Phase 2', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Ribbon of Sacrifice'),
(2, 0, 14, 0, 141, 29989, 'Phase 2', 'Paladin', 'Holy', 'Back', 'Both', 'Sunshower Light Cloak'),
(2, 0, 15, 0, 141, 30108, 'Phase 2', 'Paladin', 'Holy', 'MainHand', 'Both', 'Lightfathom Scepter'),
(2, 0, 16, 0, 141, 29458, 'Phase 2', 'Paladin', 'Holy', 'OffHand', 'Both', 'Aegis of the Vindicator'),
(2, 0, 17, 0, 141, 28592, 'Phase 2', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Souls Redeemed');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 156, 30988, 'Phase 3', 'Paladin', 'Holy', 'Head', 'Both', 'Lightbringer Greathelm'),
(2, 0, 1, 0, 156, 32370, 'Phase 3', 'Paladin', 'Holy', 'Neck', 'Both', 'Nadina''s Pendant of Purity'),
(2, 0, 2, 0, 156, 30996, 'Phase 3', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Lightbringer Pauldrons'),
(2, 0, 4, 0, 156, 30992, 'Phase 3', 'Paladin', 'Holy', 'Chest', 'Both', 'Lightbringer Chestpiece'),
(2, 0, 5, 0, 156, 30897, 'Phase 3', 'Paladin', 'Holy', 'Waist', 'Both', 'Girdle of Hope'),
(2, 0, 6, 0, 156, 30994, 'Phase 3', 'Paladin', 'Holy', 'Legs', 'Both', 'Lightbringer Leggings'),
(2, 0, 7, 0, 156, 32243, 'Phase 3', 'Paladin', 'Holy', 'Feet', 'Both', 'Pearl Inlaid Boots'),
(2, 0, 8, 0, 156, 30862, 'Phase 3', 'Paladin', 'Holy', 'Wrists', 'Both', 'Blessed Adamantite Bracers'),
(2, 0, 9, 0, 156, 30112, 'Phase 3', 'Paladin', 'Holy', 'Hands', 'Both', 'Glorious Gauntlets of Crestfall'),
(2, 0, 10, 0, 156, 32528, 'Phase 3', 'Paladin', 'Holy', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(2, 0, 11, 0, 156, 32238, 'Phase 3', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ring of Calming Waves'),
(2, 0, 12, 0, 156, 29376, 'Phase 3', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(2, 0, 13, 0, 156, 32496, 'Phase 3', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Memento of Tyrande'),
(2, 0, 14, 0, 156, 32524, 'Phase 3', 'Paladin', 'Holy', 'Back', 'Both', 'Shroud of the Highborne'),
(2, 0, 15, 0, 156, 32500, 'Phase 3', 'Paladin', 'Holy', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(2, 0, 16, 0, 156, 32255, 'Phase 3', 'Paladin', 'Holy', 'OffHand', 'Both', 'Felstone Bulwark'),
(2, 0, 17, 0, 156, 28592, 'Phase 3', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Souls Redeemed');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 164, 30988, 'Phase 4', 'Paladin', 'Holy', 'Head', 'Both', 'Lightbringer Greathelm'),
(2, 0, 1, 0, 164, 33281, 'Phase 4', 'Paladin', 'Holy', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(2, 0, 2, 0, 164, 30996, 'Phase 4', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Lightbringer Pauldrons'),
(2, 0, 4, 0, 164, 30992, 'Phase 4', 'Paladin', 'Holy', 'Chest', 'Both', 'Lightbringer Chestpiece'),
(2, 0, 5, 0, 164, 30897, 'Phase 4', 'Paladin', 'Holy', 'Waist', 'Both', 'Girdle of Hope'),
(2, 0, 6, 0, 164, 30994, 'Phase 4', 'Paladin', 'Holy', 'Legs', 'Both', 'Lightbringer Leggings'),
(2, 0, 7, 0, 164, 33324, 'Phase 4', 'Paladin', 'Holy', 'Feet', 'Both', 'Treads of the Life Path'),
(2, 0, 8, 0, 164, 30862, 'Phase 4', 'Paladin', 'Holy', 'Wrists', 'Both', 'Blessed Adamantite Bracers'),
(2, 0, 9, 0, 164, 30112, 'Phase 4', 'Paladin', 'Holy', 'Hands', 'Both', 'Glorious Gauntlets of Crestfall'),
(2, 0, 10, 0, 164, 32528, 'Phase 4', 'Paladin', 'Holy', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(2, 0, 11, 0, 164, 32238, 'Phase 4', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ring of Calming Waves'),
(2, 0, 12, 0, 164, 29376, 'Phase 4', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(2, 0, 13, 0, 164, 32496, 'Phase 4', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Memento of Tyrande'),
(2, 0, 14, 0, 164, 32524, 'Phase 4', 'Paladin', 'Holy', 'Back', 'Both', 'Shroud of the Highborne'),
(2, 0, 15, 0, 164, 32500, 'Phase 4', 'Paladin', 'Holy', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(2, 0, 16, 0, 164, 32255, 'Phase 4', 'Paladin', 'Holy', 'OffHand', 'Both', 'Felstone Bulwark'),
(2, 0, 17, 0, 164, 28592, 'Phase 4', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Souls Redeemed');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 200, 44949, 'Pre-Raid', 'Paladin', 'Holy', 'Head', 'Both', 'Unbreakable Healing Amplifiers'),
(2, 0, 1, 0, 200, 42647, 'Pre-Raid', 'Paladin', 'Holy', 'Neck', 'Both', 'Titanium Spellshock Necklace'),
(2, 0, 2, 0, 200, 37673, 'Pre-Raid', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(2, 0, 4, 0, 200, 39629, 'Pre-Raid', 'Paladin', 'Holy', 'Chest', 'Both', 'Heroes'' Redemption Tunic'),
(2, 0, 5, 0, 200, 40691, 'Pre-Raid', 'Paladin', 'Holy', 'Waist', 'Both', 'Magroth''s Meditative Cincture'),
(2, 0, 6, 0, 200, 37362, 'Pre-Raid', 'Paladin', 'Holy', 'Legs', 'Both', 'Leggings of Protective Auras'),
(2, 0, 7, 0, 200, 44202, 'Pre-Raid', 'Paladin', 'Holy', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(2, 0, 8, 0, 200, 40741, 'Pre-Raid', 'Paladin', 'Holy', 'Wrists', 'Both', 'Cuffs of the Shadow Ascendant'),
(2, 0, 9, 0, 200, 39632, 'Pre-Raid', 'Paladin', 'Holy', 'Hands', 'Both', 'Heroes'' Redemption Gloves'),
(2, 0, 12, 0, 200, 44255, 'Pre-Raid', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(2, 0, 14, 0, 200, 34242, 'Pre-Raid', 'Paladin', 'Holy', 'Back', 'Both', 'Tattered Cape of Antonidas'),
(2, 0, 15, 0, 200, 37169, 'Pre-Raid', 'Paladin', 'Holy', 'MainHand', 'Both', 'War Mace of Unrequited Love'),
(2, 0, 17, 0, 200, 40705, 'Pre-Raid', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Renewal');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 224, 44007, 'Phase 1', 'Paladin', 'Holy', 'Head', 'Both', 'Headpiece of Reconciliation'),
(2, 0, 1, 0, 224, 44661, 'Phase 1', 'Paladin', 'Holy', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(2, 0, 2, 0, 224, 40573, 'Phase 1', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Valorous Redemption Spaulders'),
(2, 0, 4, 0, 224, 40569, 'Phase 1', 'Paladin', 'Holy', 'Chest', 'Both', 'Valorous Redemption Tunic'),
(2, 0, 5, 0, 224, 40561, 'Phase 1', 'Paladin', 'Holy', 'Waist', 'Both', 'Leash of Heedless Magic'),
(2, 0, 6, 0, 224, 40572, 'Phase 1', 'Paladin', 'Holy', 'Legs', 'Both', 'Valorous Redemption Greaves'),
(2, 0, 7, 0, 224, 40592, 'Phase 1', 'Paladin', 'Holy', 'Feet', 'Both', 'Boots of Healing Energies'),
(2, 0, 8, 0, 224, 40332, 'Phase 1', 'Paladin', 'Holy', 'Wrists', 'Both', 'Abetment Bracers'),
(2, 0, 9, 0, 224, 40570, 'Phase 1', 'Paladin', 'Holy', 'Hands', 'Both', 'Valorous Redemption Gloves'),
(2, 0, 12, 0, 224, 44255, 'Phase 1', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(2, 0, 14, 0, 224, 44005, 'Phase 1', 'Paladin', 'Holy', 'Back', 'Both', 'Pennant Cloak'),
(2, 0, 17, 0, 224, 40705, 'Phase 1', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Renewal');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 245, 46180, 'Phase 2', 'Paladin', 'Holy', 'Head', 'Both', 'Conqueror''s Aegis Headpiece'),
(2, 0, 1, 0, 245, 45443, 'Phase 2', 'Paladin', 'Holy', 'Neck', 'Both', 'Charm of Meticulous Timing'),
(2, 0, 2, 0, 245, 46182, 'Phase 2', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Conqueror''s Aegis Spaulders'),
(2, 0, 4, 0, 245, 45445, 'Phase 2', 'Paladin', 'Holy', 'Chest', 'Both', 'Breastplate of the Devoted'),
(2, 0, 5, 0, 245, 45616, 'Phase 2', 'Paladin', 'Holy', 'Waist', 'Both', 'Star-beaded Clutch'),
(2, 0, 6, 0, 245, 46181, 'Phase 2', 'Paladin', 'Holy', 'Legs', 'Both', 'Conqueror''s Aegis Greaves'),
(2, 0, 7, 0, 245, 45537, 'Phase 2', 'Paladin', 'Holy', 'Feet', 'Both', 'Treads of the False Oracle'),
(2, 0, 8, 0, 245, 45460, 'Phase 2', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bindings of Winter Gale'),
(2, 0, 9, 0, 245, 46155, 'Phase 2', 'Paladin', 'Holy', 'Hands', 'Both', 'Conqueror''s Aegis Gauntlets'),
(2, 0, 12, 0, 245, 46051, 'Phase 2', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Meteorite Crystal'),
(2, 0, 14, 0, 245, 45486, 'Phase 2', 'Paladin', 'Holy', 'Back', 'Both', 'Drape of the Sullen Goddess'),
(2, 0, 15, 0, 245, 46017, 'Phase 2', 'Paladin', 'Holy', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(2, 0, 17, 0, 245, 40705, 'Phase 2', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Renewal');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 258, 48582, 'Phase 3', 'Paladin', 'Holy', 'Head', 'Both', 'Headpiece of Triumph'),
(2, 0, 2, 0, 258, 48580, 'Phase 3', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Spaulders of Triumph'),
(2, 0, 4, 1, 258, 47147, 'Phase 3', 'Paladin', 'Holy', 'Chest', 'Alliance', 'Breastplate of the Frozen Lake'),
(2, 0, 4, 2, 258, 47471, 'Phase 3', 'Paladin', 'Holy', 'Chest', 'Horde', 'Chestplate of the Frozen Lake'),
(2, 0, 5, 1, 258, 47924, 'Phase 3', 'Paladin', 'Holy', 'Waist', 'Alliance', 'Belt of the Frozen Reach'),
(2, 0, 5, 2, 258, 47997, 'Phase 3', 'Paladin', 'Holy', 'Waist', 'Horde', 'Girdle of the Frozen Reach'),
(2, 0, 6, 1, 258, 47190, 'Phase 3', 'Paladin', 'Holy', 'Legs', 'Alliance', 'Legwraps of the Awakening'),
(2, 0, 6, 2, 258, 47479, 'Phase 3', 'Paladin', 'Holy', 'Legs', 'Horde', 'Leggings of the Awakening'),
(2, 0, 7, 1, 258, 46986, 'Phase 3', 'Paladin', 'Holy', 'Feet', 'Alliance', 'Boots of the Courageous'),
(2, 0, 7, 2, 258, 47424, 'Phase 3', 'Paladin', 'Holy', 'Feet', 'Horde', 'Sabatons of the Courageous'),
(2, 0, 8, 0, 258, 45460, 'Phase 3', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bindings of Winter Gale'),
(2, 0, 9, 0, 258, 45665, 'Phase 3', 'Paladin', 'Holy', 'Hands', 'Both', 'Pharos Gloves'),
(2, 0, 10, 1, 258, 47224, 'Phase 3', 'Paladin', 'Holy', 'Finger1', 'Alliance', 'Ring of the Darkmender'),
(2, 0, 10, 2, 258, 47439, 'Phase 3', 'Paladin', 'Holy', 'Finger1', 'Horde', 'Circle of the Darkmender'),
(2, 0, 12, 0, 258, 46051, 'Phase 3', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Meteorite Crystal'),
(2, 0, 14, 1, 258, 47552, 'Phase 3', 'Paladin', 'Holy', 'Back', 'Alliance', 'Jaina''s Radiance'),
(2, 0, 14, 2, 258, 47551, 'Phase 3', 'Paladin', 'Holy', 'Back', 'Horde', 'Aethas'' Intensity');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 264, 51272, 'Phase 4', 'Paladin', 'Holy', 'Head', 'Both', 'Sanctified Lightsworn Headpiece'),
(2, 0, 2, 0, 264, 51273, 'Phase 4', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Sanctified Lightsworn Spaulders'),
(2, 0, 4, 0, 264, 50680, 'Phase 4', 'Paladin', 'Holy', 'Chest', 'Both', 'Rot-Resistant Breastplate'),
(2, 0, 5, 0, 264, 50613, 'Phase 4', 'Paladin', 'Holy', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(2, 0, 6, 0, 264, 51928, 'Phase 4', 'Paladin', 'Holy', 'Legs', 'Both', 'Corrupted Silverplate Leggings'),
(2, 0, 7, 0, 264, 50699, 'Phase 4', 'Paladin', 'Holy', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(2, 0, 8, 0, 264, 50721, 'Phase 4', 'Paladin', 'Holy', 'Wrists', 'Both', 'Crypt Keeper''s Bracers'),
(2, 0, 9, 0, 264, 50650, 'Phase 4', 'Paladin', 'Holy', 'Hands', 'Both', 'Fallen Lord''s Handguards'),
(2, 0, 10, 0, 264, 50400, 'Phase 4', 'Paladin', 'Holy', 'Finger1', 'Both', 'Ashen Band of Endless Wisdom'),
(2, 0, 12, 0, 264, 46051, 'Phase 4', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Meteorite Crystal'),
(2, 0, 14, 0, 264, 50628, 'Phase 4', 'Paladin', 'Holy', 'Back', 'Both', 'Frostbinder''s Shredded Cape');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 0, 0, 0, 290, 51272, 'Phase 5', 'Paladin', 'Holy', 'Head', 'Both', 'Sanctified Lightsworn Headpiece'),
(2, 0, 1, 0, 290, 50182, 'Phase 5', 'Paladin', 'Holy', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(2, 0, 2, 0, 290, 51273, 'Phase 5', 'Paladin', 'Holy', 'Shoulders', 'Both', 'Sanctified Lightsworn Spaulders'),
(2, 0, 4, 0, 290, 50680, 'Phase 5', 'Paladin', 'Holy', 'Chest', 'Both', 'Rot-Resistant Breastplate'),
(2, 0, 5, 0, 290, 54587, 'Phase 5', 'Paladin', 'Holy', 'Waist', 'Both', 'Split Shape Belt'),
(2, 0, 6, 0, 290, 50694, 'Phase 5', 'Paladin', 'Holy', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(2, 0, 7, 0, 290, 54586, 'Phase 5', 'Paladin', 'Holy', 'Feet', 'Both', 'Foreshadow Steps'),
(2, 0, 8, 0, 290, 54582, 'Phase 5', 'Paladin', 'Holy', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(2, 0, 9, 0, 290, 50650, 'Phase 5', 'Paladin', 'Holy', 'Hands', 'Both', 'Fallen Lord''s Handguards'),
(2, 0, 10, 0, 290, 50664, 'Phase 5', 'Paladin', 'Holy', 'Finger1', 'Both', 'Ring of Rapid Ascent'),
(2, 0, 11, 0, 290, 50400, 'Phase 5', 'Paladin', 'Holy', 'Finger2', 'Both', 'Ashen Band of Endless Wisdom'),
(2, 0, 12, 0, 290, 46051, 'Phase 5', 'Paladin', 'Holy', 'Trinket1', 'Both', 'Meteorite Crystal'),
(2, 0, 13, 0, 290, 48724, 'Phase 5', 'Paladin', 'Holy', 'Trinket2', 'Both', 'Talisman of Resurgence'),
(2, 0, 14, 0, 290, 54583, 'Phase 5', 'Paladin', 'Holy', 'Back', 'Both', 'Cloak of Burning Dusk'),
(2, 0, 15, 0, 290, 46017, 'Phase 5', 'Paladin', 'Holy', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(2, 0, 16, 0, 290, 50616, 'Phase 5', 'Paladin', 'Holy', 'OffHand', 'Both', 'Bulwark of Smouldering Steel'),
(2, 0, 17, 0, 290, 40705, 'Phase 5', 'Paladin', 'Holy', 'Ranged', 'Both', 'Libram of Renewal');

-- Protection (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 66, 12952, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Head', 'Both', 'Gyth''s Skull'),
(2, 1, 1, 0, 66, 13091, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Neck', 'Both', 'Medallion of Grand Marshal Morris'),
(2, 1, 2, 0, 66, 14552, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(2, 1, 4, 0, 66, 14624, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(2, 1, 5, 0, 66, 14620, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(2, 1, 6, 0, 66, 14623, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Legs', 'Both', 'Deathbone Legguards'),
(2, 1, 7, 0, 66, 14621, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Feet', 'Both', 'Deathbone Sabatons'),
(2, 1, 8, 0, 66, 12550, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Wrists', 'Both', 'Runed Golem Shackles'),
(2, 1, 9, 0, 66, 14622, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Hands', 'Both', 'Deathbone Gauntlets'),
(2, 1, 10, 0, 66, 11669, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Finger1', 'Both', 'Naglering'),
(2, 1, 11, 0, 66, 10795, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Finger2', 'Both', 'Drakeclaw Band'),
(2, 1, 13, 0, 66, 10779, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Demon''s Blood'),
(2, 1, 14, 0, 66, 13397, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'Back', 'Both', 'Stoneskin Gargoyle Cape'),
(2, 1, 15, 0, 66, 11784, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'MainHand', 'Both', 'Arbiter''s Blade'),
(2, 1, 16, 0, 66, 12602, 'Phase 1 (Pre-Raid)', 'Paladin', 'Protection', 'OffHand', 'Both', 'Draconian Deflector');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 76, 12952, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Head', 'Both', 'Gyth''s Skull'),
(2, 1, 1, 0, 76, 13091, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Neck', 'Both', 'Medallion of Grand Marshal Morris'),
(2, 1, 2, 0, 76, 14552, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(2, 1, 4, 0, 76, 14624, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(2, 1, 5, 0, 76, 14620, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(2, 1, 6, 0, 76, 14623, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Legs', 'Both', 'Deathbone Legguards'),
(2, 1, 7, 0, 76, 14621, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Feet', 'Both', 'Deathbone Sabatons'),
(2, 1, 8, 0, 76, 18754, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(2, 1, 9, 0, 76, 14622, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Hands', 'Both', 'Deathbone Gauntlets'),
(2, 1, 10, 0, 76, 11669, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Finger1', 'Both', 'Naglering'),
(2, 1, 11, 0, 76, 10795, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Finger2', 'Both', 'Drakeclaw Band'),
(2, 1, 13, 0, 76, 10779, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Demon''s Blood'),
(2, 1, 14, 0, 76, 18495, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(2, 1, 15, 0, 76, 18396, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'MainHand', 'Both', 'Mind Carver'),
(2, 1, 16, 0, 76, 12602, 'Phase 2 (Pre-Raid)', 'Paladin', 'Protection', 'OffHand', 'Both', 'Draconian Deflector');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 78, 12952, 'Phase 2', 'Paladin', 'Protection', 'Head', 'Both', 'Gyth''s Skull'),
(2, 1, 1, 0, 78, 17065, 'Phase 2', 'Paladin', 'Protection', 'Neck', 'Both', 'Medallion of Steadfast Might'),
(2, 1, 2, 0, 78, 14552, 'Phase 2', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(2, 1, 4, 0, 78, 14624, 'Phase 2', 'Paladin', 'Protection', 'Chest', 'Both', 'Deathbone Chestplate'),
(2, 1, 5, 0, 78, 14620, 'Phase 2', 'Paladin', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(2, 1, 6, 0, 78, 14623, 'Phase 2', 'Paladin', 'Protection', 'Legs', 'Both', 'Deathbone Legguards'),
(2, 1, 7, 0, 78, 18806, 'Phase 2', 'Paladin', 'Protection', 'Feet', 'Both', 'Core Forged Greaves'),
(2, 1, 8, 0, 78, 18754, 'Phase 2', 'Paladin', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(2, 1, 9, 0, 78, 13072, 'Phase 2', 'Paladin', 'Protection', 'Hands', 'Both', 'Stonegrip Gauntlets'),
(2, 1, 10, 0, 78, 11669, 'Phase 2', 'Paladin', 'Protection', 'Finger1', 'Both', 'Naglering'),
(2, 1, 11, 0, 78, 18879, 'Phase 2', 'Paladin', 'Protection', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(2, 1, 13, 0, 78, 18406, 'Phase 2', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(2, 1, 14, 0, 78, 18495, 'Phase 2', 'Paladin', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(2, 1, 15, 0, 78, 17103, 'Phase 2', 'Paladin', 'Protection', 'MainHand', 'Both', 'Azuresong Mageblade'),
(2, 1, 16, 0, 78, 17066, 'Phase 2', 'Paladin', 'Protection', 'OffHand', 'Both', 'Drillborer Disk');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 83, 12620, 'Phase 3', 'Paladin', 'Protection', 'Head', 'Both', 'Enchanted Thorium Helm'),
(2, 1, 1, 0, 83, 19383, 'Phase 3', 'Paladin', 'Protection', 'Neck', 'Both', 'Master Dragonslayer''s Medallion'),
(2, 1, 2, 0, 83, 14552, 'Phase 3', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Stockade Pauldrons'),
(2, 1, 4, 0, 83, 12618, 'Phase 3', 'Paladin', 'Protection', 'Chest', 'Both', 'Enchanted Thorium Breastplate'),
(2, 1, 5, 0, 83, 14620, 'Phase 3', 'Paladin', 'Protection', 'Waist', 'Both', 'Deathbone Girdle'),
(2, 1, 6, 0, 83, 14623, 'Phase 3', 'Paladin', 'Protection', 'Legs', 'Both', 'Deathbone Legguards'),
(2, 1, 7, 0, 83, 18806, 'Phase 3', 'Paladin', 'Protection', 'Feet', 'Both', 'Core Forged Greaves'),
(2, 1, 8, 0, 83, 18754, 'Phase 3', 'Paladin', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(2, 1, 9, 0, 83, 13072, 'Phase 3', 'Paladin', 'Protection', 'Hands', 'Both', 'Stonegrip Gauntlets'),
(2, 1, 10, 0, 83, 11669, 'Phase 3', 'Paladin', 'Protection', 'Finger1', 'Both', 'Naglering'),
(2, 1, 11, 0, 83, 18879, 'Phase 3', 'Paladin', 'Protection', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(2, 1, 12, 0, 83, 19431, 'Phase 3', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(2, 1, 13, 0, 83, 18406, 'Phase 3', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(2, 1, 14, 0, 83, 18495, 'Phase 3', 'Paladin', 'Protection', 'Back', 'Both', 'Redoubt Cloak'),
(2, 1, 15, 0, 83, 19360, 'Phase 3', 'Paladin', 'Protection', 'MainHand', 'Both', 'Lok''amir il Romathis');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 88, 21387, 'Phase 5', 'Paladin', 'Protection', 'Head', 'Both', 'Avenger''s Crown'),
(2, 1, 1, 0, 88, 22732, 'Phase 5', 'Paladin', 'Protection', 'Neck', 'Both', 'Mark of C''Thun'),
(2, 1, 2, 0, 88, 21639, 'Phase 5', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Pauldrons of the Unrelenting'),
(2, 1, 4, 0, 88, 21389, 'Phase 5', 'Paladin', 'Protection', 'Chest', 'Both', 'Avenger''s Breastplate'),
(2, 1, 5, 0, 88, 21598, 'Phase 5', 'Paladin', 'Protection', 'Waist', 'Both', 'Royal Qiraji Belt'),
(2, 1, 6, 0, 88, 19855, 'Phase 5', 'Paladin', 'Protection', 'Legs', 'Both', 'Bloodsoaked Legplates'),
(2, 1, 7, 0, 88, 21706, 'Phase 5', 'Paladin', 'Protection', 'Feet', 'Both', 'Boots of the Unwavering Will'),
(2, 1, 8, 0, 88, 18754, 'Phase 5', 'Paladin', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(2, 1, 9, 0, 88, 21674, 'Phase 5', 'Paladin', 'Protection', 'Hands', 'Both', 'Gauntlets of Steadfast Determination'),
(2, 1, 10, 0, 88, 21200, 'Phase 5', 'Paladin', 'Protection', 'Finger1', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(2, 1, 11, 0, 88, 21601, 'Phase 5', 'Paladin', 'Protection', 'Finger2', 'Both', 'Ring of Emperor Vek''lor'),
(2, 1, 12, 0, 88, 19431, 'Phase 5', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(2, 1, 13, 0, 88, 18406, 'Phase 5', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(2, 1, 14, 0, 88, 19888, 'Phase 5', 'Paladin', 'Protection', 'Back', 'Both', 'Overlord''s Embrace'),
(2, 1, 15, 0, 88, 21622, 'Phase 5', 'Paladin', 'Protection', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(2, 1, 16, 0, 88, 21269, 'Phase 5', 'Paladin', 'Protection', 'OffHand', 'Both', 'Blessed Qiraji Bulwark'),
(2, 1, 17, 0, 88, 22401, 'Phase 5', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Hope');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 92, 21387, 'Phase 6', 'Paladin', 'Protection', 'Head', 'Both', 'Avenger''s Crown'),
(2, 1, 1, 0, 92, 22732, 'Phase 6', 'Paladin', 'Protection', 'Neck', 'Both', 'Mark of C''Thun'),
(2, 1, 2, 0, 92, 21639, 'Phase 6', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Pauldrons of the Unrelenting'),
(2, 1, 4, 0, 92, 21389, 'Phase 6', 'Paladin', 'Protection', 'Chest', 'Both', 'Avenger''s Breastplate'),
(2, 1, 5, 0, 92, 21598, 'Phase 6', 'Paladin', 'Protection', 'Waist', 'Both', 'Royal Qiraji Belt'),
(2, 1, 6, 0, 92, 19855, 'Phase 6', 'Paladin', 'Protection', 'Legs', 'Both', 'Bloodsoaked Legplates'),
(2, 1, 7, 0, 92, 21706, 'Phase 6', 'Paladin', 'Protection', 'Feet', 'Both', 'Boots of the Unwavering Will'),
(2, 1, 8, 0, 92, 18754, 'Phase 6', 'Paladin', 'Protection', 'Wrists', 'Both', 'Fel Hardened Bracers'),
(2, 1, 9, 0, 92, 21674, 'Phase 6', 'Paladin', 'Protection', 'Hands', 'Both', 'Gauntlets of Steadfast Determination'),
(2, 1, 10, 0, 92, 21200, 'Phase 6', 'Paladin', 'Protection', 'Finger1', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(2, 1, 11, 0, 92, 21601, 'Phase 6', 'Paladin', 'Protection', 'Finger2', 'Both', 'Ring of Emperor Vek''lor'),
(2, 1, 12, 0, 92, 19431, 'Phase 6', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Styleen''s Impeding Scarab'),
(2, 1, 13, 0, 92, 18406, 'Phase 6', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Onyxia Blood Talisman'),
(2, 1, 14, 0, 92, 22938, 'Phase 6', 'Paladin', 'Protection', 'Back', 'Both', 'Cryptfiend Silk Cloak'),
(2, 1, 15, 0, 92, 22988, 'Phase 6', 'Paladin', 'Protection', 'MainHand', 'Both', 'The End of Dreams'),
(2, 1, 16, 0, 92, 22818, 'Phase 6', 'Paladin', 'Protection', 'OffHand', 'Both', 'The Plague Bearer'),
(2, 1, 17, 0, 92, 22401, 'Phase 6', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Hope');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 120, 32083, 'Pre-Raid', 'Paladin', 'Protection', 'Head', 'Both', 'Faceguard of Determination'),
(2, 1, 1, 0, 120, 29386, 'Pre-Raid', 'Paladin', 'Protection', 'Neck', 'Both', 'Necklace of the Juggernaut'),
(2, 1, 2, 0, 120, 27739, 'Pre-Raid', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Spaulders of the Righteous'),
(2, 1, 4, 0, 120, 28262, 'Pre-Raid', 'Paladin', 'Protection', 'Chest', 'Both', 'Jade-Skull Breastplate'),
(2, 1, 5, 0, 120, 29253, 'Pre-Raid', 'Paladin', 'Protection', 'Waist', 'Both', 'Girdle of Valorous Deeds'),
(2, 1, 6, 0, 120, 29184, 'Pre-Raid', 'Paladin', 'Protection', 'Legs', 'Both', 'Timewarden''s Leggings'),
(2, 1, 7, 0, 120, 29254, 'Pre-Raid', 'Paladin', 'Protection', 'Feet', 'Both', 'Boots of the Righteous Path'),
(2, 1, 8, 0, 120, 29252, 'Pre-Raid', 'Paladin', 'Protection', 'Wrists', 'Both', 'Bracers of Dignity'),
(2, 1, 9, 0, 120, 27535, 'Pre-Raid', 'Paladin', 'Protection', 'Hands', 'Both', 'Gauntlets of the Righteous'),
(2, 1, 10, 0, 120, 29323, 'Pre-Raid', 'Paladin', 'Protection', 'Finger1', 'Both', 'Andormu''s Tear'),
(2, 1, 11, 0, 120, 28407, 'Pre-Raid', 'Paladin', 'Protection', 'Finger2', 'Both', 'Elementium Band of the Sentry'),
(2, 1, 12, 0, 120, 27891, 'Pre-Raid', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Adamantine Figure'),
(2, 1, 13, 0, 120, 27529, 'Pre-Raid', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Figurine of the Colossus'),
(2, 1, 14, 0, 120, 27804, 'Pre-Raid', 'Paladin', 'Protection', 'Back', 'Both', 'Devilshark Cape'),
(2, 1, 15, 0, 120, 30832, 'Pre-Raid', 'Paladin', 'Protection', 'MainHand', 'Both', 'Gavel of Unearthed Secrets'),
(2, 1, 16, 0, 120, 29266, 'Pre-Raid', 'Paladin', 'Protection', 'OffHand', 'Both', 'Azure-Shield of Coldarra'),
(2, 1, 17, 0, 120, 29388, 'Pre-Raid', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Repentance');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 125, 29068, 'Phase 1', 'Paladin', 'Protection', 'Head', 'Both', 'Justicar Faceguard'),
(2, 1, 1, 0, 125, 28516, 'Phase 1', 'Paladin', 'Protection', 'Neck', 'Both', 'Barbed Choker of Discipline'),
(2, 1, 2, 0, 125, 29070, 'Phase 1', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Justicar Shoulderguards'),
(2, 1, 4, 0, 125, 29066, 'Phase 1', 'Paladin', 'Protection', 'Chest', 'Both', 'Justicar Chestguard'),
(2, 1, 5, 0, 125, 28566, 'Phase 1', 'Paladin', 'Protection', 'Waist', 'Both', 'Crimson Girdle of the Indomitable'),
(2, 1, 6, 0, 125, 29069, 'Phase 1', 'Paladin', 'Protection', 'Legs', 'Both', 'Justicar Legguards'),
(2, 1, 7, 0, 125, 30641, 'Phase 1', 'Paladin', 'Protection', 'Feet', 'Both', 'Boots of Elusion'),
(2, 1, 8, 0, 125, 23538, 'Phase 1', 'Paladin', 'Protection', 'Wrists', 'Both', 'Bracers of the Green Fortress'),
(2, 1, 9, 0, 125, 28518, 'Phase 1', 'Paladin', 'Protection', 'Hands', 'Both', 'Iron Gauntlets of the Maiden'),
(2, 1, 10, 0, 125, 28792, 'Phase 1', 'Paladin', 'Protection', 'Finger1', 'Both', 'A''dal''s Signet of Defense'),
(2, 1, 11, 0, 125, 29279, 'Phase 1', 'Paladin', 'Protection', 'Finger2', 'Both', 'Violet Signet of the Great Protector'),
(2, 1, 12, 0, 125, 28789, 'Phase 1', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Eye of Magtheridon'),
(2, 1, 13, 0, 125, 29370, 'Phase 1', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Icon of the Silver Crescent'),
(2, 1, 14, 0, 125, 28660, 'Phase 1', 'Paladin', 'Protection', 'Back', 'Both', 'Gilded Thorium Cloak'),
(2, 1, 15, 0, 125, 28802, 'Phase 1', 'Paladin', 'Protection', 'MainHand', 'Both', 'Bloodmaw Magus-Blade'),
(2, 1, 16, 0, 125, 28825, 'Phase 1', 'Paladin', 'Protection', 'OffHand', 'Both', 'Aldori Legacy Defender'),
(2, 1, 17, 0, 125, 29388, 'Phase 1', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Repentance');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 141, 30125, 'Phase 2', 'Paladin', 'Protection', 'Head', 'Both', 'Crystalforge Faceguard'),
(2, 1, 1, 0, 141, 30007, 'Phase 2', 'Paladin', 'Protection', 'Neck', 'Both', 'The Darkener''s Grasp'),
(2, 1, 2, 0, 141, 29070, 'Phase 2', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Justicar Shoulderguards'),
(2, 1, 4, 0, 141, 29066, 'Phase 2', 'Paladin', 'Protection', 'Chest', 'Both', 'Justicar Chestguard'),
(2, 1, 5, 0, 141, 30034, 'Phase 2', 'Paladin', 'Protection', 'Waist', 'Both', 'Belt of the Guardian'),
(2, 1, 6, 0, 141, 30126, 'Phase 2', 'Paladin', 'Protection', 'Legs', 'Both', 'Crystalforge Legguards'),
(2, 1, 7, 0, 141, 30033, 'Phase 2', 'Paladin', 'Protection', 'Feet', 'Both', 'Boots of the Protector'),
(2, 1, 8, 0, 141, 32515, 'Phase 2', 'Paladin', 'Protection', 'Wrists', 'Both', 'Wristguards of Determination'),
(2, 1, 9, 0, 141, 30124, 'Phase 2', 'Paladin', 'Protection', 'Hands', 'Both', 'Crystalforge Handguards'),
(2, 1, 10, 0, 141, 30083, 'Phase 2', 'Paladin', 'Protection', 'Finger1', 'Both', 'Ring of Sundered Souls'),
(2, 1, 11, 0, 141, 33054, 'Phase 2', 'Paladin', 'Protection', 'Finger2', 'Both', 'The Seal of Danzalar'),
(2, 1, 12, 0, 141, 28789, 'Phase 2', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Eye of Magtheridon'),
(2, 1, 13, 0, 141, 29370, 'Phase 2', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Icon of the Silver Crescent'),
(2, 1, 14, 0, 141, 29925, 'Phase 2', 'Paladin', 'Protection', 'Back', 'Both', 'Phoenix-Wing Cloak'),
(2, 1, 15, 0, 141, 32963, 'Phase 2', 'Paladin', 'Protection', 'MainHand', 'Both', 'Merciless Gladiator''s Gavel'),
(2, 1, 16, 0, 141, 33313, 'Phase 2', 'Paladin', 'Protection', 'OffHand', 'Both', 'Merciless Gladiator''s Barrier'),
(2, 1, 17, 0, 141, 29388, 'Phase 2', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Repentance');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 156, 32521, 'Phase 3', 'Paladin', 'Protection', 'Head', 'Both', 'Faceplate of the Impenetrable'),
(2, 1, 1, 0, 156, 32362, 'Phase 3', 'Paladin', 'Protection', 'Neck', 'Both', 'Pendant of Titans'),
(2, 1, 2, 0, 156, 30998, 'Phase 3', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Lightbringer Shoulderguards'),
(2, 1, 4, 0, 156, 30991, 'Phase 3', 'Paladin', 'Protection', 'Chest', 'Both', 'Lightbringer Chestguard'),
(2, 1, 5, 0, 156, 32342, 'Phase 3', 'Paladin', 'Protection', 'Waist', 'Both', 'Girdle of Mighty Resolve'),
(2, 1, 6, 0, 156, 30995, 'Phase 3', 'Paladin', 'Protection', 'Legs', 'Both', 'Lightbringer Legguards'),
(2, 1, 7, 0, 156, 32245, 'Phase 3', 'Paladin', 'Protection', 'Feet', 'Both', 'Tide-Stomper''s Greaves'),
(2, 1, 8, 0, 156, 32279, 'Phase 3', 'Paladin', 'Protection', 'Wrists', 'Both', 'The Seeker''s Wristguards'),
(2, 1, 9, 0, 156, 30985, 'Phase 3', 'Paladin', 'Protection', 'Hands', 'Both', 'Lightbringer Handguards'),
(2, 1, 10, 0, 156, 32261, 'Phase 3', 'Paladin', 'Protection', 'Finger1', 'Both', 'Band of the Abyssal Lord'),
(2, 1, 11, 0, 156, 33054, 'Phase 3', 'Paladin', 'Protection', 'Finger2', 'Both', 'The Seal of Danzalar'),
(2, 1, 12, 0, 156, 31858, 'Phase 3', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Darkmoon Card: Vengeance'),
(2, 1, 13, 0, 156, 32501, 'Phase 3', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Shadowmoon Insignia'),
(2, 1, 14, 0, 156, 34010, 'Phase 3', 'Paladin', 'Protection', 'Back', 'Both', 'Pepe''s Shroud of Pacification'),
(2, 1, 15, 0, 156, 30910, 'Phase 3', 'Paladin', 'Protection', 'MainHand', 'Both', 'Tempest of Chaos'),
(2, 1, 16, 0, 156, 32375, 'Phase 3', 'Paladin', 'Protection', 'OffHand', 'Both', 'Bulwark of Azzinoth'),
(2, 1, 17, 0, 156, 29388, 'Phase 3', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Repentance');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 164, 32521, 'Phase 4', 'Paladin', 'Protection', 'Head', 'Both', 'Faceplate of the Impenetrable'),
(2, 1, 1, 0, 164, 32362, 'Phase 4', 'Paladin', 'Protection', 'Neck', 'Both', 'Pendant of Titans'),
(2, 1, 2, 0, 164, 30998, 'Phase 4', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Lightbringer Shoulderguards'),
(2, 1, 4, 0, 164, 30991, 'Phase 4', 'Paladin', 'Protection', 'Chest', 'Both', 'Lightbringer Chestguard'),
(2, 1, 5, 0, 164, 32342, 'Phase 4', 'Paladin', 'Protection', 'Waist', 'Both', 'Girdle of Mighty Resolve'),
(2, 1, 6, 0, 164, 30995, 'Phase 4', 'Paladin', 'Protection', 'Legs', 'Both', 'Lightbringer Legguards'),
(2, 1, 7, 0, 164, 32245, 'Phase 4', 'Paladin', 'Protection', 'Feet', 'Both', 'Tide-Stomper''s Greaves'),
(2, 1, 8, 0, 164, 32279, 'Phase 4', 'Paladin', 'Protection', 'Wrists', 'Both', 'The Seeker''s Wristguards'),
(2, 1, 9, 0, 164, 30985, 'Phase 4', 'Paladin', 'Protection', 'Hands', 'Both', 'Lightbringer Handguards'),
(2, 1, 10, 0, 164, 32261, 'Phase 4', 'Paladin', 'Protection', 'Finger1', 'Both', 'Band of the Abyssal Lord'),
(2, 1, 11, 0, 164, 33054, 'Phase 4', 'Paladin', 'Protection', 'Finger2', 'Both', 'The Seal of Danzalar'),
(2, 1, 12, 0, 164, 31858, 'Phase 4', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Darkmoon Card: Vengeance'),
(2, 1, 13, 0, 164, 32501, 'Phase 4', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Shadowmoon Insignia'),
(2, 1, 14, 0, 164, 33593, 'Phase 4', 'Paladin', 'Protection', 'Back', 'Both', 'Slikk''s Cloak of Placation'),
(2, 1, 15, 0, 164, 30910, 'Phase 4', 'Paladin', 'Protection', 'MainHand', 'Both', 'Tempest of Chaos'),
(2, 1, 16, 0, 164, 32375, 'Phase 4', 'Paladin', 'Protection', 'OffHand', 'Both', 'Bulwark of Azzinoth'),
(2, 1, 17, 0, 164, 29388, 'Phase 4', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Repentance');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 200, 41387, 'Pre-Raid', 'Paladin', 'Protection', 'Head', 'Both', 'Tempered Titansteel Helm'),
(2, 1, 1, 0, 200, 40679, 'Pre-Raid', 'Paladin', 'Protection', 'Neck', 'Both', 'Chained Military Gorget'),
(2, 1, 2, 0, 200, 37814, 'Pre-Raid', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Iron Dwarf Smith Pauldrons'),
(2, 1, 4, 0, 200, 44198, 'Pre-Raid', 'Paladin', 'Protection', 'Chest', 'Both', 'Breastplate of the Solemn Council'),
(2, 1, 5, 0, 200, 37241, 'Pre-Raid', 'Paladin', 'Protection', 'Waist', 'Both', 'Ancient Aligned Girdle'),
(2, 1, 6, 0, 200, 37193, 'Pre-Raid', 'Paladin', 'Protection', 'Legs', 'Both', 'Staggering Legplates'),
(2, 1, 7, 0, 200, 44201, 'Pre-Raid', 'Paladin', 'Protection', 'Feet', 'Both', 'Sabatons of Draconic Vigor'),
(2, 1, 8, 0, 200, 37620, 'Pre-Raid', 'Paladin', 'Protection', 'Wrists', 'Both', 'Bracers of the Herald'),
(2, 1, 9, 0, 200, 37645, 'Pre-Raid', 'Paladin', 'Protection', 'Hands', 'Both', 'Horn-Tipped Gauntlets'),
(2, 1, 10, 0, 200, 36961, 'Pre-Raid', 'Paladin', 'Protection', 'Finger1', 'Both', 'Dragonflight Great-Ring'),
(2, 1, 12, 0, 200, 37220, 'Pre-Raid', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Essence of Gossamer'),
(2, 1, 14, 0, 200, 43565, 'Pre-Raid', 'Paladin', 'Protection', 'Back', 'Both', 'Durable Nerubhide Cape'),
(2, 1, 15, 0, 200, 37401, 'Pre-Raid', 'Paladin', 'Protection', 'MainHand', 'Both', 'Red Sword of Courage'),
(2, 1, 17, 0, 200, 40707, 'Pre-Raid', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Obstruction');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 224, 40581, 'Phase 1', 'Paladin', 'Protection', 'Head', 'Both', 'Valorous Redemption Faceguard'),
(2, 1, 1, 0, 224, 44665, 'Phase 1', 'Paladin', 'Protection', 'Neck', 'Both', 'Nexus War Champion Beads'),
(2, 1, 2, 0, 224, 40584, 'Phase 1', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Valorous Redemption Shoulderguards'),
(2, 1, 4, 0, 224, 40579, 'Phase 1', 'Paladin', 'Protection', 'Chest', 'Both', 'Valorous Redemption Breastplate'),
(2, 1, 5, 0, 224, 39759, 'Phase 1', 'Paladin', 'Protection', 'Waist', 'Both', 'Ablative Chitin Girdle'),
(2, 1, 6, 0, 224, 40589, 'Phase 1', 'Paladin', 'Protection', 'Legs', 'Both', 'Legplates of Sovereignty'),
(2, 1, 7, 0, 224, 40297, 'Phase 1', 'Paladin', 'Protection', 'Feet', 'Both', 'Sabatons of Endurance'),
(2, 1, 8, 0, 224, 39764, 'Phase 1', 'Paladin', 'Protection', 'Wrists', 'Both', 'Bindings of the Hapless Prey'),
(2, 1, 9, 0, 224, 40580, 'Phase 1', 'Paladin', 'Protection', 'Hands', 'Both', 'Valorous Redemption Handguards'),
(2, 1, 10, 0, 224, 40107, 'Phase 1', 'Paladin', 'Protection', 'Finger1', 'Both', 'Sand-Worn Band'),
(2, 1, 12, 0, 224, 40372, 'Phase 1', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Rune of Repulsion'),
(2, 1, 14, 0, 224, 40410, 'Phase 1', 'Paladin', 'Protection', 'Back', 'Both', 'Shadow of the Ghoul'),
(2, 1, 15, 0, 224, 40402, 'Phase 1', 'Paladin', 'Protection', 'MainHand', 'Both', 'Last Laugh'),
(2, 1, 17, 0, 224, 40337, 'Phase 1', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Resurgence');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 245, 46175, 'Phase 2', 'Paladin', 'Protection', 'Head', 'Both', 'Conqueror''s Aegis Faceguard'),
(2, 1, 1, 0, 245, 45485, 'Phase 2', 'Paladin', 'Protection', 'Neck', 'Both', 'Bronze Pendant of the Vanir'),
(2, 1, 2, 0, 245, 46177, 'Phase 2', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Conqueror''s Aegis Shoulderguards'),
(2, 1, 4, 0, 245, 46039, 'Phase 2', 'Paladin', 'Protection', 'Chest', 'Both', 'Breastplate of the Timeless'),
(2, 1, 5, 0, 245, 45825, 'Phase 2', 'Paladin', 'Protection', 'Waist', 'Both', 'Shieldwarder Girdle'),
(2, 1, 6, 0, 245, 45594, 'Phase 2', 'Paladin', 'Protection', 'Legs', 'Both', 'Legplates of the Endless Void'),
(2, 1, 7, 0, 245, 45988, 'Phase 2', 'Paladin', 'Protection', 'Feet', 'Both', 'Greaves of the Iron Army'),
(2, 1, 8, 0, 245, 45111, 'Phase 2', 'Paladin', 'Protection', 'Wrists', 'Both', 'Mimiron''s Inferno Couplings'),
(2, 1, 9, 0, 245, 45487, 'Phase 2', 'Paladin', 'Protection', 'Hands', 'Both', 'Handguards of Revitalization'),
(2, 1, 10, 0, 245, 45471, 'Phase 2', 'Paladin', 'Protection', 'Finger1', 'Both', 'Fate''s Clutch'),
(2, 1, 12, 0, 245, 45158, 'Phase 2', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Heart of Iron'),
(2, 1, 14, 0, 245, 45496, 'Phase 2', 'Paladin', 'Protection', 'Back', 'Both', 'Titanskin Cloak'),
(2, 1, 17, 0, 245, 45145, 'Phase 2', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of the Sacred Shield');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 258, 48644, 'Phase 3', 'Paladin', 'Protection', 'Head', 'Both', 'Faceguard of Triumph'),
(2, 1, 2, 0, 258, 48646, 'Phase 3', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Shoulderguards of Triumph'),
(2, 1, 4, 1, 258, 46968, 'Phase 3', 'Paladin', 'Protection', 'Chest', 'Alliance', 'Chestplate of the Towering Monstrosity'),
(2, 1, 4, 2, 258, 47415, 'Phase 3', 'Paladin', 'Protection', 'Chest', 'Horde', 'Hauberk of the Towering Monstrosity'),
(2, 1, 5, 1, 258, 47076, 'Phase 3', 'Paladin', 'Protection', 'Waist', 'Alliance', 'Girdle of Bloodied Scars'),
(2, 1, 5, 2, 258, 47444, 'Phase 3', 'Paladin', 'Protection', 'Waist', 'Horde', 'Belt of Bloodied Scars'),
(2, 1, 6, 0, 258, 48645, 'Phase 3', 'Paladin', 'Protection', 'Legs', 'Both', 'Legguards of Triumph'),
(2, 1, 7, 1, 258, 47003, 'Phase 3', 'Paladin', 'Protection', 'Feet', 'Alliance', 'Dawnbreaker Greaves'),
(2, 1, 7, 2, 258, 47430, 'Phase 3', 'Paladin', 'Protection', 'Feet', 'Horde', 'Dawnbreaker Sabatons'),
(2, 1, 8, 1, 258, 47918, 'Phase 3', 'Paladin', 'Protection', 'Wrists', 'Alliance', 'Dreadscale Armguards'),
(2, 1, 8, 2, 258, 47991, 'Phase 3', 'Paladin', 'Protection', 'Wrists', 'Horde', 'Dreadscale Bracers'),
(2, 1, 9, 0, 258, 48643, 'Phase 3', 'Paladin', 'Protection', 'Hands', 'Both', 'Handguards of Triumph'),
(2, 1, 10, 1, 258, 45471, 'Phase 3', 'Paladin', 'Protection', 'Finger1', 'Alliance', 'Fate''s Clutch'),
(2, 1, 12, 1, 258, 47216, 'Phase 3', 'Paladin', 'Protection', 'Trinket1', 'Alliance', 'The Black Heart'),
(2, 1, 14, 1, 258, 47549, 'Phase 3', 'Paladin', 'Protection', 'Back', 'Alliance', 'Magni''s Resolution'),
(2, 1, 14, 2, 258, 47550, 'Phase 3', 'Paladin', 'Protection', 'Back', 'Horde', 'Cairne''s Endurance'),
(2, 1, 17, 0, 258, 47664, 'Phase 3', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of Defiance');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 264, 50640, 'Phase 4', 'Paladin', 'Protection', 'Head', 'Both', 'Broken Ram Skull Helm'),
(2, 1, 1, 0, 264, 50627, 'Phase 4', 'Paladin', 'Protection', 'Neck', 'Both', 'Noose of Malachite'),
(2, 1, 2, 0, 264, 51269, 'Phase 4', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Sanctified Lightsworn Shoulderguards'),
(2, 1, 4, 0, 264, 51265, 'Phase 4', 'Paladin', 'Protection', 'Chest', 'Both', 'Sanctified Lightsworn Chestguard'),
(2, 1, 5, 0, 264, 50691, 'Phase 4', 'Paladin', 'Protection', 'Waist', 'Both', 'Belt of Broken Bones'),
(2, 1, 6, 0, 264, 51268, 'Phase 4', 'Paladin', 'Protection', 'Legs', 'Both', 'Sanctified Lightsworn Legguards'),
(2, 1, 7, 0, 264, 50625, 'Phase 4', 'Paladin', 'Protection', 'Feet', 'Both', 'Grinning Skull Greatboots'),
(2, 1, 8, 0, 264, 50611, 'Phase 4', 'Paladin', 'Protection', 'Wrists', 'Both', 'Bracers of Dark Reckoning'),
(2, 1, 9, 0, 264, 51267, 'Phase 4', 'Paladin', 'Protection', 'Hands', 'Both', 'Sanctified Lightsworn Handguards'),
(2, 1, 10, 0, 264, 50404, 'Phase 4', 'Paladin', 'Protection', 'Finger1', 'Both', 'Ashen Band of Endless Courage'),
(2, 1, 12, 0, 264, 50349, 'Phase 4', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Corpse Tongue Coin'),
(2, 1, 14, 0, 264, 50718, 'Phase 4', 'Paladin', 'Protection', 'Back', 'Both', 'Royal Crimson Cloak'),
(2, 1, 15, 0, 264, 50738, 'Phase 4', 'Paladin', 'Protection', 'MainHand', 'Both', 'Mithrios, Bronzebeard''s Legacy'),
(2, 1, 17, 0, 264, 50461, 'Phase 4', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of the Eternal Tower');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 1, 0, 0, 290, 50640, 'Phase 5', 'Paladin', 'Protection', 'Head', 'Both', 'Broken Ram Skull Helm'),
(2, 1, 1, 0, 290, 50682, 'Phase 5', 'Paladin', 'Protection', 'Neck', 'Both', 'Bile-Encrusted Medallion'),
(2, 1, 2, 0, 290, 50660, 'Phase 5', 'Paladin', 'Protection', 'Shoulders', 'Both', 'Boneguard Commander''s Pauldrons'),
(2, 1, 4, 0, 290, 51265, 'Phase 5', 'Paladin', 'Protection', 'Chest', 'Both', 'Sanctified Lightsworn Chestguard'),
(2, 1, 5, 0, 290, 50991, 'Phase 5', 'Paladin', 'Protection', 'Waist', 'Both', 'Verdigris Chain Belt'),
(2, 1, 6, 0, 290, 49904, 'Phase 5', 'Paladin', 'Protection', 'Legs', 'Both', 'Pillars of Might'),
(2, 1, 7, 0, 290, 54579, 'Phase 5', 'Paladin', 'Protection', 'Feet', 'Both', 'Treads of Impending Resurrection'),
(2, 1, 8, 0, 290, 51901, 'Phase 5', 'Paladin', 'Protection', 'Wrists', 'Both', 'Gargoyle Spit Bracers'),
(2, 1, 9, 0, 290, 51267, 'Phase 5', 'Paladin', 'Protection', 'Hands', 'Both', 'Sanctified Lightsworn Handguards'),
(2, 1, 10, 0, 290, 50622, 'Phase 5', 'Paladin', 'Protection', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(2, 1, 11, 0, 290, 50404, 'Phase 5', 'Paladin', 'Protection', 'Finger2', 'Both', 'Ashen Band of Endless Courage'),
(2, 1, 12, 0, 290, 54591, 'Phase 5', 'Paladin', 'Protection', 'Trinket1', 'Both', 'Petrified Twilight Scale'),
(2, 1, 13, 0, 290, 50364, 'Phase 5', 'Paladin', 'Protection', 'Trinket2', 'Both', 'Sindragosa''s Flawless Fang'),
(2, 1, 14, 0, 290, 50466, 'Phase 5', 'Paladin', 'Protection', 'Back', 'Both', 'Sentinel''s Winter Cloak'),
(2, 1, 15, 0, 290, 50738, 'Phase 5', 'Paladin', 'Protection', 'MainHand', 'Both', 'Mithrios, Bronzebeard''s Legacy'),
(2, 1, 16, 0, 290, 50729, 'Phase 5', 'Paladin', 'Protection', 'OffHand', 'Both', 'Icecrown Glacial Wall'),
(2, 1, 17, 0, 290, 50461, 'Phase 5', 'Paladin', 'Protection', 'Ranged', 'Both', 'Libram of the Eternal Tower');

-- Retribution (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Head', 'Both', 'Mask of the Unforgiven'),
(2, 2, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Neck', 'Both', 'Mark of Fordring'),
(2, 2, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(2, 2, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Chest', 'Both', 'Savage Gladiator Chain'),
(2, 2, 5, 0, 66, 13959, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Waist', 'Both', 'Omokk''s Girth Restrainer'),
(2, 2, 6, 0, 66, 14554, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(2, 2, 7, 0, 66, 14616, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Feet', 'Both', 'Bloodmail Boots'),
(2, 2, 8, 0, 66, 12936, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Battleborn Armbraces'),
(2, 2, 9, 0, 66, 13957, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gargoyle Slashers'),
(2, 2, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Painweaver Band'),
(2, 2, 11, 0, 66, 12548, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Magni''s Will'),
(2, 2, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(2, 2, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Hand of Justice'),
(2, 2, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'Back', 'Both', 'Cape of the Black Baron'),
(2, 2, 15, 0, 66, 12784, 'Phase 1 (Pre-Raid)', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Arcanite Reaper');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 76, 13404, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Head', 'Both', 'Mask of the Unforgiven'),
(2, 2, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Neck', 'Both', 'Mark of Fordring'),
(2, 2, 2, 0, 76, 12927, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(2, 2, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Chest', 'Both', 'Savage Gladiator Chain'),
(2, 2, 5, 0, 76, 13959, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Waist', 'Both', 'Omokk''s Girth Restrainer'),
(2, 2, 6, 0, 76, 14554, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(2, 2, 7, 0, 76, 14616, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Feet', 'Both', 'Bloodmail Boots'),
(2, 2, 8, 0, 76, 12936, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Battleborn Armbraces'),
(2, 2, 9, 0, 76, 13957, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gargoyle Slashers'),
(2, 2, 10, 0, 76, 13098, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Painweaver Band'),
(2, 2, 11, 0, 76, 12548, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Magni''s Will'),
(2, 2, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(2, 2, 13, 0, 76, 11815, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Hand of Justice'),
(2, 2, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'Back', 'Both', 'Cape of the Black Baron'),
(2, 2, 15, 0, 76, 12784, 'Phase 2 (Pre-Raid)', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Arcanite Reaper');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 78, 12640, 'Phase 2', 'Paladin', 'Retribution', 'Head', 'Both', 'Lionheart Helm'),
(2, 2, 1, 0, 78, 18404, 'Phase 2', 'Paladin', 'Retribution', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(2, 2, 2, 0, 78, 12927, 'Phase 2', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(2, 2, 4, 0, 78, 11726, 'Phase 2', 'Paladin', 'Retribution', 'Chest', 'Both', 'Savage Gladiator Chain'),
(2, 2, 5, 0, 78, 19137, 'Phase 2', 'Paladin', 'Retribution', 'Waist', 'Both', 'Onslaught Girdle'),
(2, 2, 6, 0, 78, 14554, 'Phase 2', 'Paladin', 'Retribution', 'Legs', 'Both', 'Cloudkeeper Legplates'),
(2, 2, 7, 0, 78, 14616, 'Phase 2', 'Paladin', 'Retribution', 'Feet', 'Both', 'Bloodmail Boots'),
(2, 2, 8, 0, 78, 19146, 'Phase 2', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Wristguards of Stability'),
(2, 2, 9, 0, 78, 19143, 'Phase 2', 'Paladin', 'Retribution', 'Hands', 'Both', 'Flameguard Gauntlets'),
(2, 2, 10, 0, 78, 13098, 'Phase 2', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Painweaver Band'),
(2, 2, 11, 0, 78, 18821, 'Phase 2', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Quick Strike Ring'),
(2, 2, 12, 0, 78, 13965, 'Phase 2', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(2, 2, 13, 0, 78, 11815, 'Phase 2', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Hand of Justice'),
(2, 2, 14, 0, 78, 13340, 'Phase 2', 'Paladin', 'Retribution', 'Back', 'Both', 'Cape of the Black Baron'),
(2, 2, 15, 0, 78, 17076, 'Phase 2', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Bonereaver''s Edge');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 83, 12640, 'Phase 3', 'Paladin', 'Retribution', 'Head', 'Both', 'Lionheart Helm'),
(2, 2, 1, 0, 83, 18404, 'Phase 3', 'Paladin', 'Retribution', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(2, 2, 2, 0, 83, 19394, 'Phase 3', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Drake Talon Pauldrons'),
(2, 2, 4, 0, 83, 11726, 'Phase 3', 'Paladin', 'Retribution', 'Chest', 'Both', 'Savage Gladiator Chain'),
(2, 2, 5, 0, 83, 19137, 'Phase 3', 'Paladin', 'Retribution', 'Waist', 'Both', 'Onslaught Girdle'),
(2, 2, 6, 0, 83, 19402, 'Phase 3', 'Paladin', 'Retribution', 'Legs', 'Both', 'Legguards of the Fallen Crusader'),
(2, 2, 7, 0, 83, 19387, 'Phase 3', 'Paladin', 'Retribution', 'Feet', 'Both', 'Chromatic Boots'),
(2, 2, 8, 0, 83, 19146, 'Phase 3', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Wristguards of Stability'),
(2, 2, 9, 0, 83, 19143, 'Phase 3', 'Paladin', 'Retribution', 'Hands', 'Both', 'Flameguard Gauntlets'),
(2, 2, 11, 0, 83, 19384, 'Phase 3', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(2, 2, 12, 0, 83, 13965, 'Phase 3', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(2, 2, 13, 0, 83, 11815, 'Phase 3', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Hand of Justice'),
(2, 2, 14, 0, 83, 19436, 'Phase 3', 'Paladin', 'Retribution', 'Back', 'Both', 'Cloak of Draconic Might'),
(2, 2, 15, 0, 83, 19334, 'Phase 3', 'Paladin', 'Retribution', 'MainHand', 'Both', 'The Untamed Blade');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 88, 21387, 'Phase 5', 'Paladin', 'Retribution', 'Head', 'Both', 'Avenger''s Crown'),
(2, 2, 1, 0, 88, 18404, 'Phase 5', 'Paladin', 'Retribution', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(2, 2, 2, 0, 88, 21391, 'Phase 5', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Avenger''s Pauldrons'),
(2, 2, 4, 0, 88, 21389, 'Phase 5', 'Paladin', 'Retribution', 'Chest', 'Both', 'Avenger''s Breastplate'),
(2, 2, 5, 0, 88, 21463, 'Phase 5', 'Paladin', 'Retribution', 'Waist', 'Both', 'Ossirian''s Binding'),
(2, 2, 6, 0, 88, 21390, 'Phase 5', 'Paladin', 'Retribution', 'Legs', 'Both', 'Avenger''s Legguards'),
(2, 2, 7, 0, 88, 21388, 'Phase 5', 'Paladin', 'Retribution', 'Feet', 'Both', 'Avenger''s Greaves'),
(2, 2, 8, 0, 88, 21618, 'Phase 5', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Hive Defiler Wristguards'),
(2, 2, 9, 0, 88, 21623, 'Phase 5', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gauntlets of the Righteous Champion'),
(2, 2, 10, 0, 88, 17063, 'Phase 5', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Band of Accuria'),
(2, 2, 11, 0, 88, 21205, 'Phase 5', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(2, 2, 12, 0, 88, 22321, 'Phase 5', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Heart of Wyrmthalak'),
(2, 2, 13, 0, 88, 19289, 'Phase 5', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Darkmoon Card: Maelstrom'),
(2, 2, 14, 0, 88, 21701, 'Phase 5', 'Paladin', 'Retribution', 'Back', 'Both', 'Cloak of Concentrated Hatred'),
(2, 2, 15, 0, 88, 21134, 'Phase 5', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Dark Edge of Insanity'),
(2, 2, 17, 0, 88, 23203, 'Phase 5', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Fervor');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 92, 21387, 'Phase 6', 'Paladin', 'Retribution', 'Head', 'Both', 'Avenger''s Crown'),
(2, 2, 1, 0, 92, 18404, 'Phase 6', 'Paladin', 'Retribution', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(2, 2, 2, 0, 92, 21391, 'Phase 6', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Avenger''s Pauldrons'),
(2, 2, 4, 0, 92, 21389, 'Phase 6', 'Paladin', 'Retribution', 'Chest', 'Both', 'Avenger''s Breastplate'),
(2, 2, 5, 0, 92, 23219, 'Phase 6', 'Paladin', 'Retribution', 'Waist', 'Both', 'Girdle of the Mentor'),
(2, 2, 6, 0, 92, 21390, 'Phase 6', 'Paladin', 'Retribution', 'Legs', 'Both', 'Avenger''s Legguards'),
(2, 2, 7, 0, 92, 21388, 'Phase 6', 'Paladin', 'Retribution', 'Feet', 'Both', 'Avenger''s Greaves'),
(2, 2, 8, 0, 92, 22936, 'Phase 6', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Wristguards of Vengeance'),
(2, 2, 9, 0, 92, 21623, 'Phase 6', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gauntlets of the Righteous Champion'),
(2, 2, 11, 0, 92, 21205, 'Phase 6', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(2, 2, 12, 0, 92, 22321, 'Phase 6', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Heart of Wyrmthalak'),
(2, 2, 13, 0, 92, 19289, 'Phase 6', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Darkmoon Card: Maelstrom'),
(2, 2, 14, 0, 92, 23045, 'Phase 6', 'Paladin', 'Retribution', 'Back', 'Both', 'Shroud of Dominion'),
(2, 2, 15, 0, 92, 22691, 'Phase 6', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Corrupted Ashbringer'),
(2, 2, 17, 0, 92, 23203, 'Phase 6', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Fervor');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 200, 41386, 'Pre-Raid', 'Paladin', 'Retribution', 'Head', 'Both', 'Spiked Titansteel Helm'),
(2, 2, 1, 0, 200, 42645, 'Pre-Raid', 'Paladin', 'Retribution', 'Neck', 'Both', 'Titanium Impact Choker'),
(2, 2, 2, 0, 200, 37627, 'Pre-Raid', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Snake Den Spaulders'),
(2, 2, 4, 0, 200, 37612, 'Pre-Raid', 'Paladin', 'Retribution', 'Chest', 'Both', 'Bonegrinder Breastplate'),
(2, 2, 5, 0, 200, 37178, 'Pre-Raid', 'Paladin', 'Retribution', 'Waist', 'Both', 'Strategist''s Belt'),
(2, 2, 6, 0, 200, 37193, 'Pre-Raid', 'Paladin', 'Retribution', 'Legs', 'Both', 'Staggering Legplates'),
(2, 2, 7, 0, 200, 43402, 'Pre-Raid', 'Paladin', 'Retribution', 'Feet', 'Both', 'The Obliterator Greaves'),
(2, 2, 8, 0, 200, 37668, 'Pre-Raid', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Bands of the Stoneforge'),
(2, 2, 9, 0, 200, 37363, 'Pre-Raid', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gauntlets of Dragon Wrath'),
(2, 2, 10, 0, 200, 37151, 'Pre-Raid', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Band of Frosted Thorns'),
(2, 2, 12, 0, 200, 40684, 'Pre-Raid', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Mirror of Truth'),
(2, 2, 14, 0, 200, 43566, 'Pre-Raid', 'Paladin', 'Retribution', 'Back', 'Both', 'Ice Striker''s Cloak'),
(2, 2, 15, 0, 200, 41257, 'Pre-Raid', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Titansteel Destroyer'),
(2, 2, 17, 0, 200, 40706, 'Pre-Raid', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Reciprocation');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 224, 44006, 'Phase 1', 'Paladin', 'Retribution', 'Head', 'Both', 'Obsidian Greathelm'),
(2, 2, 1, 0, 224, 44664, 'Phase 1', 'Paladin', 'Retribution', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(2, 2, 2, 0, 224, 40578, 'Phase 1', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Valorous Redemption Shoulderplates'),
(2, 2, 4, 0, 224, 40574, 'Phase 1', 'Paladin', 'Retribution', 'Chest', 'Both', 'Valorous Redemption Chestpiece'),
(2, 2, 5, 0, 224, 40278, 'Phase 1', 'Paladin', 'Retribution', 'Waist', 'Both', 'Girdle of Chivalry'),
(2, 2, 6, 0, 224, 44011, 'Phase 1', 'Paladin', 'Retribution', 'Legs', 'Both', 'Leggings of the Honored'),
(2, 2, 7, 0, 224, 40591, 'Phase 1', 'Paladin', 'Retribution', 'Feet', 'Both', 'Melancholy Sabatons'),
(2, 2, 8, 0, 224, 40186, 'Phase 1', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Thrusting Bands'),
(2, 2, 9, 0, 224, 40541, 'Phase 1', 'Paladin', 'Retribution', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(2, 2, 10, 0, 224, 40075, 'Phase 1', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Ruthlessness'),
(2, 2, 12, 0, 224, 42987, 'Phase 1', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(2, 2, 14, 0, 224, 40403, 'Phase 1', 'Paladin', 'Retribution', 'Back', 'Both', 'Drape of the Deadly Foe'),
(2, 2, 17, 0, 224, 40191, 'Phase 1', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Radiance');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 245, 45472, 'Phase 2', 'Paladin', 'Retribution', 'Head', 'Both', 'Warhelm of the Champion'),
(2, 2, 2, 0, 245, 45245, 'Phase 2', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Shoulderpads of the Intruder'),
(2, 2, 4, 0, 245, 45473, 'Phase 2', 'Paladin', 'Retribution', 'Chest', 'Both', 'Embrace of the Gladiator'),
(2, 2, 5, 0, 245, 46095, 'Phase 2', 'Paladin', 'Retribution', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(2, 2, 6, 0, 245, 45134, 'Phase 2', 'Paladin', 'Retribution', 'Legs', 'Both', 'Plated Leggings of Ruination'),
(2, 2, 7, 0, 245, 45599, 'Phase 2', 'Paladin', 'Retribution', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(2, 2, 8, 0, 245, 45663, 'Phase 2', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Armbands of Bedlam'),
(2, 2, 9, 0, 245, 45444, 'Phase 2', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gloves of the Steady Hand'),
(2, 2, 10, 0, 245, 45608, 'Phase 2', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Brann''s Signet Ring'),
(2, 2, 12, 0, 245, 45609, 'Phase 2', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Comet''s Trail'),
(2, 2, 14, 0, 245, 46032, 'Phase 2', 'Paladin', 'Retribution', 'Back', 'Both', 'Drape of the Faceless General'),
(2, 2, 17, 0, 245, 42853, 'Phase 2', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Furious Gladiator''s Libram of Fortitude');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 258, 48614, 'Phase 3', 'Paladin', 'Retribution', 'Head', 'Both', 'Helm of Triumph'),
(2, 2, 2, 0, 258, 48612, 'Phase 3', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Shoulderplates of Triumph'),
(2, 2, 4, 1, 258, 46965, 'Phase 3', 'Paladin', 'Retribution', 'Chest', 'Alliance', 'Breastplate of Cruel Intent'),
(2, 2, 4, 2, 258, 47412, 'Phase 3', 'Paladin', 'Retribution', 'Chest', 'Horde', 'Cuirass of Cruel Intent'),
(2, 2, 5, 1, 258, 47002, 'Phase 3', 'Paladin', 'Retribution', 'Waist', 'Alliance', 'Bloodbath Belt'),
(2, 2, 5, 2, 258, 47429, 'Phase 3', 'Paladin', 'Retribution', 'Waist', 'Horde', 'Bloodbath Girdle'),
(2, 2, 6, 1, 258, 47132, 'Phase 3', 'Paladin', 'Retribution', 'Legs', 'Alliance', 'Legguards of Ascension'),
(2, 2, 6, 2, 258, 47465, 'Phase 3', 'Paladin', 'Retribution', 'Legs', 'Horde', 'Legplates of Ascension'),
(2, 2, 7, 1, 258, 47154, 'Phase 3', 'Paladin', 'Retribution', 'Feet', 'Alliance', 'Greaves of the 7th Legion'),
(2, 2, 7, 2, 258, 47473, 'Phase 3', 'Paladin', 'Retribution', 'Feet', 'Horde', 'Greaves of the Saronite Citadel'),
(2, 2, 8, 1, 258, 47155, 'Phase 3', 'Paladin', 'Retribution', 'Wrists', 'Alliance', 'Bracers of Dark Determination'),
(2, 2, 8, 2, 258, 47474, 'Phase 3', 'Paladin', 'Retribution', 'Wrists', 'Horde', 'Armbands of Dark Determination'),
(2, 2, 9, 0, 258, 48615, 'Phase 3', 'Paladin', 'Retribution', 'Hands', 'Both', 'Gauntlets of Triumph'),
(2, 2, 10, 1, 258, 46966, 'Phase 3', 'Paladin', 'Retribution', 'Finger1', 'Alliance', 'Band of the Violent Temperment'),
(2, 2, 10, 2, 258, 47413, 'Phase 3', 'Paladin', 'Retribution', 'Finger1', 'Horde', 'Ring of the Violent Temperament'),
(2, 2, 12, 1, 258, 47131, 'Phase 3', 'Paladin', 'Retribution', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(2, 2, 12, 2, 258, 47464, 'Phase 3', 'Paladin', 'Retribution', 'Trinket1', 'Horde', 'Death''s Choice'),
(2, 2, 14, 1, 258, 47547, 'Phase 3', 'Paladin', 'Retribution', 'Back', 'Alliance', 'Varian''s Furor'),
(2, 2, 14, 2, 258, 47548, 'Phase 3', 'Paladin', 'Retribution', 'Back', 'Horde', 'Garrosh''s Rage'),
(2, 2, 17, 0, 258, 47661, 'Phase 3', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Valiance');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 264, 51277, 'Phase 4', 'Paladin', 'Retribution', 'Head', 'Both', 'Sanctified Lightsworn Helmet'),
(2, 2, 1, 0, 264, 50633, 'Phase 4', 'Paladin', 'Retribution', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(2, 2, 2, 0, 264, 51279, 'Phase 4', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Sanctified Lightsworn Shoulderplates'),
(2, 2, 4, 0, 264, 51275, 'Phase 4', 'Paladin', 'Retribution', 'Chest', 'Both', 'Sanctified Lightsworn Battleplate'),
(2, 2, 5, 0, 264, 50707, 'Phase 4', 'Paladin', 'Retribution', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(2, 2, 6, 0, 264, 51278, 'Phase 4', 'Paladin', 'Retribution', 'Legs', 'Both', 'Sanctified Lightsworn Legplates'),
(2, 2, 7, 0, 264, 50607, 'Phase 4', 'Paladin', 'Retribution', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(2, 2, 8, 0, 264, 50659, 'Phase 4', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Polar Bear Claw Bracers'),
(2, 2, 9, 0, 264, 50690, 'Phase 4', 'Paladin', 'Retribution', 'Hands', 'Both', 'Fleshrending Gauntlets'),
(2, 2, 10, 0, 264, 50657, 'Phase 4', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Skeleton Lord''s Circle'),
(2, 2, 12, 0, 264, 50706, 'Phase 4', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Tiny Abomination in a Jar'),
(2, 2, 14, 0, 264, 50653, 'Phase 4', 'Paladin', 'Retribution', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(2, 2, 15, 0, 264, 49623, 'Phase 4', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Shadowmourne'),
(2, 2, 17, 0, 264, 50455, 'Phase 4', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Three Truths');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(2, 2, 0, 0, 290, 51277, 'Phase 5', 'Paladin', 'Retribution', 'Head', 'Both', 'Sanctified Lightsworn Helmet'),
(2, 2, 1, 0, 290, 54581, 'Phase 5', 'Paladin', 'Retribution', 'Neck', 'Both', 'Penumbra Pendant'),
(2, 2, 2, 0, 290, 51279, 'Phase 5', 'Paladin', 'Retribution', 'Shoulders', 'Both', 'Sanctified Lightsworn Shoulderplates'),
(2, 2, 4, 0, 290, 51275, 'Phase 5', 'Paladin', 'Retribution', 'Chest', 'Both', 'Sanctified Lightsworn Battleplate'),
(2, 2, 5, 0, 290, 50707, 'Phase 5', 'Paladin', 'Retribution', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(2, 2, 6, 0, 290, 51278, 'Phase 5', 'Paladin', 'Retribution', 'Legs', 'Both', 'Sanctified Lightsworn Legplates'),
(2, 2, 7, 0, 290, 54578, 'Phase 5', 'Paladin', 'Retribution', 'Feet', 'Both', 'Apocalypse''s Advance'),
(2, 2, 8, 0, 290, 54580, 'Phase 5', 'Paladin', 'Retribution', 'Wrists', 'Both', 'Umbrage Armbands'),
(2, 2, 9, 0, 290, 50690, 'Phase 5', 'Paladin', 'Retribution', 'Hands', 'Both', 'Fleshrending Gauntlets'),
(2, 2, 10, 0, 290, 50402, 'Phase 5', 'Paladin', 'Retribution', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(2, 2, 11, 0, 290, 54576, 'Phase 5', 'Paladin', 'Retribution', 'Finger2', 'Both', 'Signet of Twilight'),
(2, 2, 12, 0, 290, 54590, 'Phase 5', 'Paladin', 'Retribution', 'Trinket1', 'Both', 'Sharpened Twilight Scale'),
(2, 2, 13, 0, 290, 50706, 'Phase 5', 'Paladin', 'Retribution', 'Trinket2', 'Both', 'Tiny Abomination in a Jar'),
(2, 2, 14, 0, 290, 50653, 'Phase 5', 'Paladin', 'Retribution', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(2, 2, 15, 0, 290, 49623, 'Phase 5', 'Paladin', 'Retribution', 'MainHand', 'Both', 'Shadowmourne'),
(2, 2, 17, 0, 290, 50455, 'Phase 5', 'Paladin', 'Retribution', 'Ranged', 'Both', 'Libram of Three Truths');


-- ============================================================
-- Hunter (3)
-- ============================================================
-- Beast Mastery (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Mask of the Unforgiven'),
(3, 0, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Mark of Fordring'),
(3, 0, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(3, 0, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 0, 5, 0, 66, 14502, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Frostbite Girdle'),
(3, 0, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 0, 7, 0, 66, 13967, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 0, 8, 0, 66, 13211, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 0, 9, 0, 66, 15063, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 0, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Painweaver Band'),
(3, 0, 11, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Blackstone Ring'),
(3, 0, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 0, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Hand of Justice'),
(3, 0, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 0, 15, 0, 66, 12940, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Dal''Rend''s Sacred Charge'),
(3, 0, 16, 0, 66, 12939, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'OffHand', 'Both', 'Dal''Rend''s Tribal Guardian'),
(3, 0, 17, 0, 66, 18738, 'Phase 1 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 76, 18421, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Backwood Helm'),
(3, 0, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Mark of Fordring'),
(3, 0, 2, 0, 76, 13358, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Wyrmtongue Shoulders'),
(3, 0, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 0, 5, 0, 76, 18393, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Warpwood Binding'),
(3, 0, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 0, 7, 0, 76, 13967, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 0, 8, 0, 76, 13211, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 0, 9, 0, 76, 15063, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 0, 10, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Tarnished Elven Ring'),
(3, 0, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(3, 0, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 0, 13, 0, 76, 18473, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 0, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 0, 15, 0, 76, 18520, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Barbarous Blade'),
(3, 0, 17, 0, 76, 18738, 'Phase 2 (Pre-Raid)', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 78, 16846, 'Phase 2', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Giantstalker''s Helmet'),
(3, 0, 1, 0, 78, 18404, 'Phase 2', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(3, 0, 2, 0, 78, 16848, 'Phase 2', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Giantstalker''s Epaulets'),
(3, 0, 4, 0, 78, 16845, 'Phase 2', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Giantstalker''s Breastplate'),
(3, 0, 5, 0, 78, 16851, 'Phase 2', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Giantstalker''s Belt'),
(3, 0, 6, 0, 78, 16847, 'Phase 2', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Giantstalker''s Leggings'),
(3, 0, 7, 0, 78, 16849, 'Phase 2', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Giantstalker''s Boots'),
(3, 0, 8, 0, 78, 16850, 'Phase 2', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Giantstalker''s Bracers'),
(3, 0, 9, 0, 78, 16852, 'Phase 2', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Giantstalker''s Gloves'),
(3, 0, 10, 0, 78, 17063, 'Phase 2', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Band of Accuria'),
(3, 0, 11, 0, 78, 18821, 'Phase 2', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 0, 12, 0, 78, 13965, 'Phase 2', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 0, 13, 0, 78, 18473, 'Phase 2', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 0, 14, 0, 78, 17102, 'Phase 2', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 0, 15, 0, 78, 18832, 'Phase 2', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Brutality Blade'),
(3, 0, 16, 0, 78, 18805, 'Phase 2', 'Hunter', 'Beast Mastery', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 0, 17, 0, 78, 18713, 'Phase 2', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Rhok''delar, Longbow of the Ancient Keepers');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 83, 16939, 'Phase 3', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 0, 1, 0, 83, 19377, 'Phase 3', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 0, 2, 0, 83, 16937, 'Phase 3', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 0, 4, 0, 83, 16942, 'Phase 3', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 0, 5, 0, 83, 16936, 'Phase 3', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 0, 6, 0, 83, 16938, 'Phase 3', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 0, 7, 0, 83, 16941, 'Phase 3', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 0, 8, 0, 83, 16935, 'Phase 3', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 0, 9, 0, 83, 16940, 'Phase 3', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 0, 10, 0, 83, 19325, 'Phase 3', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 0, 11, 0, 83, 18821, 'Phase 3', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 0, 12, 0, 83, 13965, 'Phase 3', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 0, 13, 0, 83, 19406, 'Phase 3', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Drake Fang Talisman'),
(3, 0, 14, 0, 83, 17102, 'Phase 3', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 0, 15, 0, 83, 18832, 'Phase 3', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Brutality Blade'),
(3, 0, 16, 0, 83, 18805, 'Phase 3', 'Hunter', 'Beast Mastery', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 0, 17, 0, 83, 19361, 'Phase 3', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 88, 16939, 'Phase 5', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 0, 1, 0, 88, 19377, 'Phase 5', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 0, 2, 0, 88, 16937, 'Phase 5', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 0, 4, 0, 88, 16942, 'Phase 5', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 0, 5, 0, 88, 16936, 'Phase 5', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 0, 6, 0, 88, 16938, 'Phase 5', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 0, 7, 0, 88, 16941, 'Phase 5', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 0, 8, 0, 88, 16935, 'Phase 5', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 0, 9, 0, 88, 16940, 'Phase 5', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 0, 10, 0, 88, 19325, 'Phase 5', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 0, 11, 0, 88, 17063, 'Phase 5', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Band of Accuria'),
(3, 0, 12, 0, 88, 21670, 'Phase 5', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 0, 13, 0, 88, 23570, 'Phase 5', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Jom Gabbar'),
(3, 0, 14, 0, 88, 21710, 'Phase 5', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Cloak of the Fallen God'),
(3, 0, 15, 0, 88, 21673, 'Phase 5', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Silithid Claw'),
(3, 0, 16, 0, 88, 19859, 'Phase 5', 'Hunter', 'Beast Mastery', 'OffHand', 'Both', 'Fang of the Faceless'),
(3, 0, 17, 0, 88, 19361, 'Phase 5', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 92, 22438, 'Phase 6', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Cryptstalker Headpiece'),
(3, 0, 1, 0, 92, 23053, 'Phase 6', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Stormrage''s Talisman of Seething'),
(3, 0, 2, 0, 92, 22439, 'Phase 6', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Cryptstalker Spaulders'),
(3, 0, 4, 0, 92, 22436, 'Phase 6', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Cryptstalker Tunic'),
(3, 0, 5, 0, 92, 22442, 'Phase 6', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Cryptstalker Girdle'),
(3, 0, 6, 0, 92, 22437, 'Phase 6', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Cryptstalker Legguards'),
(3, 0, 7, 0, 92, 22440, 'Phase 6', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Cryptstalker Boots'),
(3, 0, 8, 0, 92, 22443, 'Phase 6', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Cryptstalker Wristguards'),
(3, 0, 9, 0, 92, 16571, 'Phase 6', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'General''s Chain Gloves'),
(3, 0, 10, 0, 92, 23067, 'Phase 6', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Ring of the Cryptstalker'),
(3, 0, 11, 0, 92, 22961, 'Phase 6', 'Hunter', 'Beast Mastery', 'Finger2', 'Both', 'Band of Reanimation'),
(3, 0, 12, 0, 92, 21670, 'Phase 6', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 0, 13, 0, 92, 23041, 'Phase 6', 'Hunter', 'Beast Mastery', 'Trinket2', 'Both', 'Slayer''s Crest'),
(3, 0, 14, 0, 92, 23045, 'Phase 6', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Shroud of Dominion'),
(3, 0, 15, 0, 92, 22816, 'Phase 6', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Hatchet of Sundered Bone'),
(3, 0, 16, 0, 92, 22802, 'Phase 6', 'Hunter', 'Beast Mastery', 'OffHand', 'Both', 'Kingsfall'),
(3, 0, 17, 0, 92, 22812, 'Phase 6', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Nerubian Slavemaker');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 200, 37188, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Plunderer''s Helmet'),
(3, 0, 1, 0, 200, 40678, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Pendant of the Outcast Hero'),
(3, 0, 2, 0, 200, 37679, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Spaulders of the Abomination'),
(3, 0, 4, 0, 200, 39579, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Heroes'' Cryptstalker Tunic'),
(3, 0, 6, 0, 200, 37669, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Leggings of the Stone Halls'),
(3, 0, 7, 0, 200, 44297, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Boots of the Neverending Path'),
(3, 0, 8, 0, 200, 41224, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Deadly Gladiator''s Wristguards of Triumph'),
(3, 0, 9, 0, 200, 43734, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Heroes'' Cryptstalker Handguards'),
(3, 0, 10, 0, 200, 40586, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Band of the Kirin Tor'),
(3, 0, 12, 0, 200, 44253, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(3, 0, 14, 0, 200, 42068, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Deadly Gladiator''s Cloak of Victory'),
(3, 0, 15, 0, 200, 44249, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'MainHand', 'Both', 'Runeblade of Demonstrable Power'),
(3, 0, 17, 0, 200, 44504, 'Pre-Raid', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Nesingwary 4000');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 224, 40505, 'Phase 1', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Valorous Cryptstalker Headpiece'),
(3, 0, 1, 0, 224, 44664, 'Phase 1', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(3, 0, 2, 0, 224, 40507, 'Phase 1', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Valorous Cryptstalker Spaulders'),
(3, 0, 4, 0, 224, 43998, 'Phase 1', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Chestguard of Flagrant Prowess'),
(3, 0, 5, 0, 224, 39762, 'Phase 1', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Torn Web Wrapping'),
(3, 0, 6, 0, 224, 40331, 'Phase 1', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Leggings of Failed Escape'),
(3, 0, 7, 0, 224, 40549, 'Phase 1', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Boots of the Renewed Flight'),
(3, 0, 8, 0, 224, 40282, 'Phase 1', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Slime Stream Bands'),
(3, 0, 9, 0, 224, 40541, 'Phase 1', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(3, 0, 10, 0, 224, 40474, 'Phase 1', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Surge Needle Ring'),
(3, 0, 12, 0, 224, 40431, 'Phase 1', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Fury of the Five Flights'),
(3, 0, 14, 0, 224, 40403, 'Phase 1', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Drape of the Deadly Foe'),
(3, 0, 17, 0, 224, 40385, 'Phase 1', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Envoy of Mortality');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 245, 45610, 'Phase 2', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Boundless Gaze'),
(3, 0, 1, 0, 245, 45517, 'Phase 2', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Pendulum of Infinity'),
(3, 0, 2, 0, 245, 45300, 'Phase 2', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Mantle of Fiery Vengeance'),
(3, 0, 4, 0, 245, 45473, 'Phase 2', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Embrace of the Gladiator'),
(3, 0, 5, 0, 245, 45553, 'Phase 2', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Belt of Dragons'),
(3, 0, 6, 0, 245, 45536, 'Phase 2', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(3, 0, 7, 0, 245, 45989, 'Phase 2', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Tempered Mercury Greaves'),
(3, 0, 8, 0, 245, 45869, 'Phase 2', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Fluxing Energy Coils'),
(3, 0, 9, 0, 245, 45444, 'Phase 2', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Gloves of the Steady Hand'),
(3, 0, 12, 0, 245, 46038, 'Phase 2', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Dark Matter'),
(3, 0, 14, 0, 245, 46032, 'Phase 2', 'Hunter', 'Beast Mastery', 'Back', 'Both', 'Drape of the Faceless General');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 258, 48262, 'Phase 3', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Windrunner''s Headpiece of Triumph'),
(3, 0, 1, 1, 258, 47060, 'Phase 3', 'Hunter', 'Beast Mastery', 'Neck', 'Alliance', 'Charge of the Demon Lord'),
(3, 0, 1, 2, 258, 47433, 'Phase 3', 'Hunter', 'Beast Mastery', 'Neck', 'Horde', 'Charge of the Eredar'),
(3, 0, 2, 0, 258, 48260, 'Phase 3', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Windrunner''s Spaulders of Triumph'),
(3, 0, 4, 0, 258, 48264, 'Phase 3', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Windrunner''s Tunic of Triumph'),
(3, 0, 5, 1, 258, 47153, 'Phase 3', 'Hunter', 'Beast Mastery', 'Waist', 'Alliance', 'Belt of Deathly Dominion'),
(3, 0, 5, 2, 258, 47472, 'Phase 3', 'Hunter', 'Beast Mastery', 'Waist', 'Horde', 'Waistguard of Deathly Dominion'),
(3, 0, 6, 1, 258, 47191, 'Phase 3', 'Hunter', 'Beast Mastery', 'Legs', 'Alliance', 'Legguards of the Lurking Threat'),
(3, 0, 6, 2, 258, 47480, 'Phase 3', 'Hunter', 'Beast Mastery', 'Legs', 'Horde', 'Leggings of the Lurking Threat'),
(3, 0, 7, 1, 258, 47109, 'Phase 3', 'Hunter', 'Beast Mastery', 'Feet', 'Alliance', 'Sabatons of Ruthless Judgment'),
(3, 0, 7, 2, 258, 47457, 'Phase 3', 'Hunter', 'Beast Mastery', 'Feet', 'Horde', 'Greaves of Ruthless Judgment'),
(3, 0, 8, 1, 258, 47074, 'Phase 3', 'Hunter', 'Beast Mastery', 'Wrists', 'Alliance', 'Bracers of the Untold Massacre'),
(3, 0, 8, 2, 258, 47442, 'Phase 3', 'Hunter', 'Beast Mastery', 'Wrists', 'Horde', 'Bracers of the Silent Massacre'),
(3, 0, 9, 0, 258, 48263, 'Phase 3', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Windrunner''s Handguards of Triumph'),
(3, 0, 10, 1, 258, 47075, 'Phase 3', 'Hunter', 'Beast Mastery', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(3, 0, 10, 2, 258, 47443, 'Phase 3', 'Hunter', 'Beast Mastery', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(3, 0, 12, 2, 258, 45931, 'Phase 3', 'Hunter', 'Beast Mastery', 'Trinket1', 'Horde', 'Mjolnir Runestone'),
(3, 0, 14, 1, 258, 47545, 'Phase 3', 'Hunter', 'Beast Mastery', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 0, 14, 2, 258, 47546, 'Phase 3', 'Hunter', 'Beast Mastery', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 0, 17, 1, 258, 47521, 'Phase 3', 'Hunter', 'Beast Mastery', 'Ranged', 'Alliance', 'BRK 1000'),
(3, 0, 17, 2, 258, 47523, 'Phase 3', 'Hunter', 'Beast Mastery', 'Ranged', 'Horde', 'Fezzik''s Autocannon');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 0, 0, 0, 264, 51286, 'Phase 4', 'Hunter', 'Beast Mastery', 'Head', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Headpiece'),
(3, 0, 1, 0, 264, 50633, 'Phase 4', 'Hunter', 'Beast Mastery', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(3, 0, 2, 0, 264, 51288, 'Phase 4', 'Hunter', 'Beast Mastery', 'Shoulders', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Spaulders'),
(3, 0, 4, 0, 264, 51289, 'Phase 4', 'Hunter', 'Beast Mastery', 'Chest', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Tunic'),
(3, 0, 5, 0, 264, 50688, 'Phase 4', 'Hunter', 'Beast Mastery', 'Waist', 'Both', 'Nerub''ar Stalker''s Cord'),
(3, 0, 6, 0, 264, 50645, 'Phase 4', 'Hunter', 'Beast Mastery', 'Legs', 'Both', 'Leggings of Northern Lights'),
(3, 0, 7, 0, 264, 50607, 'Phase 4', 'Hunter', 'Beast Mastery', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(3, 0, 8, 0, 264, 50655, 'Phase 4', 'Hunter', 'Beast Mastery', 'Wrists', 'Both', 'Scourge Hunter''s Vambraces'),
(3, 0, 9, 0, 264, 51285, 'Phase 4', 'Hunter', 'Beast Mastery', 'Hands', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Handguards'),
(3, 0, 10, 0, 264, 50402, 'Phase 4', 'Hunter', 'Beast Mastery', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(3, 0, 12, 0, 264, 50363, 'Phase 4', 'Hunter', 'Beast Mastery', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(3, 0, 14, 1, 264, 47545, 'Phase 4', 'Hunter', 'Beast Mastery', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 0, 14, 2, 264, 47546, 'Phase 4', 'Hunter', 'Beast Mastery', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 0, 17, 0, 264, 50733, 'Phase 4', 'Hunter', 'Beast Mastery', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Marksmanship (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Mask of the Unforgiven'),
(3, 1, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Mark of Fordring'),
(3, 1, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(3, 1, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 1, 5, 0, 66, 14502, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Frostbite Girdle'),
(3, 1, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 1, 7, 0, 66, 13967, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 1, 8, 0, 66, 13211, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 1, 9, 0, 66, 15063, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 1, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Painweaver Band'),
(3, 1, 11, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Blackstone Ring'),
(3, 1, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 1, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Hand of Justice'),
(3, 1, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 1, 15, 0, 66, 12940, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Dal''Rend''s Sacred Charge'),
(3, 1, 16, 0, 66, 12939, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'OffHand', 'Both', 'Dal''Rend''s Tribal Guardian'),
(3, 1, 17, 0, 66, 18738, 'Phase 1 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 76, 18421, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Backwood Helm'),
(3, 1, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Mark of Fordring'),
(3, 1, 2, 0, 76, 13358, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Wyrmtongue Shoulders'),
(3, 1, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 1, 5, 0, 76, 18393, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Warpwood Binding'),
(3, 1, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 1, 7, 0, 76, 13967, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 1, 8, 0, 76, 13211, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 1, 9, 0, 76, 15063, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 1, 10, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Tarnished Elven Ring'),
(3, 1, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(3, 1, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 1, 13, 0, 76, 18473, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 1, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 1, 15, 0, 76, 18520, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Barbarous Blade'),
(3, 1, 17, 0, 76, 18738, 'Phase 2 (Pre-Raid)', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 78, 16846, 'Phase 2', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Giantstalker''s Helmet'),
(3, 1, 1, 0, 78, 18404, 'Phase 2', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(3, 1, 2, 0, 78, 16848, 'Phase 2', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Giantstalker''s Epaulets'),
(3, 1, 4, 0, 78, 16845, 'Phase 2', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Giantstalker''s Breastplate'),
(3, 1, 5, 0, 78, 16851, 'Phase 2', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Giantstalker''s Belt'),
(3, 1, 6, 0, 78, 16847, 'Phase 2', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Giantstalker''s Leggings'),
(3, 1, 7, 0, 78, 16849, 'Phase 2', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Giantstalker''s Boots'),
(3, 1, 8, 0, 78, 16850, 'Phase 2', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Giantstalker''s Bracers'),
(3, 1, 9, 0, 78, 16852, 'Phase 2', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Giantstalker''s Gloves'),
(3, 1, 10, 0, 78, 17063, 'Phase 2', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Band of Accuria'),
(3, 1, 11, 0, 78, 18821, 'Phase 2', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 1, 12, 0, 78, 13965, 'Phase 2', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 1, 13, 0, 78, 18473, 'Phase 2', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 1, 14, 0, 78, 17102, 'Phase 2', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 1, 15, 0, 78, 18832, 'Phase 2', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Brutality Blade'),
(3, 1, 16, 0, 78, 18805, 'Phase 2', 'Hunter', 'Marksmanship', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 1, 17, 0, 78, 18713, 'Phase 2', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Rhok''delar, Longbow of the Ancient Keepers');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 83, 16939, 'Phase 3', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 1, 1, 0, 83, 19377, 'Phase 3', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 1, 2, 0, 83, 16937, 'Phase 3', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 1, 4, 0, 83, 16942, 'Phase 3', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 1, 5, 0, 83, 16936, 'Phase 3', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 1, 6, 0, 83, 16938, 'Phase 3', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 1, 7, 0, 83, 16941, 'Phase 3', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 1, 8, 0, 83, 16935, 'Phase 3', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 1, 9, 0, 83, 16940, 'Phase 3', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 1, 10, 0, 83, 19325, 'Phase 3', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 1, 11, 0, 83, 18821, 'Phase 3', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 1, 12, 0, 83, 13965, 'Phase 3', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 1, 13, 0, 83, 19406, 'Phase 3', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Drake Fang Talisman'),
(3, 1, 14, 0, 83, 17102, 'Phase 3', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 1, 15, 0, 83, 18832, 'Phase 3', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Brutality Blade'),
(3, 1, 16, 0, 83, 18805, 'Phase 3', 'Hunter', 'Marksmanship', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 1, 17, 0, 83, 19361, 'Phase 3', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 88, 16939, 'Phase 5', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 1, 1, 0, 88, 19377, 'Phase 5', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 1, 2, 0, 88, 16937, 'Phase 5', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 1, 4, 0, 88, 16942, 'Phase 5', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 1, 5, 0, 88, 16936, 'Phase 5', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 1, 6, 0, 88, 16938, 'Phase 5', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 1, 7, 0, 88, 16941, 'Phase 5', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 1, 8, 0, 88, 16935, 'Phase 5', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 1, 9, 0, 88, 16940, 'Phase 5', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 1, 10, 0, 88, 19325, 'Phase 5', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 1, 11, 0, 88, 17063, 'Phase 5', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Band of Accuria'),
(3, 1, 12, 0, 88, 21670, 'Phase 5', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 1, 13, 0, 88, 23570, 'Phase 5', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Jom Gabbar'),
(3, 1, 14, 0, 88, 21710, 'Phase 5', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Cloak of the Fallen God'),
(3, 1, 15, 0, 88, 21673, 'Phase 5', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Silithid Claw'),
(3, 1, 16, 0, 88, 19859, 'Phase 5', 'Hunter', 'Marksmanship', 'OffHand', 'Both', 'Fang of the Faceless'),
(3, 1, 17, 0, 88, 19361, 'Phase 5', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 92, 22438, 'Phase 6', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Cryptstalker Headpiece'),
(3, 1, 1, 0, 92, 23053, 'Phase 6', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Stormrage''s Talisman of Seething'),
(3, 1, 2, 0, 92, 22439, 'Phase 6', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Cryptstalker Spaulders'),
(3, 1, 4, 0, 92, 22436, 'Phase 6', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Cryptstalker Tunic'),
(3, 1, 5, 0, 92, 22442, 'Phase 6', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Cryptstalker Girdle'),
(3, 1, 6, 0, 92, 22437, 'Phase 6', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Cryptstalker Legguards'),
(3, 1, 7, 0, 92, 22440, 'Phase 6', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Cryptstalker Boots'),
(3, 1, 8, 0, 92, 22443, 'Phase 6', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Cryptstalker Wristguards'),
(3, 1, 9, 0, 92, 16571, 'Phase 6', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'General''s Chain Gloves'),
(3, 1, 10, 0, 92, 23067, 'Phase 6', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Ring of the Cryptstalker'),
(3, 1, 11, 0, 92, 22961, 'Phase 6', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Band of Reanimation'),
(3, 1, 12, 0, 92, 21670, 'Phase 6', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 1, 13, 0, 92, 23041, 'Phase 6', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Slayer''s Crest'),
(3, 1, 14, 0, 92, 23045, 'Phase 6', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Shroud of Dominion'),
(3, 1, 15, 0, 92, 22816, 'Phase 6', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Hatchet of Sundered Bone'),
(3, 1, 16, 0, 92, 22802, 'Phase 6', 'Hunter', 'Marksmanship', 'OffHand', 'Both', 'Kingsfall'),
(3, 1, 17, 0, 92, 22812, 'Phase 6', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Nerubian Slavemaker');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 200, 37188, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Plunderer''s Helmet'),
(3, 1, 1, 0, 200, 42645, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Titanium Impact Choker'),
(3, 1, 2, 0, 200, 37679, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Spaulders of the Abomination'),
(3, 1, 4, 0, 200, 37144, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Hauberk of the Arcane Wraith'),
(3, 1, 5, 0, 200, 37407, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Sovereign''s Belt'),
(3, 1, 6, 0, 200, 37669, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Leggings of the Stone Halls'),
(3, 1, 7, 0, 200, 37167, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Dragon Slayer''s Sabatons'),
(3, 1, 8, 0, 200, 37170, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Interwoven Scale Bracers'),
(3, 1, 9, 0, 200, 37886, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Handgrips of the Savage Emissary'),
(3, 1, 10, 0, 200, 42642, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Titanium Impact Band'),
(3, 1, 12, 0, 200, 40684, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Mirror of Truth'),
(3, 1, 14, 0, 200, 43566, 'Pre-Raid', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Ice Striker''s Cloak'),
(3, 1, 15, 0, 200, 44249, 'Pre-Raid', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Runeblade of Demonstrable Power');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 224, 40543, 'Phase 1', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Blue Aspect Helm'),
(3, 1, 1, 0, 224, 44664, 'Phase 1', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(3, 1, 2, 0, 224, 40507, 'Phase 1', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Valorous Cryptstalker Spaulders'),
(3, 1, 4, 0, 224, 40193, 'Phase 1', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Tunic of Masked Suffering'),
(3, 1, 5, 0, 224, 40275, 'Phase 1', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Depraved Linked Belt'),
(3, 1, 6, 0, 224, 40506, 'Phase 1', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Valorous Cryptstalker Legguards'),
(3, 1, 7, 0, 224, 40549, 'Phase 1', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Boots of the Renewed Flight'),
(3, 1, 8, 0, 224, 40282, 'Phase 1', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Slime Stream Bands'),
(3, 1, 9, 0, 224, 40541, 'Phase 1', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(3, 1, 10, 0, 224, 40074, 'Phase 1', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Strong-Handed Ring'),
(3, 1, 12, 0, 224, 40684, 'Phase 1', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Mirror of Truth'),
(3, 1, 14, 0, 224, 40403, 'Phase 1', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Drape of the Deadly Foe');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 245, 46143, 'Phase 2', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Conqueror''s Scourgestalker Headpiece'),
(3, 1, 1, 0, 245, 45517, 'Phase 2', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Pendulum of Infinity'),
(3, 1, 2, 0, 245, 46145, 'Phase 2', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Conqueror''s Scourgestalker Spaulders'),
(3, 1, 4, 0, 245, 46141, 'Phase 2', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Conqueror''s Scourgestalker Tunic'),
(3, 1, 5, 0, 245, 45467, 'Phase 2', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Belt of the Betrayed'),
(3, 1, 6, 0, 245, 46144, 'Phase 2', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Conqueror''s Scourgestalker Legguards'),
(3, 1, 7, 0, 245, 45244, 'Phase 2', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Greaves of Swift Vengeance'),
(3, 1, 8, 0, 245, 45454, 'Phase 2', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Frost-bound Chain Bracers'),
(3, 1, 9, 0, 245, 45444, 'Phase 2', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Gloves of the Steady Hand'),
(3, 1, 10, 0, 245, 45456, 'Phase 2', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Loop of the Agile'),
(3, 1, 12, 0, 245, 45931, 'Phase 2', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Mjolnir Runestone'),
(3, 1, 14, 0, 245, 46032, 'Phase 2', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Drape of the Faceless General'),
(3, 1, 15, 0, 245, 45498, 'Phase 2', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Lotrafen, Spear of the Damned'),
(3, 1, 17, 0, 245, 45570, 'Phase 2', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Skyforge Crossbow');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 258, 48262, 'Phase 3', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Windrunner''s Headpiece of Triumph'),
(3, 1, 1, 1, 258, 47060, 'Phase 3', 'Hunter', 'Marksmanship', 'Neck', 'Alliance', 'Charge of the Demon Lord'),
(3, 1, 1, 2, 258, 47433, 'Phase 3', 'Hunter', 'Marksmanship', 'Neck', 'Horde', 'Charge of the Eredar'),
(3, 1, 2, 0, 258, 48260, 'Phase 3', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Windrunner''s Spaulders of Triumph'),
(3, 1, 4, 0, 258, 48264, 'Phase 3', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Windrunner''s Tunic of Triumph'),
(3, 1, 5, 1, 258, 47153, 'Phase 3', 'Hunter', 'Marksmanship', 'Waist', 'Alliance', 'Belt of Deathly Dominion'),
(3, 1, 5, 2, 258, 47472, 'Phase 3', 'Hunter', 'Marksmanship', 'Waist', 'Horde', 'Waistguard of Deathly Dominion'),
(3, 1, 6, 1, 258, 47191, 'Phase 3', 'Hunter', 'Marksmanship', 'Legs', 'Alliance', 'Legguards of the Lurking Threat'),
(3, 1, 6, 2, 258, 47480, 'Phase 3', 'Hunter', 'Marksmanship', 'Legs', 'Horde', 'Leggings of the Lurking Threat'),
(3, 1, 7, 1, 258, 47109, 'Phase 3', 'Hunter', 'Marksmanship', 'Feet', 'Alliance', 'Sabatons of Ruthless Judgment'),
(3, 1, 7, 2, 258, 47457, 'Phase 3', 'Hunter', 'Marksmanship', 'Feet', 'Horde', 'Greaves of Ruthless Judgment'),
(3, 1, 8, 1, 258, 47074, 'Phase 3', 'Hunter', 'Marksmanship', 'Wrists', 'Alliance', 'Bracers of the Untold Massacre'),
(3, 1, 8, 2, 258, 47442, 'Phase 3', 'Hunter', 'Marksmanship', 'Wrists', 'Horde', 'Bracers of the Silent Massacre'),
(3, 1, 9, 0, 258, 48263, 'Phase 3', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Windrunner''s Handguards of Triumph'),
(3, 1, 10, 1, 258, 47075, 'Phase 3', 'Hunter', 'Marksmanship', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(3, 1, 10, 2, 258, 47443, 'Phase 3', 'Hunter', 'Marksmanship', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(3, 1, 12, 2, 258, 45931, 'Phase 3', 'Hunter', 'Marksmanship', 'Trinket1', 'Horde', 'Mjolnir Runestone'),
(3, 1, 14, 1, 258, 47545, 'Phase 3', 'Hunter', 'Marksmanship', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 1, 14, 2, 258, 47546, 'Phase 3', 'Hunter', 'Marksmanship', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 1, 17, 1, 258, 47521, 'Phase 3', 'Hunter', 'Marksmanship', 'Ranged', 'Alliance', 'BRK 1000'),
(3, 1, 17, 2, 258, 47523, 'Phase 3', 'Hunter', 'Marksmanship', 'Ranged', 'Horde', 'Fezzik''s Autocannon');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 264, 51286, 'Phase 4', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Headpiece'),
(3, 1, 1, 0, 264, 50633, 'Phase 4', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(3, 1, 2, 0, 264, 51288, 'Phase 4', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Spaulders'),
(3, 1, 4, 0, 264, 51289, 'Phase 4', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Tunic'),
(3, 1, 5, 0, 264, 50688, 'Phase 4', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Nerub''ar Stalker''s Cord'),
(3, 1, 6, 0, 264, 50645, 'Phase 4', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Leggings of Northern Lights'),
(3, 1, 7, 0, 264, 50607, 'Phase 4', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(3, 1, 8, 0, 264, 50655, 'Phase 4', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Scourge Hunter''s Vambraces'),
(3, 1, 9, 0, 264, 51285, 'Phase 4', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Handguards'),
(3, 1, 10, 0, 264, 50402, 'Phase 4', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(3, 1, 12, 0, 264, 50363, 'Phase 4', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(3, 1, 14, 1, 264, 47545, 'Phase 4', 'Hunter', 'Marksmanship', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 1, 14, 2, 264, 47546, 'Phase 4', 'Hunter', 'Marksmanship', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 1, 15, 0, 264, 50735, 'Phase 4', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(3, 1, 17, 0, 264, 50733, 'Phase 4', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 1, 0, 0, 290, 51286, 'Phase 5', 'Hunter', 'Marksmanship', 'Head', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Headpiece'),
(3, 1, 1, 0, 290, 50633, 'Phase 5', 'Hunter', 'Marksmanship', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(3, 1, 2, 0, 290, 51288, 'Phase 5', 'Hunter', 'Marksmanship', 'Shoulders', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Spaulders'),
(3, 1, 4, 0, 290, 51289, 'Phase 5', 'Hunter', 'Marksmanship', 'Chest', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Tunic'),
(3, 1, 5, 0, 290, 50688, 'Phase 5', 'Hunter', 'Marksmanship', 'Waist', 'Both', 'Nerub''ar Stalker''s Cord'),
(3, 1, 6, 0, 290, 50645, 'Phase 5', 'Hunter', 'Marksmanship', 'Legs', 'Both', 'Leggings of Northern Lights'),
(3, 1, 7, 0, 290, 54577, 'Phase 5', 'Hunter', 'Marksmanship', 'Feet', 'Both', 'Returning Footfalls'),
(3, 1, 8, 0, 290, 50655, 'Phase 5', 'Hunter', 'Marksmanship', 'Wrists', 'Both', 'Scourge Hunter''s Vambraces'),
(3, 1, 9, 0, 290, 51285, 'Phase 5', 'Hunter', 'Marksmanship', 'Hands', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Handguards'),
(3, 1, 10, 0, 290, 54576, 'Phase 5', 'Hunter', 'Marksmanship', 'Finger1', 'Both', 'Signet of Twilight'),
(3, 1, 11, 0, 290, 50402, 'Phase 5', 'Hunter', 'Marksmanship', 'Finger2', 'Both', 'Ashen Band of Endless Vengeance'),
(3, 1, 12, 0, 290, 50363, 'Phase 5', 'Hunter', 'Marksmanship', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(3, 1, 13, 0, 290, 54590, 'Phase 5', 'Hunter', 'Marksmanship', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(3, 1, 14, 0, 290, 47545, 'Phase 5', 'Hunter', 'Marksmanship', 'Back', 'Both', 'Vereesa''s Dexterity'),
(3, 1, 15, 0, 290, 50735, 'Phase 5', 'Hunter', 'Marksmanship', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(3, 1, 17, 0, 290, 50733, 'Phase 5', 'Hunter', 'Marksmanship', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Survival (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Head', 'Both', 'Mask of the Unforgiven'),
(3, 2, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Neck', 'Both', 'Mark of Fordring'),
(3, 2, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(3, 2, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 2, 5, 0, 66, 14502, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Waist', 'Both', 'Frostbite Girdle'),
(3, 2, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 2, 7, 0, 66, 13967, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 2, 8, 0, 66, 13211, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 2, 9, 0, 66, 15063, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 2, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Finger1', 'Both', 'Painweaver Band'),
(3, 2, 11, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Finger2', 'Both', 'Blackstone Ring'),
(3, 2, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 2, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Hand of Justice'),
(3, 2, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 2, 15, 0, 66, 12940, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'MainHand', 'Both', 'Dal''Rend''s Sacred Charge'),
(3, 2, 16, 0, 66, 12939, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'OffHand', 'Both', 'Dal''Rend''s Tribal Guardian'),
(3, 2, 17, 0, 66, 18738, 'Phase 1 (Pre-Raid)', 'Hunter', 'Survival', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 76, 18421, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Head', 'Both', 'Backwood Helm'),
(3, 2, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Neck', 'Both', 'Mark of Fordring'),
(3, 2, 2, 0, 76, 13358, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Wyrmtongue Shoulders'),
(3, 2, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Chest', 'Both', 'Savage Gladiator Chain'),
(3, 2, 5, 0, 76, 18393, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Waist', 'Both', 'Warpwood Binding'),
(3, 2, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Legs', 'Both', 'Devilsaur Leggings'),
(3, 2, 7, 0, 76, 13967, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Feet', 'Both', 'Windreaver Greaves'),
(3, 2, 8, 0, 76, 13211, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Wrists', 'Both', 'Slashclaw Bracers'),
(3, 2, 9, 0, 76, 15063, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(3, 2, 10, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Finger1', 'Both', 'Tarnished Elven Ring'),
(3, 2, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(3, 2, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 2, 13, 0, 76, 18473, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 2, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Back', 'Both', 'Cape of the Black Baron'),
(3, 2, 15, 0, 76, 18520, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'MainHand', 'Both', 'Barbarous Blade'),
(3, 2, 17, 0, 76, 18738, 'Phase 2 (Pre-Raid)', 'Hunter', 'Survival', 'Ranged', 'Both', 'Carapace Spine Crossbow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 78, 16846, 'Phase 2', 'Hunter', 'Survival', 'Head', 'Both', 'Giantstalker''s Helmet'),
(3, 2, 1, 0, 78, 18404, 'Phase 2', 'Hunter', 'Survival', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(3, 2, 2, 0, 78, 16848, 'Phase 2', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Giantstalker''s Epaulets'),
(3, 2, 4, 0, 78, 16845, 'Phase 2', 'Hunter', 'Survival', 'Chest', 'Both', 'Giantstalker''s Breastplate'),
(3, 2, 5, 0, 78, 16851, 'Phase 2', 'Hunter', 'Survival', 'Waist', 'Both', 'Giantstalker''s Belt'),
(3, 2, 6, 0, 78, 16847, 'Phase 2', 'Hunter', 'Survival', 'Legs', 'Both', 'Giantstalker''s Leggings'),
(3, 2, 7, 0, 78, 16849, 'Phase 2', 'Hunter', 'Survival', 'Feet', 'Both', 'Giantstalker''s Boots'),
(3, 2, 8, 0, 78, 16850, 'Phase 2', 'Hunter', 'Survival', 'Wrists', 'Both', 'Giantstalker''s Bracers'),
(3, 2, 9, 0, 78, 16852, 'Phase 2', 'Hunter', 'Survival', 'Hands', 'Both', 'Giantstalker''s Gloves'),
(3, 2, 10, 0, 78, 17063, 'Phase 2', 'Hunter', 'Survival', 'Finger1', 'Both', 'Band of Accuria'),
(3, 2, 11, 0, 78, 18821, 'Phase 2', 'Hunter', 'Survival', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 2, 12, 0, 78, 13965, 'Phase 2', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 2, 13, 0, 78, 18473, 'Phase 2', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Royal Seal of Eldre''Thalas'),
(3, 2, 14, 0, 78, 17102, 'Phase 2', 'Hunter', 'Survival', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 2, 15, 0, 78, 18832, 'Phase 2', 'Hunter', 'Survival', 'MainHand', 'Both', 'Brutality Blade'),
(3, 2, 16, 0, 78, 18805, 'Phase 2', 'Hunter', 'Survival', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 2, 17, 0, 78, 18713, 'Phase 2', 'Hunter', 'Survival', 'Ranged', 'Both', 'Rhok''delar, Longbow of the Ancient Keepers');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 83, 16939, 'Phase 3', 'Hunter', 'Survival', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 2, 1, 0, 83, 19377, 'Phase 3', 'Hunter', 'Survival', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 2, 2, 0, 83, 16937, 'Phase 3', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 2, 4, 0, 83, 16942, 'Phase 3', 'Hunter', 'Survival', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 2, 5, 0, 83, 16936, 'Phase 3', 'Hunter', 'Survival', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 2, 6, 0, 83, 16938, 'Phase 3', 'Hunter', 'Survival', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 2, 7, 0, 83, 16941, 'Phase 3', 'Hunter', 'Survival', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 2, 8, 0, 83, 16935, 'Phase 3', 'Hunter', 'Survival', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 2, 9, 0, 83, 16940, 'Phase 3', 'Hunter', 'Survival', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 2, 10, 0, 83, 19325, 'Phase 3', 'Hunter', 'Survival', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 2, 11, 0, 83, 18821, 'Phase 3', 'Hunter', 'Survival', 'Finger2', 'Both', 'Quick Strike Ring'),
(3, 2, 12, 0, 83, 13965, 'Phase 3', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(3, 2, 13, 0, 83, 19406, 'Phase 3', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Drake Fang Talisman'),
(3, 2, 14, 0, 83, 17102, 'Phase 3', 'Hunter', 'Survival', 'Back', 'Both', 'Cloak of the Shrouded Mists'),
(3, 2, 15, 0, 83, 18832, 'Phase 3', 'Hunter', 'Survival', 'MainHand', 'Both', 'Brutality Blade'),
(3, 2, 16, 0, 83, 18805, 'Phase 3', 'Hunter', 'Survival', 'OffHand', 'Both', 'Core Hound Tooth'),
(3, 2, 17, 0, 83, 19361, 'Phase 3', 'Hunter', 'Survival', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 88, 16939, 'Phase 5', 'Hunter', 'Survival', 'Head', 'Both', 'Dragonstalker''s Helm'),
(3, 2, 1, 0, 88, 19377, 'Phase 5', 'Hunter', 'Survival', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(3, 2, 2, 0, 88, 16937, 'Phase 5', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Dragonstalker''s Spaulders'),
(3, 2, 4, 0, 88, 16942, 'Phase 5', 'Hunter', 'Survival', 'Chest', 'Both', 'Dragonstalker''s Breastplate'),
(3, 2, 5, 0, 88, 16936, 'Phase 5', 'Hunter', 'Survival', 'Waist', 'Both', 'Dragonstalker''s Belt'),
(3, 2, 6, 0, 88, 16938, 'Phase 5', 'Hunter', 'Survival', 'Legs', 'Both', 'Dragonstalker''s Legguards'),
(3, 2, 7, 0, 88, 16941, 'Phase 5', 'Hunter', 'Survival', 'Feet', 'Both', 'Dragonstalker''s Greaves'),
(3, 2, 8, 0, 88, 16935, 'Phase 5', 'Hunter', 'Survival', 'Wrists', 'Both', 'Dragonstalker''s Bracers'),
(3, 2, 9, 0, 88, 16940, 'Phase 5', 'Hunter', 'Survival', 'Hands', 'Both', 'Dragonstalker''s Gauntlets'),
(3, 2, 10, 0, 88, 19325, 'Phase 5', 'Hunter', 'Survival', 'Finger1', 'Both', 'Don Julio''s Band'),
(3, 2, 11, 0, 88, 17063, 'Phase 5', 'Hunter', 'Survival', 'Finger2', 'Both', 'Band of Accuria'),
(3, 2, 12, 0, 88, 21670, 'Phase 5', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 2, 13, 0, 88, 23570, 'Phase 5', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Jom Gabbar'),
(3, 2, 14, 0, 88, 21710, 'Phase 5', 'Hunter', 'Survival', 'Back', 'Both', 'Cloak of the Fallen God'),
(3, 2, 15, 0, 88, 21673, 'Phase 5', 'Hunter', 'Survival', 'MainHand', 'Both', 'Silithid Claw'),
(3, 2, 16, 0, 88, 19859, 'Phase 5', 'Hunter', 'Survival', 'OffHand', 'Both', 'Fang of the Faceless'),
(3, 2, 17, 0, 88, 19361, 'Phase 5', 'Hunter', 'Survival', 'Ranged', 'Both', 'Ashjre''thul, Crossbow of Smiting');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 92, 22438, 'Phase 6', 'Hunter', 'Survival', 'Head', 'Both', 'Cryptstalker Headpiece'),
(3, 2, 1, 0, 92, 23053, 'Phase 6', 'Hunter', 'Survival', 'Neck', 'Both', 'Stormrage''s Talisman of Seething'),
(3, 2, 2, 0, 92, 22439, 'Phase 6', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Cryptstalker Spaulders'),
(3, 2, 4, 0, 92, 22436, 'Phase 6', 'Hunter', 'Survival', 'Chest', 'Both', 'Cryptstalker Tunic'),
(3, 2, 5, 0, 92, 22442, 'Phase 6', 'Hunter', 'Survival', 'Waist', 'Both', 'Cryptstalker Girdle'),
(3, 2, 6, 0, 92, 22437, 'Phase 6', 'Hunter', 'Survival', 'Legs', 'Both', 'Cryptstalker Legguards'),
(3, 2, 7, 0, 92, 22440, 'Phase 6', 'Hunter', 'Survival', 'Feet', 'Both', 'Cryptstalker Boots'),
(3, 2, 8, 0, 92, 22443, 'Phase 6', 'Hunter', 'Survival', 'Wrists', 'Both', 'Cryptstalker Wristguards'),
(3, 2, 9, 0, 92, 16571, 'Phase 6', 'Hunter', 'Survival', 'Hands', 'Both', 'General''s Chain Gloves'),
(3, 2, 10, 0, 92, 23067, 'Phase 6', 'Hunter', 'Survival', 'Finger1', 'Both', 'Ring of the Cryptstalker'),
(3, 2, 11, 0, 92, 22961, 'Phase 6', 'Hunter', 'Survival', 'Finger2', 'Both', 'Band of Reanimation'),
(3, 2, 12, 0, 92, 21670, 'Phase 6', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Badge of the Swarmguard'),
(3, 2, 13, 0, 92, 23041, 'Phase 6', 'Hunter', 'Survival', 'Trinket2', 'Both', 'Slayer''s Crest'),
(3, 2, 14, 0, 92, 23045, 'Phase 6', 'Hunter', 'Survival', 'Back', 'Both', 'Shroud of Dominion'),
(3, 2, 15, 0, 92, 22816, 'Phase 6', 'Hunter', 'Survival', 'MainHand', 'Both', 'Hatchet of Sundered Bone'),
(3, 2, 16, 0, 92, 22802, 'Phase 6', 'Hunter', 'Survival', 'OffHand', 'Both', 'Kingsfall'),
(3, 2, 17, 0, 92, 22812, 'Phase 6', 'Hunter', 'Survival', 'Ranged', 'Both', 'Nerubian Slavemaker');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 200, 42551, 'Pre-Raid', 'Hunter', 'Survival', 'Head', 'Both', 'Truesight Ice Blinders'),
(3, 2, 1, 0, 200, 40678, 'Pre-Raid', 'Hunter', 'Survival', 'Neck', 'Both', 'Pendant of the Outcast Hero'),
(3, 2, 2, 0, 200, 37679, 'Pre-Raid', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Spaulders of the Abomination'),
(3, 2, 4, 0, 200, 44295, 'Pre-Raid', 'Hunter', 'Survival', 'Chest', 'Both', 'Polished Regimental Hauberk'),
(3, 2, 5, 0, 200, 40692, 'Pre-Raid', 'Hunter', 'Survival', 'Waist', 'Both', 'Vereesa''s Silver Chain Belt'),
(3, 2, 6, 0, 200, 37669, 'Pre-Raid', 'Hunter', 'Survival', 'Legs', 'Both', 'Leggings of the Stone Halls'),
(3, 2, 7, 0, 200, 44297, 'Pre-Raid', 'Hunter', 'Survival', 'Feet', 'Both', 'Boots of the Neverending Path'),
(3, 2, 8, 0, 200, 37170, 'Pre-Raid', 'Hunter', 'Survival', 'Wrists', 'Both', 'Interwoven Scale Bracers'),
(3, 2, 9, 0, 200, 37886, 'Pre-Raid', 'Hunter', 'Survival', 'Hands', 'Both', 'Handgrips of the Savage Emissary'),
(3, 2, 10, 0, 200, 37685, 'Pre-Raid', 'Hunter', 'Survival', 'Finger1', 'Both', 'Mobius Band'),
(3, 2, 12, 0, 200, 40684, 'Pre-Raid', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Mirror of Truth'),
(3, 2, 14, 0, 200, 43406, 'Pre-Raid', 'Hunter', 'Survival', 'Back', 'Both', 'Cloak of the Gushing Wound'),
(3, 2, 15, 0, 200, 37883, 'Pre-Raid', 'Hunter', 'Survival', 'MainHand', 'Both', 'Staff of Trickery'),
(3, 2, 17, 0, 200, 37191, 'Pre-Raid', 'Hunter', 'Survival', 'Ranged', 'Both', 'Drake-Mounted Crossbow');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 224, 40505, 'Phase 1', 'Hunter', 'Survival', 'Head', 'Both', 'Valorous Cryptstalker Headpiece'),
(3, 2, 1, 0, 224, 44664, 'Phase 1', 'Hunter', 'Survival', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(3, 2, 2, 0, 224, 40507, 'Phase 1', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Valorous Cryptstalker Spaulders'),
(3, 2, 4, 0, 224, 43998, 'Phase 1', 'Hunter', 'Survival', 'Chest', 'Both', 'Chestguard of Flagrant Prowess'),
(3, 2, 5, 0, 224, 39762, 'Phase 1', 'Hunter', 'Survival', 'Waist', 'Both', 'Torn Web Wrapping'),
(3, 2, 6, 0, 224, 40331, 'Phase 1', 'Hunter', 'Survival', 'Legs', 'Both', 'Leggings of Failed Escape'),
(3, 2, 7, 0, 224, 40549, 'Phase 1', 'Hunter', 'Survival', 'Feet', 'Both', 'Boots of the Renewed Flight'),
(3, 2, 8, 0, 224, 40282, 'Phase 1', 'Hunter', 'Survival', 'Wrists', 'Both', 'Slime Stream Bands'),
(3, 2, 9, 0, 224, 40541, 'Phase 1', 'Hunter', 'Survival', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(3, 2, 10, 0, 224, 40074, 'Phase 1', 'Hunter', 'Survival', 'Finger1', 'Both', 'Strong-Handed Ring'),
(3, 2, 12, 0, 224, 40431, 'Phase 1', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Fury of the Five Flights'),
(3, 2, 14, 0, 224, 40403, 'Phase 1', 'Hunter', 'Survival', 'Back', 'Both', 'Drape of the Deadly Foe'),
(3, 2, 15, 0, 224, 40388, 'Phase 1', 'Hunter', 'Survival', 'MainHand', 'Both', 'Journey''s End'),
(3, 2, 17, 0, 224, 40385, 'Phase 1', 'Hunter', 'Survival', 'Ranged', 'Both', 'Envoy of Mortality');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 245, 45610, 'Phase 2', 'Hunter', 'Survival', 'Head', 'Both', 'Boundless Gaze'),
(3, 2, 1, 0, 245, 45517, 'Phase 2', 'Hunter', 'Survival', 'Neck', 'Both', 'Pendulum of Infinity'),
(3, 2, 2, 0, 245, 45300, 'Phase 2', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Mantle of Fiery Vengeance'),
(3, 2, 4, 0, 245, 45473, 'Phase 2', 'Hunter', 'Survival', 'Chest', 'Both', 'Embrace of the Gladiator'),
(3, 2, 5, 0, 245, 46095, 'Phase 2', 'Hunter', 'Survival', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(3, 2, 6, 0, 245, 45536, 'Phase 2', 'Hunter', 'Survival', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(3, 2, 7, 0, 245, 45244, 'Phase 2', 'Hunter', 'Survival', 'Feet', 'Both', 'Greaves of Swift Vengeance'),
(3, 2, 8, 0, 245, 45869, 'Phase 2', 'Hunter', 'Survival', 'Wrists', 'Both', 'Fluxing Energy Coils'),
(3, 2, 9, 0, 245, 45444, 'Phase 2', 'Hunter', 'Survival', 'Hands', 'Both', 'Gloves of the Steady Hand'),
(3, 2, 12, 0, 245, 44253, 'Phase 2', 'Hunter', 'Survival', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(3, 2, 14, 0, 245, 46032, 'Phase 2', 'Hunter', 'Survival', 'Back', 'Both', 'Drape of the Faceless General');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 258, 48262, 'Phase 3', 'Hunter', 'Survival', 'Head', 'Both', 'Windrunner''s Headpiece of Triumph'),
(3, 2, 1, 1, 258, 47060, 'Phase 3', 'Hunter', 'Survival', 'Neck', 'Alliance', 'Charge of the Demon Lord'),
(3, 2, 1, 2, 258, 47433, 'Phase 3', 'Hunter', 'Survival', 'Neck', 'Horde', 'Charge of the Eredar'),
(3, 2, 2, 0, 258, 48260, 'Phase 3', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Windrunner''s Spaulders of Triumph'),
(3, 2, 4, 0, 258, 48264, 'Phase 3', 'Hunter', 'Survival', 'Chest', 'Both', 'Windrunner''s Tunic of Triumph'),
(3, 2, 5, 1, 258, 47153, 'Phase 3', 'Hunter', 'Survival', 'Waist', 'Alliance', 'Belt of Deathly Dominion'),
(3, 2, 5, 2, 258, 47472, 'Phase 3', 'Hunter', 'Survival', 'Waist', 'Horde', 'Waistguard of Deathly Dominion'),
(3, 2, 6, 1, 258, 47191, 'Phase 3', 'Hunter', 'Survival', 'Legs', 'Alliance', 'Legguards of the Lurking Threat'),
(3, 2, 6, 2, 258, 47480, 'Phase 3', 'Hunter', 'Survival', 'Legs', 'Horde', 'Leggings of the Lurking Threat'),
(3, 2, 7, 1, 258, 47109, 'Phase 3', 'Hunter', 'Survival', 'Feet', 'Alliance', 'Sabatons of Ruthless Judgment'),
(3, 2, 7, 2, 258, 47457, 'Phase 3', 'Hunter', 'Survival', 'Feet', 'Horde', 'Greaves of Ruthless Judgment'),
(3, 2, 8, 1, 258, 47074, 'Phase 3', 'Hunter', 'Survival', 'Wrists', 'Alliance', 'Bracers of the Untold Massacre'),
(3, 2, 8, 2, 258, 47442, 'Phase 3', 'Hunter', 'Survival', 'Wrists', 'Horde', 'Bracers of the Silent Massacre'),
(3, 2, 9, 0, 258, 48263, 'Phase 3', 'Hunter', 'Survival', 'Hands', 'Both', 'Windrunner''s Handguards of Triumph'),
(3, 2, 10, 1, 258, 47075, 'Phase 3', 'Hunter', 'Survival', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(3, 2, 10, 2, 258, 47443, 'Phase 3', 'Hunter', 'Survival', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(3, 2, 12, 2, 258, 44253, 'Phase 3', 'Hunter', 'Survival', 'Trinket1', 'Horde', 'Darkmoon Card: Greatness'),
(3, 2, 14, 1, 258, 47545, 'Phase 3', 'Hunter', 'Survival', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 2, 14, 2, 258, 47546, 'Phase 3', 'Hunter', 'Survival', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 2, 17, 1, 258, 47521, 'Phase 3', 'Hunter', 'Survival', 'Ranged', 'Alliance', 'BRK 1000'),
(3, 2, 17, 2, 258, 47523, 'Phase 3', 'Hunter', 'Survival', 'Ranged', 'Horde', 'Fezzik''s Autocannon');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(3, 2, 0, 0, 264, 51286, 'Phase 4', 'Hunter', 'Survival', 'Head', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Headpiece'),
(3, 2, 1, 0, 264, 50633, 'Phase 4', 'Hunter', 'Survival', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(3, 2, 2, 0, 264, 51288, 'Phase 4', 'Hunter', 'Survival', 'Shoulders', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Spaulders'),
(3, 2, 4, 0, 264, 51289, 'Phase 4', 'Hunter', 'Survival', 'Chest', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Tunic'),
(3, 2, 5, 0, 264, 50688, 'Phase 4', 'Hunter', 'Survival', 'Waist', 'Both', 'Nerub''ar Stalker''s Cord'),
(3, 2, 6, 0, 264, 50645, 'Phase 4', 'Hunter', 'Survival', 'Legs', 'Both', 'Leggings of Northern Lights'),
(3, 2, 7, 0, 264, 50607, 'Phase 4', 'Hunter', 'Survival', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(3, 2, 8, 0, 264, 50655, 'Phase 4', 'Hunter', 'Survival', 'Wrists', 'Both', 'Scourge Hunter''s Vambraces'),
(3, 2, 9, 0, 264, 51285, 'Phase 4', 'Hunter', 'Survival', 'Hands', 'Both', 'Sanctified Ahn''Kahar Blood Hunter''s Handguards'),
(3, 2, 10, 0, 264, 50402, 'Phase 4', 'Hunter', 'Survival', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(3, 2, 12, 1, 264, 47131, 'Phase 4', 'Hunter', 'Survival', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(3, 2, 12, 2, 264, 47464, 'Phase 4', 'Hunter', 'Survival', 'Trinket1', 'Horde', 'Death''s Choice'),
(3, 2, 14, 1, 264, 47545, 'Phase 4', 'Hunter', 'Survival', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(3, 2, 14, 2, 264, 47546, 'Phase 4', 'Hunter', 'Survival', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(3, 2, 17, 0, 264, 50733, 'Phase 4', 'Hunter', 'Survival', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');


-- ============================================================
-- Rogue (4)
-- ============================================================
-- Assassination (tab 0)
-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 120, 32087, 'Pre-Raid', 'Rogue', 'Assassination', 'Head', 'Both', 'Mask of the Deceiver'),
(4, 0, 1, 0, 120, 29381, 'Pre-Raid', 'Rogue', 'Assassination', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 0, 2, 0, 120, 27797, 'Pre-Raid', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 0, 4, 0, 120, 30730, 'Pre-Raid', 'Rogue', 'Assassination', 'Chest', 'Both', 'Terrorweave Tunic'),
(4, 0, 5, 0, 120, 29247, 'Pre-Raid', 'Rogue', 'Assassination', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 0, 6, 0, 120, 30538, 'Pre-Raid', 'Rogue', 'Assassination', 'Legs', 'Both', 'Midnight Legguards'),
(4, 0, 7, 0, 120, 25686, 'Pre-Raid', 'Rogue', 'Assassination', 'Feet', 'Both', 'Fel Leather Boots'),
(4, 0, 8, 0, 120, 29246, 'Pre-Raid', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 0, 9, 0, 120, 27531, 'Pre-Raid', 'Rogue', 'Assassination', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 0, 10, 0, 120, 30738, 'Pre-Raid', 'Rogue', 'Assassination', 'Finger1', 'Both', 'Ring of Reciprocity'),
(4, 0, 12, 0, 120, 28288, 'Pre-Raid', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Abacus of Violent Odds'),
(4, 0, 13, 0, 120, 29383, 'Pre-Raid', 'Rogue', 'Assassination', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(4, 0, 14, 0, 120, 24259, 'Pre-Raid', 'Rogue', 'Assassination', 'Back', 'Both', 'Vengeance Wrap'),
(4, 0, 15, 0, 120, 31331, 'Pre-Raid', 'Rogue', 'Assassination', 'MainHand', 'Both', 'The Night Blade'),
(4, 0, 16, 0, 120, 29346, 'Pre-Raid', 'Rogue', 'Assassination', 'OffHand', 'Both', 'Feltooth Eviscerator'),
(4, 0, 17, 2, 120, 29152, 'Pre-Raid', 'Rogue', 'Assassination', 'Ranged', 'Horde', 'Marksman''s Bow');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 125, 29044, 'Phase 1', 'Rogue', 'Assassination', 'Head', 'Both', 'Netherblade Facemask'),
(4, 0, 1, 0, 125, 29381, 'Phase 1', 'Rogue', 'Assassination', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 0, 2, 0, 125, 27797, 'Phase 1', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 0, 4, 0, 125, 29045, 'Phase 1', 'Rogue', 'Assassination', 'Chest', 'Both', 'Netherblade Chestpiece'),
(4, 0, 5, 0, 125, 29247, 'Phase 1', 'Rogue', 'Assassination', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 0, 6, 0, 125, 28741, 'Phase 1', 'Rogue', 'Assassination', 'Legs', 'Both', 'Skulker''s Greaves'),
(4, 0, 7, 0, 125, 28545, 'Phase 1', 'Rogue', 'Assassination', 'Feet', 'Both', 'Edgewalker Longboots'),
(4, 0, 8, 0, 125, 29246, 'Phase 1', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 0, 9, 0, 125, 27531, 'Phase 1', 'Rogue', 'Assassination', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 0, 10, 0, 125, 28649, 'Phase 1', 'Rogue', 'Assassination', 'Finger1', 'Both', 'Garona''s Signet Ring'),
(4, 0, 11, 0, 125, 28757, 'Phase 1', 'Rogue', 'Assassination', 'Finger2', 'Both', 'Ring of a Thousand Marks'),
(4, 0, 12, 0, 125, 29383, 'Phase 1', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(4, 0, 13, 0, 125, 28830, 'Phase 1', 'Rogue', 'Assassination', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(4, 0, 14, 0, 125, 28672, 'Phase 1', 'Rogue', 'Assassination', 'Back', 'Both', 'Drape of the Dark Reavers'),
(4, 0, 15, 0, 125, 28312, 'Phase 1', 'Rogue', 'Assassination', 'MainHand', 'Both', 'Gladiator''s Shanker'),
(4, 0, 16, 0, 125, 29346, 'Phase 1', 'Rogue', 'Assassination', 'OffHand', 'Both', 'Feltooth Eviscerator'),
(4, 0, 17, 0, 125, 28772, 'Phase 1', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Sunfury Bow of the Phoenix');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 200, 42550, 'Pre-Raid', 'Rogue', 'Assassination', 'Head', 'Both', 'Weakness Spectralizers'),
(4, 0, 1, 0, 200, 40678, 'Pre-Raid', 'Rogue', 'Assassination', 'Neck', 'Both', 'Pendant of the Outcast Hero'),
(4, 0, 2, 0, 200, 43481, 'Pre-Raid', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Trollwoven Spaulders'),
(4, 0, 4, 0, 200, 39558, 'Pre-Raid', 'Rogue', 'Assassination', 'Chest', 'Both', 'Heroes'' Bonescythe Breastplate'),
(4, 0, 5, 0, 200, 40694, 'Pre-Raid', 'Rogue', 'Assassination', 'Waist', 'Both', 'Jorach''s Crocolisk Skin Belt'),
(4, 0, 8, 0, 200, 34448, 'Pre-Raid', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Slayer''s Bracers'),
(4, 0, 9, 0, 200, 39560, 'Pre-Raid', 'Rogue', 'Assassination', 'Hands', 'Both', 'Heroes'' Bonescythe Gauntlets'),
(4, 0, 12, 0, 200, 40684, 'Pre-Raid', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Mirror of Truth'),
(4, 0, 14, 0, 200, 43566, 'Pre-Raid', 'Rogue', 'Assassination', 'Back', 'Both', 'Ice Striker''s Cloak'),
(4, 0, 15, 0, 200, 37856, 'Pre-Raid', 'Rogue', 'Assassination', 'MainHand', 'Both', 'Librarian''s Paper Cutter'),
(4, 0, 17, 0, 200, 44504, 'Pre-Raid', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Nesingwary 4000');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 224, 43729, 'Phase 1', 'Rogue', 'Assassination', 'Head', 'Both', 'Valorous Bonescythe Helmet'),
(4, 0, 1, 0, 224, 44664, 'Phase 1', 'Rogue', 'Assassination', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(4, 0, 2, 0, 224, 40502, 'Phase 1', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Valorous Bonescythe Pauldrons'),
(4, 0, 4, 0, 224, 40539, 'Phase 1', 'Rogue', 'Assassination', 'Chest', 'Both', 'Chestguard of the Recluse'),
(4, 0, 5, 0, 224, 40205, 'Phase 1', 'Rogue', 'Assassination', 'Waist', 'Both', 'Stalk-Skin Belt'),
(4, 0, 6, 0, 224, 44011, 'Phase 1', 'Rogue', 'Assassination', 'Legs', 'Both', 'Leggings of the Honored'),
(4, 0, 7, 0, 224, 39701, 'Phase 1', 'Rogue', 'Assassination', 'Feet', 'Both', 'Dawnwalkers'),
(4, 0, 8, 0, 224, 39765, 'Phase 1', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Sinner''s Bindings'),
(4, 0, 9, 0, 224, 40541, 'Phase 1', 'Rogue', 'Assassination', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(4, 0, 10, 0, 224, 40474, 'Phase 1', 'Rogue', 'Assassination', 'Finger1', 'Both', 'Surge Needle Ring'),
(4, 0, 12, 0, 224, 44253, 'Phase 1', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(4, 0, 14, 0, 224, 40403, 'Phase 1', 'Rogue', 'Assassination', 'Back', 'Both', 'Drape of the Deadly Foe'),
(4, 0, 15, 0, 224, 39714, 'Phase 1', 'Rogue', 'Assassination', 'MainHand', 'Both', 'Webbed Death'),
(4, 0, 17, 0, 224, 40385, 'Phase 1', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Envoy of Mortality');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 245, 46125, 'Phase 2', 'Rogue', 'Assassination', 'Head', 'Both', 'Conqueror''s Terrorblade Helmet'),
(4, 0, 1, 0, 245, 45517, 'Phase 2', 'Rogue', 'Assassination', 'Neck', 'Both', 'Pendulum of Infinity'),
(4, 0, 2, 0, 245, 45245, 'Phase 2', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Shoulderpads of the Intruder'),
(4, 0, 4, 0, 245, 45473, 'Phase 2', 'Rogue', 'Assassination', 'Chest', 'Both', 'Embrace of the Gladiator'),
(4, 0, 5, 0, 245, 46095, 'Phase 2', 'Rogue', 'Assassination', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(4, 0, 6, 0, 245, 45536, 'Phase 2', 'Rogue', 'Assassination', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(4, 0, 7, 0, 245, 45564, 'Phase 2', 'Rogue', 'Assassination', 'Feet', 'Both', 'Footpads of Silence'),
(4, 0, 8, 0, 245, 45611, 'Phase 2', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Solar Bindings'),
(4, 0, 9, 0, 245, 46124, 'Phase 2', 'Rogue', 'Assassination', 'Hands', 'Both', 'Conqueror''s Terrorblade Gauntlets'),
(4, 0, 12, 0, 245, 45609, 'Phase 2', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Comet''s Trail'),
(4, 0, 14, 0, 245, 45461, 'Phase 2', 'Rogue', 'Assassination', 'Back', 'Both', 'Drape of Icy Intent'),
(4, 0, 17, 0, 245, 45570, 'Phase 2', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Skyforge Crossbow');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 258, 48230, 'Phase 3', 'Rogue', 'Assassination', 'Head', 'Both', 'Helmet of Triumph'),
(4, 0, 2, 0, 258, 48228, 'Phase 3', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Pauldrons of Triumph'),
(4, 0, 4, 0, 258, 48232, 'Phase 3', 'Rogue', 'Assassination', 'Chest', 'Both', 'Breastplate of Triumph'),
(4, 0, 5, 1, 258, 47112, 'Phase 3', 'Rogue', 'Assassination', 'Waist', 'Alliance', 'Belt of the Merciless Killer'),
(4, 0, 5, 2, 258, 47460, 'Phase 3', 'Rogue', 'Assassination', 'Waist', 'Horde', 'Belt of the Pitiless Killer'),
(4, 0, 6, 1, 258, 46975, 'Phase 3', 'Rogue', 'Assassination', 'Legs', 'Alliance', 'Leggings of the Broken Beast'),
(4, 0, 6, 2, 258, 47420, 'Phase 3', 'Rogue', 'Assassination', 'Legs', 'Horde', 'Legwraps of the Broken Beast'),
(4, 0, 7, 1, 258, 47077, 'Phase 3', 'Rogue', 'Assassination', 'Feet', 'Alliance', 'Treads of the Icewalker'),
(4, 0, 7, 2, 258, 47445, 'Phase 3', 'Rogue', 'Assassination', 'Feet', 'Horde', 'Icewalker Treads'),
(4, 0, 8, 1, 258, 47155, 'Phase 3', 'Rogue', 'Assassination', 'Wrists', 'Alliance', 'Bracers of Dark Determination'),
(4, 0, 8, 2, 258, 47474, 'Phase 3', 'Rogue', 'Assassination', 'Wrists', 'Horde', 'Armbands of Dark Determination'),
(4, 0, 9, 0, 258, 48231, 'Phase 3', 'Rogue', 'Assassination', 'Hands', 'Both', 'Gauntlets of Triumph'),
(4, 0, 10, 1, 258, 47075, 'Phase 3', 'Rogue', 'Assassination', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(4, 0, 10, 2, 258, 47443, 'Phase 3', 'Rogue', 'Assassination', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(4, 0, 12, 1, 258, 47131, 'Phase 3', 'Rogue', 'Assassination', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(4, 0, 12, 2, 258, 47464, 'Phase 3', 'Rogue', 'Assassination', 'Trinket1', 'Horde', 'Death''s Choice'),
(4, 0, 14, 1, 258, 47545, 'Phase 3', 'Rogue', 'Assassination', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(4, 0, 14, 2, 258, 47546, 'Phase 3', 'Rogue', 'Assassination', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(4, 0, 17, 1, 258, 47521, 'Phase 3', 'Rogue', 'Assassination', 'Ranged', 'Alliance', 'BRK 1000'),
(4, 0, 17, 2, 258, 47523, 'Phase 3', 'Rogue', 'Assassination', 'Ranged', 'Horde', 'Fezzik''s Autocannon');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 264, 51252, 'Phase 4', 'Rogue', 'Assassination', 'Head', 'Both', 'Sanctified Shadowblade Helmet'),
(4, 0, 2, 0, 264, 51254, 'Phase 4', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Sanctified Shadowblade Pauldrons'),
(4, 0, 4, 0, 264, 50656, 'Phase 4', 'Rogue', 'Assassination', 'Chest', 'Both', 'Ikfirus''s Sack of Wonder'),
(4, 0, 5, 0, 264, 50707, 'Phase 4', 'Rogue', 'Assassination', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(4, 0, 6, 0, 264, 51253, 'Phase 4', 'Rogue', 'Assassination', 'Legs', 'Both', 'Sanctified Shadowblade Legplates'),
(4, 0, 7, 0, 264, 50607, 'Phase 4', 'Rogue', 'Assassination', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(4, 0, 8, 0, 264, 50670, 'Phase 4', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(4, 0, 9, 0, 264, 51251, 'Phase 4', 'Rogue', 'Assassination', 'Hands', 'Both', 'Sanctified Shadowblade Gauntlets'),
(4, 0, 10, 0, 264, 50604, 'Phase 4', 'Rogue', 'Assassination', 'Finger1', 'Both', 'Band of the Bone Colossus'),
(4, 0, 12, 0, 264, 50363, 'Phase 4', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(4, 0, 14, 0, 264, 50653, 'Phase 4', 'Rogue', 'Assassination', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(4, 0, 17, 0, 264, 50733, 'Phase 4', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 0, 0, 0, 290, 50713, 'Phase 5', 'Rogue', 'Assassination', 'Head', 'Both', 'Geistlord''s Punishment Sack'),
(4, 0, 1, 0, 290, 50633, 'Phase 5', 'Rogue', 'Assassination', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(4, 0, 2, 0, 290, 50646, 'Phase 5', 'Rogue', 'Assassination', 'Shoulders', 'Both', 'Cultist''s Bloodsoaked Spaulders'),
(4, 0, 4, 0, 290, 50656, 'Phase 5', 'Rogue', 'Assassination', 'Chest', 'Both', 'Ikfirus''s Sack of Wonder'),
(4, 0, 5, 0, 290, 47112, 'Phase 5', 'Rogue', 'Assassination', 'Waist', 'Both', 'Belt of the Merciless Killer'),
(4, 0, 6, 0, 290, 51253, 'Phase 5', 'Rogue', 'Assassination', 'Legs', 'Both', 'Sanctified Shadowblade Legplates'),
(4, 0, 7, 0, 290, 50607, 'Phase 5', 'Rogue', 'Assassination', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(4, 0, 8, 0, 290, 54580, 'Phase 5', 'Rogue', 'Assassination', 'Wrists', 'Both', 'Umbrage Armbands'),
(4, 0, 9, 0, 290, 51251, 'Phase 5', 'Rogue', 'Assassination', 'Hands', 'Both', 'Sanctified Shadowblade Gauntlets'),
(4, 0, 10, 0, 290, 50402, 'Phase 5', 'Rogue', 'Assassination', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(4, 0, 11, 0, 290, 54576, 'Phase 5', 'Rogue', 'Assassination', 'Finger2', 'Both', 'Signet of Twilight'),
(4, 0, 12, 0, 290, 54590, 'Phase 5', 'Rogue', 'Assassination', 'Trinket1', 'Both', 'Sharpened Twilight Scale'),
(4, 0, 13, 0, 290, 50706, 'Phase 5', 'Rogue', 'Assassination', 'Trinket2', 'Both', 'Tiny Abomination in a Jar'),
(4, 0, 14, 0, 290, 50653, 'Phase 5', 'Rogue', 'Assassination', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(4, 0, 15, 0, 290, 50736, 'Phase 5', 'Rogue', 'Assassination', 'MainHand', 'Both', 'Heaven''s Fall, Kryss of a Thousand Lies'),
(4, 0, 16, 0, 290, 50621, 'Phase 5', 'Rogue', 'Assassination', 'OffHand', 'Both', 'Lungbreaker'),
(4, 0, 17, 0, 290, 50733, 'Phase 5', 'Rogue', 'Assassination', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Combat (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 66, 13404, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Head', 'Both', 'Mask of the Unforgiven'),
(4, 1, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Neck', 'Both', 'Mark of Fordring'),
(4, 1, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(4, 1, 4, 0, 66, 14637, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Chest', 'Both', 'Cadaverous Armor'),
(4, 1, 5, 0, 66, 13252, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Waist', 'Both', 'Cloudrunner Girdle'),
(4, 1, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Legs', 'Both', 'Devilsaur Leggings'),
(4, 1, 7, 0, 66, 12553, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Feet', 'Both', 'Swiftwalker Boots'),
(4, 1, 8, 0, 66, 13120, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Wrists', 'Both', 'Deepfury Bracers'),
(4, 1, 9, 0, 66, 15063, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(4, 1, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Finger1', 'Both', 'Painweaver Band'),
(4, 1, 11, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Finger2', 'Both', 'Blackstone Ring'),
(4, 1, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(4, 1, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Hand of Justice'),
(4, 1, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Back', 'Both', 'Cape of the Black Baron'),
(4, 1, 15, 0, 66, 12783, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'MainHand', 'Both', 'Heartseeker'),
(4, 1, 16, 0, 66, 14555, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'OffHand', 'Both', 'Alcor''s Sunrazor'),
(4, 1, 17, 0, 66, 12651, 'Phase 1 (Pre-Raid)', 'Rogue', 'Combat', 'Ranged', 'Both', 'Blackcrow');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 76, 13404, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Head', 'Both', 'Mask of the Unforgiven'),
(4, 1, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Neck', 'Both', 'Mark of Fordring'),
(4, 1, 2, 0, 76, 12927, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(4, 1, 4, 0, 76, 14637, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Chest', 'Both', 'Cadaverous Armor'),
(4, 1, 5, 0, 76, 18505, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Waist', 'Both', 'Mugger''s Belt'),
(4, 1, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Legs', 'Both', 'Devilsaur Leggings'),
(4, 1, 7, 0, 76, 12553, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Feet', 'Both', 'Swiftwalker Boots'),
(4, 1, 8, 0, 76, 18375, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Wrists', 'Both', 'Bracers of the Eclipse'),
(4, 1, 9, 0, 76, 15063, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(4, 1, 10, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Finger1', 'Both', 'Tarnished Elven Ring'),
(4, 1, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(4, 1, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(4, 1, 13, 0, 76, 11815, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Hand of Justice'),
(4, 1, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Back', 'Both', 'Cape of the Black Baron'),
(4, 1, 15, 0, 76, 12783, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'MainHand', 'Both', 'Heartseeker'),
(4, 1, 16, 0, 76, 14555, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'OffHand', 'Both', 'Alcor''s Sunrazor'),
(4, 1, 17, 0, 76, 18323, 'Phase 2 (Pre-Raid)', 'Rogue', 'Combat', 'Ranged', 'Both', 'Satyr''s Bow');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 78, 16908, 'Phase 2', 'Rogue', 'Combat', 'Head', 'Both', 'Bloodfang Hood'),
(4, 1, 1, 0, 78, 18404, 'Phase 2', 'Rogue', 'Combat', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(4, 1, 2, 0, 78, 16823, 'Phase 2', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Nightslayer Shoulder Pads'),
(4, 1, 4, 0, 78, 16820, 'Phase 2', 'Rogue', 'Combat', 'Chest', 'Both', 'Nightslayer Chestpiece'),
(4, 1, 5, 0, 78, 16827, 'Phase 2', 'Rogue', 'Combat', 'Waist', 'Both', 'Nightslayer Belt'),
(4, 1, 6, 0, 78, 16909, 'Phase 2', 'Rogue', 'Combat', 'Legs', 'Both', 'Bloodfang Pants'),
(4, 1, 7, 0, 78, 16824, 'Phase 2', 'Rogue', 'Combat', 'Feet', 'Both', 'Nightslayer Boots'),
(4, 1, 8, 0, 78, 18375, 'Phase 2', 'Rogue', 'Combat', 'Wrists', 'Both', 'Bracers of the Eclipse'),
(4, 1, 9, 0, 78, 18823, 'Phase 2', 'Rogue', 'Combat', 'Hands', 'Both', 'Aged Core Leather Gloves'),
(4, 1, 10, 0, 78, 17063, 'Phase 2', 'Rogue', 'Combat', 'Finger1', 'Both', 'Band of Accuria'),
(4, 1, 11, 0, 78, 18500, 'Phase 2', 'Rogue', 'Combat', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(4, 1, 12, 0, 78, 13965, 'Phase 2', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(4, 1, 13, 0, 78, 11815, 'Phase 2', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Hand of Justice'),
(4, 1, 14, 0, 78, 13340, 'Phase 2', 'Rogue', 'Combat', 'Back', 'Both', 'Cape of the Black Baron'),
(4, 1, 15, 0, 78, 18816, 'Phase 2', 'Rogue', 'Combat', 'MainHand', 'Both', 'Perdition''s Blade'),
(4, 1, 16, 0, 78, 18805, 'Phase 2', 'Rogue', 'Combat', 'OffHand', 'Both', 'Core Hound Tooth'),
(4, 1, 17, 0, 78, 17069, 'Phase 2', 'Rogue', 'Combat', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 83, 16908, 'Phase 3', 'Rogue', 'Combat', 'Head', 'Both', 'Bloodfang Hood'),
(4, 1, 1, 0, 83, 19377, 'Phase 3', 'Rogue', 'Combat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(4, 1, 2, 0, 83, 16823, 'Phase 3', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Nightslayer Shoulder Pads'),
(4, 1, 4, 0, 83, 16905, 'Phase 3', 'Rogue', 'Combat', 'Chest', 'Both', 'Bloodfang Chestpiece'),
(4, 1, 5, 0, 83, 16910, 'Phase 3', 'Rogue', 'Combat', 'Waist', 'Both', 'Bloodfang Belt'),
(4, 1, 6, 0, 83, 16909, 'Phase 3', 'Rogue', 'Combat', 'Legs', 'Both', 'Bloodfang Pants'),
(4, 1, 7, 0, 83, 19381, 'Phase 3', 'Rogue', 'Combat', 'Feet', 'Both', 'Boots of the Shadow Flame'),
(4, 1, 8, 0, 83, 16911, 'Phase 3', 'Rogue', 'Combat', 'Wrists', 'Both', 'Bloodfang Bracers'),
(4, 1, 9, 0, 83, 18823, 'Phase 3', 'Rogue', 'Combat', 'Hands', 'Both', 'Aged Core Leather Gloves'),
(4, 1, 10, 0, 83, 17063, 'Phase 3', 'Rogue', 'Combat', 'Finger1', 'Both', 'Band of Accuria'),
(4, 1, 11, 0, 83, 19384, 'Phase 3', 'Rogue', 'Combat', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(4, 1, 12, 0, 83, 19406, 'Phase 3', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(4, 1, 13, 0, 83, 11815, 'Phase 3', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Hand of Justice'),
(4, 1, 14, 0, 83, 19398, 'Phase 3', 'Rogue', 'Combat', 'Back', 'Both', 'Cloak of Firemaw'),
(4, 1, 15, 0, 83, 18816, 'Phase 3', 'Rogue', 'Combat', 'MainHand', 'Both', 'Perdition''s Blade'),
(4, 1, 16, 0, 83, 18805, 'Phase 3', 'Rogue', 'Combat', 'OffHand', 'Both', 'Core Hound Tooth'),
(4, 1, 17, 0, 83, 17069, 'Phase 3', 'Rogue', 'Combat', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 88, 21360, 'Phase 5', 'Rogue', 'Combat', 'Head', 'Both', 'Deathdealer''s Helm'),
(4, 1, 1, 0, 88, 19377, 'Phase 5', 'Rogue', 'Combat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(4, 1, 2, 0, 88, 21361, 'Phase 5', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Deathdealer''s Spaulders'),
(4, 1, 4, 0, 88, 21364, 'Phase 5', 'Rogue', 'Combat', 'Chest', 'Both', 'Deathdealer''s Vest'),
(4, 1, 5, 0, 88, 21586, 'Phase 5', 'Rogue', 'Combat', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(4, 1, 6, 0, 88, 21362, 'Phase 5', 'Rogue', 'Combat', 'Legs', 'Both', 'Deathdealer''s Leggings'),
(4, 1, 7, 0, 88, 21359, 'Phase 5', 'Rogue', 'Combat', 'Feet', 'Both', 'Deathdealer''s Boots'),
(4, 1, 8, 0, 88, 21602, 'Phase 5', 'Rogue', 'Combat', 'Wrists', 'Both', 'Qiraji Execution Bracers'),
(4, 1, 9, 0, 88, 18823, 'Phase 5', 'Rogue', 'Combat', 'Hands', 'Both', 'Aged Core Leather Gloves'),
(4, 1, 10, 0, 88, 17063, 'Phase 5', 'Rogue', 'Combat', 'Finger1', 'Both', 'Band of Accuria'),
(4, 1, 11, 0, 88, 19384, 'Phase 5', 'Rogue', 'Combat', 'Finger2', 'Both', 'Master Dragonslayer''s Ring'),
(4, 1, 12, 0, 88, 19406, 'Phase 5', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(4, 1, 13, 0, 88, 23570, 'Phase 5', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Jom Gabbar'),
(4, 1, 14, 0, 88, 21701, 'Phase 5', 'Rogue', 'Combat', 'Back', 'Both', 'Cloak of Concentrated Hatred'),
(4, 1, 15, 0, 88, 21126, 'Phase 5', 'Rogue', 'Combat', 'MainHand', 'Both', 'Death''s Sting'),
(4, 1, 16, 0, 88, 21244, 'Phase 5', 'Rogue', 'Combat', 'OffHand', 'Both', 'Blessed Qiraji Pugio'),
(4, 1, 17, 0, 88, 17069, 'Phase 5', 'Rogue', 'Combat', 'Ranged', 'Both', 'Striker''s Mark');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 92, 22478, 'Phase 6', 'Rogue', 'Combat', 'Head', 'Both', 'Bonescythe Helmet'),
(4, 1, 1, 0, 92, 19377, 'Phase 6', 'Rogue', 'Combat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(4, 1, 2, 0, 92, 22479, 'Phase 6', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Bonescythe Pauldrons'),
(4, 1, 4, 0, 92, 22476, 'Phase 6', 'Rogue', 'Combat', 'Chest', 'Both', 'Bonescythe Breastplate'),
(4, 1, 5, 0, 92, 21586, 'Phase 6', 'Rogue', 'Combat', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(4, 1, 6, 0, 92, 22477, 'Phase 6', 'Rogue', 'Combat', 'Legs', 'Both', 'Bonescythe Legplates'),
(4, 1, 7, 0, 92, 22480, 'Phase 6', 'Rogue', 'Combat', 'Feet', 'Both', 'Bonescythe Sabatons'),
(4, 1, 8, 0, 92, 22483, 'Phase 6', 'Rogue', 'Combat', 'Wrists', 'Both', 'Bonescythe Bracers'),
(4, 1, 9, 0, 92, 22481, 'Phase 6', 'Rogue', 'Combat', 'Hands', 'Both', 'Bonescythe Gauntlets'),
(4, 1, 10, 0, 92, 23060, 'Phase 6', 'Rogue', 'Combat', 'Finger1', 'Both', 'Bonescythe Ring'),
(4, 1, 12, 0, 92, 23041, 'Phase 6', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Slayer''s Crest'),
(4, 1, 13, 0, 92, 22954, 'Phase 6', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Kiss of the Spider'),
(4, 1, 14, 0, 92, 23045, 'Phase 6', 'Rogue', 'Combat', 'Back', 'Both', 'Shroud of Dominion'),
(4, 1, 15, 0, 92, 22802, 'Phase 6', 'Rogue', 'Combat', 'MainHand', 'Both', 'Kingsfall'),
(4, 1, 16, 0, 92, 21126, 'Phase 6', 'Rogue', 'Combat', 'OffHand', 'Both', 'Death''s Sting'),
(4, 1, 17, 0, 92, 22812, 'Phase 6', 'Rogue', 'Combat', 'Ranged', 'Both', 'Nerubian Slavemaker');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 120, 32087, 'Pre-Raid', 'Rogue', 'Combat', 'Head', 'Both', 'Mask of the Deceiver'),
(4, 1, 1, 0, 120, 29381, 'Pre-Raid', 'Rogue', 'Combat', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 1, 2, 0, 120, 27797, 'Pre-Raid', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 1, 4, 0, 120, 30730, 'Pre-Raid', 'Rogue', 'Combat', 'Chest', 'Both', 'Terrorweave Tunic'),
(4, 1, 5, 0, 120, 29247, 'Pre-Raid', 'Rogue', 'Combat', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 1, 6, 0, 120, 30538, 'Pre-Raid', 'Rogue', 'Combat', 'Legs', 'Both', 'Midnight Legguards'),
(4, 1, 7, 0, 120, 25686, 'Pre-Raid', 'Rogue', 'Combat', 'Feet', 'Both', 'Fel Leather Boots'),
(4, 1, 8, 0, 120, 29246, 'Pre-Raid', 'Rogue', 'Combat', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 1, 9, 0, 120, 27531, 'Pre-Raid', 'Rogue', 'Combat', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 1, 10, 0, 120, 30738, 'Pre-Raid', 'Rogue', 'Combat', 'Finger1', 'Both', 'Ring of Reciprocity'),
(4, 1, 12, 0, 120, 28288, 'Pre-Raid', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Abacus of Violent Odds'),
(4, 1, 13, 0, 120, 29383, 'Pre-Raid', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(4, 1, 14, 0, 120, 24259, 'Pre-Raid', 'Rogue', 'Combat', 'Back', 'Both', 'Vengeance Wrap'),
(4, 1, 15, 0, 120, 31332, 'Pre-Raid', 'Rogue', 'Combat', 'MainHand', 'Both', 'Blinkstrike'),
(4, 1, 16, 0, 120, 28189, 'Pre-Raid', 'Rogue', 'Combat', 'OffHand', 'Both', 'Latro''s Shifting Sword'),
(4, 1, 17, 2, 120, 29152, 'Pre-Raid', 'Rogue', 'Combat', 'Ranged', 'Horde', 'Marksman''s Bow');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 125, 29044, 'Phase 1', 'Rogue', 'Combat', 'Head', 'Both', 'Netherblade Facemask'),
(4, 1, 1, 0, 125, 29381, 'Phase 1', 'Rogue', 'Combat', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 1, 2, 0, 125, 27797, 'Phase 1', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 1, 4, 0, 125, 29045, 'Phase 1', 'Rogue', 'Combat', 'Chest', 'Both', 'Netherblade Chestpiece'),
(4, 1, 5, 0, 125, 29247, 'Phase 1', 'Rogue', 'Combat', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 1, 6, 0, 125, 28741, 'Phase 1', 'Rogue', 'Combat', 'Legs', 'Both', 'Skulker''s Greaves'),
(4, 1, 7, 0, 125, 28545, 'Phase 1', 'Rogue', 'Combat', 'Feet', 'Both', 'Edgewalker Longboots'),
(4, 1, 8, 0, 125, 29246, 'Phase 1', 'Rogue', 'Combat', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 1, 9, 0, 125, 27531, 'Phase 1', 'Rogue', 'Combat', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 1, 10, 0, 125, 28649, 'Phase 1', 'Rogue', 'Combat', 'Finger1', 'Both', 'Garona''s Signet Ring'),
(4, 1, 11, 0, 125, 28757, 'Phase 1', 'Rogue', 'Combat', 'Finger2', 'Both', 'Ring of a Thousand Marks'),
(4, 1, 12, 0, 125, 29383, 'Phase 1', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(4, 1, 13, 0, 125, 28830, 'Phase 1', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(4, 1, 14, 0, 125, 28672, 'Phase 1', 'Rogue', 'Combat', 'Back', 'Both', 'Drape of the Dark Reavers'),
(4, 1, 15, 0, 125, 31332, 'Phase 1', 'Rogue', 'Combat', 'MainHand', 'Both', 'Blinkstrike'),
(4, 1, 16, 0, 125, 28307, 'Phase 1', 'Rogue', 'Combat', 'OffHand', 'Both', 'Gladiator''s Quickblade'),
(4, 1, 17, 0, 125, 28772, 'Phase 1', 'Rogue', 'Combat', 'Ranged', 'Both', 'Sunfury Bow of the Phoenix');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 200, 42550, 'Pre-Raid', 'Rogue', 'Combat', 'Head', 'Both', 'Weakness Spectralizers'),
(4, 1, 1, 0, 200, 42645, 'Pre-Raid', 'Rogue', 'Combat', 'Neck', 'Both', 'Titanium Impact Choker'),
(4, 1, 2, 0, 200, 43481, 'Pre-Raid', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Trollwoven Spaulders'),
(4, 1, 4, 0, 200, 39558, 'Pre-Raid', 'Rogue', 'Combat', 'Chest', 'Both', 'Heroes'' Bonescythe Breastplate'),
(4, 1, 5, 0, 200, 43484, 'Pre-Raid', 'Rogue', 'Combat', 'Waist', 'Both', 'Trollwoven Girdle'),
(4, 1, 6, 0, 200, 39564, 'Pre-Raid', 'Rogue', 'Combat', 'Legs', 'Both', 'Heroes'' Bonescythe Legplates'),
(4, 1, 7, 0, 200, 37666, 'Pre-Raid', 'Rogue', 'Combat', 'Feet', 'Both', 'Boots of the Whirling Mist'),
(4, 1, 8, 0, 200, 37853, 'Pre-Raid', 'Rogue', 'Combat', 'Wrists', 'Both', 'Advanced Tooled-Leather Bands'),
(4, 1, 9, 0, 200, 39560, 'Pre-Raid', 'Rogue', 'Combat', 'Hands', 'Both', 'Heroes'' Bonescythe Gauntlets'),
(4, 1, 10, 0, 200, 42642, 'Pre-Raid', 'Rogue', 'Combat', 'Finger1', 'Both', 'Titanium Impact Band'),
(4, 1, 12, 0, 200, 40684, 'Pre-Raid', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Mirror of Truth'),
(4, 1, 14, 0, 200, 43566, 'Pre-Raid', 'Rogue', 'Combat', 'Back', 'Both', 'Ice Striker''s Cloak'),
(4, 1, 15, 0, 200, 37871, 'Pre-Raid', 'Rogue', 'Combat', 'MainHand', 'Both', 'The Key'),
(4, 1, 17, 0, 200, 43284, 'Pre-Raid', 'Rogue', 'Combat', 'Ranged', 'Both', 'Amanitar Skullbow');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 224, 40499, 'Phase 1', 'Rogue', 'Combat', 'Head', 'Both', 'Valorous Bonescythe Helmet'),
(4, 1, 1, 0, 224, 44664, 'Phase 1', 'Rogue', 'Combat', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(4, 1, 2, 0, 224, 40502, 'Phase 1', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Valorous Bonescythe Pauldrons'),
(4, 1, 4, 0, 224, 40539, 'Phase 1', 'Rogue', 'Combat', 'Chest', 'Both', 'Chestguard of the Recluse'),
(4, 1, 5, 0, 224, 40205, 'Phase 1', 'Rogue', 'Combat', 'Waist', 'Both', 'Stalk-Skin Belt'),
(4, 1, 6, 0, 224, 44011, 'Phase 1', 'Rogue', 'Combat', 'Legs', 'Both', 'Leggings of the Honored'),
(4, 1, 7, 0, 224, 39701, 'Phase 1', 'Rogue', 'Combat', 'Feet', 'Both', 'Dawnwalkers'),
(4, 1, 8, 0, 224, 39765, 'Phase 1', 'Rogue', 'Combat', 'Wrists', 'Both', 'Sinner''s Bindings'),
(4, 1, 9, 0, 224, 40541, 'Phase 1', 'Rogue', 'Combat', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(4, 1, 10, 0, 224, 40074, 'Phase 1', 'Rogue', 'Combat', 'Finger1', 'Both', 'Strong-Handed Ring'),
(4, 1, 12, 0, 224, 40684, 'Phase 1', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Mirror of Truth'),
(4, 1, 14, 0, 224, 40403, 'Phase 1', 'Rogue', 'Combat', 'Back', 'Both', 'Drape of the Deadly Foe'),
(4, 1, 15, 0, 224, 40383, 'Phase 1', 'Rogue', 'Combat', 'MainHand', 'Both', 'Calamity''s Grasp'),
(4, 1, 17, 0, 224, 40385, 'Phase 1', 'Rogue', 'Combat', 'Ranged', 'Both', 'Envoy of Mortality');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 245, 46125, 'Phase 2', 'Rogue', 'Combat', 'Head', 'Both', 'Conqueror''s Terrorblade Helmet'),
(4, 1, 1, 0, 245, 45517, 'Phase 2', 'Rogue', 'Combat', 'Neck', 'Both', 'Pendulum of Infinity'),
(4, 1, 2, 0, 245, 46127, 'Phase 2', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Conqueror''s Terrorblade Pauldrons'),
(4, 1, 4, 0, 245, 45473, 'Phase 2', 'Rogue', 'Combat', 'Chest', 'Both', 'Embrace of the Gladiator'),
(4, 1, 5, 0, 245, 46095, 'Phase 2', 'Rogue', 'Combat', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(4, 1, 6, 0, 245, 45536, 'Phase 2', 'Rogue', 'Combat', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(4, 1, 7, 0, 245, 45564, 'Phase 2', 'Rogue', 'Combat', 'Feet', 'Both', 'Footpads of Silence'),
(4, 1, 8, 0, 245, 45611, 'Phase 2', 'Rogue', 'Combat', 'Wrists', 'Both', 'Solar Bindings'),
(4, 1, 9, 0, 245, 46124, 'Phase 2', 'Rogue', 'Combat', 'Hands', 'Both', 'Conqueror''s Terrorblade Gauntlets'),
(4, 1, 10, 0, 245, 45456, 'Phase 2', 'Rogue', 'Combat', 'Finger1', 'Both', 'Loop of the Agile'),
(4, 1, 12, 0, 245, 45609, 'Phase 2', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Comet''s Trail'),
(4, 1, 14, 0, 245, 46032, 'Phase 2', 'Rogue', 'Combat', 'Back', 'Both', 'Drape of the Faceless General'),
(4, 1, 17, 0, 245, 45296, 'Phase 2', 'Rogue', 'Combat', 'Ranged', 'Both', 'Twirling Blades');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 258, 48230, 'Phase 3', 'Rogue', 'Combat', 'Head', 'Both', 'Helmet of Triumph'),
(4, 1, 2, 0, 258, 48228, 'Phase 3', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Pauldrons of Triumph'),
(4, 1, 4, 0, 258, 48232, 'Phase 3', 'Rogue', 'Combat', 'Chest', 'Both', 'Breastplate of Triumph'),
(4, 1, 5, 1, 258, 47112, 'Phase 3', 'Rogue', 'Combat', 'Waist', 'Alliance', 'Belt of the Merciless Killer'),
(4, 1, 5, 2, 258, 47460, 'Phase 3', 'Rogue', 'Combat', 'Waist', 'Horde', 'Belt of the Pitiless Killer'),
(4, 1, 6, 1, 258, 46975, 'Phase 3', 'Rogue', 'Combat', 'Legs', 'Alliance', 'Leggings of the Broken Beast'),
(4, 1, 6, 2, 258, 47420, 'Phase 3', 'Rogue', 'Combat', 'Legs', 'Horde', 'Legwraps of the Broken Beast'),
(4, 1, 7, 1, 258, 47077, 'Phase 3', 'Rogue', 'Combat', 'Feet', 'Alliance', 'Treads of the Icewalker'),
(4, 1, 7, 2, 258, 47445, 'Phase 3', 'Rogue', 'Combat', 'Feet', 'Horde', 'Icewalker Treads'),
(4, 1, 8, 1, 258, 47155, 'Phase 3', 'Rogue', 'Combat', 'Wrists', 'Alliance', 'Bracers of Dark Determination'),
(4, 1, 8, 2, 258, 47474, 'Phase 3', 'Rogue', 'Combat', 'Wrists', 'Horde', 'Armbands of Dark Determination'),
(4, 1, 9, 0, 258, 48231, 'Phase 3', 'Rogue', 'Combat', 'Hands', 'Both', 'Gauntlets of Triumph'),
(4, 1, 10, 1, 258, 47075, 'Phase 3', 'Rogue', 'Combat', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(4, 1, 10, 2, 258, 47443, 'Phase 3', 'Rogue', 'Combat', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(4, 1, 12, 1, 258, 47131, 'Phase 3', 'Rogue', 'Combat', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(4, 1, 12, 2, 258, 47464, 'Phase 3', 'Rogue', 'Combat', 'Trinket1', 'Horde', 'Death''s Choice'),
(4, 1, 14, 1, 258, 47545, 'Phase 3', 'Rogue', 'Combat', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(4, 1, 14, 2, 258, 47546, 'Phase 3', 'Rogue', 'Combat', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(4, 1, 17, 1, 258, 47521, 'Phase 3', 'Rogue', 'Combat', 'Ranged', 'Alliance', 'BRK 1000'),
(4, 1, 17, 2, 258, 47523, 'Phase 3', 'Rogue', 'Combat', 'Ranged', 'Horde', 'Fezzik''s Autocannon');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 264, 51252, 'Phase 4', 'Rogue', 'Combat', 'Head', 'Both', 'Sanctified Shadowblade Helmet'),
(4, 1, 2, 0, 264, 51254, 'Phase 4', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Sanctified Shadowblade Pauldrons'),
(4, 1, 4, 0, 264, 50656, 'Phase 4', 'Rogue', 'Combat', 'Chest', 'Both', 'Ikfirus''s Sack of Wonder'),
(4, 1, 5, 0, 264, 50707, 'Phase 4', 'Rogue', 'Combat', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(4, 1, 6, 0, 264, 50697, 'Phase 4', 'Rogue', 'Combat', 'Legs', 'Both', 'Gangrenous Leggings'),
(4, 1, 7, 0, 264, 50607, 'Phase 4', 'Rogue', 'Combat', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(4, 1, 8, 0, 264, 50670, 'Phase 4', 'Rogue', 'Combat', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(4, 1, 9, 0, 264, 50675, 'Phase 4', 'Rogue', 'Combat', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(4, 1, 10, 0, 264, 50618, 'Phase 4', 'Rogue', 'Combat', 'Finger1', 'Both', 'Frostbrood Sapphire Ring'),
(4, 1, 12, 0, 264, 50363, 'Phase 4', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(4, 1, 14, 1, 264, 47545, 'Phase 4', 'Rogue', 'Combat', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(4, 1, 14, 2, 264, 47546, 'Phase 4', 'Rogue', 'Combat', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(4, 1, 17, 0, 264, 50733, 'Phase 4', 'Rogue', 'Combat', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 1, 0, 0, 290, 51252, 'Phase 5', 'Rogue', 'Combat', 'Head', 'Both', 'Sanctified Shadowblade Helmet'),
(4, 1, 1, 0, 290, 50633, 'Phase 5', 'Rogue', 'Combat', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(4, 1, 2, 0, 290, 51254, 'Phase 5', 'Rogue', 'Combat', 'Shoulders', 'Both', 'Sanctified Shadowblade Pauldrons'),
(4, 1, 4, 0, 290, 51250, 'Phase 5', 'Rogue', 'Combat', 'Chest', 'Both', 'Sanctified Shadowblade Breastplate'),
(4, 1, 5, 0, 290, 50707, 'Phase 5', 'Rogue', 'Combat', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(4, 1, 6, 0, 290, 51253, 'Phase 5', 'Rogue', 'Combat', 'Legs', 'Both', 'Sanctified Shadowblade Legplates'),
(4, 1, 7, 0, 290, 50607, 'Phase 5', 'Rogue', 'Combat', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(4, 1, 8, 0, 290, 50670, 'Phase 5', 'Rogue', 'Combat', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(4, 1, 9, 0, 290, 50675, 'Phase 5', 'Rogue', 'Combat', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(4, 1, 10, 0, 290, 50402, 'Phase 5', 'Rogue', 'Combat', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(4, 1, 11, 0, 290, 50618, 'Phase 5', 'Rogue', 'Combat', 'Finger2', 'Both', 'Frostbrood Sapphire Ring'),
(4, 1, 12, 0, 290, 50363, 'Phase 5', 'Rogue', 'Combat', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(4, 1, 13, 0, 290, 54590, 'Phase 5', 'Rogue', 'Combat', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(4, 1, 14, 0, 290, 47545, 'Phase 5', 'Rogue', 'Combat', 'Back', 'Both', 'Vereesa''s Dexterity'),
(4, 1, 15, 0, 290, 50737, 'Phase 5', 'Rogue', 'Combat', 'MainHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(4, 1, 16, 0, 290, 50654, 'Phase 5', 'Rogue', 'Combat', 'OffHand', 'Both', 'Scourgeborne Waraxe'),
(4, 1, 17, 0, 290, 50733, 'Phase 5', 'Rogue', 'Combat', 'Ranged', 'Both', 'Fal''inrush, Defender of Quel''thalas');

-- Subtlety (tab 2)
-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 2, 0, 0, 120, 32087, 'Pre-Raid', 'Rogue', 'Subtlety', 'Head', 'Both', 'Mask of the Deceiver'),
(4, 2, 1, 0, 120, 29381, 'Pre-Raid', 'Rogue', 'Subtlety', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 2, 2, 0, 120, 27797, 'Pre-Raid', 'Rogue', 'Subtlety', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 2, 4, 0, 120, 30730, 'Pre-Raid', 'Rogue', 'Subtlety', 'Chest', 'Both', 'Terrorweave Tunic'),
(4, 2, 5, 0, 120, 29247, 'Pre-Raid', 'Rogue', 'Subtlety', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 2, 6, 0, 120, 30538, 'Pre-Raid', 'Rogue', 'Subtlety', 'Legs', 'Both', 'Midnight Legguards'),
(4, 2, 7, 0, 120, 25686, 'Pre-Raid', 'Rogue', 'Subtlety', 'Feet', 'Both', 'Fel Leather Boots'),
(4, 2, 8, 0, 120, 29246, 'Pre-Raid', 'Rogue', 'Subtlety', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 2, 9, 0, 120, 27531, 'Pre-Raid', 'Rogue', 'Subtlety', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 2, 10, 0, 120, 30738, 'Pre-Raid', 'Rogue', 'Subtlety', 'Finger1', 'Both', 'Ring of Reciprocity'),
(4, 2, 12, 0, 120, 28288, 'Pre-Raid', 'Rogue', 'Subtlety', 'Trinket1', 'Both', 'Abacus of Violent Odds'),
(4, 2, 13, 0, 120, 29383, 'Pre-Raid', 'Rogue', 'Subtlety', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(4, 2, 14, 0, 120, 24259, 'Pre-Raid', 'Rogue', 'Subtlety', 'Back', 'Both', 'Vengeance Wrap'),
(4, 2, 15, 0, 120, 31332, 'Pre-Raid', 'Rogue', 'Subtlety', 'MainHand', 'Both', 'Blinkstrike'),
(4, 2, 16, 0, 120, 28189, 'Pre-Raid', 'Rogue', 'Subtlety', 'OffHand', 'Both', 'Latro''s Shifting Sword'),
(4, 2, 17, 2, 120, 29152, 'Pre-Raid', 'Rogue', 'Subtlety', 'Ranged', 'Horde', 'Marksman''s Bow');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(4, 2, 0, 0, 125, 29044, 'Phase 1', 'Rogue', 'Subtlety', 'Head', 'Both', 'Netherblade Facemask'),
(4, 2, 1, 0, 125, 29381, 'Phase 1', 'Rogue', 'Subtlety', 'Neck', 'Both', 'Choker of Vile Intent'),
(4, 2, 2, 0, 125, 27797, 'Phase 1', 'Rogue', 'Subtlety', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(4, 2, 4, 0, 125, 29045, 'Phase 1', 'Rogue', 'Subtlety', 'Chest', 'Both', 'Netherblade Chestpiece'),
(4, 2, 5, 0, 125, 29247, 'Phase 1', 'Rogue', 'Subtlety', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(4, 2, 6, 0, 125, 28741, 'Phase 1', 'Rogue', 'Subtlety', 'Legs', 'Both', 'Skulker''s Greaves'),
(4, 2, 7, 0, 125, 28545, 'Phase 1', 'Rogue', 'Subtlety', 'Feet', 'Both', 'Edgewalker Longboots'),
(4, 2, 8, 0, 125, 29246, 'Phase 1', 'Rogue', 'Subtlety', 'Wrists', 'Both', 'Nightfall Wristguards'),
(4, 2, 9, 0, 125, 27531, 'Phase 1', 'Rogue', 'Subtlety', 'Hands', 'Both', 'Wastewalker Gloves'),
(4, 2, 10, 0, 125, 28649, 'Phase 1', 'Rogue', 'Subtlety', 'Finger1', 'Both', 'Garona''s Signet Ring'),
(4, 2, 11, 0, 125, 28757, 'Phase 1', 'Rogue', 'Subtlety', 'Finger2', 'Both', 'Ring of a Thousand Marks'),
(4, 2, 12, 0, 125, 29383, 'Phase 1', 'Rogue', 'Subtlety', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(4, 2, 13, 0, 125, 28830, 'Phase 1', 'Rogue', 'Subtlety', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(4, 2, 14, 0, 125, 28672, 'Phase 1', 'Rogue', 'Subtlety', 'Back', 'Both', 'Drape of the Dark Reavers'),
(4, 2, 15, 0, 125, 31332, 'Phase 1', 'Rogue', 'Subtlety', 'MainHand', 'Both', 'Blinkstrike'),
(4, 2, 16, 0, 125, 28307, 'Phase 1', 'Rogue', 'Subtlety', 'OffHand', 'Both', 'Gladiator''s Quickblade'),
(4, 2, 17, 0, 125, 28772, 'Phase 1', 'Rogue', 'Subtlety', 'Ranged', 'Both', 'Sunfury Bow of the Phoenix');


-- ============================================================
-- Priest (5)
-- ============================================================
-- Discipline (tab 0)
-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 120, 24264, 'Pre-Raid', 'Priest', 'Discipline', 'Head', 'Both', 'Whitemend Hood'),
(5, 0, 1, 0, 120, 30377, 'Pre-Raid', 'Priest', 'Discipline', 'Neck', 'Both', 'Karja''s Medallion'),
(5, 0, 2, 0, 120, 21874, 'Pre-Raid', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(5, 0, 4, 0, 120, 21875, 'Pre-Raid', 'Priest', 'Discipline', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(5, 0, 5, 0, 120, 21873, 'Pre-Raid', 'Priest', 'Discipline', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(5, 0, 6, 0, 120, 24261, 'Pre-Raid', 'Priest', 'Discipline', 'Legs', 'Both', 'Whitemend Pants'),
(5, 0, 7, 0, 120, 29251, 'Pre-Raid', 'Priest', 'Discipline', 'Feet', 'Both', 'Boots of the Pious'),
(5, 0, 8, 0, 120, 29249, 'Pre-Raid', 'Priest', 'Discipline', 'Wrists', 'Both', 'Bands of the Benevolent'),
(5, 0, 9, 0, 120, 24393, 'Pre-Raid', 'Priest', 'Discipline', 'Hands', 'Both', 'Bloody Surgeon''s Mitts'),
(5, 0, 10, 0, 120, 27780, 'Pre-Raid', 'Priest', 'Discipline', 'Finger1', 'Both', 'Ring of Fabled Hope'),
(5, 0, 10, 2, 120, 29168, 'Pre-Raid', 'Priest', 'Discipline', 'Finger1', 'Horde', 'Ancestral Band'),
(5, 0, 12, 0, 120, 19288, 'Pre-Raid', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Darkmoon Card: Blue Dragon'),
(5, 0, 13, 0, 120, 29376, 'Pre-Raid', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Essence of the Martyr'),
(5, 0, 14, 0, 120, 29354, 'Pre-Raid', 'Priest', 'Discipline', 'Back', 'Both', 'Light-Touched Stole of Altruism'),
(5, 0, 15, 0, 120, 29353, 'Pre-Raid', 'Priest', 'Discipline', 'MainHand', 'Both', 'Shockwave Truncheon'),
(5, 0, 16, 0, 120, 29170, 'Pre-Raid', 'Priest', 'Discipline', 'OffHand', 'Both', 'Windcaller''s Orb'),
(5, 0, 17, 0, 120, 27885, 'Pre-Raid', 'Priest', 'Discipline', 'Ranged', 'Both', 'Soul-Wand of the Aldor');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 125, 29049, 'Phase 1', 'Priest', 'Discipline', 'Head', 'Both', 'Light-Collar of the Incarnate'),
(5, 0, 1, 0, 125, 30726, 'Phase 1', 'Priest', 'Discipline', 'Neck', 'Both', 'Archaic Charm of Presence'),
(5, 0, 2, 0, 125, 21874, 'Phase 1', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(5, 0, 4, 0, 125, 21875, 'Phase 1', 'Priest', 'Discipline', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(5, 0, 5, 0, 125, 21873, 'Phase 1', 'Priest', 'Discipline', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(5, 0, 6, 0, 125, 30727, 'Phase 1', 'Priest', 'Discipline', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(5, 0, 7, 0, 125, 28663, 'Phase 1', 'Priest', 'Discipline', 'Feet', 'Both', 'Boots of the Incorrupt'),
(5, 0, 8, 0, 125, 29249, 'Phase 1', 'Priest', 'Discipline', 'Wrists', 'Both', 'Bands of the Benevolent'),
(5, 0, 9, 0, 125, 28508, 'Phase 1', 'Priest', 'Discipline', 'Hands', 'Both', 'Gloves of Saintly Blessings'),
(5, 0, 10, 0, 125, 29290, 'Phase 1', 'Priest', 'Discipline', 'Finger1', 'Both', 'Violet Signet of the Grand Restorer'),
(5, 0, 11, 0, 125, 28763, 'Phase 1', 'Priest', 'Discipline', 'Finger2', 'Both', 'Jade Ring of the Everliving'),
(5, 0, 12, 0, 125, 29376, 'Phase 1', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 0, 13, 0, 125, 28823, 'Phase 1', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Eye of Gruul'),
(5, 0, 14, 0, 125, 28765, 'Phase 1', 'Priest', 'Discipline', 'Back', 'Both', 'Stainless Cloak of the Pure Hearted'),
(5, 0, 15, 0, 125, 28771, 'Phase 1', 'Priest', 'Discipline', 'MainHand', 'Both', 'Light''s Justice'),
(5, 0, 16, 0, 125, 29170, 'Phase 1', 'Priest', 'Discipline', 'OffHand', 'Both', 'Windcaller''s Orb'),
(5, 0, 17, 0, 125, 28588, 'Phase 1', 'Priest', 'Discipline', 'Ranged', 'Both', 'Blue Diamond Witchwand');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 141, 30152, 'Phase 2', 'Priest', 'Discipline', 'Head', 'Both', 'Cowl of the Avatar'),
(5, 0, 1, 0, 141, 30018, 'Phase 2', 'Priest', 'Discipline', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(5, 0, 2, 0, 141, 30154, 'Phase 2', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Mantle of the Avatar'),
(5, 0, 4, 0, 141, 30150, 'Phase 2', 'Priest', 'Discipline', 'Chest', 'Both', 'Vestments of the Avatar'),
(5, 0, 5, 0, 141, 30036, 'Phase 2', 'Priest', 'Discipline', 'Waist', 'Both', 'Belt of the Long Road'),
(5, 0, 6, 0, 141, 30727, 'Phase 2', 'Priest', 'Discipline', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(5, 0, 7, 0, 141, 30100, 'Phase 2', 'Priest', 'Discipline', 'Feet', 'Both', 'Soul-Strider Boots'),
(5, 0, 8, 0, 141, 32516, 'Phase 2', 'Priest', 'Discipline', 'Wrists', 'Both', 'Wraps of Purification'),
(5, 0, 9, 0, 141, 30151, 'Phase 2', 'Priest', 'Discipline', 'Hands', 'Both', 'Gloves of the Avatar'),
(5, 0, 10, 0, 141, 30110, 'Phase 2', 'Priest', 'Discipline', 'Finger1', 'Both', 'Coral Band of the Revived'),
(5, 0, 11, 0, 141, 29290, 'Phase 2', 'Priest', 'Discipline', 'Finger2', 'Both', 'Violet Signet of the Grand Restorer'),
(5, 0, 12, 0, 141, 29376, 'Phase 2', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 0, 13, 0, 141, 28823, 'Phase 2', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Eye of Gruul'),
(5, 0, 14, 0, 141, 29989, 'Phase 2', 'Priest', 'Discipline', 'Back', 'Both', 'Sunshower Light Cloak'),
(5, 0, 15, 0, 141, 30108, 'Phase 2', 'Priest', 'Discipline', 'MainHand', 'Both', 'Lightfathom Scepter'),
(5, 0, 16, 0, 141, 29923, 'Phase 2', 'Priest', 'Discipline', 'OffHand', 'Both', 'Talisman of the Sun King'),
(5, 0, 17, 0, 141, 30080, 'Phase 2', 'Priest', 'Discipline', 'Ranged', 'Both', 'Luminescent Rod of the Naaru');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 156, 31063, 'Phase 3', 'Priest', 'Discipline', 'Head', 'Both', 'Cowl of Absolution'),
(5, 0, 1, 0, 156, 32370, 'Phase 3', 'Priest', 'Discipline', 'Neck', 'Both', 'Nadina''s Pendant of Purity'),
(5, 0, 2, 0, 156, 31069, 'Phase 3', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Mantle of Absolution'),
(5, 0, 4, 0, 156, 31066, 'Phase 3', 'Priest', 'Discipline', 'Chest', 'Both', 'Vestments of Absolution'),
(5, 0, 5, 0, 156, 32519, 'Phase 3', 'Priest', 'Discipline', 'Waist', 'Both', 'Belt of Divine Guidance'),
(5, 0, 6, 0, 156, 30912, 'Phase 3', 'Priest', 'Discipline', 'Legs', 'Both', 'Leggings of Eternity'),
(5, 0, 7, 0, 156, 32609, 'Phase 3', 'Priest', 'Discipline', 'Feet', 'Both', 'Boots of the Divine Light'),
(5, 0, 8, 0, 156, 30871, 'Phase 3', 'Priest', 'Discipline', 'Wrists', 'Both', 'Bracers of Martyrdom'),
(5, 0, 9, 0, 156, 31060, 'Phase 3', 'Priest', 'Discipline', 'Hands', 'Both', 'Gloves of Absolution'),
(5, 0, 10, 0, 156, 32528, 'Phase 3', 'Priest', 'Discipline', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(5, 0, 11, 0, 156, 32528, 'Phase 3', 'Priest', 'Discipline', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(5, 0, 12, 0, 156, 29376, 'Phase 3', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 0, 13, 0, 156, 32496, 'Phase 3', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Memento of Tyrande'),
(5, 0, 14, 0, 156, 32524, 'Phase 3', 'Priest', 'Discipline', 'Back', 'Both', 'Shroud of the Highborne'),
(5, 0, 15, 0, 156, 32500, 'Phase 3', 'Priest', 'Discipline', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(5, 0, 16, 0, 156, 30911, 'Phase 3', 'Priest', 'Discipline', 'OffHand', 'Both', 'Scepter of Purification'),
(5, 0, 17, 0, 156, 32363, 'Phase 3', 'Priest', 'Discipline', 'Ranged', 'Both', 'Naaru-Blessed Life Rod');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 164, 31063, 'Phase 4', 'Priest', 'Discipline', 'Head', 'Both', 'Cowl of Absolution'),
(5, 0, 1, 0, 164, 33281, 'Phase 4', 'Priest', 'Discipline', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(5, 0, 2, 0, 164, 31069, 'Phase 4', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Mantle of Absolution'),
(5, 0, 4, 0, 164, 31066, 'Phase 4', 'Priest', 'Discipline', 'Chest', 'Both', 'Vestments of Absolution'),
(5, 0, 5, 0, 164, 32519, 'Phase 4', 'Priest', 'Discipline', 'Waist', 'Both', 'Belt of Divine Guidance'),
(5, 0, 6, 0, 164, 30912, 'Phase 4', 'Priest', 'Discipline', 'Legs', 'Both', 'Leggings of Eternity'),
(5, 0, 7, 0, 164, 32609, 'Phase 4', 'Priest', 'Discipline', 'Feet', 'Both', 'Boots of the Divine Light'),
(5, 0, 8, 0, 164, 30871, 'Phase 4', 'Priest', 'Discipline', 'Wrists', 'Both', 'Bracers of Martyrdom'),
(5, 0, 9, 0, 164, 31060, 'Phase 4', 'Priest', 'Discipline', 'Hands', 'Both', 'Gloves of Absolution'),
(5, 0, 10, 0, 164, 32528, 'Phase 4', 'Priest', 'Discipline', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(5, 0, 11, 0, 164, 30110, 'Phase 4', 'Priest', 'Discipline', 'Finger2', 'Both', 'Coral Band of the Revived'),
(5, 0, 12, 0, 164, 29376, 'Phase 4', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 0, 13, 0, 164, 32496, 'Phase 4', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Memento of Tyrande'),
(5, 0, 14, 0, 164, 32524, 'Phase 4', 'Priest', 'Discipline', 'Back', 'Both', 'Shroud of the Highborne'),
(5, 0, 15, 0, 164, 32500, 'Phase 4', 'Priest', 'Discipline', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(5, 0, 16, 0, 164, 30911, 'Phase 4', 'Priest', 'Discipline', 'OffHand', 'Both', 'Scepter of Purification'),
(5, 0, 17, 0, 164, 32363, 'Phase 4', 'Priest', 'Discipline', 'Ranged', 'Both', 'Naaru-Blessed Life Rod');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 200, 37294, 'Pre-Raid', 'Priest', 'Discipline', 'Head', 'Both', 'Crown of Unbridled Magic'),
(5, 0, 1, 0, 200, 40681, 'Pre-Raid', 'Priest', 'Discipline', 'Neck', 'Both', 'Lattice Choker of Light'),
(5, 0, 2, 0, 200, 37673, 'Pre-Raid', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(5, 0, 4, 0, 200, 44180, 'Pre-Raid', 'Priest', 'Discipline', 'Chest', 'Both', 'Robes of Crackling Flame'),
(5, 0, 5, 0, 200, 40697, 'Pre-Raid', 'Priest', 'Discipline', 'Waist', 'Both', 'Elegant Temple Gardens'' Girdle'),
(5, 0, 6, 0, 200, 37854, 'Pre-Raid', 'Priest', 'Discipline', 'Legs', 'Both', 'Woven Bracae Leggings'),
(5, 0, 7, 0, 200, 44202, 'Pre-Raid', 'Priest', 'Discipline', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(5, 0, 8, 0, 200, 37361, 'Pre-Raid', 'Priest', 'Discipline', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(5, 0, 9, 0, 200, 37172, 'Pre-Raid', 'Priest', 'Discipline', 'Hands', 'Both', 'Gloves of Glistening Runes'),
(5, 0, 10, 0, 200, 37694, 'Pre-Raid', 'Priest', 'Discipline', 'Finger1', 'Both', 'Band of Guile'),
(5, 0, 12, 0, 200, 37835, 'Pre-Raid', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Je''Tze''s Bell'),
(5, 0, 14, 0, 200, 41610, 'Pre-Raid', 'Priest', 'Discipline', 'Back', 'Both', 'Deathchill Cloak'),
(5, 0, 15, 0, 200, 37169, 'Pre-Raid', 'Priest', 'Discipline', 'MainHand', 'Both', 'War Mace of Unrequited Love'),
(5, 0, 17, 0, 200, 37619, 'Pre-Raid', 'Priest', 'Discipline', 'Ranged', 'Both', 'Wand of Ahn''kahet');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 224, 40447, 'Phase 1', 'Priest', 'Discipline', 'Head', 'Both', 'Valorous Crown of Faith'),
(5, 0, 1, 0, 224, 44661, 'Phase 1', 'Priest', 'Discipline', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(5, 0, 2, 0, 224, 40450, 'Phase 1', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Valorous Shoulderpads of Faith'),
(5, 0, 4, 0, 224, 44002, 'Phase 1', 'Priest', 'Discipline', 'Chest', 'Both', 'The Sanctum''s Flowing Vestments'),
(5, 0, 5, 0, 224, 40561, 'Phase 1', 'Priest', 'Discipline', 'Waist', 'Both', 'Leash of Heedless Magic'),
(5, 0, 6, 0, 224, 40448, 'Phase 1', 'Priest', 'Discipline', 'Legs', 'Both', 'Valorous Leggings of Faith'),
(5, 0, 7, 0, 224, 40558, 'Phase 1', 'Priest', 'Discipline', 'Feet', 'Both', 'Arcanic Tramplers'),
(5, 0, 8, 0, 224, 44008, 'Phase 1', 'Priest', 'Discipline', 'Wrists', 'Both', 'Unsullied Cuffs'),
(5, 0, 9, 0, 224, 40445, 'Phase 1', 'Priest', 'Discipline', 'Hands', 'Both', 'Valorous Gloves of Faith'),
(5, 0, 10, 0, 224, 40399, 'Phase 1', 'Priest', 'Discipline', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(5, 0, 12, 0, 224, 37835, 'Phase 1', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Je''Tze''s Bell'),
(5, 0, 14, 0, 224, 44005, 'Phase 1', 'Priest', 'Discipline', 'Back', 'Both', 'Pennant Cloak'),
(5, 0, 17, 0, 224, 39426, 'Phase 1', 'Priest', 'Discipline', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 245, 46197, 'Phase 2', 'Priest', 'Discipline', 'Head', 'Both', 'Conqueror''s Cowl of Sanctification'),
(5, 0, 1, 0, 245, 45443, 'Phase 2', 'Priest', 'Discipline', 'Neck', 'Both', 'Charm of Meticulous Timing'),
(5, 0, 2, 0, 245, 46190, 'Phase 2', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Conqueror''s Shoulderpads of Sanctification'),
(5, 0, 4, 0, 245, 46193, 'Phase 2', 'Priest', 'Discipline', 'Chest', 'Both', 'Conqueror''s Robe of Sanctification'),
(5, 0, 5, 0, 245, 45619, 'Phase 2', 'Priest', 'Discipline', 'Waist', 'Both', 'Starwatcher''s Binding'),
(5, 0, 6, 0, 245, 46195, 'Phase 2', 'Priest', 'Discipline', 'Legs', 'Both', 'Conqueror''s Leggings of Sanctification'),
(5, 0, 7, 0, 245, 45135, 'Phase 2', 'Priest', 'Discipline', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(5, 0, 8, 0, 245, 45446, 'Phase 2', 'Priest', 'Discipline', 'Wrists', 'Both', 'Grasps of Reason'),
(5, 0, 9, 0, 245, 45520, 'Phase 2', 'Priest', 'Discipline', 'Hands', 'Both', 'Handwraps of the Vigilant'),
(5, 0, 12, 0, 245, 45535, 'Phase 2', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Show of Faith'),
(5, 0, 14, 0, 245, 45486, 'Phase 2', 'Priest', 'Discipline', 'Back', 'Both', 'Drape of the Sullen Goddess'),
(5, 0, 15, 0, 245, 46017, 'Phase 2', 'Priest', 'Discipline', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(5, 0, 17, 0, 245, 45294, 'Phase 2', 'Priest', 'Discipline', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 258, 46197, 'Phase 3', 'Priest', 'Discipline', 'Head', 'Both', 'Conqueror''s Cowl of Sanctification'),
(5, 0, 2, 0, 258, 46190, 'Phase 3', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Conqueror''s Shoulderpads of Sanctification'),
(5, 0, 4, 0, 258, 48031, 'Phase 3', 'Priest', 'Discipline', 'Chest', 'Both', 'Robe of Triumph'),
(5, 0, 5, 1, 258, 47084, 'Phase 3', 'Priest', 'Discipline', 'Waist', 'Alliance', 'Cord of Biting Cold'),
(5, 0, 5, 2, 258, 47447, 'Phase 3', 'Priest', 'Discipline', 'Waist', 'Horde', 'Belt of Biting Cold'),
(5, 0, 6, 1, 258, 47189, 'Phase 3', 'Priest', 'Discipline', 'Legs', 'Alliance', 'Leggings of the Deepening Void'),
(5, 0, 6, 2, 258, 47478, 'Phase 3', 'Priest', 'Discipline', 'Legs', 'Horde', 'Breeches of the Deepening Void'),
(5, 0, 7, 1, 258, 47097, 'Phase 3', 'Priest', 'Discipline', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(5, 0, 7, 2, 258, 47454, 'Phase 3', 'Priest', 'Discipline', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(5, 0, 8, 1, 258, 47143, 'Phase 3', 'Priest', 'Discipline', 'Wrists', 'Alliance', 'Bindings of Dark Essence'),
(5, 0, 8, 2, 258, 47467, 'Phase 3', 'Priest', 'Discipline', 'Wrists', 'Horde', 'Dark Essence Bindings'),
(5, 0, 9, 0, 258, 46188, 'Phase 3', 'Priest', 'Discipline', 'Hands', 'Both', 'Conqueror''s Gloves of Sanctification'),
(5, 0, 10, 1, 258, 47237, 'Phase 3', 'Priest', 'Discipline', 'Finger1', 'Alliance', 'Band of Deplorable Violence'),
(5, 0, 10, 2, 258, 47489, 'Phase 3', 'Priest', 'Discipline', 'Finger1', 'Horde', 'Lurid Manifestation'),
(5, 0, 12, 1, 258, 47059, 'Phase 3', 'Priest', 'Discipline', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(5, 0, 12, 2, 258, 47432, 'Phase 3', 'Priest', 'Discipline', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(5, 0, 14, 1, 258, 47552, 'Phase 3', 'Priest', 'Discipline', 'Back', 'Alliance', 'Jaina''s Radiance'),
(5, 0, 14, 2, 258, 47551, 'Phase 3', 'Priest', 'Discipline', 'Back', 'Horde', 'Aethas'' Intensity'),
(5, 0, 17, 0, 258, 45294, 'Phase 3', 'Priest', 'Discipline', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 264, 51261, 'Phase 4', 'Priest', 'Discipline', 'Head', 'Both', 'Sanctified Crimson Acolyte Hood'),
(5, 0, 2, 0, 264, 51264, 'Phase 4', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Shoulderpads'),
(5, 0, 4, 0, 264, 51263, 'Phase 4', 'Priest', 'Discipline', 'Chest', 'Both', 'Sanctified Crimson Acolyte Robe'),
(5, 0, 5, 0, 264, 50702, 'Phase 4', 'Priest', 'Discipline', 'Waist', 'Both', 'Lingering Illness'),
(5, 0, 6, 0, 264, 51262, 'Phase 4', 'Priest', 'Discipline', 'Legs', 'Both', 'Sanctified Crimson Acolyte Leggings'),
(5, 0, 7, 0, 264, 50699, 'Phase 4', 'Priest', 'Discipline', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 0, 8, 0, 264, 50686, 'Phase 4', 'Priest', 'Discipline', 'Wrists', 'Both', 'Death Surgeon''s Sleeves'),
(5, 0, 9, 0, 264, 50722, 'Phase 4', 'Priest', 'Discipline', 'Hands', 'Both', 'San''layn Ritualist Gloves'),
(5, 0, 10, 0, 264, 50720, 'Phase 4', 'Priest', 'Discipline', 'Finger1', 'Both', 'Incarnadine Band of Mending'),
(5, 0, 12, 1, 264, 47059, 'Phase 4', 'Priest', 'Discipline', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(5, 0, 12, 2, 264, 47432, 'Phase 4', 'Priest', 'Discipline', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(5, 0, 14, 0, 264, 50628, 'Phase 4', 'Priest', 'Discipline', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(5, 0, 15, 0, 264, 50734, 'Phase 4', 'Priest', 'Discipline', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(5, 0, 17, 0, 264, 50631, 'Phase 4', 'Priest', 'Discipline', 'Ranged', 'Both', 'Nightmare Ender');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 0, 0, 0, 290, 51261, 'Phase 5', 'Priest', 'Discipline', 'Head', 'Both', 'Sanctified Crimson Acolyte Hood'),
(5, 0, 1, 0, 290, 50182, 'Phase 5', 'Priest', 'Discipline', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(5, 0, 2, 0, 290, 51264, 'Phase 5', 'Priest', 'Discipline', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Shoulderpads'),
(5, 0, 4, 0, 290, 51263, 'Phase 5', 'Priest', 'Discipline', 'Chest', 'Both', 'Sanctified Crimson Acolyte Robe'),
(5, 0, 5, 0, 290, 50613, 'Phase 5', 'Priest', 'Discipline', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(5, 0, 6, 0, 290, 51262, 'Phase 5', 'Priest', 'Discipline', 'Legs', 'Both', 'Sanctified Crimson Acolyte Leggings'),
(5, 0, 7, 0, 290, 50699, 'Phase 5', 'Priest', 'Discipline', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 0, 8, 0, 290, 54582, 'Phase 5', 'Priest', 'Discipline', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(5, 0, 9, 0, 290, 50722, 'Phase 5', 'Priest', 'Discipline', 'Hands', 'Both', 'San''layn Ritualist Gloves'),
(5, 0, 10, 0, 290, 50644, 'Phase 5', 'Priest', 'Discipline', 'Finger1', 'Both', 'Ring of Maddening Whispers'),
(5, 0, 11, 0, 290, 50636, 'Phase 5', 'Priest', 'Discipline', 'Finger2', 'Both', 'Memory of Malygos'),
(5, 0, 12, 0, 290, 50366, 'Phase 5', 'Priest', 'Discipline', 'Trinket1', 'Both', 'Althor''s Abacus'),
(5, 0, 13, 0, 290, 54589, 'Phase 5', 'Priest', 'Discipline', 'Trinket2', 'Both', 'Glowing Twilight Scale'),
(5, 0, 14, 0, 290, 54583, 'Phase 5', 'Priest', 'Discipline', 'Back', 'Both', 'Cloak of Burning Dusk'),
(5, 0, 15, 0, 290, 50734, 'Phase 5', 'Priest', 'Discipline', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(5, 0, 16, 0, 290, 50719, 'Phase 5', 'Priest', 'Discipline', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(5, 0, 17, 0, 290, 50631, 'Phase 5', 'Priest', 'Discipline', 'Ranged', 'Both', 'Nightmare Ender');

-- Holy (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 66, 13102, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Head', 'Both', 'Cassandra''s Grace'),
(5, 1, 1, 0, 66, 18723, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(5, 1, 2, 0, 66, 13013, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Shoulders', 'Both', 'Elder Wizard''s Mantle'),
(5, 1, 4, 0, 66, 14154, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Chest', 'Both', 'Truefaith Vestments'),
(5, 1, 5, 0, 66, 14143, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Waist', 'Both', 'Ghostweave Belt'),
(5, 1, 6, 0, 66, 11841, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Legs', 'Both', 'Senior Designer''s Pantaloons'),
(5, 1, 7, 0, 66, 11822, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Feet', 'Both', 'Omnicast Boots'),
(5, 1, 8, 0, 66, 11766, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Wrists', 'Both', 'Flameweave Cuffs'),
(5, 1, 9, 0, 66, 10787, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Hands', 'Both', 'Atal''ai Gloves'),
(5, 1, 10, 0, 66, 16058, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Finger1', 'Both', 'Fordring''s Seal'),
(5, 1, 11, 0, 66, 13178, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Finger2', 'Both', 'Rosewine Circle'),
(5, 1, 12, 0, 66, 11819, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Trinket1', 'Both', 'Second Wind'),
(5, 1, 13, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Trinket2', 'Both', 'Briarwood Reed'),
(5, 1, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Back', 'Both', 'Archivist Cape'),
(5, 1, 15, 0, 66, 11923, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'MainHand', 'Both', 'The Hammer of Grace'),
(5, 1, 16, 0, 66, 11928, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'OffHand', 'Both', 'Thaurissan''s Royal Scepter'),
(5, 1, 17, 0, 66, 16997, 'Phase 1 (Pre-Raid)', 'Priest', 'Holy', 'Ranged', 'Both', 'Stormrager');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 76, 13102, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Head', 'Both', 'Cassandra''s Grace'),
(5, 1, 1, 0, 76, 18723, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(5, 1, 2, 0, 76, 13013, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Shoulders', 'Both', 'Elder Wizard''s Mantle'),
(5, 1, 4, 0, 76, 14154, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Chest', 'Both', 'Truefaith Vestments'),
(5, 1, 5, 0, 76, 18327, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Waist', 'Both', 'Whipvine Cord'),
(5, 1, 6, 0, 76, 18386, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Legs', 'Both', 'Padre''s Trousers'),
(5, 1, 7, 0, 76, 18507, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of the Full Moon'),
(5, 1, 8, 0, 76, 11766, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Wrists', 'Both', 'Flameweave Cuffs'),
(5, 1, 9, 0, 76, 10787, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Hands', 'Both', 'Atal''ai Gloves'),
(5, 1, 10, 0, 76, 16058, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Finger1', 'Both', 'Fordring''s Seal'),
(5, 1, 11, 0, 76, 13178, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Finger2', 'Both', 'Rosewine Circle'),
(5, 1, 12, 0, 76, 18469, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Trinket1', 'Both', 'Royal Seal of Eldre''Thalas'),
(5, 1, 13, 0, 76, 18371, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Trinket2', 'Both', 'Mindtap Talisman'),
(5, 1, 14, 0, 76, 18510, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Back', 'Both', 'Hide of the Wild'),
(5, 1, 15, 0, 76, 11923, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'MainHand', 'Both', 'The Hammer of Grace'),
(5, 1, 16, 0, 76, 18523, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'OffHand', 'Both', 'Brightly Glowing Stone'),
(5, 1, 17, 0, 76, 18483, 'Phase 2 (Pre-Raid)', 'Priest', 'Holy', 'Ranged', 'Both', 'Mana Channeling Wand');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 78, 16921, 'Phase 2', 'Priest', 'Holy', 'Head', 'Both', 'Halo of Transcendence'),
(5, 1, 1, 0, 78, 18723, 'Phase 2', 'Priest', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(5, 1, 2, 0, 78, 16816, 'Phase 2', 'Priest', 'Holy', 'Shoulders', 'Both', 'Mantle of Prophecy'),
(5, 1, 4, 0, 78, 14154, 'Phase 2', 'Priest', 'Holy', 'Chest', 'Both', 'Truefaith Vestments'),
(5, 1, 5, 0, 78, 16817, 'Phase 2', 'Priest', 'Holy', 'Waist', 'Both', 'Girdle of Prophecy'),
(5, 1, 6, 0, 78, 16922, 'Phase 2', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Transcendence'),
(5, 1, 7, 0, 78, 16811, 'Phase 2', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of Prophecy'),
(5, 1, 8, 0, 78, 16819, 'Phase 2', 'Priest', 'Holy', 'Wrists', 'Both', 'Vambraces of Prophecy'),
(5, 1, 9, 0, 78, 16812, 'Phase 2', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Prophecy'),
(5, 1, 10, 0, 78, 19140, 'Phase 2', 'Priest', 'Holy', 'Finger1', 'Both', 'Cauterizing Band'),
(5, 1, 11, 0, 78, 19140, 'Phase 2', 'Priest', 'Holy', 'Finger2', 'Both', 'Cauterizing Band'),
(5, 1, 12, 0, 78, 18469, 'Phase 2', 'Priest', 'Holy', 'Trinket1', 'Both', 'Royal Seal of Eldre''Thalas'),
(5, 1, 13, 0, 78, 17064, 'Phase 2', 'Priest', 'Holy', 'Trinket2', 'Both', 'Shard of the Scale'),
(5, 1, 14, 0, 78, 18510, 'Phase 2', 'Priest', 'Holy', 'Back', 'Both', 'Hide of the Wild'),
(5, 1, 15, 0, 78, 18608, 'Phase 2', 'Priest', 'Holy', 'MainHand', 'Both', 'Benediction'),
(5, 1, 17, 0, 78, 18483, 'Phase 2', 'Priest', 'Holy', 'Ranged', 'Both', 'Mana Channeling Wand');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 83, 16921, 'Phase 3', 'Priest', 'Holy', 'Head', 'Both', 'Halo of Transcendence'),
(5, 1, 1, 0, 83, 18723, 'Phase 3', 'Priest', 'Holy', 'Neck', 'Both', 'Animated Chain Necklace'),
(5, 1, 2, 0, 83, 16924, 'Phase 3', 'Priest', 'Holy', 'Shoulders', 'Both', 'Pauldrons of Transcendence'),
(5, 1, 4, 0, 83, 16923, 'Phase 3', 'Priest', 'Holy', 'Chest', 'Both', 'Robes of Transcendence'),
(5, 1, 5, 0, 83, 16925, 'Phase 3', 'Priest', 'Holy', 'Waist', 'Both', 'Belt of Transcendence'),
(5, 1, 6, 0, 83, 16922, 'Phase 3', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Transcendence'),
(5, 1, 7, 0, 83, 16919, 'Phase 3', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of Transcendence'),
(5, 1, 8, 0, 83, 16926, 'Phase 3', 'Priest', 'Holy', 'Wrists', 'Both', 'Bindings of Transcendence'),
(5, 1, 9, 0, 83, 16920, 'Phase 3', 'Priest', 'Holy', 'Hands', 'Both', 'Handguards of Transcendence'),
(5, 1, 10, 0, 83, 19382, 'Phase 3', 'Priest', 'Holy', 'Finger1', 'Both', 'Pure Elementium Band'),
(5, 1, 11, 0, 83, 19140, 'Phase 3', 'Priest', 'Holy', 'Finger2', 'Both', 'Cauterizing Band'),
(5, 1, 12, 0, 83, 19395, 'Phase 3', 'Priest', 'Holy', 'Trinket1', 'Both', 'Rejuvenating Gem'),
(5, 1, 13, 0, 83, 17064, 'Phase 3', 'Priest', 'Holy', 'Trinket2', 'Both', 'Shard of the Scale'),
(5, 1, 14, 0, 83, 19430, 'Phase 3', 'Priest', 'Holy', 'Back', 'Both', 'Shroud of Pure Thought'),
(5, 1, 15, 0, 83, 18608, 'Phase 3', 'Priest', 'Holy', 'MainHand', 'Both', 'Benediction'),
(5, 1, 17, 0, 83, 19435, 'Phase 3', 'Priest', 'Holy', 'Ranged', 'Both', 'Essence Gatherer');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 88, 21615, 'Phase 5', 'Priest', 'Holy', 'Head', 'Both', 'Don Rigoberto''s Lost Hat'),
(5, 1, 1, 0, 88, 21712, 'Phase 5', 'Priest', 'Holy', 'Neck', 'Both', 'Amulet of the Fallen God'),
(5, 1, 2, 0, 88, 16924, 'Phase 5', 'Priest', 'Holy', 'Shoulders', 'Both', 'Pauldrons of Transcendence'),
(5, 1, 4, 0, 88, 21663, 'Phase 5', 'Priest', 'Holy', 'Chest', 'Both', 'Robes of the Guardian Saint'),
(5, 1, 5, 0, 88, 21582, 'Phase 5', 'Priest', 'Holy', 'Waist', 'Both', 'Grasp of the Old God'),
(5, 1, 6, 0, 88, 16922, 'Phase 5', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Transcendence'),
(5, 1, 7, 0, 88, 19437, 'Phase 5', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of Pure Thought'),
(5, 1, 8, 0, 88, 16926, 'Phase 5', 'Priest', 'Holy', 'Wrists', 'Both', 'Bindings of Transcendence'),
(5, 1, 9, 0, 88, 21619, 'Phase 5', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of the Messiah'),
(5, 1, 10, 0, 88, 19382, 'Phase 5', 'Priest', 'Holy', 'Finger1', 'Both', 'Pure Elementium Band'),
(5, 1, 11, 0, 88, 21620, 'Phase 5', 'Priest', 'Holy', 'Finger2', 'Both', 'Ring of the Martyr'),
(5, 1, 12, 0, 88, 19395, 'Phase 5', 'Priest', 'Holy', 'Trinket1', 'Both', 'Rejuvenating Gem'),
(5, 1, 13, 0, 88, 17064, 'Phase 5', 'Priest', 'Holy', 'Trinket2', 'Both', 'Shard of the Scale'),
(5, 1, 14, 0, 88, 21583, 'Phase 5', 'Priest', 'Holy', 'Back', 'Both', 'Cloak of Clarity'),
(5, 1, 15, 0, 88, 21839, 'Phase 5', 'Priest', 'Holy', 'MainHand', 'Both', 'Scepter of the False Prophet'),
(5, 1, 16, 0, 88, 21666, 'Phase 5', 'Priest', 'Holy', 'OffHand', 'Both', 'Sartura''s Might'),
(5, 1, 17, 0, 88, 21801, 'Phase 5', 'Priest', 'Holy', 'Ranged', 'Both', 'Antenna of Invigoration');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 92, 22514, 'Phase 6', 'Priest', 'Holy', 'Head', 'Both', 'Circlet of Faith'),
(5, 1, 1, 0, 92, 21712, 'Phase 6', 'Priest', 'Holy', 'Neck', 'Both', 'Amulet of the Fallen God'),
(5, 1, 2, 0, 92, 22515, 'Phase 6', 'Priest', 'Holy', 'Shoulders', 'Both', 'Shoulderpads of Faith'),
(5, 1, 4, 0, 92, 22512, 'Phase 6', 'Priest', 'Holy', 'Chest', 'Both', 'Robe of Faith'),
(5, 1, 5, 0, 92, 21582, 'Phase 6', 'Priest', 'Holy', 'Waist', 'Both', 'Grasp of the Old God'),
(5, 1, 6, 0, 92, 22513, 'Phase 6', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Faith'),
(5, 1, 7, 0, 92, 22516, 'Phase 6', 'Priest', 'Holy', 'Feet', 'Both', 'Sandals of Faith'),
(5, 1, 8, 0, 92, 21604, 'Phase 6', 'Priest', 'Holy', 'Wrists', 'Both', 'Bracelets of Royal Redemption'),
(5, 1, 9, 0, 92, 22517, 'Phase 6', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Faith'),
(5, 1, 10, 0, 92, 23061, 'Phase 6', 'Priest', 'Holy', 'Finger1', 'Both', 'Ring of Faith'),
(5, 1, 11, 0, 92, 22939, 'Phase 6', 'Priest', 'Holy', 'Finger2', 'Both', 'Band of Unanswered Prayers'),
(5, 1, 12, 0, 92, 23027, 'Phase 6', 'Priest', 'Holy', 'Trinket1', 'Both', 'Warmth of Forgiveness'),
(5, 1, 13, 0, 92, 23047, 'Phase 6', 'Priest', 'Holy', 'Trinket2', 'Both', 'Eye of the Dead'),
(5, 1, 14, 0, 92, 22960, 'Phase 6', 'Priest', 'Holy', 'Back', 'Both', 'Cloak of Suturing'),
(5, 1, 15, 0, 92, 23056, 'Phase 6', 'Priest', 'Holy', 'MainHand', 'Both', 'Hammer of the Twisting Nether'),
(5, 1, 16, 0, 92, 23048, 'Phase 6', 'Priest', 'Holy', 'OffHand', 'Both', 'Sapphiron''s Right Eye'),
(5, 1, 17, 0, 92, 23009, 'Phase 6', 'Priest', 'Holy', 'Ranged', 'Both', 'Wand of the Whispering Dead');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 120, 24264, 'Pre-Raid', 'Priest', 'Holy', 'Head', 'Both', 'Whitemend Hood'),
(5, 1, 1, 0, 120, 30377, 'Pre-Raid', 'Priest', 'Holy', 'Neck', 'Both', 'Karja''s Medallion'),
(5, 1, 2, 0, 120, 21874, 'Pre-Raid', 'Priest', 'Holy', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(5, 1, 4, 0, 120, 21875, 'Pre-Raid', 'Priest', 'Holy', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(5, 1, 5, 0, 120, 21873, 'Pre-Raid', 'Priest', 'Holy', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(5, 1, 6, 0, 120, 24261, 'Pre-Raid', 'Priest', 'Holy', 'Legs', 'Both', 'Whitemend Pants'),
(5, 1, 7, 0, 120, 29251, 'Pre-Raid', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of the Pious'),
(5, 1, 8, 0, 120, 29249, 'Pre-Raid', 'Priest', 'Holy', 'Wrists', 'Both', 'Bands of the Benevolent'),
(5, 1, 9, 0, 120, 24393, 'Pre-Raid', 'Priest', 'Holy', 'Hands', 'Both', 'Bloody Surgeon''s Mitts'),
(5, 1, 10, 0, 120, 27780, 'Pre-Raid', 'Priest', 'Holy', 'Finger1', 'Both', 'Ring of Fabled Hope'),
(5, 1, 10, 2, 120, 29168, 'Pre-Raid', 'Priest', 'Holy', 'Finger1', 'Horde', 'Ancestral Band'),
(5, 1, 12, 0, 120, 19288, 'Pre-Raid', 'Priest', 'Holy', 'Trinket1', 'Both', 'Darkmoon Card: Blue Dragon'),
(5, 1, 13, 0, 120, 29376, 'Pre-Raid', 'Priest', 'Holy', 'Trinket2', 'Both', 'Essence of the Martyr'),
(5, 1, 14, 0, 120, 29354, 'Pre-Raid', 'Priest', 'Holy', 'Back', 'Both', 'Light-Touched Stole of Altruism'),
(5, 1, 15, 0, 120, 29353, 'Pre-Raid', 'Priest', 'Holy', 'MainHand', 'Both', 'Shockwave Truncheon'),
(5, 1, 16, 0, 120, 29170, 'Pre-Raid', 'Priest', 'Holy', 'OffHand', 'Both', 'Windcaller''s Orb'),
(5, 1, 17, 0, 120, 27885, 'Pre-Raid', 'Priest', 'Holy', 'Ranged', 'Both', 'Soul-Wand of the Aldor');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 125, 29049, 'Phase 1', 'Priest', 'Holy', 'Head', 'Both', 'Light-Collar of the Incarnate'),
(5, 1, 1, 0, 125, 30726, 'Phase 1', 'Priest', 'Holy', 'Neck', 'Both', 'Archaic Charm of Presence'),
(5, 1, 2, 0, 125, 21874, 'Phase 1', 'Priest', 'Holy', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(5, 1, 4, 0, 125, 21875, 'Phase 1', 'Priest', 'Holy', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(5, 1, 5, 0, 125, 21873, 'Phase 1', 'Priest', 'Holy', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(5, 1, 6, 0, 125, 30727, 'Phase 1', 'Priest', 'Holy', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(5, 1, 7, 0, 125, 28663, 'Phase 1', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of the Incorrupt'),
(5, 1, 8, 0, 125, 29249, 'Phase 1', 'Priest', 'Holy', 'Wrists', 'Both', 'Bands of the Benevolent'),
(5, 1, 9, 0, 125, 28508, 'Phase 1', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Saintly Blessings'),
(5, 1, 10, 0, 125, 29290, 'Phase 1', 'Priest', 'Holy', 'Finger1', 'Both', 'Violet Signet of the Grand Restorer'),
(5, 1, 11, 0, 125, 28763, 'Phase 1', 'Priest', 'Holy', 'Finger2', 'Both', 'Jade Ring of the Everliving'),
(5, 1, 12, 0, 125, 29376, 'Phase 1', 'Priest', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 1, 13, 0, 125, 28823, 'Phase 1', 'Priest', 'Holy', 'Trinket2', 'Both', 'Eye of Gruul'),
(5, 1, 14, 0, 125, 28765, 'Phase 1', 'Priest', 'Holy', 'Back', 'Both', 'Stainless Cloak of the Pure Hearted'),
(5, 1, 15, 0, 125, 28771, 'Phase 1', 'Priest', 'Holy', 'MainHand', 'Both', 'Light''s Justice'),
(5, 1, 16, 0, 125, 29170, 'Phase 1', 'Priest', 'Holy', 'OffHand', 'Both', 'Windcaller''s Orb'),
(5, 1, 17, 0, 125, 28588, 'Phase 1', 'Priest', 'Holy', 'Ranged', 'Both', 'Blue Diamond Witchwand');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 141, 30152, 'Phase 2', 'Priest', 'Holy', 'Head', 'Both', 'Cowl of the Avatar'),
(5, 1, 1, 0, 141, 30018, 'Phase 2', 'Priest', 'Holy', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(5, 1, 2, 0, 141, 30154, 'Phase 2', 'Priest', 'Holy', 'Shoulders', 'Both', 'Mantle of the Avatar'),
(5, 1, 4, 0, 141, 30150, 'Phase 2', 'Priest', 'Holy', 'Chest', 'Both', 'Vestments of the Avatar'),
(5, 1, 5, 0, 141, 30036, 'Phase 2', 'Priest', 'Holy', 'Waist', 'Both', 'Belt of the Long Road'),
(5, 1, 6, 0, 141, 30727, 'Phase 2', 'Priest', 'Holy', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(5, 1, 7, 0, 141, 30100, 'Phase 2', 'Priest', 'Holy', 'Feet', 'Both', 'Soul-Strider Boots'),
(5, 1, 8, 0, 141, 32516, 'Phase 2', 'Priest', 'Holy', 'Wrists', 'Both', 'Wraps of Purification'),
(5, 1, 9, 0, 141, 30151, 'Phase 2', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of the Avatar'),
(5, 1, 10, 0, 141, 30110, 'Phase 2', 'Priest', 'Holy', 'Finger1', 'Both', 'Coral Band of the Revived'),
(5, 1, 11, 0, 141, 29290, 'Phase 2', 'Priest', 'Holy', 'Finger2', 'Both', 'Violet Signet of the Grand Restorer'),
(5, 1, 12, 0, 141, 29376, 'Phase 2', 'Priest', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 1, 13, 0, 141, 28823, 'Phase 2', 'Priest', 'Holy', 'Trinket2', 'Both', 'Eye of Gruul'),
(5, 1, 14, 0, 141, 29989, 'Phase 2', 'Priest', 'Holy', 'Back', 'Both', 'Sunshower Light Cloak'),
(5, 1, 15, 0, 141, 30108, 'Phase 2', 'Priest', 'Holy', 'MainHand', 'Both', 'Lightfathom Scepter'),
(5, 1, 16, 0, 141, 29923, 'Phase 2', 'Priest', 'Holy', 'OffHand', 'Both', 'Talisman of the Sun King'),
(5, 1, 17, 0, 141, 30080, 'Phase 2', 'Priest', 'Holy', 'Ranged', 'Both', 'Luminescent Rod of the Naaru');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 156, 31063, 'Phase 3', 'Priest', 'Holy', 'Head', 'Both', 'Cowl of Absolution'),
(5, 1, 1, 0, 156, 32370, 'Phase 3', 'Priest', 'Holy', 'Neck', 'Both', 'Nadina''s Pendant of Purity'),
(5, 1, 2, 0, 156, 31069, 'Phase 3', 'Priest', 'Holy', 'Shoulders', 'Both', 'Mantle of Absolution'),
(5, 1, 4, 0, 156, 31066, 'Phase 3', 'Priest', 'Holy', 'Chest', 'Both', 'Vestments of Absolution'),
(5, 1, 5, 0, 156, 32519, 'Phase 3', 'Priest', 'Holy', 'Waist', 'Both', 'Belt of Divine Guidance'),
(5, 1, 6, 0, 156, 30912, 'Phase 3', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Eternity'),
(5, 1, 7, 0, 156, 32609, 'Phase 3', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of the Divine Light'),
(5, 1, 8, 0, 156, 30871, 'Phase 3', 'Priest', 'Holy', 'Wrists', 'Both', 'Bracers of Martyrdom'),
(5, 1, 9, 0, 156, 31060, 'Phase 3', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Absolution'),
(5, 1, 10, 0, 156, 32528, 'Phase 3', 'Priest', 'Holy', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(5, 1, 11, 0, 156, 32528, 'Phase 3', 'Priest', 'Holy', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(5, 1, 12, 0, 156, 29376, 'Phase 3', 'Priest', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 1, 13, 0, 156, 32496, 'Phase 3', 'Priest', 'Holy', 'Trinket2', 'Both', 'Memento of Tyrande'),
(5, 1, 14, 0, 156, 32524, 'Phase 3', 'Priest', 'Holy', 'Back', 'Both', 'Shroud of the Highborne'),
(5, 1, 15, 0, 156, 32500, 'Phase 3', 'Priest', 'Holy', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(5, 1, 16, 0, 156, 30911, 'Phase 3', 'Priest', 'Holy', 'OffHand', 'Both', 'Scepter of Purification'),
(5, 1, 17, 0, 156, 32363, 'Phase 3', 'Priest', 'Holy', 'Ranged', 'Both', 'Naaru-Blessed Life Rod');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 164, 31063, 'Phase 4', 'Priest', 'Holy', 'Head', 'Both', 'Cowl of Absolution'),
(5, 1, 1, 0, 164, 33281, 'Phase 4', 'Priest', 'Holy', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(5, 1, 2, 0, 164, 31069, 'Phase 4', 'Priest', 'Holy', 'Shoulders', 'Both', 'Mantle of Absolution'),
(5, 1, 4, 0, 164, 31066, 'Phase 4', 'Priest', 'Holy', 'Chest', 'Both', 'Vestments of Absolution'),
(5, 1, 5, 0, 164, 32519, 'Phase 4', 'Priest', 'Holy', 'Waist', 'Both', 'Belt of Divine Guidance'),
(5, 1, 6, 0, 164, 30912, 'Phase 4', 'Priest', 'Holy', 'Legs', 'Both', 'Leggings of Eternity'),
(5, 1, 7, 0, 164, 32609, 'Phase 4', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of the Divine Light'),
(5, 1, 8, 0, 164, 30871, 'Phase 4', 'Priest', 'Holy', 'Wrists', 'Both', 'Bracers of Martyrdom'),
(5, 1, 9, 0, 164, 31060, 'Phase 4', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Absolution'),
(5, 1, 10, 0, 164, 32528, 'Phase 4', 'Priest', 'Holy', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(5, 1, 11, 0, 164, 30110, 'Phase 4', 'Priest', 'Holy', 'Finger2', 'Both', 'Coral Band of the Revived'),
(5, 1, 12, 0, 164, 29376, 'Phase 4', 'Priest', 'Holy', 'Trinket1', 'Both', 'Essence of the Martyr'),
(5, 1, 13, 0, 164, 32496, 'Phase 4', 'Priest', 'Holy', 'Trinket2', 'Both', 'Memento of Tyrande'),
(5, 1, 14, 0, 164, 32524, 'Phase 4', 'Priest', 'Holy', 'Back', 'Both', 'Shroud of the Highborne'),
(5, 1, 15, 0, 164, 32500, 'Phase 4', 'Priest', 'Holy', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(5, 1, 16, 0, 164, 30911, 'Phase 4', 'Priest', 'Holy', 'OffHand', 'Both', 'Scepter of Purification'),
(5, 1, 17, 0, 164, 32363, 'Phase 4', 'Priest', 'Holy', 'Ranged', 'Both', 'Naaru-Blessed Life Rod');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 200, 37294, 'Pre-Raid', 'Priest', 'Holy', 'Head', 'Both', 'Crown of Unbridled Magic'),
(5, 1, 1, 0, 200, 40681, 'Pre-Raid', 'Priest', 'Holy', 'Neck', 'Both', 'Lattice Choker of Light'),
(5, 1, 2, 0, 200, 37673, 'Pre-Raid', 'Priest', 'Holy', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(5, 1, 4, 0, 200, 44180, 'Pre-Raid', 'Priest', 'Holy', 'Chest', 'Both', 'Robes of Crackling Flame'),
(5, 1, 5, 0, 200, 40697, 'Pre-Raid', 'Priest', 'Holy', 'Waist', 'Both', 'Elegant Temple Gardens'' Girdle'),
(5, 1, 6, 0, 200, 37854, 'Pre-Raid', 'Priest', 'Holy', 'Legs', 'Both', 'Woven Bracae Leggings'),
(5, 1, 7, 0, 200, 44202, 'Pre-Raid', 'Priest', 'Holy', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(5, 1, 8, 0, 200, 37361, 'Pre-Raid', 'Priest', 'Holy', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(5, 1, 9, 0, 200, 37172, 'Pre-Raid', 'Priest', 'Holy', 'Hands', 'Both', 'Gloves of Glistening Runes'),
(5, 1, 10, 0, 200, 37694, 'Pre-Raid', 'Priest', 'Holy', 'Finger1', 'Both', 'Band of Guile'),
(5, 1, 12, 0, 200, 37835, 'Pre-Raid', 'Priest', 'Holy', 'Trinket1', 'Both', 'Je''Tze''s Bell'),
(5, 1, 14, 0, 200, 41610, 'Pre-Raid', 'Priest', 'Holy', 'Back', 'Both', 'Deathchill Cloak'),
(5, 1, 15, 0, 200, 37169, 'Pre-Raid', 'Priest', 'Holy', 'MainHand', 'Both', 'War Mace of Unrequited Love'),
(5, 1, 17, 0, 200, 37619, 'Pre-Raid', 'Priest', 'Holy', 'Ranged', 'Both', 'Wand of Ahn''kahet');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 224, 40447, 'Phase 1', 'Priest', 'Holy', 'Head', 'Both', 'Valorous Crown of Faith'),
(5, 1, 1, 0, 224, 44661, 'Phase 1', 'Priest', 'Holy', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(5, 1, 2, 0, 224, 40450, 'Phase 1', 'Priest', 'Holy', 'Shoulders', 'Both', 'Valorous Shoulderpads of Faith'),
(5, 1, 4, 0, 224, 44002, 'Phase 1', 'Priest', 'Holy', 'Chest', 'Both', 'The Sanctum''s Flowing Vestments'),
(5, 1, 5, 0, 224, 40561, 'Phase 1', 'Priest', 'Holy', 'Waist', 'Both', 'Leash of Heedless Magic'),
(5, 1, 6, 0, 224, 40448, 'Phase 1', 'Priest', 'Holy', 'Legs', 'Both', 'Valorous Leggings of Faith'),
(5, 1, 7, 0, 224, 40558, 'Phase 1', 'Priest', 'Holy', 'Feet', 'Both', 'Arcanic Tramplers'),
(5, 1, 8, 0, 224, 44008, 'Phase 1', 'Priest', 'Holy', 'Wrists', 'Both', 'Unsullied Cuffs'),
(5, 1, 9, 0, 224, 40445, 'Phase 1', 'Priest', 'Holy', 'Hands', 'Both', 'Valorous Gloves of Faith'),
(5, 1, 10, 0, 224, 40399, 'Phase 1', 'Priest', 'Holy', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(5, 1, 12, 0, 224, 37835, 'Phase 1', 'Priest', 'Holy', 'Trinket1', 'Both', 'Je''Tze''s Bell'),
(5, 1, 14, 0, 224, 44005, 'Phase 1', 'Priest', 'Holy', 'Back', 'Both', 'Pennant Cloak'),
(5, 1, 17, 0, 224, 39426, 'Phase 1', 'Priest', 'Holy', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 245, 45497, 'Phase 2', 'Priest', 'Holy', 'Head', 'Both', 'Crown of Luminescence'),
(5, 1, 1, 0, 245, 45243, 'Phase 2', 'Priest', 'Holy', 'Neck', 'Both', 'Sapphire Amulet of Renewal'),
(5, 1, 2, 0, 245, 46068, 'Phase 2', 'Priest', 'Holy', 'Shoulders', 'Both', 'Amice of Inconceivable Horror'),
(5, 1, 4, 0, 245, 45240, 'Phase 2', 'Priest', 'Holy', 'Chest', 'Both', 'Raiments of the Iron Council'),
(5, 1, 5, 0, 245, 45619, 'Phase 2', 'Priest', 'Holy', 'Waist', 'Both', 'Starwatcher''s Binding'),
(5, 1, 6, 0, 245, 46195, 'Phase 2', 'Priest', 'Holy', 'Legs', 'Both', 'Conqueror''s Leggings of Sanctification'),
(5, 1, 7, 0, 245, 45135, 'Phase 2', 'Priest', 'Holy', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(5, 1, 8, 0, 245, 45460, 'Phase 2', 'Priest', 'Holy', 'Wrists', 'Both', 'Bindings of Winter Gale'),
(5, 1, 9, 0, 245, 46188, 'Phase 2', 'Priest', 'Holy', 'Hands', 'Both', 'Conqueror''s Gloves of Sanctification'),
(5, 1, 12, 0, 245, 45535, 'Phase 2', 'Priest', 'Holy', 'Trinket1', 'Both', 'Show of Faith'),
(5, 1, 14, 0, 245, 45618, 'Phase 2', 'Priest', 'Holy', 'Back', 'Both', 'Sunglimmer Cloak'),
(5, 1, 15, 0, 245, 46017, 'Phase 2', 'Priest', 'Holy', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(5, 1, 17, 0, 245, 45170, 'Phase 2', 'Priest', 'Holy', 'Ranged', 'Both', 'Scepter of Creation');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 258, 48035, 'Phase 3', 'Priest', 'Holy', 'Head', 'Both', 'Cowl of Triumph'),
(5, 1, 2, 0, 258, 48029, 'Phase 3', 'Priest', 'Holy', 'Shoulders', 'Both', 'Shoulderpads of Triumph'),
(5, 1, 4, 1, 258, 46993, 'Phase 3', 'Priest', 'Holy', 'Chest', 'Alliance', 'Flowing Vestments of Ascent'),
(5, 1, 4, 2, 258, 47425, 'Phase 3', 'Priest', 'Holy', 'Chest', 'Horde', 'Flowing Robes of Ascent'),
(5, 1, 5, 1, 258, 46973, 'Phase 3', 'Priest', 'Holy', 'Waist', 'Alliance', 'Cord of the Tenebrous Mist'),
(5, 1, 5, 2, 258, 47419, 'Phase 3', 'Priest', 'Holy', 'Waist', 'Horde', 'Belt of the Tenebrous Mist'),
(5, 1, 6, 1, 258, 47062, 'Phase 3', 'Priest', 'Holy', 'Legs', 'Alliance', 'Leggings of the Soothing Touch'),
(5, 1, 6, 2, 258, 47435, 'Phase 3', 'Priest', 'Holy', 'Legs', 'Horde', 'Pants of the Soothing Touch'),
(5, 1, 7, 1, 258, 47097, 'Phase 3', 'Priest', 'Holy', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(5, 1, 7, 2, 258, 47454, 'Phase 3', 'Priest', 'Holy', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(5, 1, 8, 1, 258, 47208, 'Phase 3', 'Priest', 'Holy', 'Wrists', 'Alliance', 'Armbands of the Ashen Saint'),
(5, 1, 8, 2, 258, 47485, 'Phase 3', 'Priest', 'Holy', 'Wrists', 'Horde', 'Bindings of the Ashen Saint'),
(5, 1, 9, 0, 258, 45665, 'Phase 3', 'Priest', 'Holy', 'Hands', 'Both', 'Pharos Gloves'),
(5, 1, 10, 1, 258, 47224, 'Phase 3', 'Priest', 'Holy', 'Finger1', 'Alliance', 'Ring of the Darkmender'),
(5, 1, 10, 2, 258, 47439, 'Phase 3', 'Priest', 'Holy', 'Finger1', 'Horde', 'Circle of the Darkmender'),
(5, 1, 12, 1, 258, 47059, 'Phase 3', 'Priest', 'Holy', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(5, 1, 12, 2, 258, 47432, 'Phase 3', 'Priest', 'Holy', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(5, 1, 14, 1, 258, 47552, 'Phase 3', 'Priest', 'Holy', 'Back', 'Alliance', 'Jaina''s Radiance'),
(5, 1, 14, 2, 258, 47551, 'Phase 3', 'Priest', 'Holy', 'Back', 'Horde', 'Aethas'' Intensity');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 264, 51261, 'Phase 4', 'Priest', 'Holy', 'Head', 'Both', 'Sanctified Crimson Acolyte Hood'),
(5, 1, 2, 0, 264, 51264, 'Phase 4', 'Priest', 'Holy', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Shoulderpads'),
(5, 1, 4, 0, 264, 50717, 'Phase 4', 'Priest', 'Holy', 'Chest', 'Both', 'Sanguine Silk Robes'),
(5, 1, 5, 0, 264, 50613, 'Phase 4', 'Priest', 'Holy', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(5, 1, 6, 0, 264, 51262, 'Phase 4', 'Priest', 'Holy', 'Legs', 'Both', 'Sanctified Crimson Acolyte Leggings'),
(5, 1, 7, 0, 264, 50699, 'Phase 4', 'Priest', 'Holy', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 1, 8, 0, 264, 50686, 'Phase 4', 'Priest', 'Holy', 'Wrists', 'Both', 'Death Surgeon''s Sleeves'),
(5, 1, 9, 0, 264, 51260, 'Phase 4', 'Priest', 'Holy', 'Hands', 'Both', 'Sanctified Crimson Acolyte Gloves'),
(5, 1, 10, 0, 264, 50636, 'Phase 4', 'Priest', 'Holy', 'Finger1', 'Both', 'Memory of Malygos'),
(5, 1, 12, 1, 264, 47059, 'Phase 4', 'Priest', 'Holy', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(5, 1, 12, 2, 264, 47432, 'Phase 4', 'Priest', 'Holy', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(5, 1, 14, 0, 264, 50668, 'Phase 4', 'Priest', 'Holy', 'Back', 'Both', 'Greatcloak of the Turned Champion'),
(5, 1, 15, 0, 264, 46017, 'Phase 4', 'Priest', 'Holy', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(5, 1, 17, 0, 264, 50684, 'Phase 4', 'Priest', 'Holy', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 1, 0, 0, 290, 51261, 'Phase 5', 'Priest', 'Holy', 'Head', 'Both', 'Sanctified Crimson Acolyte Hood'),
(5, 1, 1, 0, 290, 50609, 'Phase 5', 'Priest', 'Holy', 'Neck', 'Both', 'Bone Sentinel''s Amulet'),
(5, 1, 2, 0, 290, 51264, 'Phase 5', 'Priest', 'Holy', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Shoulderpads'),
(5, 1, 4, 0, 290, 50717, 'Phase 5', 'Priest', 'Holy', 'Chest', 'Both', 'Sanguine Silk Robes'),
(5, 1, 5, 0, 290, 50702, 'Phase 5', 'Priest', 'Holy', 'Waist', 'Both', 'Lingering Illness'),
(5, 1, 6, 0, 290, 51262, 'Phase 5', 'Priest', 'Holy', 'Legs', 'Both', 'Sanctified Crimson Acolyte Leggings'),
(5, 1, 7, 0, 290, 50699, 'Phase 5', 'Priest', 'Holy', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 1, 8, 0, 290, 54582, 'Phase 5', 'Priest', 'Holy', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(5, 1, 9, 0, 290, 51260, 'Phase 5', 'Priest', 'Holy', 'Hands', 'Both', 'Sanctified Crimson Acolyte Gloves'),
(5, 1, 10, 0, 290, 50400, 'Phase 5', 'Priest', 'Holy', 'Finger1', 'Both', 'Ashen Band of Endless Wisdom'),
(5, 1, 11, 0, 290, 50636, 'Phase 5', 'Priest', 'Holy', 'Finger2', 'Both', 'Memory of Malygos'),
(5, 1, 12, 0, 290, 50366, 'Phase 5', 'Priest', 'Holy', 'Trinket1', 'Both', 'Althor''s Abacus'),
(5, 1, 13, 0, 290, 54589, 'Phase 5', 'Priest', 'Holy', 'Trinket2', 'Both', 'Glowing Twilight Scale'),
(5, 1, 14, 0, 290, 50668, 'Phase 5', 'Priest', 'Holy', 'Back', 'Both', 'Greatcloak of the Turned Champion'),
(5, 1, 15, 0, 290, 50731, 'Phase 5', 'Priest', 'Holy', 'MainHand', 'Both', 'Archus, Greatstaff of Antonidas'),
(5, 1, 17, 0, 290, 50631, 'Phase 5', 'Priest', 'Holy', 'Ranged', 'Both', 'Nightmare Ender');

-- Shadow (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 1, 66, 10504, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Head', 'Alliance', 'Green Lens'),
(5, 2, 0, 2, 66, 10504, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Head', 'Horde', 'Green Lens'),
(5, 2, 1, 1, 66, 18691, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(5, 2, 1, 2, 66, 18691, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(5, 2, 2, 1, 66, 14112, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Shoulders', 'Alliance', 'Felcloth Shoulders'),
(5, 2, 2, 2, 66, 14112, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Shoulders', 'Horde', 'Felcloth Shoulders'),
(5, 2, 4, 1, 66, 14136, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Chest', 'Alliance', 'Robe of Winter Night'),
(5, 2, 4, 2, 66, 14136, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Chest', 'Horde', 'Robe of Winter Night'),
(5, 2, 5, 1, 66, 11662, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Waist', 'Alliance', 'Ban''thok Sash'),
(5, 2, 5, 2, 66, 11662, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Waist', 'Horde', 'Ban''thok Sash'),
(5, 2, 6, 1, 66, 13170, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(5, 2, 6, 2, 66, 13170, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Legs', 'Horde', 'Skyshroud Leggings'),
(5, 2, 7, 1, 66, 18735, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(5, 2, 7, 2, 66, 18735, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(5, 2, 8, 1, 66, 11766, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(5, 2, 8, 2, 66, 11766, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(5, 2, 9, 1, 66, 13253, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Hands', 'Alliance', 'Hands of Power'),
(5, 2, 9, 2, 66, 13253, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Hands', 'Horde', 'Hands of Power'),
(5, 2, 10, 1, 66, 12543, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(5, 2, 10, 2, 66, 12545, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(5, 2, 11, 1, 66, 13001, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(5, 2, 11, 2, 66, 13001, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Finger2', 'Horde', 'Maiden''s Circle'),
(5, 2, 12, 1, 66, 12930, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(5, 2, 12, 2, 66, 12930, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket1', 'Horde', 'Briarwood Reed'),
(5, 2, 13, 1, 66, 11819, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket2', 'Alliance', 'Second Wind'),
(5, 2, 13, 2, 66, 11819, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket2', 'Horde', 'Second Wind'),
(5, 2, 14, 1, 66, 13386, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Back', 'Alliance', 'Archivist Cape'),
(5, 2, 14, 2, 66, 13386, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Back', 'Horde', 'Archivist Cape'),
(5, 2, 15, 1, 66, 13349, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'MainHand', 'Alliance', 'Scepter of the Unholy'),
(5, 2, 15, 2, 66, 13349, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'MainHand', 'Horde', 'Scepter of the Unholy'),
(5, 2, 16, 1, 66, 10796, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'OffHand', 'Alliance', 'Drakestone'),
(5, 2, 16, 2, 66, 10796, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'OffHand', 'Horde', 'Drakestone'),
(5, 2, 17, 1, 66, 13396, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(5, 2, 17, 2, 66, 13396, 'Phase 1 (Pre-Raid)', 'Priest', 'Shadow', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 1, 76, 10504, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Head', 'Alliance', 'Green Lens'),
(5, 2, 0, 2, 76, 10504, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Head', 'Horde', 'Green Lens'),
(5, 2, 1, 1, 76, 18691, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(5, 2, 1, 2, 76, 18691, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(5, 2, 2, 1, 76, 14112, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Shoulders', 'Alliance', 'Felcloth Shoulders'),
(5, 2, 2, 2, 76, 14112, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Shoulders', 'Horde', 'Felcloth Shoulders'),
(5, 2, 4, 1, 76, 14136, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Chest', 'Alliance', 'Robe of Winter Night'),
(5, 2, 4, 2, 76, 14136, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Chest', 'Horde', 'Robe of Winter Night'),
(5, 2, 5, 1, 76, 11662, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Waist', 'Alliance', 'Ban''thok Sash'),
(5, 2, 5, 2, 76, 11662, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Waist', 'Horde', 'Ban''thok Sash'),
(5, 2, 6, 1, 76, 13170, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(5, 2, 6, 2, 76, 13170, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Legs', 'Horde', 'Skyshroud Leggings'),
(5, 2, 7, 1, 76, 18735, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(5, 2, 7, 2, 76, 18735, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(5, 2, 8, 1, 76, 11766, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(5, 2, 8, 2, 76, 11766, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(5, 2, 9, 1, 76, 18407, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Hands', 'Alliance', 'Felcloth Gloves'),
(5, 2, 9, 2, 76, 18407, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Hands', 'Horde', 'Felcloth Gloves'),
(5, 2, 10, 1, 76, 12543, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(5, 2, 10, 2, 76, 12545, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(5, 2, 11, 1, 76, 13001, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(5, 2, 11, 2, 76, 13001, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Finger2', 'Horde', 'Maiden''s Circle'),
(5, 2, 12, 1, 76, 12930, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(5, 2, 12, 2, 76, 12930, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket1', 'Horde', 'Briarwood Reed'),
(5, 2, 13, 1, 76, 18371, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket2', 'Alliance', 'Mindtap Talisman'),
(5, 2, 13, 2, 76, 18371, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Trinket2', 'Horde', 'Mindtap Talisman'),
(5, 2, 14, 1, 76, 13386, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Back', 'Alliance', 'Archivist Cape'),
(5, 2, 14, 2, 76, 13386, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Back', 'Horde', 'Archivist Cape'),
(5, 2, 15, 1, 76, 13349, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'MainHand', 'Alliance', 'Scepter of the Unholy'),
(5, 2, 15, 2, 76, 13349, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'MainHand', 'Horde', 'Scepter of the Unholy'),
(5, 2, 16, 1, 76, 10796, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'OffHand', 'Alliance', 'Drakestone'),
(5, 2, 16, 2, 76, 10796, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'OffHand', 'Horde', 'Drakestone'),
(5, 2, 17, 1, 76, 13396, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(5, 2, 17, 2, 76, 13396, 'Phase 2 (Pre-Raid)', 'Priest', 'Shadow', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 78, 10504, 'Phase 2', 'Priest', 'Shadow', 'Head', 'Both', 'Green Lens'),
(5, 2, 1, 0, 78, 18814, 'Phase 2', 'Priest', 'Shadow', 'Neck', 'Both', 'Choker of the Fire Lord'),
(5, 2, 2, 0, 78, 14112, 'Phase 2', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Felcloth Shoulders'),
(5, 2, 4, 0, 78, 14136, 'Phase 2', 'Priest', 'Shadow', 'Chest', 'Both', 'Robe of Winter Night'),
(5, 2, 5, 0, 78, 18809, 'Phase 2', 'Priest', 'Shadow', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(5, 2, 6, 0, 78, 19133, 'Phase 2', 'Priest', 'Shadow', 'Legs', 'Both', 'Fel Infused Leggings'),
(5, 2, 7, 0, 78, 19131, 'Phase 2', 'Priest', 'Shadow', 'Feet', 'Both', 'Snowblind Shoes'),
(5, 2, 8, 0, 78, 11766, 'Phase 2', 'Priest', 'Shadow', 'Wrists', 'Both', 'Flameweave Cuffs'),
(5, 2, 9, 0, 78, 18407, 'Phase 2', 'Priest', 'Shadow', 'Hands', 'Both', 'Felcloth Gloves'),
(5, 2, 10, 0, 78, 19147, 'Phase 2', 'Priest', 'Shadow', 'Finger1', 'Both', 'Ring of Spell Power'),
(5, 2, 11, 0, 78, 19147, 'Phase 2', 'Priest', 'Shadow', 'Finger2', 'Both', 'Ring of Spell Power'),
(5, 2, 12, 0, 78, 12930, 'Phase 2', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Briarwood Reed'),
(5, 2, 13, 0, 78, 18820, 'Phase 2', 'Priest', 'Shadow', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(5, 2, 14, 0, 78, 13386, 'Phase 2', 'Priest', 'Shadow', 'Back', 'Both', 'Archivist Cape'),
(5, 2, 15, 0, 78, 18609, 'Phase 2', 'Priest', 'Shadow', 'MainHand', 'Both', 'Anathema'),
(5, 2, 17, 0, 78, 13396, 'Phase 2', 'Priest', 'Shadow', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 83, 19375, 'Phase 3', 'Priest', 'Shadow', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(5, 2, 1, 0, 83, 18814, 'Phase 3', 'Priest', 'Shadow', 'Neck', 'Both', 'Choker of the Fire Lord'),
(5, 2, 2, 0, 83, 19370, 'Phase 3', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(5, 2, 4, 0, 83, 14136, 'Phase 3', 'Priest', 'Shadow', 'Chest', 'Both', 'Robe of Winter Night'),
(5, 2, 5, 0, 83, 19400, 'Phase 3', 'Priest', 'Shadow', 'Waist', 'Both', 'Firemaw''s Clutch'),
(5, 2, 6, 0, 83, 19133, 'Phase 3', 'Priest', 'Shadow', 'Legs', 'Both', 'Fel Infused Leggings'),
(5, 2, 7, 0, 83, 19131, 'Phase 3', 'Priest', 'Shadow', 'Feet', 'Both', 'Snowblind Shoes'),
(5, 2, 8, 0, 83, 19374, 'Phase 3', 'Priest', 'Shadow', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(5, 2, 9, 0, 83, 19407, 'Phase 3', 'Priest', 'Shadow', 'Hands', 'Both', 'Ebony Flame Gloves'),
(5, 2, 11, 0, 83, 19434, 'Phase 3', 'Priest', 'Shadow', 'Finger2', 'Both', 'Band of Dark Dominion'),
(5, 2, 12, 0, 83, 19379, 'Phase 3', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(5, 2, 13, 0, 83, 18820, 'Phase 3', 'Priest', 'Shadow', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(5, 2, 14, 0, 83, 19378, 'Phase 3', 'Priest', 'Shadow', 'Back', 'Both', 'Cloak of the Brood Lord'),
(5, 2, 15, 0, 83, 19360, 'Phase 3', 'Priest', 'Shadow', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(5, 2, 16, 0, 83, 19366, 'Phase 3', 'Priest', 'Shadow', 'OffHand', 'Both', 'Master Dragonslayer''s Orb'),
(5, 2, 17, 0, 83, 13396, 'Phase 3', 'Priest', 'Shadow', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 88, 21348, 'Phase 5', 'Priest', 'Shadow', 'Head', 'Both', 'Tiara of the Oracle'),
(5, 2, 1, 0, 88, 18814, 'Phase 5', 'Priest', 'Shadow', 'Neck', 'Both', 'Choker of the Fire Lord'),
(5, 2, 2, 0, 88, 19370, 'Phase 5', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(5, 2, 4, 0, 88, 21351, 'Phase 5', 'Priest', 'Shadow', 'Chest', 'Both', 'Vestments of the Oracle'),
(5, 2, 5, 0, 88, 19400, 'Phase 5', 'Priest', 'Shadow', 'Waist', 'Both', 'Firemaw''s Clutch'),
(5, 2, 6, 0, 88, 19133, 'Phase 5', 'Priest', 'Shadow', 'Legs', 'Both', 'Fel Infused Leggings'),
(5, 2, 7, 0, 88, 19131, 'Phase 5', 'Priest', 'Shadow', 'Feet', 'Both', 'Snowblind Shoes'),
(5, 2, 8, 0, 88, 21611, 'Phase 5', 'Priest', 'Shadow', 'Wrists', 'Both', 'Burrower Bracers'),
(5, 2, 9, 0, 88, 21585, 'Phase 5', 'Priest', 'Shadow', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(5, 2, 10, 0, 88, 21709, 'Phase 5', 'Priest', 'Shadow', 'Finger1', 'Both', 'Ring of the Fallen God'),
(5, 2, 11, 0, 88, 21210, 'Phase 5', 'Priest', 'Shadow', 'Finger2', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(5, 2, 12, 0, 88, 19379, 'Phase 5', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(5, 2, 13, 0, 88, 18820, 'Phase 5', 'Priest', 'Shadow', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(5, 2, 14, 0, 88, 22731, 'Phase 5', 'Priest', 'Shadow', 'Back', 'Both', 'Cloak of the Devoured'),
(5, 2, 15, 0, 88, 19360, 'Phase 5', 'Priest', 'Shadow', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(5, 2, 16, 0, 88, 19366, 'Phase 5', 'Priest', 'Shadow', 'OffHand', 'Both', 'Master Dragonslayer''s Orb'),
(5, 2, 17, 0, 88, 21603, 'Phase 5', 'Priest', 'Shadow', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 92, 23035, 'Phase 6', 'Priest', 'Shadow', 'Head', 'Both', 'Preceptor''s Hat'),
(5, 2, 1, 0, 92, 18814, 'Phase 6', 'Priest', 'Shadow', 'Neck', 'Both', 'Choker of the Fire Lord'),
(5, 2, 2, 0, 92, 22983, 'Phase 6', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Rime Covered Mantle'),
(5, 2, 4, 0, 92, 23220, 'Phase 6', 'Priest', 'Shadow', 'Chest', 'Both', 'Crystal Webbed Robe'),
(5, 2, 5, 0, 92, 19400, 'Phase 6', 'Priest', 'Shadow', 'Waist', 'Both', 'Firemaw''s Clutch'),
(5, 2, 6, 0, 92, 19133, 'Phase 6', 'Priest', 'Shadow', 'Legs', 'Both', 'Fel Infused Leggings'),
(5, 2, 7, 0, 92, 19131, 'Phase 6', 'Priest', 'Shadow', 'Feet', 'Both', 'Snowblind Shoes'),
(5, 2, 8, 0, 92, 21611, 'Phase 6', 'Priest', 'Shadow', 'Wrists', 'Both', 'Burrower Bracers'),
(5, 2, 9, 0, 92, 21585, 'Phase 6', 'Priest', 'Shadow', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(5, 2, 10, 0, 92, 21709, 'Phase 6', 'Priest', 'Shadow', 'Finger1', 'Both', 'Ring of the Fallen God'),
(5, 2, 11, 0, 92, 21210, 'Phase 6', 'Priest', 'Shadow', 'Finger2', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(5, 2, 12, 0, 92, 19379, 'Phase 6', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(5, 2, 13, 0, 92, 23046, 'Phase 6', 'Priest', 'Shadow', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(5, 2, 14, 0, 92, 22731, 'Phase 6', 'Priest', 'Shadow', 'Back', 'Both', 'Cloak of the Devoured'),
(5, 2, 15, 0, 92, 22988, 'Phase 6', 'Priest', 'Shadow', 'MainHand', 'Both', 'The End of Dreams'),
(5, 2, 16, 0, 92, 23049, 'Phase 6', 'Priest', 'Shadow', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(5, 2, 17, 0, 92, 21603, 'Phase 6', 'Priest', 'Shadow', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 200 (Fresh 80)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 200, 43995, 'Fresh 80', 'Priest', 'Shadow', 'Head', 'Both', 'Enamored Cowl'),
(5, 2, 1, 0, 200, 44658, 'Fresh 80', 'Priest', 'Shadow', 'Neck', 'Both', 'Chain of the Ancient Wyrm'),
(5, 2, 2, 0, 200, 40459, 'Fresh 80', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Valorous Mantle of Faith'),
(5, 2, 4, 0, 200, 40526, 'Fresh 80', 'Priest', 'Shadow', 'Chest', 'Both', 'Gown of the Spell-Weaver'),
(5, 2, 5, 0, 200, 40696, 'Fresh 80', 'Priest', 'Shadow', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(5, 2, 6, 0, 200, 40457, 'Fresh 80', 'Priest', 'Shadow', 'Legs', 'Both', 'Valorous Pants of Faith'),
(5, 2, 7, 0, 200, 40751, 'Fresh 80', 'Priest', 'Shadow', 'Feet', 'Both', 'Slippers of the Holy Light'),
(5, 2, 8, 0, 200, 40740, 'Fresh 80', 'Priest', 'Shadow', 'Wrists', 'Both', 'Wraps of the Astral Traveler'),
(5, 2, 9, 0, 200, 39530, 'Fresh 80', 'Priest', 'Shadow', 'Hands', 'Both', 'Heroes'' Handwraps of Faith'),
(5, 2, 10, 0, 200, 40719, 'Fresh 80', 'Priest', 'Shadow', 'Finger1', 'Both', 'Band of Channeled Magic'),
(5, 2, 12, 0, 200, 39229, 'Fresh 80', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Embrace of the Spider'),
(5, 2, 14, 0, 200, 40723, 'Fresh 80', 'Priest', 'Shadow', 'Back', 'Both', 'Disguise of the Kumiho'),
(5, 2, 15, 0, 200, 39423, 'Fresh 80', 'Priest', 'Shadow', 'MainHand', 'Both', 'Hammer of the Astral Plane'),
(5, 2, 17, 0, 200, 39426, 'Fresh 80', 'Priest', 'Shadow', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 213 (Pre-Ulduar)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 213, 40562, 'Pre-Ulduar', 'Priest', 'Shadow', 'Head', 'Both', 'Hood of Rationality'),
(5, 2, 1, 0, 213, 44661, 'Pre-Ulduar', 'Priest', 'Shadow', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(5, 2, 2, 0, 213, 40555, 'Pre-Ulduar', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Mantle of Dissemination'),
(5, 2, 4, 0, 213, 40234, 'Pre-Ulduar', 'Priest', 'Shadow', 'Chest', 'Both', 'Heigan''s Putrid Vestments'),
(5, 2, 5, 0, 213, 40561, 'Pre-Ulduar', 'Priest', 'Shadow', 'Waist', 'Both', 'Leash of Heedless Magic'),
(5, 2, 6, 0, 213, 40560, 'Pre-Ulduar', 'Priest', 'Shadow', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(5, 2, 7, 0, 213, 40558, 'Pre-Ulduar', 'Priest', 'Shadow', 'Feet', 'Both', 'Arcanic Tramplers'),
(5, 2, 8, 0, 213, 44008, 'Pre-Ulduar', 'Priest', 'Shadow', 'Wrists', 'Both', 'Unsullied Cuffs'),
(5, 2, 9, 0, 213, 40454, 'Pre-Ulduar', 'Priest', 'Shadow', 'Hands', 'Both', 'Valorous Handwraps of Faith'),
(5, 2, 10, 0, 213, 40719, 'Pre-Ulduar', 'Priest', 'Shadow', 'Finger1', 'Both', 'Band of Channeled Magic'),
(5, 2, 12, 0, 213, 40432, 'Pre-Ulduar', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(5, 2, 14, 0, 213, 44005, 'Pre-Ulduar', 'Priest', 'Shadow', 'Back', 'Both', 'Pennant Cloak'),
(5, 2, 17, 0, 213, 39712, 'Pre-Ulduar', 'Priest', 'Shadow', 'Ranged', 'Both', 'Gemmed Wand of the Nerubians');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 245, 46172, 'Phase 2', 'Priest', 'Shadow', 'Head', 'Both', 'Conqueror''s Circlet of Sanctification'),
(5, 2, 1, 0, 245, 45243, 'Phase 2', 'Priest', 'Shadow', 'Neck', 'Both', 'Sapphire Amulet of Renewal'),
(5, 2, 2, 0, 245, 46165, 'Phase 2', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Conqueror''s Mantle of Sanctification'),
(5, 2, 4, 0, 245, 46168, 'Phase 2', 'Priest', 'Shadow', 'Chest', 'Both', 'Conqueror''s Raiments of Sanctification'),
(5, 2, 5, 0, 245, 45619, 'Phase 2', 'Priest', 'Shadow', 'Waist', 'Both', 'Starwatcher''s Binding'),
(5, 2, 6, 0, 245, 46170, 'Phase 2', 'Priest', 'Shadow', 'Legs', 'Both', 'Conqueror''s Pants of Sanctification'),
(5, 2, 7, 0, 245, 45135, 'Phase 2', 'Priest', 'Shadow', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(5, 2, 8, 0, 245, 45446, 'Phase 2', 'Priest', 'Shadow', 'Wrists', 'Both', 'Grasps of Reason'),
(5, 2, 9, 0, 245, 45665, 'Phase 2', 'Priest', 'Shadow', 'Hands', 'Both', 'Pharos Gloves'),
(5, 2, 10, 0, 245, 45495, 'Phase 2', 'Priest', 'Shadow', 'Finger1', 'Both', 'Conductive Seal'),
(5, 2, 12, 0, 245, 45466, 'Phase 2', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Scale of Fates'),
(5, 2, 14, 0, 245, 45242, 'Phase 2', 'Priest', 'Shadow', 'Back', 'Both', 'Drape of Mortal Downfall'),
(5, 2, 17, 0, 245, 45294, 'Phase 2', 'Priest', 'Shadow', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 258, 48085, 'Phase 3', 'Priest', 'Shadow', 'Head', 'Both', 'Circlet of Triumph'),
(5, 2, 1, 1, 258, 47144, 'Phase 3', 'Priest', 'Shadow', 'Neck', 'Alliance', 'Wail of the Val''kyr'),
(5, 2, 1, 2, 258, 47468, 'Phase 3', 'Priest', 'Shadow', 'Neck', 'Horde', 'Cry of the Val''kyr'),
(5, 2, 2, 1, 258, 48082, 'Phase 3', 'Priest', 'Shadow', 'Shoulders', 'Alliance', 'Velen''s Mantle of Triumph'),
(5, 2, 2, 2, 258, 48090, 'Phase 3', 'Priest', 'Shadow', 'Shoulders', 'Horde', 'Zabra''s Raiments of Triumph'),
(5, 2, 4, 0, 258, 48083, 'Phase 3', 'Priest', 'Shadow', 'Chest', 'Both', 'Raiments of Triumph'),
(5, 2, 5, 1, 258, 46973, 'Phase 3', 'Priest', 'Shadow', 'Waist', 'Alliance', 'Cord of the Tenebrous Mist'),
(5, 2, 5, 2, 258, 47419, 'Phase 3', 'Priest', 'Shadow', 'Waist', 'Horde', 'Belt of the Tenebrous Mist'),
(5, 2, 6, 0, 258, 48084, 'Phase 3', 'Priest', 'Shadow', 'Legs', 'Both', 'Pants of Triumph'),
(5, 2, 7, 1, 258, 47097, 'Phase 3', 'Priest', 'Shadow', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(5, 2, 7, 2, 258, 47454, 'Phase 3', 'Priest', 'Shadow', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(5, 2, 8, 1, 258, 47208, 'Phase 3', 'Priest', 'Shadow', 'Wrists', 'Alliance', 'Armbands of the Ashen Saint'),
(5, 2, 8, 2, 258, 47467, 'Phase 3', 'Priest', 'Shadow', 'Wrists', 'Horde', 'Dark Essence Bindings'),
(5, 2, 9, 1, 258, 45665, 'Phase 3', 'Priest', 'Shadow', 'Hands', 'Alliance', 'Pharos Gloves'),
(5, 2, 10, 1, 258, 46046, 'Phase 3', 'Priest', 'Shadow', 'Finger1', 'Alliance', 'Nebula Band'),
(5, 2, 10, 2, 258, 47237, 'Phase 3', 'Priest', 'Shadow', 'Finger1', 'Horde', 'Band of Deplorable Violence'),
(5, 2, 12, 2, 258, 45518, 'Phase 3', 'Priest', 'Shadow', 'Trinket1', 'Horde', 'Flare of the Heavens'),
(5, 2, 14, 1, 258, 47552, 'Phase 3', 'Priest', 'Shadow', 'Back', 'Alliance', 'Jaina''s Radiance'),
(5, 2, 14, 2, 258, 47551, 'Phase 3', 'Priest', 'Shadow', 'Back', 'Horde', 'Aethas'' Intensity'),
(5, 2, 17, 1, 258, 47922, 'Phase 3', 'Priest', 'Shadow', 'Ranged', 'Alliance', 'Rod of Imprisoned Souls'),
(5, 2, 17, 2, 258, 45294, 'Phase 3', 'Priest', 'Shadow', 'Ranged', 'Horde', 'Petrified Ivy Sprig');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 264, 51255, 'Phase 4', 'Priest', 'Shadow', 'Head', 'Both', 'Sanctified Crimson Acolyte Cowl'),
(5, 2, 1, 0, 264, 50724, 'Phase 4', 'Priest', 'Shadow', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(5, 2, 2, 0, 264, 51257, 'Phase 4', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Mantle'),
(5, 2, 4, 0, 264, 51259, 'Phase 4', 'Priest', 'Shadow', 'Chest', 'Both', 'Sanctified Crimson Acolyte Raiments'),
(5, 2, 5, 0, 264, 50613, 'Phase 4', 'Priest', 'Shadow', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(5, 2, 6, 0, 264, 50694, 'Phase 4', 'Priest', 'Shadow', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(5, 2, 7, 0, 264, 50699, 'Phase 4', 'Priest', 'Shadow', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 2, 8, 0, 264, 50651, 'Phase 4', 'Priest', 'Shadow', 'Wrists', 'Both', 'The Lady''s Brittle Bracers'),
(5, 2, 9, 0, 264, 51256, 'Phase 4', 'Priest', 'Shadow', 'Hands', 'Both', 'Sanctified Crimson Acolyte Handwraps'),
(5, 2, 10, 0, 264, 50398, 'Phase 4', 'Priest', 'Shadow', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(5, 2, 12, 0, 264, 50348, 'Phase 4', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Dislodged Foreign Object'),
(5, 2, 14, 0, 264, 50628, 'Phase 4', 'Priest', 'Shadow', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(5, 2, 15, 0, 264, 50734, 'Phase 4', 'Priest', 'Shadow', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(5, 2, 17, 0, 264, 50684, 'Phase 4', 'Priest', 'Shadow', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(5, 2, 0, 0, 290, 51255, 'Phase 5', 'Priest', 'Shadow', 'Head', 'Both', 'Sanctified Crimson Acolyte Cowl'),
(5, 2, 1, 0, 290, 50182, 'Phase 5', 'Priest', 'Shadow', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(5, 2, 2, 0, 290, 51257, 'Phase 5', 'Priest', 'Shadow', 'Shoulders', 'Both', 'Sanctified Crimson Acolyte Mantle'),
(5, 2, 4, 0, 290, 51259, 'Phase 5', 'Priest', 'Shadow', 'Chest', 'Both', 'Sanctified Crimson Acolyte Raiments'),
(5, 2, 5, 0, 290, 50613, 'Phase 5', 'Priest', 'Shadow', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(5, 2, 6, 0, 290, 50694, 'Phase 5', 'Priest', 'Shadow', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(5, 2, 7, 0, 290, 50699, 'Phase 5', 'Priest', 'Shadow', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(5, 2, 8, 0, 290, 54582, 'Phase 5', 'Priest', 'Shadow', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(5, 2, 9, 0, 290, 51256, 'Phase 5', 'Priest', 'Shadow', 'Hands', 'Both', 'Sanctified Crimson Acolyte Handwraps'),
(5, 2, 10, 0, 290, 50664, 'Phase 5', 'Priest', 'Shadow', 'Finger1', 'Both', 'Ring of Rapid Ascent'),
(5, 2, 11, 0, 290, 50398, 'Phase 5', 'Priest', 'Shadow', 'Finger2', 'Both', 'Ashen Band of Endless Destruction'),
(5, 2, 12, 0, 290, 54588, 'Phase 5', 'Priest', 'Shadow', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(5, 2, 13, 0, 290, 50348, 'Phase 5', 'Priest', 'Shadow', 'Trinket2', 'Both', 'Dislodged Foreign Object'),
(5, 2, 14, 0, 290, 54583, 'Phase 5', 'Priest', 'Shadow', 'Back', 'Both', 'Cloak of Burning Dusk'),
(5, 2, 15, 0, 290, 50734, 'Phase 5', 'Priest', 'Shadow', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(5, 2, 16, 0, 290, 50719, 'Phase 5', 'Priest', 'Shadow', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(5, 2, 17, 0, 290, 50684, 'Phase 5', 'Priest', 'Shadow', 'Ranged', 'Both', 'Corpse-Impaling Spike');


-- ============================================================
-- Death Knight (6)
-- ============================================================
-- Blood Tank (tab 0)
-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 200, 41387, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Head', 'Both', 'Tempered Titansteel Helm'),
(6, 0, 1, 0, 200, 42646, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Neck', 'Both', 'Titanium Earthguard Chain'),
(6, 0, 2, 0, 200, 37814, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Shoulders', 'Both', 'Iron Dwarf Smith Pauldrons'),
(6, 0, 4, 0, 200, 37735, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Chest', 'Both', 'Ziggurat Imprinted Chestguard'),
(6, 0, 5, 0, 200, 37241, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Waist', 'Both', 'Ancient Aligned Girdle'),
(6, 0, 6, 0, 200, 37193, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Legs', 'Both', 'Staggering Legplates'),
(6, 0, 7, 0, 200, 41392, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Feet', 'Both', 'Tempered Titansteel Treads'),
(6, 0, 8, 0, 200, 37620, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Wrists', 'Both', 'Bracers of the Herald'),
(6, 0, 9, 0, 200, 37645, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Hands', 'Both', 'Horn-Tipped Gauntlets'),
(6, 0, 10, 0, 200, 42643, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Finger1', 'Both', 'Titanium Earthguard Ring'),
(6, 0, 12, 0, 200, 37220, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Trinket1', 'Both', 'Essence of Gossamer'),
(6, 0, 14, 0, 200, 43565, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Back', 'Both', 'Durable Nerubhide Cape'),
(6, 0, 15, 0, 200, 41257, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'MainHand', 'Both', 'Titansteel Destroyer'),
(6, 0, 17, 0, 200, 40714, 'Pre-Raid', 'Death Knight', 'Blood Tank', 'Ranged', 'Both', 'Sigil of the Unfaltering Knight');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 224, 40565, 'Phase 1', 'Death Knight', 'Blood Tank', 'Head', 'Both', 'Valorous Scourgeborne Faceguard'),
(6, 0, 1, 0, 224, 40387, 'Phase 1', 'Death Knight', 'Blood Tank', 'Neck', 'Both', 'Boundless Ambition'),
(6, 0, 2, 0, 224, 40568, 'Phase 1', 'Death Knight', 'Blood Tank', 'Shoulders', 'Both', 'Valorous Scourgeborne Pauldrons'),
(6, 0, 4, 0, 224, 40559, 'Phase 1', 'Death Knight', 'Blood Tank', 'Chest', 'Both', 'Valorous Scourgeborne Chestguard'),
(6, 0, 5, 0, 224, 39759, 'Phase 1', 'Death Knight', 'Blood Tank', 'Waist', 'Both', 'Ablative Chitin Girdle'),
(6, 0, 6, 0, 224, 40589, 'Phase 1', 'Death Knight', 'Blood Tank', 'Legs', 'Both', 'Legplates of Sovereignty'),
(6, 0, 7, 0, 224, 40297, 'Phase 1', 'Death Knight', 'Blood Tank', 'Feet', 'Both', 'Sabatons of Endurance'),
(6, 0, 8, 0, 224, 40306, 'Phase 1', 'Death Knight', 'Blood Tank', 'Wrists', 'Both', 'Bracers of the Unholy Knight'),
(6, 0, 9, 0, 224, 40563, 'Phase 1', 'Death Knight', 'Blood Tank', 'Hands', 'Both', 'Valorous Scourgeborne Handguards'),
(6, 0, 10, 0, 224, 40107, 'Phase 1', 'Death Knight', 'Blood Tank', 'Finger1', 'Both', 'Sand-Worn Band'),
(6, 0, 12, 0, 224, 37220, 'Phase 1', 'Death Knight', 'Blood Tank', 'Trinket1', 'Both', 'Essence of Gossamer'),
(6, 0, 14, 0, 224, 40252, 'Phase 1', 'Death Knight', 'Blood Tank', 'Back', 'Both', 'Cloak of the Shadowed Sun'),
(6, 0, 15, 0, 224, 40406, 'Phase 1', 'Death Knight', 'Blood Tank', 'MainHand', 'Both', 'Inevitable Defeat'),
(6, 0, 17, 0, 224, 40714, 'Phase 1', 'Death Knight', 'Blood Tank', 'Ranged', 'Both', 'Sigil of the Unfaltering Knight');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 245, 46120, 'Phase 2', 'Death Knight', 'Blood Tank', 'Head', 'Both', 'Conqueror''s Darkruned Faceguard'),
(6, 0, 1, 0, 245, 45485, 'Phase 2', 'Death Knight', 'Blood Tank', 'Neck', 'Both', 'Bronze Pendant of the Vanir'),
(6, 0, 2, 0, 245, 46122, 'Phase 2', 'Death Knight', 'Blood Tank', 'Shoulders', 'Both', 'Conqueror''s Darkruned Pauldrons'),
(6, 0, 4, 0, 245, 46039, 'Phase 2', 'Death Knight', 'Blood Tank', 'Chest', 'Both', 'Breastplate of the Timeless'),
(6, 0, 5, 0, 245, 45825, 'Phase 2', 'Death Knight', 'Blood Tank', 'Waist', 'Both', 'Shieldwarder Girdle'),
(6, 0, 6, 0, 245, 45594, 'Phase 2', 'Death Knight', 'Blood Tank', 'Legs', 'Both', 'Legplates of the Endless Void'),
(6, 0, 7, 0, 245, 45988, 'Phase 2', 'Death Knight', 'Blood Tank', 'Feet', 'Both', 'Greaves of the Iron Army'),
(6, 0, 8, 0, 245, 45111, 'Phase 2', 'Death Knight', 'Blood Tank', 'Wrists', 'Both', 'Mimiron''s Inferno Couplings'),
(6, 0, 9, 0, 245, 45487, 'Phase 2', 'Death Knight', 'Blood Tank', 'Hands', 'Both', 'Handguards of Revitalization'),
(6, 0, 10, 0, 245, 45471, 'Phase 2', 'Death Knight', 'Blood Tank', 'Finger1', 'Both', 'Fate''s Clutch'),
(6, 0, 12, 0, 245, 45158, 'Phase 2', 'Death Knight', 'Blood Tank', 'Trinket1', 'Both', 'Heart of Iron'),
(6, 0, 14, 0, 245, 45496, 'Phase 2', 'Death Knight', 'Blood Tank', 'Back', 'Both', 'Titanskin Cloak'),
(6, 0, 15, 0, 245, 45516, 'Phase 2', 'Death Knight', 'Blood', 'MainHand', 'Both', 'Voldrethar, Dark Blade of Oblivion'),
(6, 0, 17, 0, 245, 40207, 'Phase 2', 'Death Knight', 'Blood Tank', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 258, 48545, 'Phase 3', 'Death Knight', 'Blood Tank', 'Head', 'Both', 'Faceguard of Triumph'),
(6, 0, 0, 1, 258, 48488, 'Phase 3', 'Death Knight', 'Blood', 'Head', 'Alliance', 'Thassarian''s Helmet of Triumph'),
(6, 0, 0, 2, 258, 48493, 'Phase 3', 'Death Knight', 'Blood', 'Head', 'Horde', 'Koltira''s Helmet of Triumph'),
(6, 0, 1, 1, 258, 47110, 'Phase 3', 'Death Knight', 'Blood', 'Neck', 'Alliance', 'The Executioner''s Malice'),
(6, 0, 1, 2, 258, 47458, 'Phase 3', 'Death Knight', 'Blood', 'Neck', 'Horde', 'The Executioner''s Vice'),
(6, 0, 2, 0, 258, 48543, 'Phase 3', 'Death Knight', 'Blood Tank', 'Shoulders', 'Both', 'Pauldrons of Triumph'),
(6, 0, 2, 1, 258, 48486, 'Phase 3', 'Death Knight', 'Blood', 'Shoulders', 'Alliance', 'Thassarian''s Shoulderplates of Triumph'),
(6, 0, 2, 2, 258, 48495, 'Phase 3', 'Death Knight', 'Blood', 'Shoulders', 'Horde', 'Koltira''s Shoulderplates of Triumph'),
(6, 0, 4, 1, 258, 46968, 'Phase 3', 'Death Knight', 'Blood Tank', 'Chest', 'Alliance', 'Chestplate of the Towering Monstrosity'),
(6, 0, 4, 2, 258, 47415, 'Phase 3', 'Death Knight', 'Blood Tank', 'Chest', 'Horde', 'Hauberk of the Towering Monstrosity'),
(6, 0, 5, 1, 258, 47076, 'Phase 3', 'Death Knight', 'Blood Tank', 'Waist', 'Alliance', 'Girdle of Bloodied Scars'),
(6, 0, 5, 2, 258, 47444, 'Phase 3', 'Death Knight', 'Blood Tank', 'Waist', 'Horde', 'Belt of Bloodied Scars'),
(6, 0, 6, 0, 258, 48544, 'Phase 3', 'Death Knight', 'Blood Tank', 'Legs', 'Both', 'Legguards of Triumph'),
(6, 0, 6, 1, 258, 48487, 'Phase 3', 'Death Knight', 'Blood', 'Legs', 'Alliance', 'Thassarian''s Legplates of Triumph'),
(6, 0, 6, 2, 258, 48494, 'Phase 3', 'Death Knight', 'Blood', 'Legs', 'Horde', 'Koltira''s Legplates of Triumph'),
(6, 0, 7, 0, 258, 45599, 'Phase 3', 'Death Knight', 'Blood', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(6, 0, 7, 1, 258, 47003, 'Phase 3', 'Death Knight', 'Blood Tank', 'Feet', 'Alliance', 'Dawnbreaker Greaves'),
(6, 0, 7, 2, 258, 47430, 'Phase 3', 'Death Knight', 'Blood Tank', 'Feet', 'Horde', 'Dawnbreaker Sabatons'),
(6, 0, 8, 0, 258, 45663, 'Phase 3', 'Death Knight', 'Blood', 'Wrists', 'Both', 'Armbands of Bedlam'),
(6, 0, 8, 1, 258, 47111, 'Phase 3', 'Death Knight', 'Blood Tank', 'Wrists', 'Alliance', 'Bracers of the Shieldmaiden'),
(6, 0, 8, 2, 258, 47459, 'Phase 3', 'Death Knight', 'Blood Tank', 'Wrists', 'Horde', 'Armguards of the Shieldmaiden'),
(6, 0, 9, 0, 258, 48546, 'Phase 3', 'Death Knight', 'Blood Tank', 'Hands', 'Both', 'Handguards of Triumph'),
(6, 0, 9, 1, 258, 47240, 'Phase 3', 'Death Knight', 'Blood', 'Hands', 'Alliance', 'Gloves of Bitter Reprisal'),
(6, 0, 9, 2, 258, 47492, 'Phase 3', 'Death Knight', 'Blood', 'Hands', 'Horde', 'Gauntlets of Bitter Reprisal'),
(6, 0, 10, 1, 258, 45471, 'Phase 3', 'Death Knight', 'Blood Tank', 'Finger1', 'Alliance', 'Fate''s Clutch'),
(6, 0, 10, 2, 258, 47413, 'Phase 3', 'Death Knight', 'Blood', 'Finger1', 'Horde', 'Ring of the Violent Temperament'),
(6, 0, 12, 0, 258, 45931, 'Phase 3', 'Death Knight', 'Blood', 'Trinket1', 'Both', 'Mjolnir Runestone'),
(6, 0, 12, 1, 258, 47216, 'Phase 3', 'Death Knight', 'Blood Tank', 'Trinket1', 'Alliance', 'The Black Heart'),
(6, 0, 14, 1, 258, 47549, 'Phase 3', 'Death Knight', 'Blood Tank', 'Back', 'Alliance', 'Magni''s Resolution'),
(6, 0, 14, 2, 258, 47550, 'Phase 3', 'Death Knight', 'Blood Tank', 'Back', 'Horde', 'Cairne''s Endurance'),
(6, 0, 15, 0, 258, 47156, 'Phase 3', 'Death Knight', 'Blood', 'MainHand', 'Both', 'Stormpike Cleaver'),
(6, 0, 15, 2, 258, 47475, 'Phase 3', 'Death Knight', 'Blood', 'MainHand', 'Horde', 'Hellscream Slicer'),
(6, 0, 17, 0, 258, 47672, 'Phase 3', 'Death Knight', 'Blood Tank', 'Ranged', 'Both', 'Sigil of Insolence');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 264, 51306, 'Phase 4', 'Death Knight', 'Blood Tank', 'Head', 'Both', 'Sanctified Scourgelord Faceguard'),
(6, 0, 1, 0, 264, 54581, 'Phase 4', 'Death Knight', 'Blood', 'Neck', 'Both', 'Penumbra Pendant'),
(6, 0, 2, 0, 264, 51309, 'Phase 4', 'Death Knight', 'Blood Tank', 'Shoulders', 'Both', 'Sanctified Scourgelord Pauldrons'),
(6, 0, 4, 0, 264, 51305, 'Phase 4', 'Death Knight', 'Blood Tank', 'Chest', 'Both', 'Sanctified Scourgelord Chestguard'),
(6, 0, 5, 0, 264, 50691, 'Phase 4', 'Death Knight', 'Blood Tank', 'Waist', 'Both', 'Belt of Broken Bones'),
(6, 0, 6, 0, 264, 50612, 'Phase 4', 'Death Knight', 'Blood Tank', 'Legs', 'Both', 'Legguards of Lost Hope'),
(6, 0, 7, 0, 264, 50625, 'Phase 4', 'Death Knight', 'Blood Tank', 'Feet', 'Both', 'Grinning Skull Greatboots'),
(6, 0, 8, 0, 264, 50611, 'Phase 4', 'Death Knight', 'Blood Tank', 'Wrists', 'Both', 'Bracers of Dark Reckoning'),
(6, 0, 9, 0, 264, 51307, 'Phase 4', 'Death Knight', 'Blood Tank', 'Hands', 'Both', 'Sanctified Scourgelord Handguards'),
(6, 0, 10, 0, 264, 50622, 'Phase 4', 'Death Knight', 'Blood Tank', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(6, 0, 12, 0, 264, 50364, 'Phase 4', 'Death Knight', 'Blood Tank', 'Trinket1', 'Both', 'Sindragosa''s Flawless Fang'),
(6, 0, 14, 0, 264, 50718, 'Phase 4', 'Death Knight', 'Blood Tank', 'Back', 'Both', 'Royal Crimson Cloak'),
(6, 0, 15, 0, 264, 49623, 'Phase 4', 'Death Knight', 'Blood', 'MainHand', 'Both', 'Shadowmourne'),
(6, 0, 17, 0, 264, 50462, 'Phase 4', 'Death Knight', 'Blood Tank', 'Ranged', 'Both', 'Sigil of the Bone Gryphon');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 0, 0, 0, 290, 51306, 'Phase 5', 'DeathKnight', 'BloodTank', 'Head', 'Both', 'Sanctified Scourgelord Faceguard'),
(6, 0, 1, 0, 290, 50682, 'Phase 5', 'DeathKnight', 'BloodTank', 'Neck', 'Both', 'Bile-Encrusted Medallion'),
(6, 0, 2, 0, 290, 51309, 'Phase 5', 'DeathKnight', 'BloodTank', 'Shoulders', 'Both', 'Sanctified Scourgelord Pauldrons'),
(6, 0, 4, 0, 290, 51305, 'Phase 5', 'DeathKnight', 'BloodTank', 'Chest', 'Both', 'Sanctified Scourgelord Chestguard'),
(6, 0, 5, 0, 290, 50991, 'Phase 5', 'DeathKnight', 'BloodTank', 'Waist', 'Both', 'Verdigris Chain Belt'),
(6, 0, 6, 0, 290, 50612, 'Phase 5', 'DeathKnight', 'BloodTank', 'Legs', 'Both', 'Legguards of Lost Hope'),
(6, 0, 7, 0, 290, 54579, 'Phase 5', 'DeathKnight', 'BloodTank', 'Feet', 'Both', 'Treads of Impending Resurrection'),
(6, 0, 8, 0, 290, 51901, 'Phase 5', 'DeathKnight', 'BloodTank', 'Wrists', 'Both', 'Gargoyle Spit Bracers'),
(6, 0, 9, 0, 290, 51307, 'Phase 5', 'DeathKnight', 'BloodTank', 'Hands', 'Both', 'Sanctified Scourgelord Handguards'),
(6, 0, 10, 0, 290, 50622, 'Phase 5', 'DeathKnight', 'BloodTank', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(6, 0, 11, 0, 290, 50404, 'Phase 5', 'DeathKnight', 'BloodTank', 'Finger2', 'Both', 'Ashen Band of Endless Courage'),
(6, 0, 12, 0, 290, 54591, 'Phase 5', 'DeathKnight', 'BloodTank', 'Trinket1', 'Both', 'Petrified Twilight Scale'),
(6, 0, 13, 0, 290, 50364, 'Phase 5', 'DeathKnight', 'BloodTank', 'Trinket2', 'Both', 'Sindragosa''s Flawless Fang'),
(6, 0, 14, 0, 290, 50466, 'Phase 5', 'DeathKnight', 'BloodTank', 'Back', 'Both', 'Sentinel''s Winter Cloak'),
(6, 0, 15, 0, 290, 49623, 'Phase 5', 'DeathKnight', 'BloodTank', 'MainHand', 'Both', 'Shadowmourne'),
(6, 0, 17, 0, 290, 50462, 'Phase 5', 'DeathKnight', 'BloodTank', 'Ranged', 'Both', 'Sigil of the Bone Gryphon');

-- Frost (tab 1)
-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 0, 200, 41386, 'Pre-Raid', 'Death Knight', 'Frost', 'Head', 'Both', 'Spiked Titansteel Helm'),
(6, 1, 1, 0, 200, 42645, 'Pre-Raid', 'Death Knight', 'Frost', 'Neck', 'Both', 'Titanium Impact Choker'),
(6, 1, 2, 0, 200, 37627, 'Pre-Raid', 'Death Knight', 'Frost', 'Shoulders', 'Both', 'Snake Den Spaulders'),
(6, 1, 4, 0, 200, 37612, 'Pre-Raid', 'Death Knight', 'Frost', 'Chest', 'Both', 'Bonegrinder Breastplate'),
(6, 1, 5, 0, 200, 37171, 'Pre-Raid', 'Death Knight', 'Frost', 'Waist', 'Both', 'Flame-Bathed Steel Girdle'),
(6, 1, 6, 0, 200, 37193, 'Pre-Raid', 'Death Knight', 'Frost', 'Legs', 'Both', 'Staggering Legplates'),
(6, 1, 7, 0, 200, 41391, 'Pre-Raid', 'Death Knight', 'Frost', 'Feet', 'Both', 'Spiked Titansteel Treads'),
(6, 1, 8, 0, 200, 37668, 'Pre-Raid', 'Death Knight', 'Frost', 'Wrists', 'Both', 'Bands of the Stoneforge'),
(6, 1, 9, 0, 200, 37363, 'Pre-Raid', 'Death Knight', 'Frost', 'Hands', 'Both', 'Gauntlets of Dragon Wrath'),
(6, 1, 10, 0, 200, 37642, 'Pre-Raid', 'Death Knight', 'Frost', 'Finger1', 'Both', 'Hemorrhaging Circle'),
(6, 1, 12, 0, 200, 40684, 'Pre-Raid', 'Death Knight', 'Frost', 'Trinket1', 'Both', 'Mirror of Truth'),
(6, 1, 14, 0, 200, 43566, 'Pre-Raid', 'Death Knight', 'Frost', 'Back', 'Both', 'Ice Striker''s Cloak'),
(6, 1, 15, 0, 200, 41383, 'Pre-Raid', 'Death Knight', 'Frost', 'MainHand', 'Both', 'Titansteel Bonecrusher'),
(6, 1, 17, 0, 200, 40715, 'Pre-Raid', 'Death Knight', 'Frost', 'Ranged', 'Both', 'Sigil of Haunted Dreams');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 0, 224, 44006, 'Phase 1', 'Death Knight', 'Frost', 'Head', 'Both', 'Obsidian Greathelm'),
(6, 1, 1, 0, 224, 44664, 'Phase 1', 'Death Knight', 'Frost', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(6, 1, 2, 0, 224, 40557, 'Phase 1', 'Death Knight', 'Frost', 'Shoulders', 'Both', 'Valorous Scourgeborne Shoulderplates'),
(6, 1, 4, 0, 224, 40550, 'Phase 1', 'Death Knight', 'Frost', 'Chest', 'Both', 'Valorous Scourgeborne Battleplate'),
(6, 1, 5, 0, 224, 40317, 'Phase 1', 'Death Knight', 'Frost', 'Waist', 'Both', 'Girdle of Razuvious'),
(6, 1, 6, 0, 224, 40556, 'Phase 1', 'Death Knight', 'Frost', 'Legs', 'Both', 'Valorous Scourgeborne Legplates'),
(6, 1, 7, 0, 224, 40591, 'Phase 1', 'Death Knight', 'Frost', 'Feet', 'Both', 'Melancholy Sabatons'),
(6, 1, 8, 0, 224, 40330, 'Phase 1', 'Death Knight', 'Frost', 'Wrists', 'Both', 'Bracers of Unrelenting Attack'),
(6, 1, 9, 0, 224, 40552, 'Phase 1', 'Death Knight', 'Frost', 'Hands', 'Both', 'Valorous Scourgeborne Gauntlets'),
(6, 1, 10, 0, 224, 40075, 'Phase 1', 'Death Knight', 'Frost', 'Finger1', 'Both', 'Ruthlessness'),
(6, 1, 12, 0, 224, 40256, 'Phase 1', 'Death Knight', 'Frost', 'Trinket1', 'Both', 'Grim Toll'),
(6, 1, 14, 0, 224, 40403, 'Phase 1', 'Death Knight', 'Frost', 'Back', 'Both', 'Drape of the Deadly Foe'),
(6, 1, 15, 0, 224, 40189, 'Phase 1', 'Death Knight', 'Frost', 'MainHand', 'Both', 'Angry Dread'),
(6, 1, 17, 0, 224, 40207, 'Phase 1', 'Death Knight', 'Frost', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 0, 245, 46115, 'Phase 2', 'Death Knight', 'Frost', 'Head', 'Both', 'Conqueror''s Darkruned Helmet'),
(6, 1, 1, 0, 245, 45459, 'Phase 2', 'Death Knight', 'Frost', 'Neck', 'Both', 'Frigid Strength of Hodir'),
(6, 1, 2, 0, 245, 46117, 'Phase 2', 'Death Knight', 'Frost', 'Shoulders', 'Both', 'Conqueror''s Darkruned Shoulderplates'),
(6, 1, 4, 0, 245, 46111, 'Phase 2', 'Death Knight', 'Frost', 'Chest', 'Both', 'Conqueror''s Darkruned Battleplate'),
(6, 1, 5, 0, 245, 45241, 'Phase 2', 'Death Knight', 'Frost', 'Waist', 'Both', 'Belt of Colossal Rage'),
(6, 1, 6, 0, 245, 45134, 'Phase 2', 'Death Knight', 'Frost', 'Legs', 'Both', 'Plated Leggings of Ruination'),
(6, 1, 7, 0, 245, 45599, 'Phase 2', 'Death Knight', 'Frost', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(6, 1, 8, 0, 245, 45663, 'Phase 2', 'Death Knight', 'Frost', 'Wrists', 'Both', 'Armbands of Bedlam'),
(6, 1, 9, 0, 245, 46113, 'Phase 2', 'Death Knight', 'Frost', 'Hands', 'Both', 'Conqueror''s Darkruned Gauntlets'),
(6, 1, 10, 0, 245, 45534, 'Phase 2', 'Death Knight', 'Frost', 'Finger1', 'Both', 'Seal of the Betrayed King'),
(6, 1, 12, 0, 245, 45931, 'Phase 2', 'Death Knight', 'Frost', 'Trinket1', 'Both', 'Mjolnir Runestone'),
(6, 1, 14, 0, 245, 46032, 'Phase 2', 'Death Knight', 'Frost', 'Back', 'Both', 'Drape of the Faceless General'),
(6, 1, 15, 0, 245, 46097, 'Phase 2', 'Death Knight', 'Frost', 'MainHand', 'Both', 'Caress of Insanity'),
(6, 1, 17, 0, 245, 40207, 'Phase 2', 'Death Knight', 'Frost', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 1, 258, 48488, 'Phase 3', 'Death Knight', 'Frost', 'Head', 'Alliance', 'Thassarian''s Helmet of Triumph'),
(6, 1, 0, 2, 258, 48493, 'Phase 3', 'Death Knight', 'Frost', 'Head', 'Horde', 'Koltira''s Helmet of Triumph'),
(6, 1, 1, 1, 258, 47110, 'Phase 3', 'Death Knight', 'Frost', 'Neck', 'Alliance', 'The Executioner''s Malice'),
(6, 1, 1, 2, 258, 47458, 'Phase 3', 'Death Knight', 'Frost', 'Neck', 'Horde', 'The Executioner''s Vice'),
(6, 1, 2, 1, 258, 48486, 'Phase 3', 'Death Knight', 'Frost', 'Shoulders', 'Alliance', 'Thassarian''s Shoulderplates of Triumph'),
(6, 1, 2, 2, 258, 48495, 'Phase 3', 'Death Knight', 'Frost', 'Shoulders', 'Horde', 'Koltira''s Shoulderplates of Triumph'),
(6, 1, 4, 1, 258, 48490, 'Phase 3', 'Death Knight', 'Frost', 'Chest', 'Alliance', 'Thassarian''s Battleplate of Triumph'),
(6, 1, 4, 2, 258, 48491, 'Phase 3', 'Death Knight', 'Frost', 'Chest', 'Horde', 'Koltira''s Battleplate of Triumph'),
(6, 1, 5, 0, 258, 45241, 'Phase 3', 'Death Knight', 'Frost', 'Waist', 'Both', 'Belt of Colossal Rage'),
(6, 1, 6, 1, 258, 48487, 'Phase 3', 'Death Knight', 'Frost', 'Legs', 'Alliance', 'Thassarian''s Legplates of Triumph'),
(6, 1, 6, 2, 258, 48494, 'Phase 3', 'Death Knight', 'Frost', 'Legs', 'Horde', 'Koltira''s Legplates of Triumph'),
(6, 1, 7, 0, 258, 45599, 'Phase 3', 'Death Knight', 'Frost', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(6, 1, 8, 0, 258, 45663, 'Phase 3', 'Death Knight', 'Frost', 'Wrists', 'Both', 'Armbands of Bedlam'),
(6, 1, 9, 1, 258, 47240, 'Phase 3', 'Death Knight', 'Frost', 'Hands', 'Alliance', 'Gloves of Bitter Reprisal'),
(6, 1, 9, 2, 258, 47492, 'Phase 3', 'Death Knight', 'Frost', 'Hands', 'Horde', 'Gauntlets of Bitter Reprisal'),
(6, 1, 10, 1, 258, 46966, 'Phase 3', 'Death Knight', 'Frost', 'Finger1', 'Alliance', 'Band of the Violent Temperment'),
(6, 1, 10, 2, 258, 47413, 'Phase 3', 'Death Knight', 'Frost', 'Finger1', 'Horde', 'Ring of the Violent Temperament'),
(6, 1, 12, 0, 258, 45931, 'Phase 3', 'Death Knight', 'Frost', 'Trinket1', 'Both', 'Mjolnir Runestone'),
(6, 1, 14, 1, 258, 47547, 'Phase 3', 'Death Knight', 'Frost', 'Back', 'Alliance', 'Varian''s Furor'),
(6, 1, 14, 2, 258, 47548, 'Phase 3', 'Death Knight', 'Frost', 'Back', 'Horde', 'Garrosh''s Rage'),
(6, 1, 15, 0, 258, 47156, 'Phase 3', 'Death Knight', 'Frost', 'MainHand', 'Both', 'Stormpike Cleaver'),
(6, 1, 15, 2, 258, 47475, 'Phase 3', 'Death Knight', 'Frost', 'MainHand', 'Horde', 'Hellscream Slicer'),
(6, 1, 17, 0, 258, 40207, 'Phase 3', 'Death Knight', 'Frost', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 0, 264, 51312, 'Phase 4', 'Death Knight', 'Frost', 'Head', 'Both', 'Sanctified Scourgelord Helmet'),
(6, 1, 1, 0, 264, 50728, 'Phase 4', 'Death Knight', 'Frost', 'Neck', 'Both', 'Lana''thel''s Chain of Flagellation'),
(6, 1, 2, 0, 264, 51314, 'Phase 4', 'Death Knight', 'Frost', 'Shoulders', 'Both', 'Sanctified Scourgelord Shoulderplates'),
(6, 1, 4, 0, 264, 51310, 'Phase 4', 'Death Knight', 'Frost', 'Chest', 'Both', 'Sanctified Scourgelord Battleplate'),
(6, 1, 5, 0, 264, 50620, 'Phase 4', 'Death Knight', 'Frost', 'Waist', 'Both', 'Coldwraith Links'),
(6, 1, 6, 0, 264, 51817, 'Phase 4', 'Death Knight', 'Frost', 'Legs', 'Both', 'Legplates of Aetheric Strife'),
(6, 1, 7, 0, 264, 50639, 'Phase 4', 'Death Knight', 'Frost', 'Feet', 'Both', 'Blood-Soaked Saronite Stompers'),
(6, 1, 8, 0, 264, 50659, 'Phase 4', 'Death Knight', 'Frost', 'Wrists', 'Both', 'Polar Bear Claw Bracers'),
(6, 1, 9, 0, 264, 51311, 'Phase 4', 'Death Knight', 'Frost', 'Hands', 'Both', 'Sanctified Scourgelord Gauntlets'),
(6, 1, 10, 0, 264, 52572, 'Phase 4', 'Death Knight', 'Frost', 'Finger1', 'Both', 'Ashen Band of Endless Might'),
(6, 1, 12, 0, 264, 50363, 'Phase 4', 'Death Knight', 'Frost', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(6, 1, 14, 1, 264, 47547, 'Phase 4', 'Death Knight', 'Frost', 'Back', 'Alliance', 'Varian''s Furor'),
(6, 1, 14, 2, 264, 47548, 'Phase 4', 'Death Knight', 'Frost', 'Back', 'Horde', 'Garrosh''s Rage'),
(6, 1, 15, 0, 264, 50737, 'Phase 4', 'Death Knight', 'Frost', 'MainHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(6, 1, 17, 0, 264, 40207, 'Phase 4', 'Death Knight', 'Frost', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 1, 0, 0, 290, 51312, 'Phase 5', 'DeathKnight', 'Frost', 'Head', 'Both', 'Sanctified Scourgelord Helmet'),
(6, 1, 1, 0, 290, 54581, 'Phase 5', 'DeathKnight', 'Frost', 'Neck', 'Both', 'Penumbra Pendant'),
(6, 1, 2, 0, 290, 51314, 'Phase 5', 'DeathKnight', 'Frost', 'Shoulders', 'Both', 'Sanctified Scourgelord Shoulderplates'),
(6, 1, 4, 0, 290, 51310, 'Phase 5', 'DeathKnight', 'Frost', 'Chest', 'Both', 'Sanctified Scourgelord Battleplate'),
(6, 1, 5, 0, 290, 50620, 'Phase 5', 'DeathKnight', 'Frost', 'Waist', 'Both', 'Coldwraith Links'),
(6, 1, 6, 0, 290, 51313, 'Phase 5', 'DeathKnight', 'Frost', 'Legs', 'Both', 'Sanctified Scourgelord Legplates'),
(6, 1, 7, 0, 290, 54578, 'Phase 5', 'DeathKnight', 'Frost', 'Feet', 'Both', 'Apocalypse''s Advance'),
(6, 1, 8, 0, 290, 50670, 'Phase 5', 'DeathKnight', 'Frost', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(6, 1, 9, 0, 290, 50690, 'Phase 5', 'DeathKnight', 'Frost', 'Hands', 'Both', 'Fleshrending Gauntlets'),
(6, 1, 10, 0, 290, 50693, 'Phase 5', 'DeathKnight', 'Frost', 'Finger1', 'Both', 'Might of Blight'),
(6, 1, 11, 0, 290, 52572, 'Phase 5', 'DeathKnight', 'Frost', 'Finger2', 'Both', 'Ashen Band of Endless Might'),
(6, 1, 12, 0, 290, 54590, 'Phase 5', 'DeathKnight', 'Frost', 'Trinket1', 'Both', 'Sharpened Twilight Scale'),
(6, 1, 13, 0, 290, 50363, 'Phase 5', 'DeathKnight', 'Frost', 'Trinket2', 'Both', 'Deathbringer''s Will'),
(6, 1, 14, 0, 290, 50677, 'Phase 5', 'DeathKnight', 'Frost', 'Back', 'Both', 'Winding Sheet'),
(6, 1, 15, 0, 290, 50737, 'Phase 5', 'DeathKnight', 'Frost', 'MainHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(6, 1, 16, 0, 290, 50737, 'Phase 5', 'DeathKnight', 'Frost', 'OffHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(6, 1, 17, 0, 290, 50459, 'Phase 5', 'DeathKnight', 'Frost', 'Ranged', 'Both', 'Sigil of the Hanged Man');

-- Unholy (tab 2)
-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 0, 200, 41386, 'Pre-Raid', 'Death Knight', 'Unholy', 'Head', 'Both', 'Spiked Titansteel Helm'),
(6, 2, 1, 0, 200, 42645, 'Pre-Raid', 'Death Knight', 'Unholy', 'Neck', 'Both', 'Titanium Impact Choker'),
(6, 2, 2, 0, 200, 37627, 'Pre-Raid', 'Death Knight', 'Unholy', 'Shoulders', 'Both', 'Snake Den Spaulders'),
(6, 2, 4, 0, 200, 39617, 'Pre-Raid', 'Death Knight', 'Unholy', 'Chest', 'Both', 'Heroes'' Scourgeborne Battleplate'),
(6, 2, 5, 0, 200, 37178, 'Pre-Raid', 'Death Knight', 'Unholy', 'Waist', 'Both', 'Strategist''s Belt'),
(6, 2, 6, 0, 200, 39620, 'Pre-Raid', 'Death Knight', 'Unholy', 'Legs', 'Both', 'Heroes'' Scourgeborne Legplates'),
(6, 2, 7, 0, 200, 43402, 'Pre-Raid', 'Death Knight', 'Unholy', 'Feet', 'Both', 'The Obliterator Greaves'),
(6, 2, 8, 0, 200, 37668, 'Pre-Raid', 'Death Knight', 'Unholy', 'Wrists', 'Both', 'Bands of the Stoneforge'),
(6, 2, 9, 0, 200, 39618, 'Pre-Raid', 'Death Knight', 'Unholy', 'Hands', 'Both', 'Heroes'' Scourgeborne Gauntlets'),
(6, 2, 10, 0, 200, 37151, 'Pre-Raid', 'Death Knight', 'Unholy', 'Finger1', 'Both', 'Band of Frosted Thorns'),
(6, 2, 12, 0, 200, 40684, 'Pre-Raid', 'Death Knight', 'Unholy', 'Trinket1', 'Both', 'Mirror of Truth'),
(6, 2, 14, 0, 200, 43566, 'Pre-Raid', 'Death Knight', 'Unholy', 'Back', 'Both', 'Ice Striker''s Cloak'),
(6, 2, 15, 0, 200, 41257, 'Pre-Raid', 'Death Knight', 'Unholy', 'MainHand', 'Both', 'Titansteel Destroyer'),
(6, 2, 17, 0, 200, 40715, 'Pre-Raid', 'Death Knight', 'Unholy', 'Ranged', 'Both', 'Sigil of Haunted Dreams');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 0, 224, 44006, 'Phase 1', 'Death Knight', 'Unholy', 'Head', 'Both', 'Obsidian Greathelm'),
(6, 2, 1, 0, 224, 44664, 'Phase 1', 'Death Knight', 'Unholy', 'Neck', 'Both', 'Favor of the Dragon Queen'),
(6, 2, 2, 0, 224, 40557, 'Phase 1', 'Death Knight', 'Unholy', 'Shoulders', 'Both', 'Valorous Scourgeborne Shoulderplates'),
(6, 2, 4, 0, 224, 40550, 'Phase 1', 'Death Knight', 'Unholy', 'Chest', 'Both', 'Valorous Scourgeborne Battleplate'),
(6, 2, 5, 0, 224, 40317, 'Phase 1', 'Death Knight', 'Unholy', 'Waist', 'Both', 'Girdle of Razuvious'),
(6, 2, 6, 0, 224, 40556, 'Phase 1', 'Death Knight', 'Unholy', 'Legs', 'Both', 'Valorous Scourgeborne Legplates'),
(6, 2, 7, 0, 224, 40206, 'Phase 1', 'Death Knight', 'Unholy', 'Feet', 'Both', 'Iron-Spring Jumpers'),
(6, 2, 8, 0, 224, 40330, 'Phase 1', 'Death Knight', 'Unholy', 'Wrists', 'Both', 'Bracers of Unrelenting Attack'),
(6, 2, 9, 0, 224, 40552, 'Phase 1', 'Death Knight', 'Unholy', 'Hands', 'Both', 'Valorous Scourgeborne Gauntlets'),
(6, 2, 10, 0, 224, 40074, 'Phase 1', 'Death Knight', 'Unholy', 'Finger1', 'Both', 'Strong-Handed Ring'),
(6, 2, 12, 0, 224, 42987, 'Phase 1', 'Death Knight', 'Unholy', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(6, 2, 14, 0, 224, 40250, 'Phase 1', 'Death Knight', 'Unholy', 'Back', 'Both', 'Aged Winter Cloak'),
(6, 2, 17, 0, 224, 40207, 'Phase 1', 'Death Knight', 'Unholy', 'Ranged', 'Both', 'Sigil of Awareness');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 1, 245, 51312, 'Phase 2', 'Death Knight', 'Unholy', 'Head', 'Alliance', 'Sanctified Scourgelord Helmet'),
(6, 2, 0, 2, 245, 45472, 'Phase 2', 'Death Knight', 'Unholy', 'Head', 'Horde', 'Warhelm of the Champion'),
(6, 2, 1, 1, 245, 54581, 'Phase 2', 'Death Knight', 'Unholy', 'Neck', 'Alliance', 'Penumbra Pendant'),
(6, 2, 1, 2, 245, 46040, 'Phase 2', 'Death Knight', 'Unholy', 'Neck', 'Horde', 'Strength of the Heavens'),
(6, 2, 2, 1, 245, 51314, 'Phase 2', 'Death Knight', 'Unholy', 'Shoulders', 'Alliance', 'Sanctified Scourgelord Shoulderplates'),
(6, 2, 2, 2, 245, 46117, 'Phase 2', 'Death Knight', 'Unholy', 'Shoulders', 'Horde', 'Conqueror''s Darkruned Shoulderplates'),
(6, 2, 4, 1, 245, 51310, 'Phase 2', 'Death Knight', 'Unholy', 'Chest', 'Alliance', 'Sanctified Scourgelord Battleplate'),
(6, 2, 4, 2, 245, 46111, 'Phase 2', 'Death Knight', 'Unholy', 'Chest', 'Horde', 'Conqueror''s Darkruned Battleplate'),
(6, 2, 5, 1, 245, 50620, 'Phase 2', 'Death Knight', 'Unholy', 'Waist', 'Alliance', 'Coldwraith Links'),
(6, 2, 5, 2, 245, 45241, 'Phase 2', 'Death Knight', 'Unholy', 'Waist', 'Horde', 'Belt of Colossal Rage'),
(6, 2, 6, 1, 245, 50624, 'Phase 2', 'Death Knight', 'Unholy', 'Legs', 'Alliance', 'Scourge Reaver''s Legplates'),
(6, 2, 6, 2, 245, 46116, 'Phase 2', 'Death Knight', 'Unholy', 'Legs', 'Horde', 'Conqueror''s Darkruned Legplates'),
(6, 2, 7, 1, 245, 54578, 'Phase 2', 'Death Knight', 'Unholy', 'Feet', 'Alliance', 'Apocalypse''s Advance'),
(6, 2, 7, 2, 245, 45599, 'Phase 2', 'Death Knight', 'Unholy', 'Feet', 'Horde', 'Sabatons of Lifeless Night'),
(6, 2, 8, 1, 245, 51842, 'Phase 2', 'Death Knight', 'Unholy', 'Wrists', 'Alliance', 'Collar of Haughty Disdain'),
(6, 2, 8, 2, 245, 45663, 'Phase 2', 'Death Knight', 'Unholy', 'Wrists', 'Horde', 'Armbands of Bedlam'),
(6, 2, 9, 1, 245, 51311, 'Phase 2', 'Death Knight', 'Unholy', 'Hands', 'Alliance', 'Sanctified Scourgelord Gauntlets'),
(6, 2, 9, 2, 245, 46113, 'Phase 2', 'Death Knight', 'Unholy', 'Hands', 'Horde', 'Conqueror''s Darkruned Gauntlets'),
(6, 2, 10, 1, 245, 50693, 'Phase 2', 'Death Knight', 'Unholy', 'Finger1', 'Alliance', 'Might of Blight'),
(6, 2, 10, 2, 245, 45469, 'Phase 2', 'Death Knight', 'Unholy', 'Finger1', 'Horde', 'Sif''s Promise'),
(6, 2, 12, 1, 245, 54590, 'Phase 2', 'Death Knight', 'Unholy', 'Trinket1', 'Alliance', 'Sharpened Twilight Scale'),
(6, 2, 12, 2, 245, 45609, 'Phase 2', 'Death Knight', 'Unholy', 'Trinket1', 'Horde', 'Comet''s Trail'),
(6, 2, 14, 1, 245, 50677, 'Phase 2', 'Death Knight', 'Unholy', 'Back', 'Alliance', 'Winding Sheet'),
(6, 2, 14, 2, 245, 46032, 'Phase 2', 'Death Knight', 'Unholy', 'Back', 'Horde', 'Drape of the Faceless General'),
(6, 2, 17, 1, 245, 50459, 'Phase 2', 'Death Knight', 'Unholy', 'Ranged', 'Alliance', 'Sigil of the Hanged Man'),
(6, 2, 17, 2, 245, 45254, 'Phase 2', 'Death Knight', 'Unholy', 'Ranged', 'Horde', 'Sigil of the Vengeful Heart');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 0, 258, 45472, 'Phase 3', 'Death Knight', 'Unholy', 'Head', 'Both', 'Warhelm of the Champion'),
(6, 2, 1, 0, 258, 46040, 'Phase 3', 'Death Knight', 'Unholy', 'Neck', 'Both', 'Strength of the Heavens'),
(6, 2, 2, 0, 258, 46117, 'Phase 3', 'Death Knight', 'Unholy', 'Shoulders', 'Both', 'Conqueror''s Darkruned Shoulderplates'),
(6, 2, 4, 0, 258, 45712, 'Phase 3', 'Death Knight', 'Unholy', 'Chest', 'Both', 'Chestplate of Titanic Fury'),
(6, 2, 5, 0, 258, 45241, 'Phase 3', 'Death Knight', 'Unholy', 'Waist', 'Both', 'Belt of Colossal Rage'),
(6, 2, 6, 0, 258, 46116, 'Phase 3', 'Death Knight', 'Unholy', 'Legs', 'Both', 'Conqueror''s Darkruned Legplates'),
(6, 2, 7, 0, 258, 45599, 'Phase 3', 'Death Knight', 'Unholy', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(6, 2, 8, 0, 258, 45663, 'Phase 3', 'Death Knight', 'Unholy', 'Wrists', 'Both', 'Armbands of Bedlam'),
(6, 2, 9, 0, 258, 46113, 'Phase 3', 'Death Knight', 'Unholy', 'Hands', 'Both', 'Conqueror''s Darkruned Gauntlets'),
(6, 2, 10, 0, 258, 45250, 'Phase 3', 'Death Knight', 'Unholy', 'Finger1', 'Both', 'Crazed Construct Ring'),
(6, 2, 12, 0, 258, 40531, 'Phase 3', 'Death Knight', 'Unholy', 'Trinket1', 'Both', 'Mark of Norgannon'),
(6, 2, 14, 0, 258, 45588, 'Phase 3', 'Death Knight', 'Unholy', 'Back', 'Both', 'Drape of the Skyborn'),
(6, 2, 17, 0, 258, 45254, 'Phase 3', 'Death Knight', 'Unholy', 'Ranged', 'Both', 'Sigil of the Vengeful Heart');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 1, 264, 48488, 'Phase 4', 'Death Knight', 'Unholy', 'Head', 'Alliance', 'Thassarian''s Helmet of Triumph'),
(6, 2, 0, 2, 264, 48493, 'Phase 4', 'Death Knight', 'Unholy', 'Head', 'Horde', 'Koltira''s Helmet of Triumph'),
(6, 2, 1, 1, 264, 47110, 'Phase 4', 'Death Knight', 'Unholy', 'Neck', 'Alliance', 'The Executioner''s Malice'),
(6, 2, 1, 2, 264, 47458, 'Phase 4', 'Death Knight', 'Unholy', 'Neck', 'Horde', 'The Executioner''s Vice'),
(6, 2, 2, 1, 264, 48486, 'Phase 4', 'Death Knight', 'Unholy', 'Shoulders', 'Alliance', 'Thassarian''s Shoulderplates of Triumph'),
(6, 2, 2, 2, 264, 48495, 'Phase 4', 'Death Knight', 'Unholy', 'Shoulders', 'Horde', 'Koltira''s Shoulderplates of Triumph'),
(6, 2, 4, 1, 264, 48490, 'Phase 4', 'Death Knight', 'Unholy', 'Chest', 'Alliance', 'Thassarian''s Battleplate of Triumph'),
(6, 2, 4, 2, 264, 48491, 'Phase 4', 'Death Knight', 'Unholy', 'Chest', 'Horde', 'Koltira''s Battleplate of Triumph'),
(6, 2, 5, 1, 264, 47002, 'Phase 4', 'Death Knight', 'Unholy', 'Waist', 'Alliance', 'Bloodbath Belt'),
(6, 2, 5, 2, 264, 47429, 'Phase 4', 'Death Knight', 'Unholy', 'Waist', 'Horde', 'Bloodbath Girdle'),
(6, 2, 6, 1, 264, 47132, 'Phase 4', 'Death Knight', 'Unholy', 'Legs', 'Alliance', 'Legguards of Ascension'),
(6, 2, 6, 2, 264, 47465, 'Phase 4', 'Death Knight', 'Unholy', 'Legs', 'Horde', 'Legplates of Ascension'),
(6, 2, 7, 0, 264, 45599, 'Phase 4', 'Death Knight', 'Unholy', 'Feet', 'Both', 'Sabatons of Lifeless Night'),
(6, 2, 8, 0, 264, 45663, 'Phase 4', 'Death Knight', 'Unholy', 'Wrists', 'Both', 'Armbands of Bedlam'),
(6, 2, 9, 1, 264, 48489, 'Phase 4', 'Death Knight', 'Unholy', 'Hands', 'Alliance', 'Thassarian''s Gauntlets of Triumph'),
(6, 2, 9, 2, 264, 48492, 'Phase 4', 'Death Knight', 'Unholy', 'Hands', 'Horde', 'Koltira''s Gauntlets of Triumph'),
(6, 2, 10, 1, 264, 47920, 'Phase 4', 'Death Knight', 'Unholy', 'Finger1', 'Alliance', 'Carnivorous Band'),
(6, 2, 10, 2, 264, 47993, 'Phase 4', 'Death Knight', 'Unholy', 'Finger1', 'Horde', 'Gormok''s Band'),
(6, 2, 12, 1, 264, 47131, 'Phase 4', 'Death Knight', 'Unholy', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(6, 2, 12, 2, 264, 47464, 'Phase 4', 'Death Knight', 'Unholy', 'Trinket1', 'Horde', 'Death''s Choice'),
(6, 2, 14, 1, 264, 47547, 'Phase 4', 'Death Knight', 'Unholy', 'Back', 'Alliance', 'Varian''s Furor'),
(6, 2, 14, 2, 264, 47548, 'Phase 4', 'Death Knight', 'Unholy', 'Back', 'Horde', 'Garrosh''s Rage'),
(6, 2, 17, 0, 264, 47673, 'Phase 4', 'Death Knight', 'Unholy', 'Ranged', 'Both', 'Sigil of Virulence');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(6, 2, 0, 0, 290, 51312, 'Phase 5', 'DeathKnight', 'Unholy', 'Head', 'Both', 'Sanctified Scourgelord Helmet'),
(6, 2, 1, 0, 290, 54581, 'Phase 5', 'DeathKnight', 'Unholy', 'Neck', 'Both', 'Penumbra Pendant'),
(6, 2, 2, 0, 290, 51314, 'Phase 5', 'DeathKnight', 'Unholy', 'Shoulders', 'Both', 'Sanctified Scourgelord Shoulderplates'),
(6, 2, 4, 0, 290, 51310, 'Phase 5', 'DeathKnight', 'Unholy', 'Chest', 'Both', 'Sanctified Scourgelord Battleplate'),
(6, 2, 5, 0, 290, 50620, 'Phase 5', 'DeathKnight', 'Unholy', 'Waist', 'Both', 'Coldwraith Links'),
(6, 2, 6, 0, 290, 51313, 'Phase 5', 'DeathKnight', 'Unholy', 'Legs', 'Both', 'Sanctified Scourgelord Legplates'),
(6, 2, 7, 0, 290, 54578, 'Phase 5', 'DeathKnight', 'Unholy', 'Feet', 'Both', 'Apocalypse''s Advance'),
(6, 2, 8, 0, 290, 50670, 'Phase 5', 'DeathKnight', 'Unholy', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(6, 2, 9, 0, 290, 50690, 'Phase 5', 'DeathKnight', 'Unholy', 'Hands', 'Both', 'Fleshrending Gauntlets'),
(6, 2, 10, 0, 290, 50693, 'Phase 5', 'DeathKnight', 'Unholy', 'Finger1', 'Both', 'Might of Blight'),
(6, 2, 11, 0, 290, 52572, 'Phase 5', 'DeathKnight', 'Unholy', 'Finger2', 'Both', 'Ashen Band of Endless Might'),
(6, 2, 12, 0, 290, 54590, 'Phase 5', 'DeathKnight', 'Unholy', 'Trinket1', 'Both', 'Sharpened Twilight Scale'),
(6, 2, 13, 0, 290, 50363, 'Phase 5', 'DeathKnight', 'Unholy', 'Trinket2', 'Both', 'Deathbringer''s Will'),
(6, 2, 14, 0, 290, 50677, 'Phase 5', 'DeathKnight', 'Unholy', 'Back', 'Both', 'Winding Sheet'),
(6, 2, 15, 0, 290, 49623, 'Phase 5', 'DeathKnight', 'Unholy', 'MainHand', 'Both', 'Shadowmourne'),
(6, 2, 17, 0, 290, 50459, 'Phase 5', 'DeathKnight', 'Unholy', 'Ranged', 'Both', 'Sigil of the Hanged Man');


-- ============================================================
-- Shaman (7)
-- ============================================================
-- Elemental (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 66, 10504, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Head', 'Both', 'Green Lens'),
(7, 0, 1, 0, 66, 12103, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Neck', 'Both', 'Star of Mystaria'),
(7, 0, 2, 0, 66, 13013, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Elder Wizard''s Mantle'),
(7, 0, 4, 0, 66, 11924, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robes of the Royal Crown'),
(7, 0, 5, 0, 66, 11662, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Waist', 'Both', 'Ban''thok Sash'),
(7, 0, 6, 0, 66, 13170, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Legs', 'Both', 'Skyshroud Leggings'),
(7, 0, 7, 0, 66, 13954, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Feet', 'Both', 'Verdant Footpads'),
(7, 0, 8, 0, 66, 11765, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Pyremail Wristguards'),
(7, 0, 9, 0, 66, 13253, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Hands', 'Both', 'Hands of Power'),
(7, 0, 10, 0, 66, 12545, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Eye of Orgrimmar'),
(7, 0, 11, 0, 66, 13001, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Maiden''s Circle'),
(7, 0, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Briarwood Reed'),
(7, 0, 13, 0, 66, 13968, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Eye of the Beast'),
(7, 0, 14, 0, 66, 15421, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'Back', 'Both', 'Shroud of the Exile'),
(7, 0, 15, 0, 66, 13964, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Witchblade'),
(7, 0, 16, 0, 66, 11904, 'Phase 1 (Pre-Raid)', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Spirit of Aquementas');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 76, 10504, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Head', 'Both', 'Green Lens'),
(7, 0, 1, 0, 76, 12103, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Neck', 'Both', 'Star of Mystaria'),
(7, 0, 2, 0, 76, 23260, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Champion''s Mail Pauldrons'),
(7, 0, 4, 0, 76, 18385, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robe of Everlasting Night'),
(7, 0, 5, 0, 76, 11662, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Waist', 'Both', 'Ban''thok Sash'),
(7, 0, 6, 0, 76, 13170, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Legs', 'Both', 'Skyshroud Leggings'),
(7, 0, 7, 0, 76, 18322, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Feet', 'Both', 'Waterspout Boots'),
(7, 0, 8, 0, 76, 11765, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Pyremail Wristguards'),
(7, 0, 9, 0, 76, 13253, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Hands', 'Both', 'Hands of Power'),
(7, 0, 10, 0, 76, 12545, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Eye of Orgrimmar'),
(7, 0, 11, 0, 76, 13001, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Maiden''s Circle'),
(7, 0, 12, 0, 76, 12930, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Briarwood Reed'),
(7, 0, 13, 0, 76, 13968, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Eye of the Beast'),
(7, 0, 14, 0, 76, 18496, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'Back', 'Both', 'Heliotrope Cloak'),
(7, 0, 15, 0, 76, 18534, 'Phase 2 (Pre-Raid)', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Rod of the Ogre Magi');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 78, 10504, 'Phase 2', 'Shaman', 'Elemental', 'Head', 'Both', 'Green Lens'),
(7, 0, 1, 0, 78, 18814, 'Phase 2', 'Shaman', 'Elemental', 'Neck', 'Both', 'Choker of the Fire Lord'),
(7, 0, 2, 0, 78, 18829, 'Phase 2', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Deep Earth Spaulders'),
(7, 0, 4, 0, 78, 19145, 'Phase 2', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robe of Volatile Power'),
(7, 0, 5, 0, 78, 19136, 'Phase 2', 'Shaman', 'Elemental', 'Waist', 'Both', 'Mana Igniting Cord'),
(7, 0, 6, 0, 78, 16946, 'Phase 2', 'Shaman', 'Elemental', 'Legs', 'Both', 'Legplates of Ten Storms'),
(7, 0, 7, 0, 78, 18322, 'Phase 2', 'Shaman', 'Elemental', 'Feet', 'Both', 'Waterspout Boots'),
(7, 0, 8, 0, 78, 11765, 'Phase 2', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Pyremail Wristguards'),
(7, 0, 9, 0, 78, 16839, 'Phase 2', 'Shaman', 'Elemental', 'Hands', 'Both', 'Earthfury Gauntlets'),
(7, 0, 10, 0, 78, 19147, 'Phase 2', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Spell Power'),
(7, 0, 11, 0, 78, 19147, 'Phase 2', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of Spell Power'),
(7, 0, 12, 0, 78, 12930, 'Phase 2', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Briarwood Reed'),
(7, 0, 13, 0, 78, 18820, 'Phase 2', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(7, 0, 14, 0, 78, 18496, 'Phase 2', 'Shaman', 'Elemental', 'Back', 'Both', 'Heliotrope Cloak'),
(7, 0, 15, 0, 78, 18842, 'Phase 2', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Staff of Dominance');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 83, 19375, 'Phase 3', 'Shaman', 'Elemental', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(7, 0, 1, 0, 83, 18814, 'Phase 3', 'Shaman', 'Elemental', 'Neck', 'Both', 'Choker of the Fire Lord'),
(7, 0, 2, 0, 83, 18829, 'Phase 3', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Deep Earth Spaulders'),
(7, 0, 4, 0, 83, 19145, 'Phase 3', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robe of Volatile Power'),
(7, 0, 5, 0, 83, 19400, 'Phase 3', 'Shaman', 'Elemental', 'Waist', 'Both', 'Firemaw''s Clutch'),
(7, 0, 6, 0, 83, 16946, 'Phase 3', 'Shaman', 'Elemental', 'Legs', 'Both', 'Legplates of Ten Storms'),
(7, 0, 7, 0, 83, 18322, 'Phase 3', 'Shaman', 'Elemental', 'Feet', 'Both', 'Waterspout Boots'),
(7, 0, 8, 0, 83, 19374, 'Phase 3', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(7, 0, 9, 0, 83, 16839, 'Phase 3', 'Shaman', 'Elemental', 'Hands', 'Both', 'Earthfury Gauntlets'),
(7, 0, 10, 0, 83, 19397, 'Phase 3', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Blackrock'),
(7, 0, 12, 0, 83, 19379, 'Phase 3', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(7, 0, 13, 0, 83, 19344, 'Phase 3', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Natural Alignment Crystal'),
(7, 0, 14, 0, 83, 19378, 'Phase 3', 'Shaman', 'Elemental', 'Back', 'Both', 'Cloak of the Brood Lord'),
(7, 0, 15, 0, 83, 19360, 'Phase 3', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(7, 0, 16, 0, 83, 19366, 'Phase 3', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Master Dragonslayer''s Orb');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 88, 19375, 'Phase 5', 'Shaman', 'Elemental', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(7, 0, 1, 0, 88, 21608, 'Phase 5', 'Shaman', 'Elemental', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(7, 0, 2, 0, 88, 21376, 'Phase 5', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Stormcaller''s Pauldrons'),
(7, 0, 4, 0, 88, 21671, 'Phase 5', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robes of the Battleguard'),
(7, 0, 5, 0, 88, 22730, 'Phase 5', 'Shaman', 'Elemental', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(7, 0, 6, 0, 88, 21375, 'Phase 5', 'Shaman', 'Elemental', 'Legs', 'Both', 'Stormcaller''s Leggings'),
(7, 0, 7, 0, 88, 21373, 'Phase 5', 'Shaman', 'Elemental', 'Feet', 'Both', 'Stormcaller''s Footguards'),
(7, 0, 8, 0, 88, 21464, 'Phase 5', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Shackles of the Unscarred'),
(7, 0, 9, 0, 88, 21585, 'Phase 5', 'Shaman', 'Elemental', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(7, 0, 10, 0, 88, 21707, 'Phase 5', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Swarming Thought'),
(7, 0, 11, 0, 88, 21709, 'Phase 5', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of the Fallen God'),
(7, 0, 12, 0, 88, 19379, 'Phase 5', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(7, 0, 13, 0, 88, 19344, 'Phase 5', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Natural Alignment Crystal'),
(7, 0, 14, 0, 88, 22731, 'Phase 5', 'Shaman', 'Elemental', 'Back', 'Both', 'Cloak of the Devoured'),
(7, 0, 15, 0, 88, 19360, 'Phase 5', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(7, 0, 16, 0, 88, 19366, 'Phase 5', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Master Dragonslayer''s Orb'),
(7, 0, 17, 0, 88, 23199, 'Phase 5', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Storm');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 92, 19375, 'Phase 6', 'Shaman', 'Elemental', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(7, 0, 1, 0, 92, 22943, 'Phase 6', 'Shaman', 'Elemental', 'Neck', 'Both', 'Malice Stone Pendant'),
(7, 0, 2, 0, 92, 21376, 'Phase 6', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Stormcaller''s Pauldrons'),
(7, 0, 4, 0, 92, 21671, 'Phase 6', 'Shaman', 'Elemental', 'Chest', 'Both', 'Robes of the Battleguard'),
(7, 0, 5, 0, 92, 22730, 'Phase 6', 'Shaman', 'Elemental', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(7, 0, 6, 0, 92, 21375, 'Phase 6', 'Shaman', 'Elemental', 'Legs', 'Both', 'Stormcaller''s Leggings'),
(7, 0, 7, 0, 92, 21373, 'Phase 6', 'Shaman', 'Elemental', 'Feet', 'Both', 'Stormcaller''s Footguards'),
(7, 0, 8, 0, 92, 21464, 'Phase 6', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Shackles of the Unscarred'),
(7, 0, 9, 0, 92, 21585, 'Phase 6', 'Shaman', 'Elemental', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(7, 0, 10, 0, 92, 21707, 'Phase 6', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Swarming Thought'),
(7, 0, 11, 0, 92, 21709, 'Phase 6', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of the Fallen God'),
(7, 0, 12, 0, 92, 19379, 'Phase 6', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(7, 0, 13, 0, 92, 23046, 'Phase 6', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(7, 0, 14, 0, 92, 23050, 'Phase 6', 'Shaman', 'Elemental', 'Back', 'Both', 'Cloak of the Necropolis'),
(7, 0, 15, 0, 92, 22988, 'Phase 6', 'Shaman', 'Elemental', 'MainHand', 'Both', 'The End of Dreams'),
(7, 0, 16, 0, 92, 23049, 'Phase 6', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(7, 0, 17, 0, 92, 23199, 'Phase 6', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Storm');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 120, 32086, 'Pre-Raid', 'Shaman', 'Elemental', 'Head', 'Both', 'Storm Master''s Helmet'),
(7, 0, 1, 0, 120, 31692, 'Pre-Raid', 'Shaman', 'Elemental', 'Neck', 'Both', 'Natasha''s Ember Necklace'),
(7, 0, 2, 0, 120, 27796, 'Pre-Raid', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Mana-Etched Spaulders'),
(7, 0, 4, 0, 120, 29519, 'Pre-Raid', 'Shaman', 'Elemental', 'Chest', 'Both', 'Netherstrike Breastplate'),
(7, 0, 5, 0, 120, 29520, 'Pre-Raid', 'Shaman', 'Elemental', 'Waist', 'Both', 'Netherstrike Belt'),
(7, 0, 6, 0, 120, 24262, 'Pre-Raid', 'Shaman', 'Elemental', 'Legs', 'Both', 'Spellstrike Pants'),
(7, 0, 7, 0, 120, 28406, 'Pre-Raid', 'Shaman', 'Elemental', 'Feet', 'Both', 'Sigil-Laced Boots'),
(7, 0, 8, 0, 120, 29521, 'Pre-Raid', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Netherstrike Bracers'),
(7, 0, 9, 0, 120, 27465, 'Pre-Raid', 'Shaman', 'Elemental', 'Hands', 'Both', 'Mana-Etched Gloves'),
(7, 0, 10, 0, 120, 29367, 'Pre-Raid', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Cryptic Dreams'),
(7, 0, 11, 0, 120, 29126, 'Pre-Raid', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Seer''s Signet'),
(7, 0, 12, 0, 120, 29370, 'Pre-Raid', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(7, 0, 13, 0, 120, 27683, 'Pre-Raid', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(7, 0, 14, 0, 120, 29369, 'Pre-Raid', 'Shaman', 'Elemental', 'Back', 'Both', 'Shawl of Shifting Probabilities'),
(7, 0, 15, 0, 120, 23554, 'Pre-Raid', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Eternium Runed Blade'),
(7, 0, 16, 0, 120, 29268, 'Pre-Raid', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Mazthoril Honor Shield'),
(7, 0, 17, 0, 120, 28248, 'Pre-Raid', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Void');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 125, 29035, 'Phase 1', 'Shaman', 'Elemental', 'Head', 'Both', 'Cyclone Faceguard'),
(7, 0, 1, 0, 125, 28762, 'Phase 1', 'Shaman', 'Elemental', 'Neck', 'Both', 'Adornment of Stolen Souls'),
(7, 0, 2, 0, 125, 29037, 'Phase 1', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Cyclone Shoulderguards'),
(7, 0, 4, 0, 125, 29519, 'Phase 1', 'Shaman', 'Elemental', 'Chest', 'Both', 'Netherstrike Breastplate'),
(7, 0, 5, 0, 125, 29520, 'Phase 1', 'Shaman', 'Elemental', 'Waist', 'Both', 'Netherstrike Belt'),
(7, 0, 6, 0, 125, 30734, 'Phase 1', 'Shaman', 'Elemental', 'Legs', 'Both', 'Leggings of the Seventh Circle'),
(7, 0, 7, 0, 125, 28517, 'Phase 1', 'Shaman', 'Elemental', 'Feet', 'Both', 'Boots of Foretelling'),
(7, 0, 8, 0, 125, 29521, 'Phase 1', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Netherstrike Bracers'),
(7, 0, 9, 0, 125, 30725, 'Phase 1', 'Shaman', 'Elemental', 'Hands', 'Both', 'Anger-Spark Gloves'),
(7, 0, 10, 0, 125, 28753, 'Phase 1', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Recurrence'),
(7, 0, 11, 0, 125, 30667, 'Phase 1', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of Unrelenting Storms'),
(7, 0, 12, 0, 125, 29370, 'Phase 1', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(7, 0, 13, 0, 125, 28785, 'Phase 1', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'The Lightning Capacitor'),
(7, 0, 14, 0, 125, 30735, 'Phase 1', 'Shaman', 'Elemental', 'Back', 'Both', 'Ancient Spellcloak of the Highborne'),
(7, 0, 16, 0, 125, 29268, 'Phase 1', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Mazthoril Honor Shield'),
(7, 0, 17, 0, 125, 28248, 'Phase 1', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Void');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 141, 29035, 'Phase 2', 'Shaman', 'Elemental', 'Head', 'Both', 'Cyclone Faceguard'),
(7, 0, 1, 0, 141, 30015, 'Phase 2', 'Shaman', 'Elemental', 'Neck', 'Both', 'The Sun King''s Talisman'),
(7, 0, 2, 0, 141, 29037, 'Phase 2', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Cyclone Shoulderguards'),
(7, 0, 4, 0, 141, 30107, 'Phase 2', 'Shaman', 'Elemental', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(7, 0, 5, 0, 141, 30038, 'Phase 2', 'Shaman', 'Elemental', 'Waist', 'Both', 'Belt of Blasting'),
(7, 0, 6, 0, 141, 30734, 'Phase 2', 'Shaman', 'Elemental', 'Legs', 'Both', 'Leggings of the Seventh Circle'),
(7, 0, 7, 0, 141, 30067, 'Phase 2', 'Shaman', 'Elemental', 'Feet', 'Both', 'Velvet Boots of the Guardian'),
(7, 0, 8, 0, 141, 29918, 'Phase 2', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(7, 0, 9, 0, 141, 28780, 'Phase 2', 'Shaman', 'Elemental', 'Hands', 'Both', 'Soul-Eater''s Handwraps'),
(7, 0, 10, 0, 141, 30109, 'Phase 2', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Endless Coils'),
(7, 0, 11, 0, 141, 30667, 'Phase 2', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of Unrelenting Storms'),
(7, 0, 12, 0, 141, 29370, 'Phase 2', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(7, 0, 13, 0, 141, 28785, 'Phase 2', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'The Lightning Capacitor'),
(7, 0, 14, 0, 141, 30735, 'Phase 2', 'Shaman', 'Elemental', 'Back', 'Both', 'Ancient Spellcloak of the Highborne'),
(7, 0, 15, 0, 141, 29988, 'Phase 2', 'Shaman', 'Elemental', 'MainHand', 'Both', 'The Nexus Key'),
(7, 0, 17, 0, 141, 28248, 'Phase 2', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Void');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 156, 31014, 'Phase 3', 'Shaman', 'Elemental', 'Head', 'Both', 'Skyshatter Headguard'),
(7, 0, 1, 0, 156, 32349, 'Phase 3', 'Shaman', 'Elemental', 'Neck', 'Both', 'Translucent Spellthread Necklace'),
(7, 0, 2, 0, 156, 31023, 'Phase 3', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Skyshatter Mantle'),
(7, 0, 4, 0, 156, 31017, 'Phase 3', 'Shaman', 'Elemental', 'Chest', 'Both', 'Skyshatter Breastplate'),
(7, 0, 5, 0, 156, 32276, 'Phase 3', 'Shaman', 'Elemental', 'Waist', 'Both', 'Flashfire Girdle'),
(7, 0, 6, 0, 156, 30916, 'Phase 3', 'Shaman', 'Elemental', 'Legs', 'Both', 'Leggings of Channeled Elements'),
(7, 0, 7, 0, 156, 32239, 'Phase 3', 'Shaman', 'Elemental', 'Feet', 'Both', 'Slippers of the Seacaller'),
(7, 0, 8, 0, 156, 32586, 'Phase 3', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(7, 0, 9, 0, 156, 31008, 'Phase 3', 'Shaman', 'Elemental', 'Hands', 'Both', 'Skyshatter Gauntlets'),
(7, 0, 10, 0, 156, 32527, 'Phase 3', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(7, 0, 11, 0, 156, 32527, 'Phase 3', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(7, 0, 12, 0, 156, 28785, 'Phase 3', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'The Lightning Capacitor'),
(7, 0, 13, 0, 156, 32483, 'Phase 3', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(7, 0, 14, 0, 156, 32524, 'Phase 3', 'Shaman', 'Elemental', 'Back', 'Both', 'Shroud of the Highborne'),
(7, 0, 15, 0, 156, 32374, 'Phase 3', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(7, 0, 17, 0, 156, 32330, 'Phase 3', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of Ancestral Guidance');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 164, 31014, 'Phase 4', 'Shaman', 'Elemental', 'Head', 'Both', 'Skyshatter Headguard'),
(7, 0, 1, 0, 164, 33281, 'Phase 4', 'Shaman', 'Elemental', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(7, 0, 2, 0, 164, 31023, 'Phase 4', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Skyshatter Mantle'),
(7, 0, 4, 0, 164, 31017, 'Phase 4', 'Shaman', 'Elemental', 'Chest', 'Both', 'Skyshatter Breastplate'),
(7, 0, 5, 0, 164, 32276, 'Phase 4', 'Shaman', 'Elemental', 'Waist', 'Both', 'Flashfire Girdle'),
(7, 0, 6, 0, 164, 33584, 'Phase 4', 'Shaman', 'Elemental', 'Legs', 'Both', 'Pantaloons of Arcane Annihilation'),
(7, 0, 7, 0, 164, 33357, 'Phase 4', 'Shaman', 'Elemental', 'Feet', 'Both', 'Footpads of Madness'),
(7, 0, 8, 0, 164, 32586, 'Phase 4', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(7, 0, 9, 0, 164, 31008, 'Phase 4', 'Shaman', 'Elemental', 'Hands', 'Both', 'Skyshatter Gauntlets'),
(7, 0, 10, 0, 164, 32527, 'Phase 4', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(7, 0, 11, 0, 164, 32527, 'Phase 4', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(7, 0, 12, 0, 164, 33829, 'Phase 4', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Hex Shrunken Head'),
(7, 0, 13, 0, 164, 32483, 'Phase 4', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(7, 0, 14, 0, 164, 32524, 'Phase 4', 'Shaman', 'Elemental', 'Back', 'Both', 'Shroud of the Highborne'),
(7, 0, 15, 0, 164, 32374, 'Phase 4', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(7, 0, 16, 0, 164, 30909, 'Phase 5', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Antonidas''s Aegis of Rapt Concentration'),
(7, 0, 17, 0, 164, 32330, 'Phase 4', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of Ancestral Guidance');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 200, 42555, 'Pre-Raid', 'Shaman', 'Elemental', 'Head', 'Both', 'Electroflux Sight Enhancers'),
(7, 0, 1, 0, 200, 37595, 'Pre-Raid', 'Shaman', 'Elemental', 'Neck', 'Both', 'Necklace of Taldaram'),
(7, 0, 2, 0, 200, 37398, 'Pre-Raid', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Mantle of Discarded Ways'),
(7, 0, 4, 0, 200, 42101, 'Pre-Raid', 'Shaman', 'Elemental', 'Chest', 'Both', 'Ebonweave Robe'),
(7, 0, 5, 0, 200, 37855, 'Pre-Raid', 'Shaman', 'Elemental', 'Waist', 'Both', 'Mail Girdle of the Audient Earth'),
(7, 0, 6, 0, 200, 37695, 'Pre-Raid', 'Shaman', 'Elemental', 'Legs', 'Both', 'Legguards of Nature''s Power'),
(7, 0, 7, 0, 200, 43469, 'Pre-Raid', 'Shaman', 'Elemental', 'Feet', 'Both', 'Revenant''s Treads'),
(7, 0, 8, 0, 200, 37788, 'Pre-Raid', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Limb Regeneration Bracers'),
(7, 0, 9, 0, 200, 37623, 'Pre-Raid', 'Shaman', 'Elemental', 'Hands', 'Both', 'Fiery Obelisk Handguards'),
(7, 0, 10, 0, 200, 37192, 'Pre-Raid', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Annhylde''s Ring'),
(7, 0, 12, 0, 200, 40682, 'Pre-Raid', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Sundial of the Exiled'),
(7, 0, 14, 0, 200, 41610, 'Pre-Raid', 'Shaman', 'Elemental', 'Back', 'Both', 'Deathchill Cloak'),
(7, 0, 15, 0, 200, 41384, 'Pre-Raid', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Titansteel Guardian'),
(7, 0, 17, 0, 200, 40708, 'Pre-Raid', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of the Elemental Plane');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 224, 40516, 'Phase 1', 'Shaman', 'Elemental', 'Head', 'Both', 'Valorous Earthshatter Helm'),
(7, 0, 1, 0, 224, 44661, 'Phase 1', 'Shaman', 'Elemental', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(7, 0, 2, 0, 224, 40518, 'Phase 1', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Valorous Earthshatter Shoulderpads'),
(7, 0, 4, 0, 224, 40283, 'Phase 1', 'Shaman', 'Elemental', 'Chest', 'Both', 'Fallout Impervious Tunic'),
(7, 0, 5, 0, 224, 40327, 'Phase 1', 'Shaman', 'Elemental', 'Waist', 'Both', 'Girdle of Recuperation'),
(7, 0, 6, 0, 224, 40560, 'Phase 1', 'Shaman', 'Elemental', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(7, 0, 7, 0, 224, 40237, 'Phase 1', 'Shaman', 'Elemental', 'Feet', 'Both', 'Eruption-Scarred Boots'),
(7, 0, 8, 0, 224, 40324, 'Phase 1', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bands of Mutual Respect'),
(7, 0, 9, 0, 224, 40302, 'Phase 1', 'Shaman', 'Elemental', 'Hands', 'Both', 'Benefactor''s Gauntlets'),
(7, 0, 10, 0, 224, 40399, 'Phase 1', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(7, 0, 12, 0, 224, 40255, 'Phase 1', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Dying Curse'),
(7, 0, 14, 0, 224, 44005, 'Phase 1', 'Shaman', 'Elemental', 'Back', 'Both', 'Pennant Cloak'),
(7, 0, 17, 0, 224, 40267, 'Phase 1', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of Hex');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 245, 46209, 'Phase 2', 'Shaman', 'Elemental', 'Head', 'Both', 'Conqueror''s Worldbreaker Helm'),
(7, 0, 2, 0, 245, 46211, 'Phase 2', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Conqueror''s Worldbreaker Shoulderpads'),
(7, 0, 4, 0, 245, 46206, 'Phase 2', 'Shaman', 'Elemental', 'Chest', 'Both', 'Conqueror''s Worldbreaker Hauberk'),
(7, 0, 6, 0, 245, 46210, 'Phase 2', 'Shaman', 'Elemental', 'Legs', 'Both', 'Conqueror''s Worldbreaker Kilt'),
(7, 0, 7, 0, 245, 45537, 'Phase 2', 'Shaman', 'Elemental', 'Feet', 'Both', 'Treads of the False Oracle'),
(7, 0, 8, 1, 245, 45616, 'Phase 2', 'Shaman', 'Elemental', 'Wrists', 'Alliance', 'Star-beaded Clutch'),
(7, 0, 8, 2, 245, 45460, 'Phase 2', 'Shaman', 'Elemental', 'Wrists', 'Horde', 'Bindings of Winter Gale'),
(7, 0, 9, 0, 245, 45665, 'Phase 2', 'Shaman', 'Elemental', 'Hands', 'Both', 'Pharos Gloves'),
(7, 0, 10, 0, 245, 45495, 'Phase 2', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Conductive Seal'),
(7, 0, 12, 0, 245, 45518, 'Phase 2', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Flare of the Heavens'),
(7, 0, 14, 0, 245, 45242, 'Phase 2', 'Shaman', 'Elemental', 'Back', 'Both', 'Drape of Mortal Downfall'),
(7, 0, 17, 0, 245, 40267, 'Phase 2', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Totem of Hex');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 264, 51237, 'Phase 4', 'Shaman', 'Elemental', 'Head', 'Both', 'Sanctified Frost Witch''s Helm'),
(7, 0, 2, 0, 264, 50698, 'Phase 4', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Horrific Flesh Epaulets'),
(7, 0, 4, 0, 264, 51239, 'Phase 4', 'Shaman', 'Elemental', 'Chest', 'Both', 'Sanctified Frost Witch''s Hauberk'),
(7, 0, 5, 0, 264, 50613, 'Phase 4', 'Shaman', 'Elemental', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(7, 0, 6, 0, 264, 51236, 'Phase 4', 'Shaman', 'Elemental', 'Legs', 'Both', 'Sanctified Frost Witch''s Kilt'),
(7, 0, 7, 0, 264, 50699, 'Phase 4', 'Shaman', 'Elemental', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(7, 0, 8, 0, 264, 50687, 'Phase 4', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bloodsunder''s Bracers'),
(7, 0, 9, 0, 264, 51238, 'Phase 4', 'Shaman', 'Elemental', 'Hands', 'Both', 'Sanctified Frost Witch''s Gloves'),
(7, 0, 10, 0, 264, 50664, 'Phase 4', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Rapid Ascent'),
(7, 0, 12, 0, 264, 50348, 'Phase 4', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Dislodged Foreign Object'),
(7, 0, 14, 0, 264, 50628, 'Phase 4', 'Shaman', 'Elemental', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(7, 0, 15, 0, 264, 50734, 'Phase 4', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(7, 0, 17, 0, 264, 50458, 'Phase 4', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Bizuri''s Totem of Shattered Ice');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 0, 0, 0, 290, 51237, 'Phase 5', 'Shaman', 'Elemental', 'Head', 'Both', 'Sanctified Frost Witch''s Helm'),
(7, 0, 1, 0, 290, 50182, 'Phase 5', 'Shaman', 'Elemental', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(7, 0, 2, 0, 290, 51235, 'Phase 5', 'Shaman', 'Elemental', 'Shoulders', 'Both', 'Sanctified Frost Witch''s Shoulderpads'),
(7, 0, 4, 0, 290, 51239, 'Phase 5', 'Shaman', 'Elemental', 'Chest', 'Both', 'Sanctified Frost Witch''s Hauberk'),
(7, 0, 5, 0, 290, 54587, 'Phase 5', 'Shaman', 'Elemental', 'Waist', 'Both', 'Split Shape Belt'),
(7, 0, 6, 0, 290, 50694, 'Phase 5', 'Shaman', 'Elemental', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(7, 0, 7, 0, 290, 50699, 'Phase 5', 'Shaman', 'Elemental', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(7, 0, 8, 0, 290, 54582, 'Phase 5', 'Shaman', 'Elemental', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(7, 0, 9, 0, 290, 51238, 'Phase 5', 'Shaman', 'Elemental', 'Hands', 'Both', 'Sanctified Frost Witch''s Gloves'),
(7, 0, 10, 0, 290, 50664, 'Phase 5', 'Shaman', 'Elemental', 'Finger1', 'Both', 'Ring of Rapid Ascent'),
(7, 0, 11, 0, 290, 50398, 'Phase 5', 'Shaman', 'Elemental', 'Finger2', 'Both', 'Ashen Band of Endless Destruction'),
(7, 0, 12, 0, 290, 54588, 'Phase 5', 'Shaman', 'Elemental', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(7, 0, 13, 0, 290, 50365, 'Phase 5', 'Shaman', 'Elemental', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(7, 0, 14, 0, 290, 54583, 'Phase 5', 'Shaman', 'Elemental', 'Back', 'Both', 'Cloak of Burning Dusk'),
(7, 0, 15, 0, 290, 50734, 'Phase 5', 'Shaman', 'Elemental', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(7, 0, 16, 0, 290, 50616, 'Phase 5', 'Shaman', 'Elemental', 'OffHand', 'Both', 'Bulwark of Smouldering Steel'),
(7, 0, 17, 0, 290, 50458, 'Phase 5', 'Shaman', 'Elemental', 'Ranged', 'Both', 'Bizuri''s Totem of Shattered Ice');

-- Enhancement (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 66, 12587, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Head', 'Both', 'Eye of Rend'),
(7, 1, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Mark of Fordring'),
(7, 1, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(7, 1, 4, 0, 66, 11726, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Savage Gladiator Chain'),
(7, 1, 5, 0, 66, 22232, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Marksman''s Girdle'),
(7, 1, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Devilsaur Leggings'),
(7, 1, 7, 0, 66, 14616, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Bloodmail Boots'),
(7, 1, 8, 0, 66, 13211, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Slashclaw Bracers'),
(7, 1, 9, 0, 66, 13957, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Gargoyle Slashers'),
(7, 1, 10, 0, 66, 13098, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Painweaver Band'),
(7, 1, 11, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Blackstone Ring'),
(7, 1, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(7, 1, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Hand of Justice'),
(7, 1, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'Back', 'Both', 'Cape of the Black Baron'),
(7, 1, 15, 0, 66, 12784, 'Phase 1 (Pre-Raid)', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Arcanite Reaper');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 76, 12587, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Head', 'Both', 'Eye of Rend'),
(7, 1, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Mark of Fordring'),
(7, 1, 2, 0, 76, 12927, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(7, 1, 4, 0, 76, 11726, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Savage Gladiator Chain'),
(7, 1, 5, 0, 76, 22232, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Marksman''s Girdle'),
(7, 1, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Devilsaur Leggings'),
(7, 1, 7, 0, 76, 14616, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Bloodmail Boots'),
(7, 1, 8, 0, 76, 13211, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Slashclaw Bracers'),
(7, 1, 9, 0, 76, 13957, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Gargoyle Slashers'),
(7, 1, 10, 0, 76, 13098, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Painweaver Band'),
(7, 1, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(7, 1, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(7, 1, 13, 0, 76, 11815, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Hand of Justice'),
(7, 1, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'Back', 'Both', 'Cape of the Black Baron'),
(7, 1, 15, 0, 76, 12784, 'Phase 2 (Pre-Raid)', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Arcanite Reaper');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 78, 18817, 'Phase 2', 'Shaman', 'Enhancement', 'Head', 'Both', 'Crown of Destruction'),
(7, 1, 1, 0, 78, 18404, 'Phase 2', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(7, 1, 2, 0, 78, 12927, 'Phase 2', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(7, 1, 4, 0, 78, 11726, 'Phase 2', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Savage Gladiator Chain'),
(7, 1, 5, 0, 78, 18393, 'Phase 2', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Warpwood Binding'),
(7, 1, 6, 0, 78, 15062, 'Phase 2', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Devilsaur Leggings'),
(7, 1, 7, 0, 78, 14616, 'Phase 2', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Bloodmail Boots'),
(7, 1, 8, 0, 78, 19146, 'Phase 2', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Wristguards of Stability'),
(7, 1, 9, 0, 78, 13957, 'Phase 2', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Gargoyle Slashers'),
(7, 1, 10, 0, 78, 17063, 'Phase 2', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Band of Accuria'),
(7, 1, 11, 0, 78, 18821, 'Phase 2', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Quick Strike Ring'),
(7, 1, 12, 0, 78, 13965, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(7, 1, 13, 0, 78, 11815, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Hand of Justice'),
(7, 1, 14, 0, 78, 13340, 'Phase 2', 'Shaman', 'Enhancement', 'Back', 'Both', 'Cape of the Black Baron'),
(7, 1, 15, 0, 78, 17104, 'Phase 2', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Spinal Reaper');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 83, 18817, 'Phase 3', 'Shaman', 'Enhancement', 'Head', 'Both', 'Crown of Destruction'),
(7, 1, 1, 0, 83, 18404, 'Phase 3', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(7, 1, 2, 0, 83, 12927, 'Phase 3', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(7, 1, 4, 0, 83, 11726, 'Phase 3', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Savage Gladiator Chain'),
(7, 1, 5, 0, 83, 19380, 'Phase 3', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Therazane''s Link'),
(7, 1, 6, 0, 83, 15062, 'Phase 3', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Devilsaur Leggings'),
(7, 1, 7, 0, 83, 19381, 'Phase 3', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Boots of the Shadow Flame'),
(7, 1, 8, 0, 83, 19146, 'Phase 3', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Wristguards of Stability'),
(7, 1, 9, 0, 83, 19157, 'Phase 3', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Chromatic Gauntlets'),
(7, 1, 10, 0, 83, 19384, 'Phase 3', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Master Dragonslayer''s Ring'),
(7, 1, 11, 0, 83, 18821, 'Phase 3', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Quick Strike Ring'),
(7, 1, 12, 0, 83, 13965, 'Phase 3', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(7, 1, 13, 0, 83, 11815, 'Phase 3', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Hand of Justice'),
(7, 1, 14, 0, 83, 19436, 'Phase 3', 'Shaman', 'Enhancement', 'Back', 'Both', 'Cloak of Draconic Might'),
(7, 1, 15, 0, 83, 17104, 'Phase 3', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Spinal Reaper');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 88, 18817, 'Phase 5', 'Shaman', 'Enhancement', 'Head', 'Both', 'Crown of Destruction'),
(7, 1, 1, 0, 88, 18404, 'Phase 5', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(7, 1, 2, 0, 88, 21684, 'Phase 5', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Mantle of the Desert''s Fury'),
(7, 1, 4, 0, 88, 21374, 'Phase 5', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Stormcaller''s Hauberk'),
(7, 1, 5, 0, 88, 21586, 'Phase 5', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(7, 1, 6, 0, 88, 21651, 'Phase 5', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Scaled Sand Reaver Leggings'),
(7, 1, 7, 0, 88, 21705, 'Phase 5', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Boots of the Fallen Prophet'),
(7, 1, 8, 0, 88, 21602, 'Phase 5', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Qiraji Execution Bracers'),
(7, 1, 9, 0, 88, 21624, 'Phase 5', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Gauntlets of Kalimdor'),
(7, 1, 10, 0, 88, 17063, 'Phase 5', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Band of Accuria'),
(7, 1, 11, 0, 88, 18821, 'Phase 5', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Quick Strike Ring'),
(7, 1, 12, 0, 88, 22321, 'Phase 5', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Heart of Wyrmthalak'),
(7, 1, 13, 0, 88, 19289, 'Phase 5', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Darkmoon Card: Maelstrom'),
(7, 1, 14, 0, 88, 21701, 'Phase 5', 'Shaman', 'Enhancement', 'Back', 'Both', 'Cloak of Concentrated Hatred'),
(7, 1, 15, 0, 88, 21134, 'Phase 5', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Dark Edge of Insanity'),
(7, 1, 17, 0, 88, 22395, 'Phase 5', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of Rage');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 92, 18817, 'Phase 6', 'Shaman', 'Enhancement', 'Head', 'Both', 'Crown of Destruction'),
(7, 1, 1, 0, 92, 18404, 'Phase 6', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(7, 1, 2, 0, 92, 21684, 'Phase 6', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Mantle of the Desert''s Fury'),
(7, 1, 4, 0, 92, 21374, 'Phase 6', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Stormcaller''s Hauberk'),
(7, 1, 5, 0, 92, 21586, 'Phase 6', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(7, 1, 6, 0, 92, 21651, 'Phase 6', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Scaled Sand Reaver Leggings'),
(7, 1, 7, 0, 92, 21705, 'Phase 6', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Boots of the Fallen Prophet'),
(7, 1, 8, 0, 92, 21602, 'Phase 6', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Qiraji Execution Bracers'),
(7, 1, 9, 0, 92, 21624, 'Phase 6', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Gauntlets of Kalimdor'),
(7, 1, 10, 0, 92, 17063, 'Phase 6', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Band of Accuria'),
(7, 1, 12, 0, 92, 22321, 'Phase 6', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Heart of Wyrmthalak'),
(7, 1, 13, 0, 92, 19289, 'Phase 6', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Darkmoon Card: Maelstrom'),
(7, 1, 14, 0, 92, 23045, 'Phase 6', 'Shaman', 'Enhancement', 'Back', 'Both', 'Shroud of Dominion'),
(7, 1, 15, 0, 92, 22798, 'Phase 6', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Might of Menethil'),
(7, 1, 17, 0, 92, 22395, 'Phase 6', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of Rage');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 120, 28224, 'Pre-Raid', 'Shaman', 'Enhancement', 'Head', 'Both', 'Wastewalker Helm'),
(7, 1, 1, 0, 120, 29381, 'Pre-Raid', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Choker of Vile Intent'),
(7, 1, 2, 0, 120, 27797, 'Pre-Raid', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(7, 1, 4, 0, 120, 29515, 'Pre-Raid', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Ebon Netherscale Breastplate'),
(7, 1, 5, 0, 120, 29516, 'Pre-Raid', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Ebon Netherscale Belt'),
(7, 1, 6, 0, 120, 31544, 'Pre-Raid', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Clefthoof Hide Leggings'),
(7, 1, 7, 0, 120, 25686, 'Pre-Raid', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Fel Leather Boots'),
(7, 1, 8, 0, 120, 29517, 'Pre-Raid', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Ebon Netherscale Bracers'),
(7, 1, 9, 0, 120, 25685, 'Pre-Raid', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Fel Leather Gloves'),
(7, 1, 10, 0, 120, 30365, 'Pre-Raid', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Overseer''s Signet'),
(7, 1, 11, 0, 120, 30834, 'Pre-Raid', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(7, 1, 12, 0, 120, 28288, 'Pre-Raid', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Abacus of Violent Odds'),
(7, 1, 13, 0, 120, 29383, 'Pre-Raid', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(7, 1, 14, 0, 120, 24259, 'Pre-Raid', 'Shaman', 'Enhancement', 'Back', 'Both', 'Vengeance Wrap'),
(7, 1, 15, 0, 120, 27872, 'Pre-Raid', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'The Harvester of Souls'),
(7, 1, 16, 0, 120, 27872, 'Pre-Raid', 'Shaman', 'Enhancement', 'OffHand', 'Both', 'The Harvester of Souls'),
(7, 1, 17, 0, 120, 27815, 'Pre-Raid', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of the Astral Winds');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 125, 29040, 'Phase 1', 'Shaman', 'Enhancement', 'Head', 'Both', 'Cyclone Helm'),
(7, 1, 1, 0, 125, 29381, 'Phase 1', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Choker of Vile Intent'),
(7, 1, 2, 0, 125, 29043, 'Phase 1', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Cyclone Shoulderplates'),
(7, 1, 4, 0, 125, 29515, 'Phase 1', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Ebon Netherscale Breastplate'),
(7, 1, 5, 0, 125, 29516, 'Phase 1', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Ebon Netherscale Belt'),
(7, 1, 6, 0, 125, 30739, 'Phase 1', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Scaled Greaves of the Marksman'),
(7, 1, 7, 0, 125, 28545, 'Phase 1', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Edgewalker Longboots'),
(7, 1, 8, 0, 125, 29517, 'Phase 1', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Ebon Netherscale Bracers'),
(7, 1, 9, 0, 125, 28776, 'Phase 1', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Liar''s Tongue Gloves'),
(7, 1, 10, 0, 125, 28757, 'Phase 1', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Ring of a Thousand Marks'),
(7, 1, 11, 0, 125, 30834, 'Phase 1', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(7, 1, 12, 0, 125, 29383, 'Phase 1', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(7, 1, 13, 0, 125, 28830, 'Phase 1', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(7, 1, 14, 0, 125, 24259, 'Phase 1', 'Shaman', 'Enhancement', 'Back', 'Both', 'Vengeance Wrap'),
(7, 1, 15, 0, 125, 28308, 'Phase 1', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Gladiator''s Cleaver'),
(7, 1, 16, 0, 125, 28308, 'Phase 1', 'Shaman', 'Enhancement', 'OffHand', 'Both', 'Gladiator''s Cleaver'),
(7, 1, 17, 0, 125, 27815, 'Phase 1', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of the Astral Winds');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 141, 30190, 'Phase 2', 'Shaman', 'Enhancement', 'Head', 'Both', 'Cataclysm Helm'),
(7, 1, 1, 0, 141, 30017, 'Phase 2', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Telonicus''s Pendant of Mayhem'),
(7, 1, 2, 0, 141, 30055, 'Phase 2', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Shoulderpads of the Stranger'),
(7, 1, 4, 0, 141, 30185, 'Phase 2', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Cataclysm Chestplate'),
(7, 1, 5, 0, 141, 30106, 'Phase 2', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(7, 1, 6, 0, 141, 30192, 'Phase 2', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Cataclysm Legplates'),
(7, 1, 7, 0, 141, 30039, 'Phase 2', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Boots of Utter Darkness'),
(7, 1, 8, 0, 141, 30091, 'Phase 2', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'True-Aim Stalker Bands'),
(7, 1, 9, 0, 141, 30189, 'Phase 2', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Cataclysm Gauntlets'),
(7, 1, 10, 0, 141, 29997, 'Phase 2', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Band of the Ranger-General'),
(7, 1, 11, 0, 141, 30052, 'Phase 2', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Ring of Lethality'),
(7, 1, 12, 0, 141, 29383, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(7, 1, 13, 0, 141, 28830, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(7, 1, 14, 0, 141, 29994, 'Phase 2', 'Shaman', 'Enhancement', 'Back', 'Both', 'Thalassian Wildercloak'),
(7, 1, 16, 0, 141, 28433, 'Phase 2', 'Shaman', 'Enhancement', 'OffHand', 'Both', 'Wicked Edge of the Planes'),
(7, 1, 17, 0, 141, 27815, 'Phase 2', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of the Astral Winds');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 156, 32235, 'Phase 3', 'Shaman', 'Enhancement', 'Head', 'Both', 'Cursed Vision of Sargeras'),
(7, 1, 1, 0, 156, 32260, 'Phase 3', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Choker of Endless Nightmares'),
(7, 1, 2, 0, 156, 32581, 'Phase 3', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Swiftstrike Shoulders'),
(7, 1, 4, 0, 156, 30905, 'Phase 3', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Midnight Chestguard'),
(7, 1, 5, 0, 156, 30106, 'Phase 3', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(7, 1, 6, 0, 156, 30900, 'Phase 3', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Bow-Stitched Leggings'),
(7, 1, 7, 0, 156, 32366, 'Phase 3', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Shadowmaster''s Boots'),
(7, 1, 8, 0, 156, 32574, 'Phase 3', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Bindings of Lightning Reflexes'),
(7, 1, 9, 0, 156, 32347, 'Phase 3', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Grips of Damnation'),
(7, 1, 10, 0, 156, 29301, 'Phase 3', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Band of the Eternal Champion'),
(7, 1, 11, 0, 156, 32497, 'Phase 3', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Stormrage Signet Ring'),
(7, 1, 12, 0, 156, 28830, 'Phase 3', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Dragonspine Trophy'),
(7, 1, 13, 0, 156, 32505, 'Phase 3', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Madness of the Betrayer'),
(7, 1, 14, 0, 156, 32323, 'Phase 3', 'Shaman', 'Enhancement', 'Back', 'Both', 'Shadowmoon Destroyer''s Drape'),
(7, 1, 17, 0, 156, 27815, 'Phase 3', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of the Astral Winds');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 164, 32235, 'Phase 4', 'Shaman', 'Enhancement', 'Head', 'Both', 'Cursed Vision of Sargeras'),
(7, 1, 1, 0, 164, 32260, 'Phase 4', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Choker of Endless Nightmares'),
(7, 1, 2, 0, 164, 32581, 'Phase 4', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Swiftstrike Shoulders'),
(7, 1, 4, 0, 164, 30905, 'Phase 4', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Midnight Chestguard'),
(7, 1, 5, 0, 164, 30106, 'Phase 4', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of One-Hundred Deaths'),
(7, 1, 6, 0, 164, 30900, 'Phase 4', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Bow-Stitched Leggings'),
(7, 1, 7, 0, 164, 32366, 'Phase 4', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Shadowmaster''s Boots'),
(7, 1, 8, 0, 164, 32574, 'Phase 4', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Bindings of Lightning Reflexes'),
(7, 1, 9, 0, 164, 32347, 'Phase 4', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Grips of Damnation'),
(7, 1, 10, 0, 164, 33496, 'Phase 4', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Signet of Primal Wrath'),
(7, 1, 11, 0, 164, 32497, 'Phase 4', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Stormrage Signet Ring'),
(7, 1, 12, 0, 164, 28830, 'Phase 4', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Dragonspine Trophy'),
(7, 1, 13, 0, 164, 32505, 'Phase 4', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Madness of the Betrayer'),
(7, 1, 14, 0, 164, 32323, 'Phase 4', 'Shaman', 'Enhancement', 'Back', 'Both', 'Shadowmoon Destroyer''s Drape'),
(7, 1, 15, 0, 164, 34331, 'Phase 5', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Hand of the Deceiver'),
(7, 1, 16, 0, 164, 34346, 'Phase 5', 'Shaman', 'Enhancement', 'OffHand', 'Both', 'Mounting Vengeance'),
(7, 1, 17, 0, 164, 33507, 'Phase 4', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Stonebreaker''s Totem');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 224, 39602, 'Phase 1', 'Shaman', 'Enhancement', 'Head', 'Both', 'Heroes'' Earthshatter Faceguard'),
(7, 1, 1, 0, 224, 39146, 'Phase 1', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Collar of Dissolution'),
(7, 1, 2, 0, 224, 40524, 'Phase 1', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Valorous Earthshatter Shoulderguards'),
(7, 1, 4, 0, 224, 39597, 'Phase 1', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Heroes'' Earthshatter Chestguard'),
(7, 1, 5, 0, 224, 39762, 'Phase 1', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Torn Web Wrapping'),
(7, 1, 6, 0, 224, 40522, 'Phase 1', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Valorous Earthshatter War-Kilt'),
(7, 1, 7, 0, 224, 40746, 'Phase 1', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Pack-Ice Striders'),
(7, 1, 8, 0, 224, 40282, 'Phase 1', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Slime Stream Bands'),
(7, 1, 9, 0, 224, 39601, 'Phase 1', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Heroes'' Earthshatter Grips'),
(7, 1, 10, 0, 224, 45688, 'Phase 1', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Inscribed Band of the Kirin Tor'),
(7, 1, 12, 1, 224, 40322, 'Phase 1', 'Shaman', 'Enhancement', 'Trinket1', 'Alliance', 'Totem of Dueling'),
(7, 1, 12, 2, 224, 37390, 'Phase 1', 'Shaman', 'Enhancement', 'Trinket1', 'Horde', 'Meteorite Whetstone'),
(7, 1, 14, 0, 224, 40721, 'Phase 1', 'Shaman', 'Enhancement', 'Back', 'Both', 'Hammerhead Sharkskin Cloak');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 1, 245, 51242, 'Phase 2', 'Shaman', 'Enhancement', 'Head', 'Alliance', 'Sanctified Frost Witch''s Faceguard'),
(7, 1, 0, 2, 245, 40543, 'Phase 2', 'Shaman', 'Enhancement', 'Head', 'Horde', 'Blue Aspect Helm'),
(7, 1, 1, 1, 245, 50658, 'Phase 2', 'Shaman', 'Enhancement', 'Neck', 'Alliance', 'Amulet of the Silent Eulogy'),
(7, 1, 1, 2, 245, 44661, 'Phase 2', 'Shaman', 'Enhancement', 'Neck', 'Horde', 'Wyrmrest Necklace of Power'),
(7, 1, 2, 1, 245, 51240, 'Phase 2', 'Shaman', 'Enhancement', 'Shoulders', 'Alliance', 'Sanctified Frost Witch''s Shoulderguards'),
(7, 1, 2, 2, 245, 40524, 'Phase 2', 'Shaman', 'Enhancement', 'Shoulders', 'Horde', 'Valorous Earthshatter Shoulderguards'),
(7, 1, 4, 1, 245, 50689, 'Phase 2', 'Shaman', 'Enhancement', 'Chest', 'Alliance', 'Carapace of Forgotten Kings'),
(7, 1, 4, 2, 245, 40523, 'Phase 2', 'Shaman', 'Enhancement', 'Chest', 'Horde', 'Valorous Earthshatter Chestguard'),
(7, 1, 5, 1, 245, 54587, 'Phase 2', 'Shaman', 'Enhancement', 'Waist', 'Alliance', 'Split Shape Belt'),
(7, 1, 5, 2, 245, 39762, 'Phase 2', 'Shaman', 'Enhancement', 'Waist', 'Horde', 'Torn Web Wrapping'),
(7, 1, 6, 1, 245, 51241, 'Phase 2', 'Shaman', 'Enhancement', 'Legs', 'Alliance', 'Sanctified Frost Witch''s War-Kilt'),
(7, 1, 6, 2, 245, 40522, 'Phase 2', 'Shaman', 'Enhancement', 'Legs', 'Horde', 'Valorous Earthshatter War-Kilt'),
(7, 1, 7, 1, 245, 50711, 'Phase 2', 'Shaman', 'Enhancement', 'Feet', 'Alliance', 'Treads of the Wasteland'),
(7, 1, 7, 2, 245, 40367, 'Phase 2', 'Shaman', 'Enhancement', 'Feet', 'Horde', 'Boots of the Great Construct'),
(7, 1, 8, 1, 245, 54582, 'Phase 2', 'Shaman', 'Enhancement', 'Wrists', 'Alliance', 'Bracers of Fiery Night'),
(7, 1, 8, 2, 245, 40282, 'Phase 2', 'Shaman', 'Enhancement', 'Wrists', 'Horde', 'Slime Stream Bands'),
(7, 1, 9, 1, 245, 51243, 'Phase 2', 'Shaman', 'Enhancement', 'Hands', 'Alliance', 'Sanctified Frost Witch''s Grips'),
(7, 1, 9, 2, 245, 40520, 'Phase 2', 'Shaman', 'Enhancement', 'Hands', 'Horde', 'Valorous Earthshatter Grips'),
(7, 1, 10, 1, 245, 54576, 'Phase 2', 'Shaman', 'Enhancement', 'Finger1', 'Alliance', 'Signet of Twilight'),
(7, 1, 10, 2, 245, 40074, 'Phase 2', 'Shaman', 'Enhancement', 'Finger1', 'Horde', 'Strong-Handed Ring'),
(7, 1, 12, 1, 245, 50365, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket1', 'Alliance', 'Phylactery of the Nameless Lich'),
(7, 1, 12, 2, 245, 40684, 'Phase 2', 'Shaman', 'Enhancement', 'Trinket1', 'Horde', 'Mirror of Truth'),
(7, 1, 14, 1, 245, 54583, 'Phase 2', 'Shaman', 'Enhancement', 'Back', 'Alliance', 'Cloak of Burning Dusk'),
(7, 1, 14, 2, 245, 40403, 'Phase 2', 'Shaman', 'Enhancement', 'Back', 'Horde', 'Drape of the Deadly Foe'),
(7, 1, 15, 0, 245, 50734, 'Phase 2', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(7, 1, 17, 1, 245, 50458, 'Phase 2', 'Shaman', 'Enhancement', 'Ranged', 'Alliance', 'Bizuri''s Totem of Shattered Ice'),
(7, 1, 17, 2, 245, 40322, 'Phase 2', 'Shaman', 'Enhancement', 'Ranged', 'Horde', 'Totem of Dueling');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 258, 45610, 'Phase 3', 'Shaman', 'Enhancement', 'Head', 'Both', 'Boundless Gaze'),
(7, 1, 1, 0, 258, 45517, 'Phase 3', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Pendulum of Infinity'),
(7, 1, 2, 0, 258, 46203, 'Phase 3', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Conqueror''s Worldbreaker Shoulderguards'),
(7, 1, 4, 0, 258, 46205, 'Phase 3', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Conqueror''s Worldbreaker Chestguard'),
(7, 1, 5, 0, 258, 45553, 'Phase 3', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Belt of Dragons'),
(7, 1, 6, 0, 258, 46208, 'Phase 3', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Conqueror''s Worldbreaker War-Kilt'),
(7, 1, 7, 0, 258, 45244, 'Phase 3', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Greaves of Swift Vengeance'),
(7, 1, 8, 0, 258, 45611, 'Phase 3', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Solar Bindings'),
(7, 1, 9, 0, 258, 46200, 'Phase 3', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Conqueror''s Worldbreaker Grips'),
(7, 1, 12, 0, 258, 45609, 'Phase 3', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Comet''s Trail'),
(7, 1, 14, 0, 258, 45461, 'Phase 3', 'Shaman', 'Enhancement', 'Back', 'Both', 'Drape of Icy Intent'),
(7, 1, 15, 0, 258, 45132, 'Phase 3', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Golden Saronite Dragon'),
(7, 1, 17, 0, 258, 42608, 'Phase 3', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Furious Gladiator''s Totem of Indomitability');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 264, 48353, 'Phase 4', 'Shaman', 'Enhancement', 'Head', 'Both', 'Faceguard of Triumph'),
(7, 1, 2, 0, 264, 48351, 'Phase 4', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Shoulderguards of Triumph'),
(7, 1, 4, 1, 264, 46965, 'Phase 4', 'Shaman', 'Enhancement', 'Chest', 'Alliance', 'Breastplate of Cruel Intent'),
(7, 1, 4, 2, 264, 47412, 'Phase 4', 'Shaman', 'Enhancement', 'Chest', 'Horde', 'Cuirass of Cruel Intent'),
(7, 1, 5, 1, 264, 47112, 'Phase 4', 'Shaman', 'Enhancement', 'Waist', 'Alliance', 'Belt of the Merciless Killer'),
(7, 1, 5, 2, 264, 47460, 'Phase 4', 'Shaman', 'Enhancement', 'Waist', 'Horde', 'Belt of the Pitiless Killer'),
(7, 1, 6, 0, 264, 48352, 'Phase 4', 'Shaman', 'Enhancement', 'Legs', 'Both', 'War-Kilt of Triumph'),
(7, 1, 7, 1, 264, 47099, 'Phase 4', 'Shaman', 'Enhancement', 'Feet', 'Alliance', 'Boots of Tremoring Earth'),
(7, 1, 7, 2, 264, 47456, 'Phase 4', 'Shaman', 'Enhancement', 'Feet', 'Horde', 'Sabatons of Tremoring Earth'),
(7, 1, 8, 1, 264, 47143, 'Phase 4', 'Shaman', 'Enhancement', 'Wrists', 'Alliance', 'Bindings of Dark Essence'),
(7, 1, 8, 2, 264, 47467, 'Phase 4', 'Shaman', 'Enhancement', 'Wrists', 'Horde', 'Dark Essence Bindings'),
(7, 1, 9, 0, 264, 48354, 'Phase 4', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Grips of Triumph'),
(7, 1, 10, 1, 264, 47075, 'Phase 4', 'Shaman', 'Enhancement', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(7, 1, 10, 2, 264, 47443, 'Phase 4', 'Shaman', 'Enhancement', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(7, 1, 12, 1, 264, 45609, 'Phase 4', 'Shaman', 'Enhancement', 'Trinket1', 'Alliance', 'Comet''s Trail'),
(7, 1, 14, 1, 264, 47552, 'Phase 4', 'Shaman', 'Enhancement', 'Back', 'Alliance', 'Jaina''s Radiance'),
(7, 1, 14, 2, 264, 47551, 'Phase 4', 'Shaman', 'Enhancement', 'Back', 'Horde', 'Aethas'' Intensity'),
(7, 1, 17, 1, 264, 47666, 'Phase 4', 'Shaman', 'Enhancement', 'Ranged', 'Alliance', 'Totem of Electrifying Wind');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 1, 0, 0, 290, 51242, 'Phase 5', 'Shaman', 'Enhancement', 'Head', 'Both', 'Sanctified Frost Witch''s Faceguard'),
(7, 1, 1, 0, 290, 50633, 'Phase 5', 'Shaman', 'Enhancement', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(7, 1, 2, 0, 290, 51240, 'Phase 5', 'Shaman', 'Enhancement', 'Shoulders', 'Both', 'Sanctified Frost Witch''s Shoulderguards'),
(7, 1, 4, 0, 290, 50689, 'Phase 5', 'Shaman', 'Enhancement', 'Chest', 'Both', 'Carapace of Forgotten Kings'),
(7, 1, 5, 0, 290, 50688, 'Phase 5', 'Shaman', 'Enhancement', 'Waist', 'Both', 'Nerub''ar Stalker''s Cord'),
(7, 1, 6, 0, 290, 51241, 'Phase 5', 'Shaman', 'Enhancement', 'Legs', 'Both', 'Sanctified Frost Witch''s War-Kilt'),
(7, 1, 7, 0, 290, 54577, 'Phase 5', 'Shaman', 'Enhancement', 'Feet', 'Both', 'Returning Footfalls'),
(7, 1, 8, 0, 290, 50655, 'Phase 5', 'Shaman', 'Enhancement', 'Wrists', 'Both', 'Scourge Hunter''s Vambraces'),
(7, 1, 9, 0, 290, 51243, 'Phase 5', 'Shaman', 'Enhancement', 'Hands', 'Both', 'Sanctified Frost Witch''s Grips'),
(7, 1, 10, 0, 290, 50678, 'Phase 5', 'Shaman', 'Enhancement', 'Finger1', 'Both', 'Seal of Many Mouths'),
(7, 1, 11, 0, 290, 50402, 'Phase 5', 'Shaman', 'Enhancement', 'Finger2', 'Both', 'Ashen Band of Endless Vengeance'),
(7, 1, 12, 0, 290, 50706, 'Phase 5', 'Shaman', 'Enhancement', 'Trinket1', 'Both', 'Tiny Abomination in a Jar'),
(7, 1, 13, 0, 290, 54590, 'Phase 5', 'Shaman', 'Enhancement', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(7, 1, 14, 0, 290, 50653, 'Phase 5', 'Shaman', 'Enhancement', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(7, 1, 15, 0, 290, 50737, 'Phase 5', 'Shaman', 'Enhancement', 'MainHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(7, 1, 16, 0, 290, 50737, 'Phase 5', 'Shaman', 'Enhancement', 'OffHand', 'Both', 'Havoc''s Call, Blade of Lordaeron Kings'),
(7, 1, 17, 0, 290, 50463, 'Phase 5', 'Shaman', 'Enhancement', 'Ranged', 'Both', 'Totem of the Avalanche');

-- Restoration (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 66, 13216, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Head', 'Both', 'Crown of the Penitent'),
(7, 2, 1, 0, 66, 18723, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(7, 2, 2, 0, 66, 15061, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Living Shoulders'),
(7, 2, 4, 0, 66, 13346, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(7, 2, 5, 0, 66, 14553, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Waist', 'Both', 'Sash of Mercy'),
(7, 2, 6, 0, 66, 11841, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Legs', 'Both', 'Senior Designer''s Pantaloons'),
(7, 2, 7, 0, 66, 13954, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Feet', 'Both', 'Verdant Footpads'),
(7, 2, 8, 0, 66, 13969, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Loomguard Armbraces'),
(7, 2, 9, 0, 66, 10787, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Hands', 'Both', 'Atal''ai Gloves'),
(7, 2, 10, 0, 66, 16058, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Fordring''s Seal'),
(7, 2, 11, 0, 66, 13178, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Rosewine Circle'),
(7, 2, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Briarwood Reed'),
(7, 2, 13, 0, 66, 11819, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Second Wind'),
(7, 2, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'Back', 'Both', 'Archivist Cape'),
(7, 2, 15, 0, 66, 11923, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'MainHand', 'Both', 'The Hammer of Grace'),
(7, 2, 16, 0, 66, 11928, 'Phase 1 (Pre-Raid)', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Thaurissan''s Royal Scepter');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 76, 13216, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Head', 'Both', 'Crown of the Penitent'),
(7, 2, 1, 0, 76, 18723, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(7, 2, 2, 0, 76, 18757, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Diabolic Mantle'),
(7, 2, 4, 0, 76, 13346, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(7, 2, 5, 0, 76, 14553, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Waist', 'Both', 'Sash of Mercy'),
(7, 2, 6, 0, 76, 18386, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Legs', 'Both', 'Padre''s Trousers'),
(7, 2, 7, 0, 76, 13954, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Feet', 'Both', 'Verdant Footpads'),
(7, 2, 8, 0, 76, 13969, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Loomguard Armbraces'),
(7, 2, 9, 0, 76, 18527, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Hands', 'Both', 'Harmonious Gauntlets'),
(7, 2, 10, 0, 76, 16058, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Fordring''s Seal'),
(7, 2, 11, 0, 76, 13178, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Rosewine Circle'),
(7, 2, 12, 0, 76, 18371, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Mindtap Talisman'),
(7, 2, 13, 0, 76, 18371, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Mindtap Talisman'),
(7, 2, 14, 0, 76, 13386, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'Back', 'Both', 'Archivist Cape'),
(7, 2, 15, 0, 76, 11923, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'MainHand', 'Both', 'The Hammer of Grace'),
(7, 2, 16, 0, 76, 18523, 'Phase 2 (Pre-Raid)', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 78, 16842, 'Phase 2', 'Shaman', 'Restoration', 'Head', 'Both', 'Earthfury Helmet'),
(7, 2, 1, 0, 78, 18723, 'Phase 2', 'Shaman', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(7, 2, 2, 0, 78, 18810, 'Phase 2', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(7, 2, 4, 0, 78, 13346, 'Phase 2', 'Shaman', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(7, 2, 5, 0, 78, 19162, 'Phase 2', 'Shaman', 'Restoration', 'Waist', 'Both', 'Corehound Belt'),
(7, 2, 6, 0, 78, 18875, 'Phase 2', 'Shaman', 'Restoration', 'Legs', 'Both', 'Salamander Scale Pants'),
(7, 2, 7, 0, 78, 13954, 'Phase 2', 'Shaman', 'Restoration', 'Feet', 'Both', 'Verdant Footpads'),
(7, 2, 8, 0, 78, 13969, 'Phase 2', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Loomguard Armbraces'),
(7, 2, 9, 0, 78, 18527, 'Phase 2', 'Shaman', 'Restoration', 'Hands', 'Both', 'Harmonious Gauntlets'),
(7, 2, 10, 0, 78, 19140, 'Phase 2', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Cauterizing Band'),
(7, 2, 11, 0, 78, 13178, 'Phase 2', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Rosewine Circle'),
(7, 2, 12, 0, 78, 17064, 'Phase 2', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Shard of the Scale'),
(7, 2, 13, 0, 78, 18371, 'Phase 2', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Mindtap Talisman'),
(7, 2, 14, 0, 78, 13386, 'Phase 2', 'Shaman', 'Restoration', 'Back', 'Both', 'Archivist Cape'),
(7, 2, 15, 0, 78, 17070, 'Phase 2', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Fang of the Mystics'),
(7, 2, 16, 0, 78, 18523, 'Phase 2', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 83, 16947, 'Phase 3', 'Shaman', 'Restoration', 'Head', 'Both', 'Helmet of Ten Storms'),
(7, 2, 1, 0, 83, 19371, 'Phase 3', 'Shaman', 'Restoration', 'Neck', 'Both', 'Pendant of the Fallen Dragon'),
(7, 2, 2, 0, 83, 18810, 'Phase 3', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(7, 2, 4, 0, 83, 13346, 'Phase 3', 'Shaman', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(7, 2, 5, 0, 83, 19162, 'Phase 3', 'Shaman', 'Restoration', 'Waist', 'Both', 'Corehound Belt'),
(7, 2, 6, 0, 83, 18875, 'Phase 3', 'Shaman', 'Restoration', 'Legs', 'Both', 'Salamander Scale Pants'),
(7, 2, 7, 0, 83, 19391, 'Phase 3', 'Shaman', 'Restoration', 'Feet', 'Both', 'Shimmering Geta'),
(7, 2, 8, 0, 83, 16943, 'Phase 3', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bracers of Ten Storms'),
(7, 2, 9, 0, 83, 16948, 'Phase 3', 'Shaman', 'Restoration', 'Hands', 'Both', 'Gauntlets of Ten Storms'),
(7, 2, 10, 0, 83, 19397, 'Phase 3', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ring of Blackrock'),
(7, 2, 11, 0, 83, 19382, 'Phase 3', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Pure Elementium Band'),
(7, 2, 12, 0, 83, 17064, 'Phase 3', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Shard of the Scale'),
(7, 2, 13, 0, 83, 19395, 'Phase 3', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(7, 2, 14, 0, 83, 19430, 'Phase 3', 'Shaman', 'Restoration', 'Back', 'Both', 'Shroud of Pure Thought'),
(7, 2, 15, 0, 83, 19347, 'Phase 3', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Claw of Chromaggus'),
(7, 2, 16, 0, 83, 19312, 'Phase 3', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Lei of the Lifegiver');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 88, 21372, 'Phase 5', 'Shaman', 'Restoration', 'Head', 'Both', 'Stormcaller''s Diadem'),
(7, 2, 1, 0, 88, 21712, 'Phase 5', 'Shaman', 'Restoration', 'Neck', 'Both', 'Amulet of the Fallen God'),
(7, 2, 2, 0, 88, 21376, 'Phase 5', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Stormcaller''s Pauldrons'),
(7, 2, 4, 0, 88, 21374, 'Phase 5', 'Shaman', 'Restoration', 'Chest', 'Both', 'Stormcaller''s Hauberk'),
(7, 2, 5, 0, 88, 16944, 'Phase 5', 'Shaman', 'Restoration', 'Waist', 'Both', 'Belt of Ten Storms'),
(7, 2, 6, 0, 88, 21375, 'Phase 5', 'Shaman', 'Restoration', 'Legs', 'Both', 'Stormcaller''s Leggings'),
(7, 2, 7, 0, 88, 21373, 'Phase 5', 'Shaman', 'Restoration', 'Feet', 'Both', 'Stormcaller''s Footguards'),
(7, 2, 8, 0, 88, 16943, 'Phase 5', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bracers of Ten Storms'),
(7, 2, 9, 0, 88, 16948, 'Phase 5', 'Shaman', 'Restoration', 'Hands', 'Both', 'Gauntlets of Ten Storms'),
(7, 2, 10, 0, 88, 21620, 'Phase 5', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ring of the Martyr'),
(7, 2, 11, 0, 88, 21681, 'Phase 5', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Ring of the Devoured'),
(7, 2, 12, 0, 88, 17064, 'Phase 5', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Shard of the Scale'),
(7, 2, 13, 0, 88, 19395, 'Phase 5', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(7, 2, 14, 0, 88, 21583, 'Phase 5', 'Shaman', 'Restoration', 'Back', 'Both', 'Cloak of Clarity'),
(7, 2, 15, 0, 88, 21839, 'Phase 5', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Scepter of the False Prophet'),
(7, 2, 16, 0, 88, 21610, 'Phase 5', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Wormscale Blocker'),
(7, 2, 17, 0, 88, 22396, 'Phase 5', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Life');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 92, 22466, 'Phase 6', 'Shaman', 'Restoration', 'Head', 'Both', 'Earthshatter Headpiece'),
(7, 2, 1, 0, 92, 21712, 'Phase 6', 'Shaman', 'Restoration', 'Neck', 'Both', 'Amulet of the Fallen God'),
(7, 2, 2, 0, 92, 22467, 'Phase 6', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Earthshatter Spaulders'),
(7, 2, 4, 0, 92, 22464, 'Phase 6', 'Shaman', 'Restoration', 'Chest', 'Both', 'Earthshatter Tunic'),
(7, 2, 5, 0, 92, 22470, 'Phase 6', 'Shaman', 'Restoration', 'Waist', 'Both', 'Earthshatter Girdle'),
(7, 2, 6, 0, 92, 22465, 'Phase 6', 'Shaman', 'Restoration', 'Legs', 'Both', 'Earthshatter Legguards'),
(7, 2, 7, 0, 92, 22468, 'Phase 6', 'Shaman', 'Restoration', 'Feet', 'Both', 'Earthshatter Boots'),
(7, 2, 8, 0, 92, 22471, 'Phase 6', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Earthshatter Wristguards'),
(7, 2, 9, 0, 92, 21619, 'Phase 6', 'Shaman', 'Restoration', 'Hands', 'Both', 'Gloves of the Messiah'),
(7, 2, 10, 0, 92, 21620, 'Phase 6', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ring of the Martyr'),
(7, 2, 11, 0, 92, 23065, 'Phase 6', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Ring of the Earthshatterer'),
(7, 2, 12, 0, 92, 23027, 'Phase 6', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Warmth of Forgiveness'),
(7, 2, 13, 0, 92, 19395, 'Phase 6', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Rejuvenating Gem'),
(7, 2, 14, 0, 92, 21583, 'Phase 6', 'Shaman', 'Restoration', 'Back', 'Both', 'Cloak of Clarity'),
(7, 2, 15, 0, 92, 23056, 'Phase 6', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Hammer of the Twisting Nether'),
(7, 2, 16, 0, 92, 22819, 'Phase 6', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Shield of Condemnation'),
(7, 2, 17, 0, 92, 22396, 'Phase 6', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Life');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 120, 24264, 'Pre-Raid', 'Shaman', 'Restoration', 'Head', 'Both', 'Whitemend Hood'),
(7, 2, 1, 0, 120, 31691, 'Pre-Raid', 'Shaman', 'Restoration', 'Neck', 'Both', 'Natasha''s Guardian Cord'),
(7, 2, 2, 0, 120, 35395, 'Pre-Raid', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Seer''s Ringmail Shoulderpads'),
(7, 2, 4, 0, 120, 29522, 'Pre-Raid', 'Shaman', 'Restoration', 'Chest', 'Both', 'Windhawk Hauberk'),
(7, 2, 5, 0, 120, 29524, 'Pre-Raid', 'Shaman', 'Restoration', 'Waist', 'Both', 'Windhawk Belt'),
(7, 2, 6, 0, 120, 24261, 'Pre-Raid', 'Shaman', 'Restoration', 'Legs', 'Both', 'Whitemend Pants'),
(7, 2, 7, 0, 120, 27549, 'Pre-Raid', 'Shaman', 'Restoration', 'Feet', 'Both', 'Wavefury Boots'),
(7, 2, 8, 0, 120, 29523, 'Pre-Raid', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Windhawk Bracers'),
(7, 2, 9, 0, 120, 29506, 'Pre-Raid', 'Shaman', 'Restoration', 'Hands', 'Both', 'Gloves of the Living Touch'),
(7, 2, 10, 0, 120, 28259, 'Pre-Raid', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Cosmic Lifeband'),
(7, 2, 10, 2, 120, 29168, 'Pre-Raid', 'Shaman', 'Restoration', 'Finger1', 'Horde', 'Ancestral Band'),
(7, 2, 12, 0, 120, 29376, 'Pre-Raid', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(7, 2, 13, 0, 120, 30841, 'Pre-Raid', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Lower City Prayerbook'),
(7, 2, 14, 0, 120, 24254, 'Pre-Raid', 'Shaman', 'Restoration', 'Back', 'Both', 'White Remedy Cape'),
(7, 2, 15, 0, 120, 23556, 'Pre-Raid', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Hand of Eternity'),
(7, 2, 16, 0, 120, 29267, 'Pre-Raid', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Light-Bearer''s Faith Shield'),
(7, 2, 17, 0, 120, 27544, 'Pre-Raid', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Spontaneous Regrowth');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 125, 29035, 'Phase 1', 'Shaman', 'Restoration', 'Head', 'Both', 'Cyclone Faceguard'),
(7, 2, 1, 0, 125, 28609, 'Phase 1', 'Shaman', 'Restoration', 'Neck', 'Both', 'Emberspur Talisman'),
(7, 2, 2, 0, 125, 29037, 'Phase 1', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Cyclone Shoulderguards'),
(7, 2, 4, 0, 125, 29522, 'Phase 1', 'Shaman', 'Restoration', 'Chest', 'Both', 'Windhawk Hauberk'),
(7, 2, 5, 0, 125, 29524, 'Phase 1', 'Shaman', 'Restoration', 'Waist', 'Both', 'Windhawk Belt'),
(7, 2, 6, 0, 125, 30727, 'Phase 1', 'Shaman', 'Restoration', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(7, 2, 7, 0, 125, 30737, 'Phase 1', 'Shaman', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(7, 2, 8, 0, 125, 29523, 'Phase 1', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Windhawk Bracers'),
(7, 2, 9, 0, 125, 28520, 'Phase 1', 'Shaman', 'Restoration', 'Hands', 'Both', 'Gloves of Centering'),
(7, 2, 10, 0, 125, 28763, 'Phase 1', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Jade Ring of the Everliving'),
(7, 2, 11, 0, 125, 28790, 'Phase 1', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Naaru Lightwarden''s Band'),
(7, 2, 12, 0, 125, 29376, 'Phase 1', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(7, 2, 13, 0, 125, 28590, 'Phase 1', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Ribbon of Sacrifice'),
(7, 2, 14, 0, 125, 28765, 'Phase 1', 'Shaman', 'Restoration', 'Back', 'Both', 'Stainless Cloak of the Pure Hearted'),
(7, 2, 15, 0, 125, 28771, 'Phase 1', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Light''s Justice'),
(7, 2, 16, 0, 125, 29458, 'Phase 1', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Aegis of the Vindicator'),
(7, 2, 17, 0, 125, 28523, 'Phase 1', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Healing Rains');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 141, 32475, 'Phase 2', 'Shaman', 'Restoration', 'Head', 'Both', 'Living Replicator Specs'),
(7, 2, 1, 0, 141, 30018, 'Phase 2', 'Shaman', 'Restoration', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(7, 2, 2, 0, 141, 30168, 'Phase 2', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Cataclysm Shoulderguards'),
(7, 2, 4, 0, 141, 30164, 'Phase 2', 'Shaman', 'Restoration', 'Chest', 'Both', 'Cataclysm Chestguard'),
(7, 2, 5, 0, 141, 21873, 'Phase 2', 'Shaman', 'Restoration', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(7, 2, 6, 0, 141, 30727, 'Phase 2', 'Shaman', 'Restoration', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(7, 2, 7, 0, 141, 30737, 'Phase 2', 'Shaman', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(7, 2, 8, 0, 141, 30047, 'Phase 2', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Blackfathom Warbands'),
(7, 2, 9, 0, 141, 29976, 'Phase 2', 'Shaman', 'Restoration', 'Hands', 'Both', 'Worldstorm Gauntlets'),
(7, 2, 10, 0, 141, 28763, 'Phase 2', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Jade Ring of the Everliving'),
(7, 2, 11, 0, 141, 30736, 'Phase 2', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Ring of Flowing Light'),
(7, 2, 12, 0, 141, 29376, 'Phase 2', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(7, 2, 13, 0, 141, 28590, 'Phase 2', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Ribbon of Sacrifice'),
(7, 2, 14, 0, 141, 29989, 'Phase 2', 'Shaman', 'Restoration', 'Back', 'Both', 'Sunshower Light Cloak'),
(7, 2, 15, 0, 141, 30108, 'Phase 2', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Lightfathom Scepter'),
(7, 2, 16, 0, 141, 29458, 'Phase 2', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Aegis of the Vindicator'),
(7, 2, 17, 0, 141, 28523, 'Phase 2', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Healing Rains');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 156, 31012, 'Phase 3', 'Shaman', 'Restoration', 'Head', 'Both', 'Skyshatter Helmet'),
(7, 2, 1, 0, 156, 32370, 'Phase 3', 'Shaman', 'Restoration', 'Neck', 'Both', 'Nadina''s Pendant of Purity'),
(7, 2, 2, 0, 156, 31022, 'Phase 3', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Skyshatter Shoulderpads'),
(7, 2, 4, 0, 156, 31016, 'Phase 3', 'Shaman', 'Restoration', 'Chest', 'Both', 'Skyshatter Chestguard'),
(7, 2, 5, 0, 156, 32258, 'Phase 3', 'Shaman', 'Restoration', 'Waist', 'Both', 'Naturalist''s Preserving Cinch'),
(7, 2, 6, 0, 156, 31019, 'Phase 3', 'Shaman', 'Restoration', 'Legs', 'Both', 'Skyshatter Leggings'),
(7, 2, 7, 0, 156, 30737, 'Phase 3', 'Shaman', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(7, 2, 8, 0, 156, 32584, 'Phase 3', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Swiftheal Wraps'),
(7, 2, 9, 0, 156, 32328, 'Phase 3', 'Shaman', 'Restoration', 'Hands', 'Both', 'Botanist''s Gloves of Growth'),
(7, 2, 10, 0, 156, 32528, 'Phase 3', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(7, 2, 11, 0, 156, 32528, 'Phase 3', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(7, 2, 12, 0, 156, 29376, 'Phase 3', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(7, 2, 13, 0, 156, 32496, 'Phase 3', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Memento of Tyrande'),
(7, 2, 14, 0, 156, 32524, 'Phase 3', 'Shaman', 'Restoration', 'Back', 'Both', 'Shroud of the Highborne'),
(7, 2, 15, 0, 156, 32500, 'Phase 3', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(7, 2, 16, 0, 156, 30882, 'Phase 3', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Bastion of Light'),
(7, 2, 17, 0, 156, 28523, 'Phase 3', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Healing Rains');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 164, 31012, 'Phase 4', 'Shaman', 'Restoration', 'Head', 'Both', 'Skyshatter Helmet'),
(7, 2, 1, 0, 164, 33281, 'Phase 4', 'Shaman', 'Restoration', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(7, 2, 2, 0, 164, 31022, 'Phase 4', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Skyshatter Shoulderpads'),
(7, 2, 4, 0, 164, 31016, 'Phase 4', 'Shaman', 'Restoration', 'Chest', 'Both', 'Skyshatter Chestguard'),
(7, 2, 5, 0, 164, 32258, 'Phase 4', 'Shaman', 'Restoration', 'Waist', 'Both', 'Naturalist''s Preserving Cinch'),
(7, 2, 6, 0, 164, 31019, 'Phase 4', 'Shaman', 'Restoration', 'Legs', 'Both', 'Skyshatter Leggings'),
(7, 2, 7, 0, 164, 33471, 'Phase 4', 'Shaman', 'Restoration', 'Feet', 'Both', 'Two-Toed Sandals'),
(7, 2, 8, 0, 164, 32584, 'Phase 4', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Swiftheal Wraps'),
(7, 2, 9, 0, 164, 32328, 'Phase 4', 'Shaman', 'Restoration', 'Hands', 'Both', 'Botanist''s Gloves of Growth'),
(7, 2, 10, 0, 164, 32528, 'Phase 4', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(7, 2, 11, 0, 164, 32528, 'Phase 4', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(7, 2, 12, 0, 164, 29376, 'Phase 4', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(7, 2, 13, 0, 164, 32496, 'Phase 4', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Memento of Tyrande'),
(7, 2, 14, 0, 164, 32524, 'Phase 4', 'Shaman', 'Restoration', 'Back', 'Both', 'Shroud of the Highborne'),
(7, 2, 15, 0, 164, 32500, 'Phase 4', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(7, 2, 16, 0, 164, 30882, 'Phase 4', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Bastion of Light'),
(7, 2, 17, 0, 164, 28523, 'Phase 4', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Healing Rains');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 200, 42555, 'Pre-Raid', 'Shaman', 'Restoration', 'Head', 'Both', 'Electroflux Sight Enhancers'),
(7, 2, 1, 0, 200, 42647, 'Pre-Raid', 'Shaman', 'Restoration', 'Neck', 'Both', 'Titanium Spellshock Necklace'),
(7, 2, 2, 0, 200, 37875, 'Pre-Raid', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Spaulders of the Violet Hold'),
(7, 2, 4, 0, 200, 43461, 'Pre-Raid', 'Shaman', 'Restoration', 'Chest', 'Both', 'Revenant''s Breastplate'),
(7, 2, 5, 0, 200, 37855, 'Pre-Raid', 'Shaman', 'Restoration', 'Waist', 'Both', 'Mail Girdle of the Audient Earth'),
(7, 2, 6, 0, 200, 44305, 'Pre-Raid', 'Shaman', 'Restoration', 'Legs', 'Both', 'Kilt of Dark Mercy'),
(7, 2, 7, 0, 200, 43469, 'Pre-Raid', 'Shaman', 'Restoration', 'Feet', 'Both', 'Revenant''s Treads'),
(7, 2, 8, 0, 200, 37788, 'Pre-Raid', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Limb Regeneration Bracers'),
(7, 2, 9, 0, 200, 44204, 'Pre-Raid', 'Shaman', 'Restoration', 'Hands', 'Both', 'Grips of Fierce Pronouncements'),
(7, 2, 10, 0, 200, 37192, 'Pre-Raid', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Annhylde''s Ring'),
(7, 2, 12, 0, 200, 40685, 'Pre-Raid', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'The Egg of Mortal Essence'),
(7, 2, 14, 0, 200, 41609, 'Pre-Raid', 'Shaman', 'Restoration', 'Back', 'Both', 'Wispcloak'),
(7, 2, 15, 0, 200, 37169, 'Pre-Raid', 'Shaman', 'Restoration', 'MainHand', 'Both', 'War Mace of Unrequited Love'),
(7, 2, 17, 0, 200, 40709, 'Pre-Raid', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Forest Growth');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 224, 40510, 'Phase 1', 'Shaman', 'Restoration', 'Head', 'Both', 'Valorous Earthshatter Headpiece'),
(7, 2, 1, 0, 224, 44662, 'Phase 1', 'Shaman', 'Restoration', 'Neck', 'Both', 'Life-Binder''s Locket'),
(7, 2, 2, 0, 224, 40513, 'Phase 1', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Valorous Earthshatter Spaulders'),
(7, 2, 4, 0, 224, 40508, 'Phase 1', 'Shaman', 'Restoration', 'Chest', 'Both', 'Valorous Earthshatter Tunic'),
(7, 2, 5, 0, 224, 40327, 'Phase 1', 'Shaman', 'Restoration', 'Waist', 'Both', 'Girdle of Recuperation'),
(7, 2, 6, 0, 224, 40512, 'Phase 1', 'Shaman', 'Restoration', 'Legs', 'Both', 'Valorous Earthshatter Legguards'),
(7, 2, 7, 0, 224, 40519, 'Phase 1', 'Shaman', 'Restoration', 'Feet', 'Both', 'Footsteps of Malygos'),
(7, 2, 8, 0, 224, 40324, 'Phase 1', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bands of Mutual Respect'),
(7, 2, 9, 0, 224, 40564, 'Phase 1', 'Shaman', 'Restoration', 'Hands', 'Both', 'Winter Spectacle Gloves'),
(7, 2, 10, 0, 224, 40375, 'Phase 1', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ring of Decaying Beauty'),
(7, 2, 12, 0, 224, 40258, 'Phase 1', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Forethought Talisman'),
(7, 2, 14, 0, 224, 44005, 'Phase 1', 'Shaman', 'Restoration', 'Back', 'Both', 'Pennant Cloak'),
(7, 2, 17, 0, 224, 39728, 'Phase 1', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of Misery');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 245, 46201, 'Phase 2', 'Shaman', 'Restoration', 'Head', 'Both', 'Conqueror''s Worldbreaker Headpiece'),
(7, 2, 1, 0, 245, 45443, 'Phase 2', 'Shaman', 'Restoration', 'Neck', 'Both', 'Charm of Meticulous Timing'),
(7, 2, 2, 0, 245, 46204, 'Phase 2', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Conqueror''s Worldbreaker Spaulders'),
(7, 2, 4, 0, 245, 45867, 'Phase 2', 'Shaman', 'Restoration', 'Chest', 'Both', 'Breastplate of the Stoneshaper'),
(7, 2, 5, 0, 245, 45151, 'Phase 2', 'Shaman', 'Restoration', 'Waist', 'Both', 'Belt of the Fallen Wyrm'),
(7, 2, 6, 0, 245, 46202, 'Phase 2', 'Shaman', 'Restoration', 'Legs', 'Both', 'Conqueror''s Worldbreaker Legguards'),
(7, 2, 7, 0, 245, 45537, 'Phase 2', 'Shaman', 'Restoration', 'Feet', 'Both', 'Treads of the False Oracle'),
(7, 2, 8, 0, 245, 45460, 'Phase 2', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bindings of Winter Gale'),
(7, 2, 9, 0, 245, 46199, 'Phase 2', 'Shaman', 'Restoration', 'Hands', 'Both', 'Conqueror''s Worldbreaker Handguards'),
(7, 2, 10, 0, 245, 45614, 'Phase 2', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Starshine Circle'),
(7, 2, 12, 0, 245, 40432, 'Phase 2', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(7, 2, 14, 0, 245, 44005, 'Phase 2', 'Shaman', 'Restoration', 'Back', 'Both', 'Pennant Cloak'),
(7, 2, 15, 0, 245, 46017, 'Phase 2', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(7, 2, 17, 0, 245, 45114, 'Phase 2', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Steamcaller''s Totem');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 258, 46201, 'Phase 3', 'Shaman', 'Restoration', 'Head', 'Both', 'Conqueror''s Worldbreaker Headpiece'),
(7, 2, 1, 1, 258, 47144, 'Phase 3', 'Shaman', 'Restoration', 'Neck', 'Alliance', 'Wail of the Val''kyr'),
(7, 2, 1, 2, 258, 47468, 'Phase 3', 'Shaman', 'Restoration', 'Neck', 'Horde', 'Cry of the Val''kyr'),
(7, 2, 2, 0, 258, 46204, 'Phase 3', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Conqueror''s Worldbreaker Spaulders'),
(7, 2, 4, 0, 258, 46198, 'Phase 3', 'Shaman', 'Restoration', 'Chest', 'Both', 'Conqueror''s Worldbreaker Tunic'),
(7, 2, 5, 0, 258, 46991, 'Phase 3', 'Shaman', 'Restoration', 'Waist', 'Both', 'Belt of the Ice Burrower'),
(7, 2, 6, 1, 258, 47190, 'Phase 3', 'Shaman', 'Restoration', 'Legs', 'Alliance', 'Legwraps of the Awakening'),
(7, 2, 6, 2, 258, 47479, 'Phase 3', 'Shaman', 'Restoration', 'Legs', 'Horde', 'Leggings of the Awakening'),
(7, 2, 7, 1, 258, 47099, 'Phase 3', 'Shaman', 'Restoration', 'Feet', 'Alliance', 'Boots of Tremoring Earth'),
(7, 2, 7, 2, 258, 47456, 'Phase 3', 'Shaman', 'Restoration', 'Feet', 'Horde', 'Sabatons of Tremoring Earth'),
(7, 2, 8, 0, 258, 45460, 'Phase 3', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bindings of Winter Gale'),
(7, 2, 9, 0, 258, 46199, 'Phase 3', 'Shaman', 'Restoration', 'Hands', 'Both', 'Conqueror''s Worldbreaker Handguards'),
(7, 2, 10, 1, 258, 47224, 'Phase 3', 'Shaman', 'Restoration', 'Finger1', 'Alliance', 'Ring of the Darkmender'),
(7, 2, 10, 2, 258, 47439, 'Phase 3', 'Shaman', 'Restoration', 'Finger1', 'Horde', 'Circle of the Darkmender'),
(7, 2, 12, 1, 258, 47059, 'Phase 3', 'Shaman', 'Restoration', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(7, 2, 12, 2, 258, 47432, 'Phase 3', 'Shaman', 'Restoration', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(7, 2, 14, 1, 258, 47552, 'Phase 3', 'Shaman', 'Restoration', 'Back', 'Alliance', 'Jaina''s Radiance'),
(7, 2, 14, 2, 258, 47551, 'Phase 3', 'Shaman', 'Restoration', 'Back', 'Horde', 'Aethas'' Intensity'),
(7, 2, 15, 0, 258, 46017, 'Phase 3', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 264, 51247, 'Phase 4', 'Shaman', 'Restoration', 'Head', 'Both', 'Sanctified Frost Witch''s Headpiece'),
(7, 2, 1, 0, 264, 50724, 'Phase 4', 'Shaman', 'Restoration', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(7, 2, 2, 0, 264, 51245, 'Phase 4', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Sanctified Frost Witch''s Spaulders'),
(7, 2, 4, 0, 264, 51249, 'Phase 4', 'Shaman', 'Restoration', 'Chest', 'Both', 'Sanctified Frost Witch''s Tunic'),
(7, 2, 5, 0, 264, 50613, 'Phase 4', 'Shaman', 'Restoration', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(7, 2, 6, 0, 264, 51246, 'Phase 4', 'Shaman', 'Restoration', 'Legs', 'Both', 'Sanctified Frost Witch''s Legguards'),
(7, 2, 7, 0, 264, 50699, 'Phase 4', 'Shaman', 'Restoration', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(7, 2, 8, 0, 264, 50687, 'Phase 4', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bloodsunder''s Bracers'),
(7, 2, 9, 0, 264, 50703, 'Phase 4', 'Shaman', 'Restoration', 'Hands', 'Both', 'Unclean Surgical Gloves'),
(7, 2, 10, 0, 264, 50400, 'Phase 4', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ashen Band of Endless Wisdom'),
(7, 2, 12, 1, 264, 47059, 'Phase 4', 'Shaman', 'Restoration', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(7, 2, 12, 2, 264, 47432, 'Phase 4', 'Shaman', 'Restoration', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(7, 2, 14, 0, 264, 50628, 'Phase 4', 'Shaman', 'Restoration', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(7, 2, 15, 0, 264, 46017, 'Phase 4', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(7, 2, 17, 0, 264, 50464, 'Phase 4', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Totem of the Surging Sea');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(7, 2, 0, 0, 290, 51247, 'Phase 5', 'Shaman', 'Restoration', 'Head', 'Both', 'Sanctified Frost Witch''s Headpiece'),
(7, 2, 1, 0, 290, 50182, 'Phase 5', 'Shaman', 'Restoration', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(7, 2, 2, 0, 290, 51245, 'Phase 5', 'Shaman', 'Restoration', 'Shoulders', 'Both', 'Sanctified Frost Witch''s Spaulders'),
(7, 2, 4, 0, 290, 51249, 'Phase 5', 'Shaman', 'Restoration', 'Chest', 'Both', 'Sanctified Frost Witch''s Tunic'),
(7, 2, 5, 0, 290, 54587, 'Phase 5', 'Shaman', 'Restoration', 'Waist', 'Both', 'Split Shape Belt'),
(7, 2, 6, 0, 290, 51246, 'Phase 5', 'Shaman', 'Restoration', 'Legs', 'Both', 'Sanctified Frost Witch''s Legguards'),
(7, 2, 7, 0, 290, 50699, 'Phase 5', 'Shaman', 'Restoration', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(7, 2, 8, 0, 290, 50687, 'Phase 5', 'Shaman', 'Restoration', 'Wrists', 'Both', 'Bloodsunder''s Bracers'),
(7, 2, 9, 0, 290, 50703, 'Phase 5', 'Shaman', 'Restoration', 'Hands', 'Both', 'Unclean Surgical Gloves'),
(7, 2, 10, 0, 290, 50664, 'Phase 5', 'Shaman', 'Restoration', 'Finger1', 'Both', 'Ring of Rapid Ascent'),
(7, 2, 11, 0, 290, 50400, 'Phase 5', 'Shaman', 'Restoration', 'Finger2', 'Both', 'Ashen Band of Endless Wisdom'),
(7, 2, 12, 0, 290, 50366, 'Phase 5', 'Shaman', 'Restoration', 'Trinket1', 'Both', 'Althor''s Abacus'),
(7, 2, 13, 0, 290, 54589, 'Phase 5', 'Shaman', 'Restoration', 'Trinket2', 'Both', 'Glowing Twilight Scale'),
(7, 2, 14, 0, 290, 54583, 'Phase 5', 'Shaman', 'Restoration', 'Back', 'Both', 'Cloak of Burning Dusk'),
(7, 2, 15, 0, 290, 46017, 'Phase 5', 'Shaman', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(7, 2, 16, 0, 290, 50616, 'Phase 5', 'Shaman', 'Restoration', 'OffHand', 'Both', 'Bulwark of Smouldering Steel'),
(7, 2, 17, 0, 290, 50458, 'Phase 5', 'Shaman', 'Restoration', 'Ranged', 'Both', 'Bizuri''s Totem of Shattered Ice');


-- ============================================================
-- Mage (8)
-- ============================================================
-- Arcane (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 66, 10504, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Head', 'Both', 'Green Lens'),
(8, 0, 1, 0, 66, 12103, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Neck', 'Both', 'Star of Mystaria'),
(8, 0, 2, 0, 66, 11782, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Boreal Mantle'),
(8, 0, 4, 0, 66, 14152, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 0, 5, 0, 66, 11662, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 0, 6, 0, 66, 13170, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Legs', 'Both', 'Skyshroud Leggings'),
(8, 0, 7, 0, 66, 11822, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Feet', 'Both', 'Omnicast Boots'),
(8, 0, 8, 0, 66, 11766, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 0, 9, 0, 66, 13253, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Hands', 'Both', 'Hands of Power'),
(8, 0, 10, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Finger1', 'Both', 'Freezing Band'),
(8, 0, 11, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Finger2', 'Both', 'Freezing Band'),
(8, 0, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 0, 13, 0, 66, 13968, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 0, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Back', 'Both', 'Archivist Cape'),
(8, 0, 15, 0, 66, 13964, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'MainHand', 'Both', 'Witchblade'),
(8, 0, 16, 0, 66, 10796, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'OffHand', 'Both', 'Drakestone'),
(8, 0, 17, 0, 66, 13938, 'Phase 1 (Pre-Raid)', 'Mage', 'Arcane', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 76, 23318, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Head', 'Both', 'Lieutenant Commander''s Silk Cowl'),
(8, 0, 1, 0, 76, 12103, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Neck', 'Both', 'Star of Mystaria'),
(8, 0, 2, 0, 76, 23319, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 0, 4, 0, 76, 14152, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 0, 5, 0, 76, 11662, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 0, 6, 0, 76, 23304, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Legs', 'Both', 'Knight-Captain''s Silk Legguards'),
(8, 0, 7, 0, 76, 23291, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 0, 8, 0, 76, 11766, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 0, 9, 0, 76, 13253, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Hands', 'Both', 'Hands of Power'),
(8, 0, 10, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Finger1', 'Both', 'Freezing Band'),
(8, 0, 11, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Finger2', 'Both', 'Freezing Band'),
(8, 0, 12, 0, 76, 12930, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 0, 13, 0, 76, 13968, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 0, 14, 0, 76, 13386, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Back', 'Both', 'Archivist Cape'),
(8, 0, 15, 0, 76, 13964, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'MainHand', 'Both', 'Witchblade'),
(8, 0, 16, 0, 76, 10796, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'OffHand', 'Both', 'Drakestone'),
(8, 0, 17, 0, 76, 13938, 'Phase 2 (Pre-Raid)', 'Mage', 'Arcane', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 78, 16914, 'Phase 2', 'Mage', 'Arcane', 'Head', 'Both', 'Netherwind Crown'),
(8, 0, 1, 0, 78, 18814, 'Phase 2', 'Mage', 'Arcane', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 0, 2, 0, 78, 23319, 'Phase 2', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 0, 4, 0, 78, 14152, 'Phase 2', 'Mage', 'Arcane', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 0, 5, 0, 78, 19136, 'Phase 2', 'Mage', 'Arcane', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 0, 6, 0, 78, 16915, 'Phase 2', 'Mage', 'Arcane', 'Legs', 'Both', 'Netherwind Pants'),
(8, 0, 7, 0, 78, 23291, 'Phase 2', 'Mage', 'Arcane', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 0, 8, 0, 78, 11766, 'Phase 2', 'Mage', 'Arcane', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 0, 9, 0, 78, 13253, 'Phase 2', 'Mage', 'Arcane', 'Hands', 'Both', 'Hands of Power'),
(8, 0, 10, 0, 78, 19147, 'Phase 2', 'Mage', 'Arcane', 'Finger1', 'Both', 'Ring of Spell Power'),
(8, 0, 11, 0, 78, 19147, 'Phase 2', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 0, 12, 0, 78, 12930, 'Phase 2', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 0, 13, 0, 78, 18820, 'Phase 2', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(8, 0, 14, 0, 78, 13386, 'Phase 2', 'Mage', 'Arcane', 'Back', 'Both', 'Archivist Cape'),
(8, 0, 15, 0, 78, 17103, 'Phase 2', 'Mage', 'Arcane', 'MainHand', 'Both', 'Azuresong Mageblade'),
(8, 0, 16, 0, 78, 10796, 'Phase 2', 'Mage', 'Arcane', 'OffHand', 'Both', 'Drakestone'),
(8, 0, 17, 0, 78, 19130, 'Phase 2', 'Mage', 'Arcane', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 83, 19375, 'Phase 3', 'Mage', 'Arcane', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 0, 1, 0, 83, 18814, 'Phase 3', 'Mage', 'Arcane', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 0, 2, 0, 83, 19370, 'Phase 3', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 0, 4, 0, 83, 14152, 'Phase 3', 'Mage', 'Arcane', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 0, 5, 0, 83, 19136, 'Phase 3', 'Mage', 'Arcane', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 0, 6, 0, 83, 16915, 'Phase 3', 'Mage', 'Arcane', 'Legs', 'Both', 'Netherwind Pants'),
(8, 0, 7, 0, 83, 19438, 'Phase 3', 'Mage', 'Arcane', 'Feet', 'Both', 'Ringo''s Blizzard Boots'),
(8, 0, 8, 0, 83, 19374, 'Phase 3', 'Mage', 'Arcane', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(8, 0, 9, 0, 83, 16913, 'Phase 3', 'Mage', 'Arcane', 'Hands', 'Both', 'Netherwind Gloves'),
(8, 0, 11, 0, 83, 19147, 'Phase 3', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 0, 12, 0, 83, 19379, 'Phase 3', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 0, 13, 0, 83, 19339, 'Phase 3', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 0, 14, 0, 83, 19378, 'Phase 3', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of the Brood Lord'),
(8, 0, 15, 0, 83, 19356, 'Phase 3', 'Mage', 'Arcane', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(8, 0, 17, 0, 83, 19130, 'Phase 3', 'Mage', 'Arcane', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 88, 19375, 'Phase 5', 'Mage', 'Arcane', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 0, 1, 0, 88, 21608, 'Phase 5', 'Mage', 'Arcane', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(8, 0, 2, 0, 88, 19370, 'Phase 5', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 0, 4, 0, 88, 19145, 'Phase 5', 'Mage', 'Arcane', 'Chest', 'Both', 'Robe of Volatile Power'),
(8, 0, 5, 0, 88, 22730, 'Phase 5', 'Mage', 'Arcane', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 0, 6, 0, 88, 21461, 'Phase 5', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of the Black Blizzard'),
(8, 0, 7, 0, 88, 21344, 'Phase 5', 'Mage', 'Arcane', 'Feet', 'Both', 'Enigma Boots'),
(8, 0, 8, 0, 88, 21186, 'Phase 5', 'Mage', 'Arcane', 'Wrists', 'Both', 'Rockfury Bracers'),
(8, 0, 9, 0, 88, 21585, 'Phase 5', 'Mage', 'Arcane', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 0, 10, 0, 88, 21836, 'Phase 5', 'Mage', 'Arcane', 'Finger1', 'Both', 'Ritssyn''s Ring of Chaos'),
(8, 0, 11, 0, 88, 21709, 'Phase 5', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ring of the Fallen God'),
(8, 0, 12, 0, 88, 19379, 'Phase 5', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 0, 13, 0, 88, 19339, 'Phase 5', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 0, 14, 0, 88, 22731, 'Phase 5', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of the Devoured'),
(8, 0, 15, 0, 88, 21622, 'Phase 5', 'Mage', 'Arcane', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(8, 0, 16, 0, 88, 21597, 'Phase 5', 'Mage', 'Arcane', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(8, 0, 17, 0, 88, 21603, 'Phase 5', 'Mage', 'Arcane', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 92, 22498, 'Phase 6', 'Mage', 'Arcane', 'Head', 'Both', 'Frostfire Circlet'),
(8, 0, 1, 0, 92, 23057, 'Phase 6', 'Mage', 'Arcane', 'Neck', 'Both', 'Gem of Trapped Innocents'),
(8, 0, 2, 0, 92, 22983, 'Phase 6', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Rime Covered Mantle'),
(8, 0, 4, 0, 92, 22496, 'Phase 6', 'Mage', 'Arcane', 'Chest', 'Both', 'Frostfire Robe'),
(8, 0, 5, 0, 92, 22730, 'Phase 6', 'Mage', 'Arcane', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 0, 6, 0, 92, 23070, 'Phase 6', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of Polarity'),
(8, 0, 7, 0, 92, 22500, 'Phase 6', 'Mage', 'Arcane', 'Feet', 'Both', 'Frostfire Sandals'),
(8, 0, 8, 0, 92, 23021, 'Phase 6', 'Mage', 'Arcane', 'Wrists', 'Both', 'The Soul Harvester''s Bindings'),
(8, 0, 9, 0, 92, 21585, 'Phase 6', 'Mage', 'Arcane', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 0, 10, 0, 92, 23237, 'Phase 6', 'Mage', 'Arcane', 'Finger1', 'Both', 'Ring of the Eternal Flame'),
(8, 0, 11, 0, 92, 23062, 'Phase 6', 'Mage', 'Arcane', 'Finger2', 'Both', 'Frostfire Ring'),
(8, 0, 12, 0, 92, 19379, 'Phase 6', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 0, 13, 0, 92, 23046, 'Phase 6', 'Mage', 'Arcane', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(8, 0, 14, 0, 92, 23050, 'Phase 6', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of the Necropolis'),
(8, 0, 15, 0, 92, 22807, 'Phase 6', 'Mage', 'Arcane', 'MainHand', 'Both', 'Wraith Blade'),
(8, 0, 16, 0, 92, 23049, 'Phase 6', 'Mage', 'Arcane', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(8, 0, 17, 0, 92, 22821, 'Phase 6', 'Mage', 'Arcane', 'Ranged', 'Both', 'Doomfinger');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 120, 28278, 'Pre-Raid', 'Mage', 'Arcane', 'Head', 'Both', 'Incanter''s Cowl'),
(8, 0, 1, 0, 120, 28134, 'Pre-Raid', 'Mage', 'Arcane', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(8, 0, 2, 0, 120, 27796, 'Pre-Raid', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mana-Etched Spaulders'),
(8, 0, 4, 0, 120, 21848, 'Pre-Raid', 'Mage', 'Arcane', 'Chest', 'Both', 'Spellfire Robe'),
(8, 0, 5, 0, 120, 21846, 'Pre-Raid', 'Mage', 'Arcane', 'Waist', 'Both', 'Spellfire Belt'),
(8, 0, 6, 0, 120, 30532, 'Pre-Raid', 'Mage', 'Arcane', 'Legs', 'Both', 'Kirin Tor Master''s Trousers'),
(8, 0, 7, 0, 120, 28406, 'Pre-Raid', 'Mage', 'Arcane', 'Feet', 'Both', 'Sigil-Laced Boots'),
(8, 0, 8, 0, 120, 29240, 'Pre-Raid', 'Mage', 'Arcane', 'Wrists', 'Both', 'Bands of Negation'),
(8, 0, 9, 0, 120, 21847, 'Pre-Raid', 'Mage', 'Arcane', 'Hands', 'Both', 'Spellfire Gloves'),
(8, 0, 10, 0, 120, 29172, 'Pre-Raid', 'Mage', 'Arcane', 'Finger1', 'Both', 'Ashyen''s Gift'),
(8, 0, 11, 0, 120, 28227, 'Pre-Raid', 'Mage', 'Arcane', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(8, 0, 12, 0, 120, 29370, 'Pre-Raid', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 0, 13, 0, 120, 27683, 'Pre-Raid', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(8, 0, 14, 0, 120, 27981, 'Pre-Raid', 'Mage', 'Arcane', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(8, 0, 15, 0, 120, 23554, 'Pre-Raid', 'Mage', 'Arcane', 'MainHand', 'Both', 'Eternium Runed Blade'),
(8, 0, 16, 0, 120, 29271, 'Pre-Raid', 'Mage', 'Arcane', 'OffHand', 'Both', 'Talisman of Kalecgos'),
(8, 0, 17, 0, 120, 29350, 'Pre-Raid', 'Mage', 'Arcane', 'Ranged', 'Both', 'The Black Stalk');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 125, 29076, 'Phase 1', 'Mage', 'Arcane', 'Head', 'Both', 'Collar of the Aldor'),
(8, 0, 1, 0, 125, 28762, 'Phase 1', 'Mage', 'Arcane', 'Neck', 'Both', 'Adornment of Stolen Souls'),
(8, 0, 2, 0, 125, 29079, 'Phase 1', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Pauldrons of the Aldor'),
(8, 0, 4, 0, 125, 21848, 'Phase 1', 'Mage', 'Arcane', 'Chest', 'Both', 'Spellfire Robe'),
(8, 0, 5, 0, 125, 21846, 'Phase 1', 'Mage', 'Arcane', 'Waist', 'Both', 'Spellfire Belt'),
(8, 0, 6, 0, 125, 29078, 'Phase 1', 'Mage', 'Arcane', 'Legs', 'Both', 'Legwraps of the Aldor'),
(8, 0, 7, 0, 125, 28517, 'Phase 1', 'Mage', 'Arcane', 'Feet', 'Both', 'Boots of Foretelling'),
(8, 0, 8, 0, 125, 28515, 'Phase 1', 'Mage', 'Arcane', 'Wrists', 'Both', 'Bands of Nefarious Deeds'),
(8, 0, 9, 0, 125, 21847, 'Phase 1', 'Mage', 'Arcane', 'Hands', 'Both', 'Spellfire Gloves'),
(8, 0, 10, 0, 125, 28793, 'Phase 1', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of Crimson Fury'),
(8, 0, 11, 0, 125, 29287, 'Phase 1', 'Mage', 'Arcane', 'Finger2', 'Both', 'Violet Signet of the Archmage'),
(8, 0, 12, 0, 125, 29370, 'Phase 1', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 0, 13, 0, 125, 28785, 'Phase 1', 'Mage', 'Arcane', 'Trinket2', 'Both', 'The Lightning Capacitor'),
(8, 0, 14, 0, 125, 28766, 'Phase 1', 'Mage', 'Arcane', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(8, 0, 15, 0, 125, 28770, 'Phase 1', 'Mage', 'Arcane', 'MainHand', 'Both', 'Nathrezim Mindblade'),
(8, 0, 16, 0, 125, 29271, 'Phase 1', 'Mage', 'Arcane', 'OffHand', 'Both', 'Talisman of Kalecgos'),
(8, 0, 17, 0, 125, 28783, 'Phase 1', 'Mage', 'Arcane', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 141, 30206, 'Phase 2', 'Mage', 'Arcane', 'Head', 'Both', 'Cowl of Tirisfal'),
(8, 0, 1, 0, 141, 30015, 'Phase 2', 'Mage', 'Arcane', 'Neck', 'Both', 'The Sun King''s Talisman'),
(8, 0, 2, 0, 141, 30210, 'Phase 2', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mantle of Tirisfal'),
(8, 0, 4, 0, 141, 30196, 'Phase 2', 'Mage', 'Arcane', 'Chest', 'Both', 'Robes of Tirisfal'),
(8, 0, 5, 0, 141, 30038, 'Phase 2', 'Mage', 'Arcane', 'Waist', 'Both', 'Belt of Blasting'),
(8, 0, 6, 0, 141, 30207, 'Phase 2', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of Tirisfal'),
(8, 0, 7, 0, 141, 30067, 'Phase 2', 'Mage', 'Arcane', 'Feet', 'Both', 'Velvet Boots of the Guardian'),
(8, 0, 8, 0, 141, 29918, 'Phase 2', 'Mage', 'Arcane', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(8, 0, 9, 0, 141, 29987, 'Phase 2', 'Mage', 'Arcane', 'Hands', 'Both', 'Gauntlets of the Sun King'),
(8, 0, 10, 0, 141, 29302, 'Phase 2', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of Eternity'),
(8, 0, 11, 0, 141, 29287, 'Phase 2', 'Mage', 'Arcane', 'Finger2', 'Both', 'Violet Signet of the Archmage'),
(8, 0, 12, 0, 141, 29370, 'Phase 2', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 0, 13, 0, 141, 30720, 'Phase 2', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Serpent-Coil Braid'),
(8, 0, 14, 0, 141, 29992, 'Phase 2', 'Mage', 'Arcane', 'Back', 'Both', 'Royal Cloak of the Sunstriders'),
(8, 0, 15, 0, 141, 30095, 'Phase 2', 'Mage', 'Arcane', 'MainHand', 'Both', 'Fang of the Leviathan'),
(8, 0, 16, 0, 141, 30049, 'Phase 2', 'Mage', 'Arcane', 'OffHand', 'Both', 'Fathomstone'),
(8, 0, 17, 0, 141, 28783, 'Phase 2', 'Mage', 'Arcane', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 156, 30206, 'Phase 3', 'Mage', 'Arcane', 'Head', 'Both', 'Cowl of Tirisfal'),
(8, 0, 1, 0, 156, 30015, 'Phase 3', 'Mage', 'Arcane', 'Neck', 'Both', 'The Sun King''s Talisman'),
(8, 0, 2, 0, 156, 30210, 'Phase 3', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mantle of Tirisfal'),
(8, 0, 4, 0, 156, 30196, 'Phase 3', 'Mage', 'Arcane', 'Chest', 'Both', 'Robes of Tirisfal'),
(8, 0, 5, 0, 156, 30888, 'Phase 3', 'Mage', 'Arcane', 'Waist', 'Both', 'Anetheron''s Noose'),
(8, 0, 6, 0, 156, 31058, 'Phase 3', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of the Tempest'),
(8, 0, 7, 0, 156, 32239, 'Phase 3', 'Mage', 'Arcane', 'Feet', 'Both', 'Slippers of the Seacaller'),
(8, 0, 8, 0, 156, 30870, 'Phase 3', 'Mage', 'Arcane', 'Wrists', 'Both', 'Cuffs of Devastation'),
(8, 0, 9, 0, 156, 30205, 'Phase 3', 'Mage', 'Arcane', 'Hands', 'Both', 'Gloves of Tirisfal'),
(8, 0, 10, 0, 156, 29305, 'Phase 3', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of the Eternal Sage'),
(8, 0, 11, 0, 156, 32527, 'Phase 3', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(8, 0, 12, 0, 156, 32488, 'Phase 3', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Ashtongue Talisman of Insight'),
(8, 0, 13, 0, 156, 32483, 'Phase 3', 'Mage', 'Arcane', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(8, 0, 14, 0, 156, 32331, 'Phase 3', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of the Illidari Council'),
(8, 0, 15, 0, 156, 30910, 'Phase 3', 'Mage', 'Arcane', 'MainHand', 'Both', 'Tempest of Chaos'),
(8, 0, 16, 0, 156, 30872, 'Phase 3', 'Mage', 'Arcane', 'OffHand', 'Both', 'Chronicle of Dark Secrets'),
(8, 0, 17, 0, 156, 28783, 'Phase 3', 'Mage', 'Arcane', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 164, 30206, 'Phase 4', 'Mage', 'Arcane', 'Head', 'Both', 'Cowl of Tirisfal'),
(8, 0, 1, 0, 164, 30015, 'Phase 4', 'Mage', 'Arcane', 'Neck', 'Both', 'The Sun King''s Talisman'),
(8, 0, 2, 0, 164, 30210, 'Phase 4', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Mantle of Tirisfal'),
(8, 0, 4, 0, 164, 30196, 'Phase 4', 'Mage', 'Arcane', 'Chest', 'Both', 'Robes of Tirisfal'),
(8, 0, 5, 0, 164, 30888, 'Phase 4', 'Mage', 'Arcane', 'Waist', 'Both', 'Anetheron''s Noose'),
(8, 0, 6, 0, 164, 31058, 'Phase 4', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of the Tempest'),
(8, 0, 7, 0, 164, 32239, 'Phase 4', 'Mage', 'Arcane', 'Feet', 'Both', 'Slippers of the Seacaller'),
(8, 0, 8, 0, 164, 30870, 'Phase 4', 'Mage', 'Arcane', 'Wrists', 'Both', 'Cuffs of Devastation'),
(8, 0, 9, 0, 164, 30205, 'Phase 4', 'Mage', 'Arcane', 'Hands', 'Both', 'Gloves of Tirisfal'),
(8, 0, 10, 0, 164, 29305, 'Phase 4', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of the Eternal Sage'),
(8, 0, 11, 0, 164, 32527, 'Phase 4', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(8, 0, 12, 0, 164, 32488, 'Phase 4', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Ashtongue Talisman of Insight'),
(8, 0, 13, 0, 164, 32483, 'Phase 4', 'Mage', 'Arcane', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(8, 0, 14, 0, 164, 32331, 'Phase 4', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of the Illidari Council'),
(8, 0, 15, 0, 164, 30910, 'Phase 4', 'Mage', 'Arcane', 'MainHand', 'Both', 'Tempest of Chaos'),
(8, 0, 16, 0, 164, 30872, 'Phase 4', 'Mage', 'Arcane', 'OffHand', 'Both', 'Chronicle of Dark Secrets'),
(8, 0, 17, 0, 164, 33192, 'Phase 4', 'Mage', 'Arcane', 'Ranged', 'Both', 'Carved Witch Doctor''s Stick');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 200, 37294, 'Pre-Raid', 'Mage', 'Arcane', 'Head', 'Both', 'Crown of Unbridled Magic'),
(8, 0, 1, 0, 200, 39472, 'Pre-Raid', 'Mage', 'Arcane', 'Neck', 'Both', 'Chain of Latent Energies'),
(8, 0, 2, 0, 200, 37673, 'Pre-Raid', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(8, 0, 4, 0, 200, 39492, 'Pre-Raid', 'Mage', 'Arcane', 'Chest', 'Both', 'Heroes'' Frostfire Robe'),
(8, 0, 5, 0, 200, 40696, 'Pre-Raid', 'Mage', 'Arcane', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(8, 0, 6, 0, 200, 37854, 'Pre-Raid', 'Mage', 'Arcane', 'Legs', 'Both', 'Woven Bracae Leggings'),
(8, 0, 7, 0, 200, 44202, 'Pre-Raid', 'Mage', 'Arcane', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(8, 0, 8, 0, 200, 37361, 'Pre-Raid', 'Mage', 'Arcane', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(8, 0, 9, 0, 200, 39495, 'Pre-Raid', 'Mage', 'Arcane', 'Hands', 'Both', 'Heroes'' Frostfire Gloves'),
(8, 0, 10, 0, 200, 37694, 'Pre-Raid', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of Guile'),
(8, 0, 12, 0, 200, 37660, 'Pre-Raid', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Forge Ember'),
(8, 0, 14, 0, 200, 41610, 'Pre-Raid', 'Mage', 'Arcane', 'Back', 'Both', 'Deathchill Cloak'),
(8, 0, 15, 0, 200, 37360, 'Pre-Raid', 'Mage', 'Arcane', 'MainHand', 'Both', 'Staff of Draconic Combat'),
(8, 0, 17, 0, 200, 37177, 'Pre-Raid', 'Mage', 'Arcane', 'Ranged', 'Both', 'Wand of the San''layn');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 1, 0, 224, 44661, 'Phase 1', 'Mage', 'Arcane', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(8, 0, 2, 0, 224, 40419, 'Phase 1', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Valorous Frostfire Shoulderpads'),
(8, 0, 4, 0, 224, 44002, 'Phase 1', 'Mage', 'Arcane', 'Chest', 'Both', 'The Sanctum''s Flowing Vestments'),
(8, 0, 5, 0, 224, 40561, 'Phase 1', 'Mage', 'Arcane', 'Waist', 'Both', 'Leash of Heedless Magic'),
(8, 0, 6, 0, 224, 40417, 'Phase 1', 'Mage', 'Arcane', 'Legs', 'Both', 'Valorous Frostfire Leggings'),
(8, 0, 7, 0, 224, 40558, 'Phase 1', 'Mage', 'Arcane', 'Feet', 'Both', 'Arcanic Tramplers'),
(8, 0, 8, 0, 224, 44008, 'Phase 1', 'Mage', 'Arcane', 'Wrists', 'Both', 'Unsullied Cuffs'),
(8, 0, 9, 0, 224, 40415, 'Phase 1', 'Mage', 'Arcane', 'Hands', 'Both', 'Valorous Frostfire Gloves'),
(8, 0, 10, 0, 224, 40719, 'Phase 1', 'Mage', 'Arcane', 'Finger1', 'Both', 'Band of Channeled Magic'),
(8, 0, 12, 0, 224, 40255, 'Phase 1', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Dying Curse'),
(8, 0, 14, 0, 224, 44005, 'Phase 1', 'Mage', 'Arcane', 'Back', 'Both', 'Pennant Cloak'),
(8, 0, 15, 0, 224, 40396, 'Phase 1', 'Mage', 'Arcane', 'MainHand', 'Both', 'The Turning Tide'),
(8, 0, 17, 0, 224, 39426, 'Phase 1', 'Mage', 'Arcane', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 245, 46129, 'Phase 2', 'Mage', 'Arcane', 'Head', 'Both', 'Conqueror''s Kirin Tor Hood'),
(8, 0, 1, 0, 245, 45243, 'Phase 2', 'Mage', 'Arcane', 'Neck', 'Both', 'Sapphire Amulet of Renewal'),
(8, 0, 2, 0, 245, 46134, 'Phase 2', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Conqueror''s Kirin Tor Shoulderpads'),
(8, 0, 4, 0, 245, 46130, 'Phase 2', 'Mage', 'Arcane', 'Chest', 'Both', 'Conqueror''s Kirin Tor Tunic'),
(8, 0, 5, 0, 245, 45557, 'Phase 2', 'Mage', 'Arcane', 'Waist', 'Both', 'Sash of Ancient Power'),
(8, 0, 6, 0, 245, 46133, 'Phase 2', 'Mage', 'Arcane', 'Legs', 'Both', 'Conqueror''s Kirin Tor Leggings'),
(8, 0, 7, 0, 245, 45135, 'Phase 2', 'Mage', 'Arcane', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(8, 0, 8, 0, 245, 45446, 'Phase 2', 'Mage', 'Arcane', 'Wrists', 'Both', 'Grasps of Reason'),
(8, 0, 9, 0, 245, 45665, 'Phase 2', 'Mage', 'Arcane', 'Hands', 'Both', 'Pharos Gloves'),
(8, 0, 10, 0, 245, 46046, 'Phase 2', 'Mage', 'Arcane', 'Finger1', 'Both', 'Nebula Band'),
(8, 0, 12, 0, 245, 45518, 'Phase 2', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Flare of the Heavens'),
(8, 0, 14, 0, 245, 45618, 'Phase 2', 'Mage', 'Arcane', 'Back', 'Both', 'Sunglimmer Cloak'),
(8, 0, 15, 0, 245, 45457, 'Phase 2', 'Mage', 'Arcane', 'MainHand', 'Both', 'Staff of Endless Winter'),
(8, 0, 17, 0, 245, 45511, 'Phase 2', 'Mage', 'Arcane', 'Ranged', 'Both', 'Scepter of Lost Souls');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 258, 47761, 'Phase 3', 'Mage', 'Arcane', 'Head', 'Both', 'Hood of Triumph'),
(8, 0, 2, 0, 258, 47758, 'Phase 3', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Shoulderpads of Triumph'),
(8, 0, 4, 1, 258, 46993, 'Phase 3', 'Mage', 'Arcane', 'Chest', 'Alliance', 'Flowing Vestments of Ascent'),
(8, 0, 4, 2, 258, 47425, 'Phase 3', 'Mage', 'Arcane', 'Chest', 'Horde', 'Flowing Robes of Ascent'),
(8, 0, 5, 1, 258, 47419, 'Phase 3', 'Mage', 'Arcane', 'Waist', 'Alliance', 'Belt of the Tenebrous Mist'),
(8, 0, 5, 2, 258, 46973, 'Phase 3', 'Mage', 'Arcane', 'Waist', 'Horde', 'Cord of the Tenebrous Mist'),
(8, 0, 6, 0, 258, 47760, 'Phase 3', 'Mage', 'Arcane', 'Legs', 'Both', 'Leggings of Triumph'),
(8, 0, 7, 1, 258, 47097, 'Phase 3', 'Mage', 'Arcane', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(8, 0, 7, 2, 258, 47454, 'Phase 3', 'Mage', 'Arcane', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(8, 0, 8, 1, 258, 47208, 'Phase 3', 'Mage', 'Arcane', 'Wrists', 'Alliance', 'Armbands of the Ashen Saint'),
(8, 0, 8, 2, 258, 47485, 'Phase 3', 'Mage', 'Arcane', 'Wrists', 'Horde', 'Bindings of the Ashen Saint'),
(8, 0, 9, 0, 258, 47762, 'Phase 3', 'Mage', 'Arcane', 'Hands', 'Both', 'Gauntlets of Triumph'),
(8, 0, 10, 1, 258, 47489, 'Phase 3', 'Mage', 'Arcane', 'Finger1', 'Alliance', 'Lurid Manifestation'),
(8, 0, 10, 2, 258, 47237, 'Phase 3', 'Mage', 'Arcane', 'Finger1', 'Horde', 'Band of Deplorable Violence'),
(8, 0, 12, 1, 258, 47188, 'Phase 3', 'Mage', 'Arcane', 'Trinket1', 'Alliance', 'Reign of the Unliving'),
(8, 0, 12, 2, 258, 47477, 'Phase 3', 'Mage', 'Arcane', 'Trinket1', 'Horde', 'Reign of the Dead'),
(8, 0, 14, 1, 258, 47552, 'Phase 3', 'Mage', 'Arcane', 'Back', 'Alliance', 'Jaina''s Radiance'),
(8, 0, 14, 2, 258, 47551, 'Phase 3', 'Mage', 'Arcane', 'Back', 'Horde', 'Aethas'' Intensity'),
(8, 0, 17, 0, 258, 45294, 'Phase 3', 'Mage', 'Arcane', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 264, 51281, 'Phase 4', 'Mage', 'Arcane', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 0, 1, 0, 264, 50724, 'Phase 4', 'Mage', 'Arcane', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 0, 2, 0, 264, 51284, 'Phase 4', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 0, 4, 0, 264, 51283, 'Phase 4', 'Mage', 'Arcane', 'Chest', 'Both', 'Sanctified Bloodmage Robe'),
(8, 0, 5, 0, 264, 50613, 'Phase 4', 'Mage', 'Arcane', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 0, 6, 0, 264, 51282, 'Phase 4', 'Mage', 'Arcane', 'Legs', 'Both', 'Sanctified Bloodmage Leggings'),
(8, 0, 7, 0, 264, 50699, 'Phase 4', 'Mage', 'Arcane', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 0, 8, 0, 264, 50651, 'Phase 4', 'Mage', 'Arcane', 'Wrists', 'Both', 'The Lady''s Brittle Bracers'),
(8, 0, 9, 0, 264, 50722, 'Phase 4', 'Mage', 'Arcane', 'Hands', 'Both', 'San''layn Ritualist Gloves'),
(8, 0, 10, 0, 264, 50398, 'Phase 4', 'Mage', 'Arcane', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(8, 0, 12, 1, 264, 47188, 'Phase 4', 'Mage', 'Arcane', 'Trinket1', 'Alliance', 'Reign of the Unliving'),
(8, 0, 12, 2, 264, 47477, 'Phase 4', 'Mage', 'Arcane', 'Trinket1', 'Horde', 'Reign of the Dead'),
(8, 0, 14, 0, 264, 50628, 'Phase 4', 'Mage', 'Arcane', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(8, 0, 15, 0, 264, 50732, 'Phase 4', 'Mage', 'Arcane', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 0, 17, 0, 264, 50684, 'Phase 4', 'Mage', 'Arcane', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 0, 0, 0, 290, 51281, 'Phase 5', 'Mage', 'Arcane', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 0, 1, 0, 290, 50182, 'Phase 5', 'Mage', 'Arcane', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 0, 2, 0, 290, 51284, 'Phase 5', 'Mage', 'Arcane', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 0, 4, 0, 290, 51283, 'Phase 5', 'Mage', 'Arcane', 'Chest', 'Both', 'Sanctified Bloodmage Robe'),
(8, 0, 5, 0, 290, 50613, 'Phase 5', 'Mage', 'Arcane', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 0, 6, 0, 290, 50694, 'Phase 5', 'Mage', 'Arcane', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(8, 0, 7, 0, 290, 50699, 'Phase 5', 'Mage', 'Arcane', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 0, 8, 0, 290, 54582, 'Phase 5', 'Mage', 'Arcane', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(8, 0, 9, 0, 290, 51280, 'Phase 5', 'Mage', 'Arcane', 'Hands', 'Both', 'Sanctified Bloodmage Gloves'),
(8, 0, 10, 0, 290, 50614, 'Phase 5', 'Mage', 'Arcane', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(8, 0, 11, 0, 290, 50398, 'Phase 5', 'Mage', 'Arcane', 'Finger2', 'Both', 'Ashen Band of Endless Destruction'),
(8, 0, 12, 0, 290, 54588, 'Phase 5', 'Mage', 'Arcane', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(8, 0, 13, 0, 290, 50348, 'Phase 5', 'Mage', 'Arcane', 'Trinket2', 'Both', 'Dislodged Foreign Object'),
(8, 0, 14, 0, 290, 54583, 'Phase 5', 'Mage', 'Arcane', 'Back', 'Both', 'Cloak of Burning Dusk'),
(8, 0, 15, 0, 290, 50732, 'Phase 5', 'Mage', 'Arcane', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 0, 16, 0, 290, 50719, 'Phase 5', 'Mage', 'Arcane', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(8, 0, 17, 0, 290, 50684, 'Phase 5', 'Mage', 'Arcane', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- Fire (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 66, 10504, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Head', 'Both', 'Green Lens'),
(8, 1, 1, 0, 66, 12103, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Neck', 'Both', 'Star of Mystaria'),
(8, 1, 2, 0, 66, 11782, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Shoulders', 'Both', 'Boreal Mantle'),
(8, 1, 4, 0, 66, 14152, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 1, 5, 0, 66, 11662, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 1, 6, 0, 66, 13170, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Legs', 'Both', 'Skyshroud Leggings'),
(8, 1, 7, 0, 66, 11822, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Feet', 'Both', 'Omnicast Boots'),
(8, 1, 8, 0, 66, 11766, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 1, 9, 0, 66, 13253, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Hands', 'Both', 'Hands of Power'),
(8, 1, 10, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Finger1', 'Both', 'Freezing Band'),
(8, 1, 11, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Finger2', 'Both', 'Freezing Band'),
(8, 1, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 1, 13, 0, 66, 13968, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 1, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Back', 'Both', 'Archivist Cape'),
(8, 1, 15, 0, 66, 13964, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'MainHand', 'Both', 'Witchblade'),
(8, 1, 16, 0, 66, 10796, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'OffHand', 'Both', 'Drakestone'),
(8, 1, 17, 0, 66, 13938, 'Phase 1 (Pre-Raid)', 'Mage', 'Fire', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 76, 23318, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Head', 'Both', 'Lieutenant Commander''s Silk Cowl'),
(8, 1, 1, 0, 76, 12103, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Neck', 'Both', 'Star of Mystaria'),
(8, 1, 2, 0, 76, 23319, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 1, 4, 0, 76, 14152, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 1, 5, 0, 76, 11662, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 1, 6, 0, 76, 23304, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Legs', 'Both', 'Knight-Captain''s Silk Legguards'),
(8, 1, 7, 0, 76, 23291, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 1, 8, 0, 76, 11766, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 1, 9, 0, 76, 13253, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Hands', 'Both', 'Hands of Power'),
(8, 1, 10, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Finger1', 'Both', 'Freezing Band'),
(8, 1, 11, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Finger2', 'Both', 'Freezing Band'),
(8, 1, 12, 0, 76, 12930, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 1, 13, 0, 76, 13968, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 1, 14, 0, 76, 13386, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Back', 'Both', 'Archivist Cape'),
(8, 1, 15, 0, 76, 13964, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'MainHand', 'Both', 'Witchblade'),
(8, 1, 16, 0, 76, 10796, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'OffHand', 'Both', 'Drakestone'),
(8, 1, 17, 0, 76, 13938, 'Phase 2 (Pre-Raid)', 'Mage', 'Fire', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 78, 16914, 'Phase 2', 'Mage', 'Fire', 'Head', 'Both', 'Netherwind Crown'),
(8, 1, 1, 0, 78, 18814, 'Phase 2', 'Mage', 'Fire', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 1, 2, 0, 78, 23319, 'Phase 2', 'Mage', 'Fire', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 1, 4, 0, 78, 14152, 'Phase 2', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 1, 5, 0, 78, 19136, 'Phase 2', 'Mage', 'Fire', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 1, 6, 0, 78, 16915, 'Phase 2', 'Mage', 'Fire', 'Legs', 'Both', 'Netherwind Pants'),
(8, 1, 7, 0, 78, 23291, 'Phase 2', 'Mage', 'Fire', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 1, 8, 0, 78, 11766, 'Phase 2', 'Mage', 'Fire', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 1, 9, 0, 78, 13253, 'Phase 2', 'Mage', 'Fire', 'Hands', 'Both', 'Hands of Power'),
(8, 1, 10, 0, 78, 19147, 'Phase 2', 'Mage', 'Fire', 'Finger1', 'Both', 'Ring of Spell Power'),
(8, 1, 11, 0, 78, 19147, 'Phase 2', 'Mage', 'Fire', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 1, 12, 0, 78, 12930, 'Phase 2', 'Mage', 'Fire', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 1, 13, 0, 78, 18820, 'Phase 2', 'Mage', 'Fire', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(8, 1, 14, 0, 78, 13386, 'Phase 2', 'Mage', 'Fire', 'Back', 'Both', 'Archivist Cape'),
(8, 1, 15, 0, 78, 17103, 'Phase 2', 'Mage', 'Fire', 'MainHand', 'Both', 'Azuresong Mageblade'),
(8, 1, 16, 0, 78, 10796, 'Phase 2', 'Mage', 'Fire', 'OffHand', 'Both', 'Drakestone'),
(8, 1, 17, 0, 78, 19130, 'Phase 2', 'Mage', 'Fire', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 83, 19375, 'Phase 3', 'Mage', 'Fire', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 1, 1, 0, 83, 18814, 'Phase 3', 'Mage', 'Fire', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 1, 2, 0, 83, 19370, 'Phase 3', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 1, 4, 0, 83, 14152, 'Phase 3', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 1, 5, 0, 83, 19136, 'Phase 3', 'Mage', 'Fire', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 1, 6, 0, 83, 16915, 'Phase 3', 'Mage', 'Fire', 'Legs', 'Both', 'Netherwind Pants'),
(8, 1, 7, 0, 83, 19438, 'Phase 3', 'Mage', 'Fire', 'Feet', 'Both', 'Ringo''s Blizzard Boots'),
(8, 1, 8, 0, 83, 19374, 'Phase 3', 'Mage', 'Fire', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(8, 1, 9, 0, 83, 16913, 'Phase 3', 'Mage', 'Fire', 'Hands', 'Both', 'Netherwind Gloves'),
(8, 1, 11, 0, 83, 19147, 'Phase 3', 'Mage', 'Fire', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 1, 12, 0, 83, 19379, 'Phase 3', 'Mage', 'Fire', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 1, 13, 0, 83, 19339, 'Phase 3', 'Mage', 'Fire', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 1, 14, 0, 83, 19378, 'Phase 3', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of the Brood Lord'),
(8, 1, 15, 0, 83, 19356, 'Phase 3', 'Mage', 'Fire', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(8, 1, 17, 0, 83, 19130, 'Phase 3', 'Mage', 'Fire', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 88, 19375, 'Phase 5', 'Mage', 'Fire', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 1, 1, 0, 88, 21608, 'Phase 5', 'Mage', 'Fire', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(8, 1, 2, 0, 88, 19370, 'Phase 5', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 1, 4, 0, 88, 19145, 'Phase 5', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of Volatile Power'),
(8, 1, 5, 0, 88, 22730, 'Phase 5', 'Mage', 'Fire', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 1, 6, 0, 88, 21461, 'Phase 5', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of the Black Blizzard'),
(8, 1, 7, 0, 88, 21344, 'Phase 5', 'Mage', 'Fire', 'Feet', 'Both', 'Enigma Boots'),
(8, 1, 8, 0, 88, 21186, 'Phase 5', 'Mage', 'Fire', 'Wrists', 'Both', 'Rockfury Bracers'),
(8, 1, 9, 0, 88, 21585, 'Phase 5', 'Mage', 'Fire', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 1, 10, 0, 88, 21836, 'Phase 5', 'Mage', 'Fire', 'Finger1', 'Both', 'Ritssyn''s Ring of Chaos'),
(8, 1, 11, 0, 88, 21709, 'Phase 5', 'Mage', 'Fire', 'Finger2', 'Both', 'Ring of the Fallen God'),
(8, 1, 12, 0, 88, 19379, 'Phase 5', 'Mage', 'Fire', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 1, 13, 0, 88, 19339, 'Phase 5', 'Mage', 'Fire', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 1, 14, 0, 88, 22731, 'Phase 5', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of the Devoured'),
(8, 1, 15, 0, 88, 21622, 'Phase 5', 'Mage', 'Fire', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(8, 1, 16, 0, 88, 21597, 'Phase 5', 'Mage', 'Fire', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(8, 1, 17, 0, 88, 21603, 'Phase 5', 'Mage', 'Fire', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 92, 22498, 'Phase 6', 'Mage', 'Fire', 'Head', 'Both', 'Frostfire Circlet'),
(8, 1, 1, 0, 92, 23057, 'Phase 6', 'Mage', 'Fire', 'Neck', 'Both', 'Gem of Trapped Innocents'),
(8, 1, 2, 0, 92, 22983, 'Phase 6', 'Mage', 'Fire', 'Shoulders', 'Both', 'Rime Covered Mantle'),
(8, 1, 4, 0, 92, 22496, 'Phase 6', 'Mage', 'Fire', 'Chest', 'Both', 'Frostfire Robe'),
(8, 1, 5, 0, 92, 22730, 'Phase 6', 'Mage', 'Fire', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 1, 6, 0, 92, 23070, 'Phase 6', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of Polarity'),
(8, 1, 7, 0, 92, 22500, 'Phase 6', 'Mage', 'Fire', 'Feet', 'Both', 'Frostfire Sandals'),
(8, 1, 8, 0, 92, 23021, 'Phase 6', 'Mage', 'Fire', 'Wrists', 'Both', 'The Soul Harvester''s Bindings'),
(8, 1, 9, 0, 92, 21585, 'Phase 6', 'Mage', 'Fire', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 1, 10, 0, 92, 23237, 'Phase 6', 'Mage', 'Fire', 'Finger1', 'Both', 'Ring of the Eternal Flame'),
(8, 1, 11, 0, 92, 23062, 'Phase 6', 'Mage', 'Fire', 'Finger2', 'Both', 'Frostfire Ring'),
(8, 1, 12, 0, 92, 19379, 'Phase 6', 'Mage', 'Fire', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 1, 13, 0, 92, 23046, 'Phase 6', 'Mage', 'Fire', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(8, 1, 14, 0, 92, 23050, 'Phase 6', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of the Necropolis'),
(8, 1, 15, 0, 92, 22807, 'Phase 6', 'Mage', 'Fire', 'MainHand', 'Both', 'Wraith Blade'),
(8, 1, 16, 0, 92, 23049, 'Phase 6', 'Mage', 'Fire', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(8, 1, 17, 0, 92, 22821, 'Phase 6', 'Mage', 'Fire', 'Ranged', 'Both', 'Doomfinger');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 120, 24266, 'Pre-Raid', 'Mage', 'Fire', 'Head', 'Both', 'Spellstrike Hood'),
(8, 1, 1, 0, 120, 28134, 'Pre-Raid', 'Mage', 'Fire', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(8, 1, 2, 0, 120, 27994, 'Pre-Raid', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of Three Terrors'),
(8, 1, 4, 0, 120, 21848, 'Pre-Raid', 'Mage', 'Fire', 'Chest', 'Both', 'Spellfire Robe'),
(8, 1, 5, 0, 120, 21846, 'Pre-Raid', 'Mage', 'Fire', 'Waist', 'Both', 'Spellfire Belt'),
(8, 1, 6, 0, 120, 24262, 'Pre-Raid', 'Mage', 'Fire', 'Legs', 'Both', 'Spellstrike Pants'),
(8, 1, 7, 0, 120, 28406, 'Pre-Raid', 'Mage', 'Fire', 'Feet', 'Both', 'Sigil-Laced Boots'),
(8, 1, 8, 0, 120, 29240, 'Pre-Raid', 'Mage', 'Fire', 'Wrists', 'Both', 'Bands of Negation'),
(8, 1, 9, 0, 120, 21847, 'Pre-Raid', 'Mage', 'Fire', 'Hands', 'Both', 'Spellfire Gloves'),
(8, 1, 10, 0, 120, 29172, 'Pre-Raid', 'Mage', 'Fire', 'Finger1', 'Both', 'Ashyen''s Gift'),
(8, 1, 11, 0, 120, 28227, 'Pre-Raid', 'Mage', 'Fire', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(8, 1, 12, 0, 120, 29370, 'Pre-Raid', 'Mage', 'Fire', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 1, 13, 0, 120, 27683, 'Pre-Raid', 'Mage', 'Fire', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(8, 1, 14, 0, 120, 27981, 'Pre-Raid', 'Mage', 'Fire', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(8, 1, 15, 0, 120, 23554, 'Pre-Raid', 'Mage', 'Fire', 'MainHand', 'Both', 'Eternium Runed Blade'),
(8, 1, 16, 0, 120, 29270, 'Pre-Raid', 'Mage', 'Fire', 'OffHand', 'Both', 'Flametongue Seal'),
(8, 1, 17, 0, 120, 28386, 'Pre-Raid', 'Mage', 'Fire', 'Ranged', 'Both', 'Nether Core''s Control Rod');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 125, 29076, 'Phase 1', 'Mage', 'Fire', 'Head', 'Both', 'Collar of the Aldor'),
(8, 1, 1, 0, 125, 28530, 'Phase 1', 'Mage', 'Fire', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(8, 1, 2, 0, 125, 29079, 'Phase 1', 'Mage', 'Fire', 'Shoulders', 'Both', 'Pauldrons of the Aldor'),
(8, 1, 4, 0, 125, 21848, 'Phase 1', 'Mage', 'Fire', 'Chest', 'Both', 'Spellfire Robe'),
(8, 1, 5, 0, 125, 21846, 'Phase 1', 'Mage', 'Fire', 'Waist', 'Both', 'Spellfire Belt'),
(8, 1, 6, 0, 125, 29078, 'Phase 1', 'Mage', 'Fire', 'Legs', 'Both', 'Legwraps of the Aldor'),
(8, 1, 7, 0, 125, 28517, 'Phase 1', 'Mage', 'Fire', 'Feet', 'Both', 'Boots of Foretelling'),
(8, 1, 8, 0, 125, 28515, 'Phase 1', 'Mage', 'Fire', 'Wrists', 'Both', 'Bands of Nefarious Deeds'),
(8, 1, 9, 0, 125, 21847, 'Phase 1', 'Mage', 'Fire', 'Hands', 'Both', 'Spellfire Gloves'),
(8, 1, 10, 0, 125, 28793, 'Phase 1', 'Mage', 'Fire', 'Finger1', 'Both', 'Band of Crimson Fury'),
(8, 1, 11, 0, 125, 29287, 'Phase 1', 'Mage', 'Fire', 'Finger2', 'Both', 'Violet Signet of the Archmage'),
(8, 1, 12, 0, 125, 29370, 'Phase 1', 'Mage', 'Fire', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 1, 13, 0, 125, 27683, 'Phase 1', 'Mage', 'Fire', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(8, 1, 14, 0, 125, 28766, 'Phase 1', 'Mage', 'Fire', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(8, 1, 15, 0, 125, 28770, 'Phase 1', 'Mage', 'Fire', 'MainHand', 'Both', 'Nathrezim Mindblade'),
(8, 1, 16, 0, 125, 28734, 'Phase 1', 'Mage', 'Fire', 'OffHand', 'Both', 'Jewel of Infinite Possibilities'),
(8, 1, 17, 0, 125, 28673, 'Phase 1', 'Mage', 'Fire', 'Ranged', 'Both', 'Tirisfal Wand of Ascendancy');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 141, 32494, 'Phase 2', 'Mage', 'Fire', 'Head', 'Both', 'Destruction Holo-gogs'),
(8, 1, 1, 0, 141, 30015, 'Phase 2', 'Mage', 'Fire', 'Neck', 'Both', 'The Sun King''s Talisman'),
(8, 1, 2, 0, 141, 30024, 'Phase 2', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of the Elven Kings'),
(8, 1, 4, 0, 141, 30107, 'Phase 2', 'Mage', 'Fire', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(8, 1, 5, 0, 141, 30038, 'Phase 2', 'Mage', 'Fire', 'Waist', 'Both', 'Belt of Blasting'),
(8, 1, 6, 0, 141, 30207, 'Phase 2', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of Tirisfal'),
(8, 1, 7, 0, 141, 30067, 'Phase 2', 'Mage', 'Fire', 'Feet', 'Both', 'Velvet Boots of the Guardian'),
(8, 1, 8, 0, 141, 29918, 'Phase 2', 'Mage', 'Fire', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(8, 1, 9, 0, 141, 29987, 'Phase 2', 'Mage', 'Fire', 'Hands', 'Both', 'Gauntlets of the Sun King'),
(8, 1, 10, 0, 141, 29302, 'Phase 2', 'Mage', 'Fire', 'Finger1', 'Both', 'Band of Eternity'),
(8, 1, 11, 0, 141, 29287, 'Phase 2', 'Mage', 'Fire', 'Finger2', 'Both', 'Violet Signet of the Archmage'),
(8, 1, 12, 0, 141, 27683, 'Phase 2', 'Mage', 'Fire', 'Trinket1', 'Both', 'Quagmirran''s Eye'),
(8, 1, 13, 0, 141, 30626, 'Phase 2', 'Mage', 'Fire', 'Trinket2', 'Both', 'Sextant of Unstable Currents'),
(8, 1, 14, 0, 141, 28766, 'Phase 2', 'Mage', 'Fire', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(8, 1, 15, 0, 141, 30095, 'Phase 2', 'Mage', 'Fire', 'MainHand', 'Both', 'Fang of the Leviathan'),
(8, 1, 16, 0, 141, 29270, 'Phase 2', 'Mage', 'Fire', 'OffHand', 'Both', 'Flametongue Seal'),
(8, 1, 17, 0, 141, 29982, 'Phase 2', 'Mage', 'Fire', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 156, 32525, 'Phase 3', 'Mage', 'Fire', 'Head', 'Both', 'Cowl of the Illidari High Lord'),
(8, 1, 1, 0, 156, 32589, 'Phase 3', 'Mage', 'Fire', 'Neck', 'Both', 'Hellfire-Encased Pendant'),
(8, 1, 2, 0, 156, 31059, 'Phase 3', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of the Tempest'),
(8, 1, 4, 0, 156, 31057, 'Phase 3', 'Mage', 'Fire', 'Chest', 'Both', 'Robes of the Tempest'),
(8, 1, 5, 0, 156, 30038, 'Phase 3', 'Mage', 'Fire', 'Waist', 'Both', 'Belt of Blasting'),
(8, 1, 6, 0, 156, 31058, 'Phase 3', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of the Tempest'),
(8, 1, 7, 0, 156, 32239, 'Phase 3', 'Mage', 'Fire', 'Feet', 'Both', 'Slippers of the Seacaller'),
(8, 1, 8, 0, 156, 32586, 'Phase 3', 'Mage', 'Fire', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(8, 1, 9, 0, 156, 31055, 'Phase 3', 'Mage', 'Fire', 'Hands', 'Both', 'Gloves of the Tempest'),
(8, 1, 10, 0, 156, 32527, 'Phase 3', 'Mage', 'Fire', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(8, 1, 11, 0, 156, 32527, 'Phase 3', 'Mage', 'Fire', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(8, 1, 12, 0, 156, 32488, 'Phase 3', 'Mage', 'Fire', 'Trinket1', 'Both', 'Ashtongue Talisman of Insight'),
(8, 1, 13, 0, 156, 32483, 'Phase 3', 'Mage', 'Fire', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(8, 1, 14, 0, 156, 32331, 'Phase 3', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of the Illidari Council'),
(8, 1, 15, 0, 156, 30910, 'Phase 3', 'Mage', 'Fire', 'MainHand', 'Both', 'Tempest of Chaos'),
(8, 1, 16, 0, 156, 30872, 'Phase 3', 'Mage', 'Fire', 'OffHand', 'Both', 'Chronicle of Dark Secrets'),
(8, 1, 17, 0, 156, 29982, 'Phase 3', 'Mage', 'Fire', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 164, 32525, 'Phase 4', 'Mage', 'Fire', 'Head', 'Both', 'Cowl of the Illidari High Lord'),
(8, 1, 1, 0, 164, 32589, 'Phase 4', 'Mage', 'Fire', 'Neck', 'Both', 'Hellfire-Encased Pendant'),
(8, 1, 2, 0, 164, 31059, 'Phase 4', 'Mage', 'Fire', 'Shoulders', 'Both', 'Mantle of the Tempest'),
(8, 1, 4, 0, 164, 31057, 'Phase 4', 'Mage', 'Fire', 'Chest', 'Both', 'Robes of the Tempest'),
(8, 1, 5, 0, 164, 30038, 'Phase 4', 'Mage', 'Fire', 'Waist', 'Both', 'Belt of Blasting'),
(8, 1, 6, 0, 164, 31058, 'Phase 4', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of the Tempest'),
(8, 1, 7, 0, 164, 32239, 'Phase 4', 'Mage', 'Fire', 'Feet', 'Both', 'Slippers of the Seacaller'),
(8, 1, 8, 0, 164, 32586, 'Phase 4', 'Mage', 'Fire', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(8, 1, 9, 0, 164, 31055, 'Phase 4', 'Mage', 'Fire', 'Hands', 'Both', 'Gloves of the Tempest'),
(8, 1, 10, 0, 164, 32527, 'Phase 4', 'Mage', 'Fire', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(8, 1, 11, 0, 164, 32527, 'Phase 4', 'Mage', 'Fire', 'Finger2', 'Both', 'Ring of Ancient Knowledge'),
(8, 1, 12, 0, 164, 33829, 'Phase 4', 'Mage', 'Fire', 'Trinket1', 'Both', 'Hex Shrunken Head'),
(8, 1, 13, 0, 164, 32483, 'Phase 4', 'Mage', 'Fire', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(8, 1, 14, 0, 164, 32331, 'Phase 4', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of the Illidari Council'),
(8, 1, 15, 0, 164, 30910, 'Phase 4', 'Mage', 'Fire', 'MainHand', 'Both', 'Tempest of Chaos'),
(8, 1, 16, 0, 164, 30872, 'Phase 4', 'Mage', 'Fire', 'OffHand', 'Both', 'Chronicle of Dark Secrets'),
(8, 1, 17, 0, 164, 29982, 'Phase 4', 'Mage', 'Fire', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 200, 42553, 'Pre-Raid', 'Mage', 'Fire', 'Head', 'Both', 'Visage Liquification Goggles'),
(8, 1, 1, 0, 200, 39472, 'Pre-Raid', 'Mage', 'Fire', 'Neck', 'Both', 'Chain of Latent Energies'),
(8, 1, 2, 0, 200, 37673, 'Pre-Raid', 'Mage', 'Fire', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(8, 1, 4, 0, 200, 39492, 'Pre-Raid', 'Mage', 'Fire', 'Chest', 'Both', 'Heroes'' Frostfire Robe'),
(8, 1, 5, 0, 200, 40696, 'Pre-Raid', 'Mage', 'Fire', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(8, 1, 6, 0, 200, 37854, 'Pre-Raid', 'Mage', 'Fire', 'Legs', 'Both', 'Woven Bracae Leggings'),
(8, 1, 7, 0, 200, 44202, 'Pre-Raid', 'Mage', 'Fire', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(8, 1, 8, 0, 200, 37884, 'Pre-Raid', 'Mage', 'Fire', 'Wrists', 'Both', 'Azure Cloth Bindings'),
(8, 1, 9, 0, 200, 39495, 'Pre-Raid', 'Mage', 'Fire', 'Hands', 'Both', 'Heroes'' Frostfire Gloves'),
(8, 1, 10, 0, 200, 42644, 'Pre-Raid', 'Mage', 'Fire', 'Finger1', 'Both', 'Titanium Spellshock Ring'),
(8, 1, 12, 0, 200, 37873, 'Pre-Raid', 'Mage', 'Fire', 'Trinket1', 'Both', 'Mark of the War Prisoner'),
(8, 1, 14, 0, 200, 41610, 'Pre-Raid', 'Mage', 'Fire', 'Back', 'Both', 'Deathchill Cloak'),
(8, 1, 15, 0, 200, 45085, 'Pre-Raid', 'Mage', 'Fire', 'MainHand', 'Both', 'Titansteel Spellblade'),
(8, 1, 17, 0, 200, 37238, 'Pre-Raid', 'Mage', 'Fire', 'Ranged', 'Both', 'Rod of the Fallen Monarch');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 224, 40416, 'Phase 1', 'Mage', 'Fire', 'Head', 'Both', 'Valorous Frostfire Circlet'),
(8, 1, 1, 0, 224, 44661, 'Phase 1', 'Mage', 'Fire', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(8, 1, 2, 0, 224, 40419, 'Phase 1', 'Mage', 'Fire', 'Shoulders', 'Both', 'Valorous Frostfire Shoulderpads'),
(8, 1, 4, 0, 224, 40418, 'Phase 1', 'Mage', 'Fire', 'Chest', 'Both', 'Valorous Frostfire Robe'),
(8, 1, 5, 0, 224, 40301, 'Phase 1', 'Mage', 'Fire', 'Waist', 'Both', 'Cincture of Polarity'),
(8, 1, 6, 0, 224, 40560, 'Phase 1', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(8, 1, 7, 0, 224, 40246, 'Phase 1', 'Mage', 'Fire', 'Feet', 'Both', 'Boots of Impetuous Ideals'),
(8, 1, 8, 0, 224, 44008, 'Phase 1', 'Mage', 'Fire', 'Wrists', 'Both', 'Unsullied Cuffs'),
(8, 1, 9, 0, 224, 40415, 'Phase 1', 'Mage', 'Fire', 'Hands', 'Both', 'Valorous Frostfire Gloves'),
(8, 1, 10, 0, 224, 40399, 'Phase 1', 'Mage', 'Fire', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(8, 1, 12, 0, 224, 40255, 'Phase 1', 'Mage', 'Fire', 'Trinket1', 'Both', 'Dying Curse'),
(8, 1, 14, 0, 224, 44005, 'Phase 1', 'Mage', 'Fire', 'Back', 'Both', 'Pennant Cloak'),
(8, 1, 15, 0, 224, 40396, 'Phase 1', 'Mage', 'Fire', 'MainHand', 'Both', 'The Turning Tide'),
(8, 1, 17, 0, 224, 39712, 'Phase 1', 'Mage', 'Fire', 'Ranged', 'Both', 'Gemmed Wand of the Nerubians');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 245, 46129, 'Phase 2', 'Mage', 'Fire', 'Head', 'Both', 'Conqueror''s Kirin Tor Hood'),
(8, 1, 1, 0, 245, 45133, 'Phase 2', 'Mage', 'Fire', 'Neck', 'Both', 'Pendant of Fiery Havoc'),
(8, 1, 2, 0, 245, 46134, 'Phase 2', 'Mage', 'Fire', 'Shoulders', 'Both', 'Conqueror''s Kirin Tor Shoulderpads'),
(8, 1, 4, 0, 245, 46130, 'Phase 2', 'Mage', 'Fire', 'Chest', 'Both', 'Conqueror''s Kirin Tor Tunic'),
(8, 1, 5, 0, 245, 45619, 'Phase 2', 'Mage', 'Fire', 'Waist', 'Both', 'Starwatcher''s Binding'),
(8, 1, 6, 0, 245, 46133, 'Phase 2', 'Mage', 'Fire', 'Legs', 'Both', 'Conqueror''s Kirin Tor Leggings'),
(8, 1, 7, 0, 245, 45135, 'Phase 2', 'Mage', 'Fire', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(8, 1, 8, 0, 245, 45446, 'Phase 2', 'Mage', 'Fire', 'Wrists', 'Both', 'Grasps of Reason'),
(8, 1, 9, 0, 245, 45665, 'Phase 2', 'Mage', 'Fire', 'Hands', 'Both', 'Pharos Gloves'),
(8, 1, 10, 0, 245, 46046, 'Phase 2', 'Mage', 'Fire', 'Finger1', 'Both', 'Nebula Band'),
(8, 1, 12, 0, 245, 45518, 'Phase 2', 'Mage', 'Fire', 'Trinket1', 'Both', 'Flare of the Heavens'),
(8, 1, 14, 0, 245, 45242, 'Phase 2', 'Mage', 'Fire', 'Back', 'Both', 'Drape of Mortal Downfall'),
(8, 1, 15, 0, 245, 45620, 'Phase 2', 'Mage', 'Fire', 'MainHand', 'Both', 'Starshard Edge'),
(8, 1, 17, 0, 245, 45511, 'Phase 2', 'Mage', 'Fire', 'Ranged', 'Both', 'Scepter of Lost Souls');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 258, 47761, 'Phase 3', 'Mage', 'Fire', 'Head', 'Both', 'Hood of Triumph'),
(8, 1, 2, 0, 258, 47758, 'Phase 3', 'Mage', 'Fire', 'Shoulders', 'Both', 'Shoulderpads of Triumph'),
(8, 1, 4, 1, 258, 47129, 'Phase 3', 'Mage', 'Fire', 'Chest', 'Alliance', 'Skyweaver Robes'),
(8, 1, 4, 2, 258, 47462, 'Phase 3', 'Mage', 'Fire', 'Chest', 'Horde', 'Skyweaver Vestments'),
(8, 1, 5, 1, 258, 47084, 'Phase 3', 'Mage', 'Fire', 'Waist', 'Alliance', 'Cord of Biting Cold'),
(8, 1, 5, 2, 258, 47447, 'Phase 3', 'Mage', 'Fire', 'Waist', 'Horde', 'Belt of Biting Cold'),
(8, 1, 6, 0, 258, 47760, 'Phase 3', 'Mage', 'Fire', 'Legs', 'Both', 'Leggings of Triumph'),
(8, 1, 7, 1, 258, 47097, 'Phase 3', 'Mage', 'Fire', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(8, 1, 7, 2, 258, 47454, 'Phase 3', 'Mage', 'Fire', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(8, 1, 8, 1, 258, 47143, 'Phase 3', 'Mage', 'Fire', 'Wrists', 'Alliance', 'Bindings of Dark Essence'),
(8, 1, 8, 2, 258, 47467, 'Phase 3', 'Mage', 'Fire', 'Wrists', 'Horde', 'Dark Essence Bindings'),
(8, 1, 9, 0, 258, 47762, 'Phase 3', 'Mage', 'Fire', 'Hands', 'Both', 'Gauntlets of Triumph'),
(8, 1, 10, 1, 258, 47489, 'Phase 3', 'Mage', 'Fire', 'Finger1', 'Alliance', 'Lurid Manifestation'),
(8, 1, 10, 2, 258, 47237, 'Phase 3', 'Mage', 'Fire', 'Finger1', 'Horde', 'Band of Deplorable Violence'),
(8, 1, 12, 1, 258, 47188, 'Phase 3', 'Mage', 'Fire', 'Trinket1', 'Alliance', 'Reign of the Unliving'),
(8, 1, 12, 2, 258, 47477, 'Phase 3', 'Mage', 'Fire', 'Trinket1', 'Horde', 'Reign of the Dead'),
(8, 1, 14, 1, 258, 47552, 'Phase 3', 'Mage', 'Fire', 'Back', 'Alliance', 'Jaina''s Radiance'),
(8, 1, 14, 2, 258, 47551, 'Phase 3', 'Mage', 'Fire', 'Back', 'Horde', 'Aethas'' Intensity'),
(8, 1, 17, 0, 258, 45294, 'Phase 3', 'Mage', 'Fire', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 264, 51281, 'Phase 4', 'Mage', 'Fire', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 1, 1, 0, 264, 50724, 'Phase 4', 'Mage', 'Fire', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 1, 2, 0, 264, 51284, 'Phase 4', 'Mage', 'Fire', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 1, 4, 0, 264, 50629, 'Phase 4', 'Mage', 'Fire', 'Chest', 'Both', 'Robe of the Waking Nightmare'),
(8, 1, 5, 0, 264, 50613, 'Phase 4', 'Mage', 'Fire', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 1, 6, 0, 264, 51282, 'Phase 4', 'Mage', 'Fire', 'Legs', 'Both', 'Sanctified Bloodmage Leggings'),
(8, 1, 7, 0, 264, 50699, 'Phase 4', 'Mage', 'Fire', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 1, 8, 0, 264, 50651, 'Phase 4', 'Mage', 'Fire', 'Wrists', 'Both', 'The Lady''s Brittle Bracers'),
(8, 1, 9, 0, 264, 51280, 'Phase 4', 'Mage', 'Fire', 'Hands', 'Both', 'Sanctified Bloodmage Gloves'),
(8, 1, 10, 0, 264, 50398, 'Phase 4', 'Mage', 'Fire', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(8, 1, 12, 0, 264, 50365, 'Phase 4', 'Mage', 'Fire', 'Trinket1', 'Both', 'Phylactery of the Nameless Lich'),
(8, 1, 14, 0, 264, 50628, 'Phase 4', 'Mage', 'Fire', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(8, 1, 15, 0, 264, 50732, 'Phase 4', 'Mage', 'Fire', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 1, 17, 0, 264, 50684, 'Phase 4', 'Mage', 'Fire', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 1, 0, 0, 290, 51281, 'Phase 5', 'Mage', 'Fire', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 1, 1, 0, 290, 50182, 'Phase 5', 'Mage', 'Fire', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 1, 2, 0, 290, 51284, 'Phase 5', 'Mage', 'Fire', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 1, 4, 0, 290, 51283, 'Phase 5', 'Mage', 'Fire', 'Chest', 'Both', 'Sanctified Bloodmage Robe'),
(8, 1, 5, 0, 290, 50613, 'Phase 5', 'Mage', 'Fire', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 1, 6, 0, 290, 50694, 'Phase 5', 'Mage', 'Fire', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(8, 1, 7, 0, 290, 50699, 'Phase 5', 'Mage', 'Fire', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 1, 8, 0, 290, 54582, 'Phase 5', 'Mage', 'Fire', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(8, 1, 9, 0, 290, 51280, 'Phase 5', 'Mage', 'Fire', 'Hands', 'Both', 'Sanctified Bloodmage Gloves'),
(8, 1, 10, 0, 290, 50614, 'Phase 5', 'Mage', 'Fire', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(8, 1, 11, 0, 290, 50398, 'Phase 5', 'Mage', 'Fire', 'Finger2', 'Both', 'Ashen Band of Endless Destruction'),
(8, 1, 12, 0, 290, 54588, 'Phase 5', 'Mage', 'Fire', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(8, 1, 13, 0, 290, 50365, 'Phase 5', 'Mage', 'Fire', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(8, 1, 14, 0, 290, 54583, 'Phase 5', 'Mage', 'Fire', 'Back', 'Both', 'Cloak of Burning Dusk'),
(8, 1, 15, 0, 290, 50732, 'Phase 5', 'Mage', 'Fire', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 1, 16, 0, 290, 50719, 'Phase 5', 'Mage', 'Fire', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(8, 1, 17, 0, 290, 50684, 'Phase 5', 'Mage', 'Fire', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- Frost (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 66, 10504, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Head', 'Both', 'Green Lens'),
(8, 2, 1, 0, 66, 12103, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Neck', 'Both', 'Star of Mystaria'),
(8, 2, 2, 0, 66, 11782, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Shoulders', 'Both', 'Boreal Mantle'),
(8, 2, 4, 0, 66, 14152, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 2, 5, 0, 66, 11662, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 2, 6, 0, 66, 13170, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Legs', 'Both', 'Skyshroud Leggings'),
(8, 2, 7, 0, 66, 11822, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Feet', 'Both', 'Omnicast Boots'),
(8, 2, 8, 0, 66, 11766, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 2, 9, 0, 66, 13253, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Hands', 'Both', 'Hands of Power'),
(8, 2, 10, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Finger1', 'Both', 'Freezing Band'),
(8, 2, 11, 0, 66, 942, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Finger2', 'Both', 'Freezing Band'),
(8, 2, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 2, 13, 0, 66, 13968, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 2, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Back', 'Both', 'Archivist Cape'),
(8, 2, 15, 0, 66, 13964, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'MainHand', 'Both', 'Witchblade'),
(8, 2, 16, 0, 66, 10796, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'OffHand', 'Both', 'Drakestone'),
(8, 2, 17, 0, 66, 13938, 'Phase 1 (Pre-Raid)', 'Mage', 'Frost', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 76, 23318, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Head', 'Both', 'Lieutenant Commander''s Silk Cowl'),
(8, 2, 1, 0, 76, 12103, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Neck', 'Both', 'Star of Mystaria'),
(8, 2, 2, 0, 76, 23319, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 2, 4, 0, 76, 14152, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 2, 5, 0, 76, 11662, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Waist', 'Both', 'Ban''thok Sash'),
(8, 2, 6, 0, 76, 23304, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Legs', 'Both', 'Knight-Captain''s Silk Legguards'),
(8, 2, 7, 0, 76, 23291, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 2, 8, 0, 76, 11766, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 2, 9, 0, 76, 13253, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Hands', 'Both', 'Hands of Power'),
(8, 2, 10, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Finger1', 'Both', 'Freezing Band'),
(8, 2, 11, 0, 76, 942, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Finger2', 'Both', 'Freezing Band'),
(8, 2, 12, 0, 76, 12930, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 2, 13, 0, 76, 13968, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Trinket2', 'Both', 'Eye of the Beast'),
(8, 2, 14, 0, 76, 13386, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Back', 'Both', 'Archivist Cape'),
(8, 2, 15, 0, 76, 13964, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'MainHand', 'Both', 'Witchblade'),
(8, 2, 16, 0, 76, 10796, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'OffHand', 'Both', 'Drakestone'),
(8, 2, 17, 0, 76, 13938, 'Phase 2 (Pre-Raid)', 'Mage', 'Frost', 'Ranged', 'Both', 'Bonecreeper Stylus');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 78, 16914, 'Phase 2', 'Mage', 'Frost', 'Head', 'Both', 'Netherwind Crown'),
(8, 2, 1, 0, 78, 18814, 'Phase 2', 'Mage', 'Frost', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 2, 2, 0, 78, 23319, 'Phase 2', 'Mage', 'Frost', 'Shoulders', 'Both', 'Lieutenant Commander''s Silk Mantle'),
(8, 2, 4, 0, 78, 14152, 'Phase 2', 'Mage', 'Frost', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 2, 5, 0, 78, 19136, 'Phase 2', 'Mage', 'Frost', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 2, 6, 0, 78, 16915, 'Phase 2', 'Mage', 'Frost', 'Legs', 'Both', 'Netherwind Pants'),
(8, 2, 7, 0, 78, 23291, 'Phase 2', 'Mage', 'Frost', 'Feet', 'Both', 'Knight-Lieutenant''s Silk Walkers'),
(8, 2, 8, 0, 78, 11766, 'Phase 2', 'Mage', 'Frost', 'Wrists', 'Both', 'Flameweave Cuffs'),
(8, 2, 9, 0, 78, 13253, 'Phase 2', 'Mage', 'Frost', 'Hands', 'Both', 'Hands of Power'),
(8, 2, 10, 0, 78, 19147, 'Phase 2', 'Mage', 'Frost', 'Finger1', 'Both', 'Ring of Spell Power'),
(8, 2, 11, 0, 78, 19147, 'Phase 2', 'Mage', 'Frost', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 2, 12, 0, 78, 12930, 'Phase 2', 'Mage', 'Frost', 'Trinket1', 'Both', 'Briarwood Reed'),
(8, 2, 13, 0, 78, 18820, 'Phase 2', 'Mage', 'Frost', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(8, 2, 14, 0, 78, 13386, 'Phase 2', 'Mage', 'Frost', 'Back', 'Both', 'Archivist Cape'),
(8, 2, 15, 0, 78, 17103, 'Phase 2', 'Mage', 'Frost', 'MainHand', 'Both', 'Azuresong Mageblade'),
(8, 2, 16, 0, 78, 10796, 'Phase 2', 'Mage', 'Frost', 'OffHand', 'Both', 'Drakestone'),
(8, 2, 17, 0, 78, 19130, 'Phase 2', 'Mage', 'Frost', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 83, 19375, 'Phase 3', 'Mage', 'Frost', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 2, 1, 0, 83, 18814, 'Phase 3', 'Mage', 'Frost', 'Neck', 'Both', 'Choker of the Fire Lord'),
(8, 2, 2, 0, 83, 19370, 'Phase 3', 'Mage', 'Frost', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 2, 4, 0, 83, 14152, 'Phase 3', 'Mage', 'Frost', 'Chest', 'Both', 'Robe of the Archmage'),
(8, 2, 5, 0, 83, 19136, 'Phase 3', 'Mage', 'Frost', 'Waist', 'Both', 'Mana Igniting Cord'),
(8, 2, 6, 0, 83, 16915, 'Phase 3', 'Mage', 'Frost', 'Legs', 'Both', 'Netherwind Pants'),
(8, 2, 7, 0, 83, 19438, 'Phase 3', 'Mage', 'Frost', 'Feet', 'Both', 'Ringo''s Blizzard Boots'),
(8, 2, 8, 0, 83, 19374, 'Phase 3', 'Mage', 'Frost', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(8, 2, 9, 0, 83, 16913, 'Phase 3', 'Mage', 'Frost', 'Hands', 'Both', 'Netherwind Gloves'),
(8, 2, 11, 0, 83, 19147, 'Phase 3', 'Mage', 'Frost', 'Finger2', 'Both', 'Ring of Spell Power'),
(8, 2, 12, 0, 83, 19379, 'Phase 3', 'Mage', 'Frost', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 2, 13, 0, 83, 19339, 'Phase 3', 'Mage', 'Frost', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 2, 14, 0, 83, 19378, 'Phase 3', 'Mage', 'Frost', 'Back', 'Both', 'Cloak of the Brood Lord'),
(8, 2, 15, 0, 83, 19356, 'Phase 3', 'Mage', 'Frost', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(8, 2, 17, 0, 83, 19130, 'Phase 3', 'Mage', 'Frost', 'Ranged', 'Both', 'Cold Snap');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 88, 19375, 'Phase 5', 'Mage', 'Frost', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(8, 2, 1, 0, 88, 21608, 'Phase 5', 'Mage', 'Frost', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(8, 2, 2, 0, 88, 19370, 'Phase 5', 'Mage', 'Frost', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(8, 2, 4, 0, 88, 19145, 'Phase 5', 'Mage', 'Frost', 'Chest', 'Both', 'Robe of Volatile Power'),
(8, 2, 5, 0, 88, 22730, 'Phase 5', 'Mage', 'Frost', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 2, 6, 0, 88, 21461, 'Phase 5', 'Mage', 'Frost', 'Legs', 'Both', 'Leggings of the Black Blizzard'),
(8, 2, 7, 0, 88, 21344, 'Phase 5', 'Mage', 'Frost', 'Feet', 'Both', 'Enigma Boots'),
(8, 2, 8, 0, 88, 21186, 'Phase 5', 'Mage', 'Frost', 'Wrists', 'Both', 'Rockfury Bracers'),
(8, 2, 9, 0, 88, 21585, 'Phase 5', 'Mage', 'Frost', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 2, 10, 0, 88, 21836, 'Phase 5', 'Mage', 'Frost', 'Finger1', 'Both', 'Ritssyn''s Ring of Chaos'),
(8, 2, 11, 0, 88, 21709, 'Phase 5', 'Mage', 'Frost', 'Finger2', 'Both', 'Ring of the Fallen God'),
(8, 2, 12, 0, 88, 19379, 'Phase 5', 'Mage', 'Frost', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 2, 13, 0, 88, 19339, 'Phase 5', 'Mage', 'Frost', 'Trinket2', 'Both', 'Mind Quickening Gem'),
(8, 2, 14, 0, 88, 22731, 'Phase 5', 'Mage', 'Frost', 'Back', 'Both', 'Cloak of the Devoured'),
(8, 2, 15, 0, 88, 21622, 'Phase 5', 'Mage', 'Frost', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(8, 2, 16, 0, 88, 21597, 'Phase 5', 'Mage', 'Frost', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(8, 2, 17, 0, 88, 21603, 'Phase 5', 'Mage', 'Frost', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 92, 22498, 'Phase 6', 'Mage', 'Frost', 'Head', 'Both', 'Frostfire Circlet'),
(8, 2, 1, 0, 92, 23057, 'Phase 6', 'Mage', 'Frost', 'Neck', 'Both', 'Gem of Trapped Innocents'),
(8, 2, 2, 0, 92, 22983, 'Phase 6', 'Mage', 'Frost', 'Shoulders', 'Both', 'Rime Covered Mantle'),
(8, 2, 4, 0, 92, 22496, 'Phase 6', 'Mage', 'Frost', 'Chest', 'Both', 'Frostfire Robe'),
(8, 2, 5, 0, 92, 22730, 'Phase 6', 'Mage', 'Frost', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(8, 2, 6, 0, 92, 23070, 'Phase 6', 'Mage', 'Frost', 'Legs', 'Both', 'Leggings of Polarity'),
(8, 2, 7, 0, 92, 22500, 'Phase 6', 'Mage', 'Frost', 'Feet', 'Both', 'Frostfire Sandals'),
(8, 2, 8, 0, 92, 23021, 'Phase 6', 'Mage', 'Frost', 'Wrists', 'Both', 'The Soul Harvester''s Bindings'),
(8, 2, 9, 0, 92, 21585, 'Phase 6', 'Mage', 'Frost', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(8, 2, 10, 0, 92, 23237, 'Phase 6', 'Mage', 'Frost', 'Finger1', 'Both', 'Ring of the Eternal Flame'),
(8, 2, 11, 0, 92, 23062, 'Phase 6', 'Mage', 'Frost', 'Finger2', 'Both', 'Frostfire Ring'),
(8, 2, 12, 0, 92, 19379, 'Phase 6', 'Mage', 'Frost', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(8, 2, 13, 0, 92, 23046, 'Phase 6', 'Mage', 'Frost', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(8, 2, 14, 0, 92, 23050, 'Phase 6', 'Mage', 'Frost', 'Back', 'Both', 'Cloak of the Necropolis'),
(8, 2, 15, 0, 92, 22807, 'Phase 6', 'Mage', 'Frost', 'MainHand', 'Both', 'Wraith Blade'),
(8, 2, 16, 0, 92, 23049, 'Phase 6', 'Mage', 'Frost', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(8, 2, 17, 0, 92, 22821, 'Phase 6', 'Mage', 'Frost', 'Ranged', 'Both', 'Doomfinger');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 120, 24266, 'Pre-Raid', 'Mage', 'Frost', 'Head', 'Both', 'Spellstrike Hood'),
(8, 2, 1, 0, 120, 28134, 'Pre-Raid', 'Mage', 'Frost', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(8, 2, 2, 0, 120, 21869, 'Pre-Raid', 'Mage', 'Frost', 'Shoulders', 'Both', 'Frozen Shadoweave Shoulders'),
(8, 2, 4, 0, 120, 21871, 'Pre-Raid', 'Mage', 'Frost', 'Chest', 'Both', 'Frozen Shadoweave Robe'),
(8, 2, 5, 0, 120, 24256, 'Pre-Raid', 'Mage', 'Frost', 'Waist', 'Both', 'Girdle of Ruination'),
(8, 2, 6, 0, 120, 24262, 'Pre-Raid', 'Mage', 'Frost', 'Legs', 'Both', 'Spellstrike Pants'),
(8, 2, 7, 0, 120, 21870, 'Pre-Raid', 'Mage', 'Frost', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(8, 2, 8, 0, 120, 24250, 'Pre-Raid', 'Mage', 'Frost', 'Wrists', 'Both', 'Bracers of Havok'),
(8, 2, 9, 0, 120, 30725, 'Pre-Raid', 'Mage', 'Frost', 'Hands', 'Both', 'Anger-Spark Gloves'),
(8, 2, 10, 0, 120, 29172, 'Pre-Raid', 'Mage', 'Frost', 'Finger1', 'Both', 'Ashyen''s Gift'),
(8, 2, 11, 0, 120, 28227, 'Pre-Raid', 'Mage', 'Frost', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(8, 2, 12, 0, 120, 29370, 'Pre-Raid', 'Mage', 'Frost', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 2, 13, 0, 120, 27683, 'Pre-Raid', 'Mage', 'Frost', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(8, 2, 14, 0, 120, 27981, 'Pre-Raid', 'Mage', 'Frost', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(8, 2, 15, 0, 120, 23554, 'Pre-Raid', 'Mage', 'Frost', 'MainHand', 'Both', 'Eternium Runed Blade'),
(8, 2, 16, 0, 120, 29273, 'Pre-Raid', 'Mage', 'Frost', 'OffHand', 'Both', 'Khadgar''s Knapsack'),
(8, 2, 17, 0, 120, 29350, 'Pre-Raid', 'Mage', 'Frost', 'Ranged', 'Both', 'The Black Stalk');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 125, 29076, 'Phase 1', 'Mage', 'Frost', 'Head', 'Both', 'Collar of the Aldor'),
(8, 2, 1, 0, 125, 28530, 'Phase 1', 'Mage', 'Frost', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(8, 2, 2, 0, 125, 21869, 'Phase 1', 'Mage', 'Frost', 'Shoulders', 'Both', 'Frozen Shadoweave Shoulders'),
(8, 2, 4, 0, 125, 21871, 'Phase 1', 'Mage', 'Frost', 'Chest', 'Both', 'Frozen Shadoweave Robe'),
(8, 2, 5, 0, 125, 24256, 'Phase 1', 'Mage', 'Frost', 'Waist', 'Both', 'Girdle of Ruination'),
(8, 2, 6, 0, 125, 29078, 'Phase 1', 'Mage', 'Frost', 'Legs', 'Both', 'Legwraps of the Aldor'),
(8, 2, 7, 0, 125, 21870, 'Phase 1', 'Mage', 'Frost', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(8, 2, 8, 0, 125, 28515, 'Phase 1', 'Mage', 'Frost', 'Wrists', 'Both', 'Bands of Nefarious Deeds'),
(8, 2, 9, 0, 125, 30725, 'Phase 1', 'Mage', 'Frost', 'Hands', 'Both', 'Anger-Spark Gloves'),
(8, 2, 10, 0, 125, 28793, 'Phase 1', 'Mage', 'Frost', 'Finger1', 'Both', 'Band of Crimson Fury'),
(8, 2, 11, 0, 125, 29287, 'Phase 1', 'Mage', 'Frost', 'Finger2', 'Both', 'Violet Signet of the Archmage'),
(8, 2, 12, 0, 125, 29370, 'Phase 1', 'Mage', 'Frost', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(8, 2, 13, 0, 125, 27683, 'Phase 1', 'Mage', 'Frost', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(8, 2, 14, 0, 125, 28766, 'Phase 1', 'Mage', 'Frost', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(8, 2, 15, 0, 125, 28770, 'Phase 1', 'Mage', 'Frost', 'MainHand', 'Both', 'Nathrezim Mindblade'),
(8, 2, 16, 0, 125, 29269, 'Phase 1', 'Mage', 'Frost', 'OffHand', 'Both', 'Sapphiron''s Wing Bone'),
(8, 2, 17, 0, 125, 28783, 'Phase 1', 'Mage', 'Frost', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 200, 37294, 'Pre-Raid', 'Mage', 'Frost', 'Head', 'Both', 'Crown of Unbridled Magic'),
(8, 2, 1, 0, 200, 39472, 'Pre-Raid', 'Mage', 'Frost', 'Neck', 'Both', 'Chain of Latent Energies'),
(8, 2, 2, 0, 200, 37673, 'Pre-Raid', 'Mage', 'Frost', 'Shoulders', 'Both', 'Dark Runic Mantle'),
(8, 2, 4, 0, 200, 39492, 'Pre-Raid', 'Mage', 'Frost', 'Chest', 'Both', 'Heroes'' Frostfire Robe'),
(8, 2, 5, 0, 200, 40696, 'Pre-Raid', 'Mage', 'Frost', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(8, 2, 6, 0, 200, 37854, 'Pre-Raid', 'Mage', 'Frost', 'Legs', 'Both', 'Woven Bracae Leggings'),
(8, 2, 7, 0, 200, 44202, 'Pre-Raid', 'Mage', 'Frost', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(8, 2, 8, 0, 200, 37361, 'Pre-Raid', 'Mage', 'Frost', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(8, 2, 9, 0, 200, 39495, 'Pre-Raid', 'Mage', 'Frost', 'Hands', 'Both', 'Heroes'' Frostfire Gloves'),
(8, 2, 10, 0, 200, 40585, 'Pre-Raid', 'Mage', 'Frost', 'Finger1', 'Both', 'Signet of the Kirin Tor'),
(8, 2, 12, 0, 200, 37660, 'Pre-Raid', 'Mage', 'Frost', 'Trinket1', 'Both', 'Forge Ember'),
(8, 2, 14, 0, 200, 41610, 'Pre-Raid', 'Mage', 'Frost', 'Back', 'Both', 'Deathchill Cloak'),
(8, 2, 15, 0, 200, 45085, 'Pre-Raid', 'Mage', 'Frost', 'MainHand', 'Both', 'Titansteel Spellblade'),
(8, 2, 17, 0, 200, 37177, 'Pre-Raid', 'Mage', 'Frost', 'Ranged', 'Both', 'Wand of the San''layn');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 1, 0, 224, 44661, 'Phase 1', 'Mage', 'Frost', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(8, 2, 2, 0, 224, 40419, 'Phase 1', 'Mage', 'Frost', 'Shoulders', 'Both', 'Valorous Frostfire Shoulderpads'),
(8, 2, 4, 0, 224, 40418, 'Phase 1', 'Mage', 'Frost', 'Chest', 'Both', 'Valorous Frostfire Robe'),
(8, 2, 5, 0, 224, 40561, 'Phase 1', 'Mage', 'Frost', 'Waist', 'Both', 'Leash of Heedless Magic'),
(8, 2, 6, 0, 224, 40560, 'Phase 1', 'Mage', 'Frost', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(8, 2, 7, 0, 224, 40558, 'Phase 1', 'Mage', 'Frost', 'Feet', 'Both', 'Arcanic Tramplers'),
(8, 2, 8, 0, 224, 44008, 'Phase 1', 'Mage', 'Frost', 'Wrists', 'Both', 'Unsullied Cuffs'),
(8, 2, 9, 0, 224, 40415, 'Phase 1', 'Mage', 'Frost', 'Hands', 'Both', 'Valorous Frostfire Gloves'),
(8, 2, 10, 0, 224, 40719, 'Phase 1', 'Mage', 'Frost', 'Finger1', 'Both', 'Band of Channeled Magic'),
(8, 2, 12, 0, 224, 40255, 'Phase 1', 'Mage', 'Frost', 'Trinket1', 'Both', 'Dying Curse'),
(8, 2, 14, 0, 224, 44005, 'Phase 1', 'Mage', 'Frost', 'Back', 'Both', 'Pennant Cloak'),
(8, 2, 15, 0, 224, 40396, 'Phase 1', 'Mage', 'Frost', 'MainHand', 'Both', 'The Turning Tide'),
(8, 2, 17, 0, 224, 39426, 'Phase 1', 'Mage', 'Frost', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 245, 46129, 'Phase 2', 'Mage', 'Frost', 'Head', 'Both', 'Conqueror''s Kirin Tor Hood'),
(8, 2, 1, 0, 245, 45133, 'Phase 2', 'Mage', 'Frost', 'Neck', 'Both', 'Pendant of Fiery Havoc'),
(8, 2, 2, 0, 245, 46134, 'Phase 2', 'Mage', 'Frost', 'Shoulders', 'Both', 'Conqueror''s Kirin Tor Shoulderpads'),
(8, 2, 4, 0, 245, 46130, 'Phase 2', 'Mage', 'Frost', 'Chest', 'Both', 'Conqueror''s Kirin Tor Tunic'),
(8, 2, 5, 0, 245, 45619, 'Phase 2', 'Mage', 'Frost', 'Waist', 'Both', 'Starwatcher''s Binding'),
(8, 2, 6, 0, 245, 46133, 'Phase 2', 'Mage', 'Frost', 'Legs', 'Both', 'Conqueror''s Kirin Tor Leggings'),
(8, 2, 7, 0, 245, 45135, 'Phase 2', 'Mage', 'Frost', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(8, 2, 8, 0, 245, 45446, 'Phase 2', 'Mage', 'Frost', 'Wrists', 'Both', 'Grasps of Reason'),
(8, 2, 9, 0, 245, 45665, 'Phase 2', 'Mage', 'Frost', 'Hands', 'Both', 'Pharos Gloves'),
(8, 2, 10, 0, 245, 46046, 'Phase 2', 'Mage', 'Frost', 'Finger1', 'Both', 'Nebula Band'),
(8, 2, 12, 0, 245, 45466, 'Phase 2', 'Mage', 'Frost', 'Trinket1', 'Both', 'Scale of Fates'),
(8, 2, 14, 0, 245, 45242, 'Phase 2', 'Mage', 'Frost', 'Back', 'Both', 'Drape of Mortal Downfall'),
(8, 2, 15, 0, 245, 45620, 'Phase 2', 'Mage', 'Frost', 'MainHand', 'Both', 'Starshard Edge'),
(8, 2, 17, 0, 245, 45511, 'Phase 2', 'Mage', 'Frost', 'Ranged', 'Both', 'Scepter of Lost Souls');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 1, 258, 47761, 'Phase 3', 'Mage', 'Frost', 'Head', 'Alliance', 'Khadgar''s Hood of Triumph'),
(8, 2, 2, 1, 258, 47758, 'Phase 3', 'Mage', 'Frost', 'Shoulders', 'Alliance', 'Khadgar''s Shoulderpads of Triumph'),
(8, 2, 4, 1, 258, 47129, 'Phase 3', 'Mage', 'Frost', 'Chest', 'Alliance', 'Skyweaver Robes'),
(8, 2, 5, 1, 258, 47419, 'Phase 3', 'Mage', 'Frost', 'Waist', 'Alliance', 'Belt of the Tenebrous Mist'),
(8, 2, 6, 1, 258, 47760, 'Phase 3', 'Mage', 'Frost', 'Legs', 'Alliance', 'Khadgar''s Leggings of Triumph'),
(8, 2, 7, 1, 258, 47097, 'Phase 3', 'Mage', 'Frost', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(8, 2, 8, 1, 258, 47143, 'Phase 3', 'Mage', 'Frost', 'Wrists', 'Alliance', 'Bindings of Dark Essence'),
(8, 2, 9, 1, 258, 47762, 'Phase 3', 'Mage', 'Frost', 'Hands', 'Alliance', 'Khadgar''s Gauntlets of Triumph'),
(8, 2, 10, 1, 258, 47489, 'Phase 3', 'Mage', 'Frost', 'Finger1', 'Alliance', 'Lurid Manifestation'),
(8, 2, 12, 1, 258, 47188, 'Phase 3', 'Mage', 'Frost', 'Trinket1', 'Alliance', 'Reign of the Unliving'),
(8, 2, 14, 1, 258, 47552, 'Phase 3', 'Mage', 'Frost', 'Back', 'Alliance', 'Jaina''s Radiance'),
(8, 2, 17, 0, 258, 45294, 'Phase 3', 'Mage', 'Frost', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 264, 51281, 'Phase 4', 'Mage', 'Frost', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 2, 1, 0, 264, 50724, 'Phase 4', 'Mage', 'Frost', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 2, 2, 0, 264, 51284, 'Phase 4', 'Mage', 'Frost', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 2, 4, 0, 264, 50717, 'Phase 4', 'Mage', 'Frost', 'Chest', 'Both', 'Sanguine Silk Robes'),
(8, 2, 5, 0, 264, 50613, 'Phase 4', 'Mage', 'Frost', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 2, 6, 0, 264, 51282, 'Phase 4', 'Mage', 'Frost', 'Legs', 'Both', 'Sanctified Bloodmage Leggings'),
(8, 2, 7, 0, 264, 50699, 'Phase 4', 'Mage', 'Frost', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 2, 8, 0, 264, 50651, 'Phase 4', 'Mage', 'Frost', 'Wrists', 'Both', 'The Lady''s Brittle Bracers'),
(8, 2, 9, 0, 264, 51280, 'Phase 4', 'Mage', 'Frost', 'Hands', 'Both', 'Sanctified Bloodmage Gloves'),
(8, 2, 10, 0, 264, 50398, 'Phase 4', 'Mage', 'Frost', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(8, 2, 12, 0, 264, 50365, 'Phase 4', 'Mage', 'Frost', 'Trinket1', 'Both', 'Phylactery of the Nameless Lich'),
(8, 2, 14, 0, 264, 50628, 'Phase 4', 'Mage', 'Frost', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(8, 2, 15, 0, 264, 50732, 'Phase 4', 'Mage', 'Frost', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 2, 17, 0, 264, 50684, 'Phase 4', 'Mage', 'Frost', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(8, 2, 0, 0, 290, 51281, 'Phase 5', 'Mage', 'Frost', 'Head', 'Both', 'Sanctified Bloodmage Hood'),
(8, 2, 1, 0, 290, 50182, 'Phase 5', 'Mage', 'Frost', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(8, 2, 2, 0, 290, 51284, 'Phase 5', 'Mage', 'Frost', 'Shoulders', 'Both', 'Sanctified Bloodmage Shoulderpads'),
(8, 2, 4, 0, 290, 51283, 'Phase 5', 'Mage', 'Frost', 'Chest', 'Both', 'Sanctified Bloodmage Robe'),
(8, 2, 5, 0, 290, 50613, 'Phase 5', 'Mage', 'Frost', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(8, 2, 6, 0, 290, 50694, 'Phase 5', 'Mage', 'Frost', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(8, 2, 7, 0, 290, 50699, 'Phase 5', 'Mage', 'Frost', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(8, 2, 8, 0, 290, 54582, 'Phase 5', 'Mage', 'Frost', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(8, 2, 9, 0, 290, 51280, 'Phase 5', 'Mage', 'Frost', 'Hands', 'Both', 'Sanctified Bloodmage Gloves'),
(8, 2, 10, 0, 290, 50614, 'Phase 5', 'Mage', 'Frost', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(8, 2, 11, 0, 290, 50398, 'Phase 5', 'Mage', 'Frost', 'Finger2', 'Both', 'Ashen Band of Endless Destruction'),
(8, 2, 12, 0, 290, 54588, 'Phase 5', 'Mage', 'Frost', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(8, 2, 13, 0, 290, 50348, 'Phase 5', 'Mage', 'Frost', 'Trinket2', 'Both', 'Dislodged Foreign Object'),
(8, 2, 14, 0, 290, 54583, 'Phase 5', 'Mage', 'Frost', 'Back', 'Both', 'Cloak of Burning Dusk'),
(8, 2, 15, 0, 290, 50732, 'Phase 5', 'Mage', 'Frost', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(8, 2, 16, 0, 290, 50719, 'Phase 5', 'Mage', 'Frost', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(8, 2, 17, 0, 290, 50684, 'Phase 5', 'Mage', 'Frost', 'Ranged', 'Both', 'Corpse-Impaling Spike');


-- ============================================================
-- Warlock (9)
-- ============================================================
-- Affliction (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 1, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Head', 'Alliance', 'Green Lens'),
(9, 0, 0, 2, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Head', 'Horde', 'Green Lens'),
(9, 0, 1, 1, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 0, 1, 2, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 0, 2, 1, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Shoulders', 'Alliance', 'Felcloth Shoulders'),
(9, 0, 2, 2, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Shoulders', 'Horde', 'Felcloth Shoulders'),
(9, 0, 4, 1, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 0, 4, 2, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Chest', 'Horde', 'Robe of the Void'),
(9, 0, 5, 1, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 0, 5, 2, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 0, 6, 1, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 0, 6, 2, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 0, 7, 1, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 0, 7, 2, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 0, 8, 1, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 0, 8, 2, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 0, 9, 1, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Hands', 'Alliance', 'Hands of Power'),
(9, 0, 9, 2, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Hands', 'Horde', 'Hands of Power'),
(9, 0, 10, 1, 66, 12543, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 0, 10, 2, 66, 12545, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 0, 11, 1, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 0, 11, 2, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 0, 12, 1, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 0, 12, 2, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 0, 13, 1, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket2', 'Alliance', 'Eye of the Beast'),
(9, 0, 13, 2, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket2', 'Horde', 'Eye of the Beast'),
(9, 0, 14, 1, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Back', 'Alliance', 'Archivist Cape'),
(9, 0, 14, 2, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Back', 'Horde', 'Archivist Cape'),
(9, 0, 15, 1, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'MainHand', 'Alliance', 'Witchblade'),
(9, 0, 15, 2, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'MainHand', 'Horde', 'Witchblade'),
(9, 0, 16, 1, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'OffHand', 'Alliance', 'Drakestone'),
(9, 0, 16, 2, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'OffHand', 'Horde', 'Drakestone'),
(9, 0, 17, 1, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 0, 17, 2, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Affliction', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 1, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Head', 'Alliance', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 0, 0, 2, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Head', 'Horde', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 0, 1, 1, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 0, 1, 2, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 0, 2, 1, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Shoulders', 'Alliance', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 0, 2, 2, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Shoulders', 'Horde', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 0, 4, 1, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 0, 4, 2, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Chest', 'Horde', 'Robe of the Void'),
(9, 0, 5, 1, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 0, 5, 2, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 0, 6, 1, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 0, 6, 2, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 0, 7, 1, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 0, 7, 2, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 0, 8, 1, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 0, 8, 2, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 0, 9, 1, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Hands', 'Alliance', 'Felcloth Gloves'),
(9, 0, 9, 2, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Hands', 'Horde', 'Felcloth Gloves'),
(9, 0, 10, 1, 76, 12543, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 0, 10, 2, 76, 12545, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 0, 11, 1, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 0, 11, 2, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 0, 12, 1, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 0, 12, 2, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 0, 13, 1, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket2', 'Alliance', 'Royal Seal of Eldre''Thalas'),
(9, 0, 13, 2, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Trinket2', 'Horde', 'Royal Seal of Eldre''Thalas'),
(9, 0, 14, 1, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Back', 'Alliance', 'Archivist Cape'),
(9, 0, 14, 2, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Back', 'Horde', 'Archivist Cape'),
(9, 0, 15, 1, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'MainHand', 'Alliance', 'Blade of the New Moon'),
(9, 0, 15, 2, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'MainHand', 'Horde', 'Blade of the New Moon'),
(9, 0, 16, 1, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'OffHand', 'Alliance', 'Drakestone'),
(9, 0, 16, 2, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'OffHand', 'Horde', 'Drakestone'),
(9, 0, 17, 1, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 0, 17, 2, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Affliction', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 78, 23310, 'Phase 2', 'Warlock', 'Affliction', 'Head', 'Both', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 0, 1, 0, 78, 18814, 'Phase 2', 'Warlock', 'Affliction', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 0, 2, 0, 78, 23311, 'Phase 2', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 0, 4, 0, 78, 19145, 'Phase 2', 'Warlock', 'Affliction', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 0, 5, 0, 78, 18809, 'Phase 2', 'Warlock', 'Affliction', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 0, 6, 0, 78, 19133, 'Phase 2', 'Warlock', 'Affliction', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 0, 7, 0, 78, 19131, 'Phase 2', 'Warlock', 'Affliction', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 0, 8, 0, 78, 11766, 'Phase 2', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Flameweave Cuffs'),
(9, 0, 9, 0, 78, 18407, 'Phase 2', 'Warlock', 'Affliction', 'Hands', 'Both', 'Felcloth Gloves'),
(9, 0, 10, 0, 78, 19147, 'Phase 2', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ring of Spell Power'),
(9, 0, 11, 0, 78, 19147, 'Phase 2', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of Spell Power'),
(9, 0, 12, 0, 78, 12930, 'Phase 2', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Briarwood Reed'),
(9, 0, 13, 0, 78, 18820, 'Phase 2', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 0, 14, 0, 78, 13386, 'Phase 2', 'Warlock', 'Affliction', 'Back', 'Both', 'Archivist Cape'),
(9, 0, 15, 0, 78, 17103, 'Phase 2', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Azuresong Mageblade'),
(9, 0, 16, 0, 78, 10796, 'Phase 2', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Drakestone'),
(9, 0, 17, 0, 78, 13396, 'Phase 2', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 83, 19375, 'Phase 3', 'Warlock', 'Affliction', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(9, 0, 1, 0, 83, 18814, 'Phase 3', 'Warlock', 'Affliction', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 0, 2, 0, 83, 19370, 'Phase 3', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(9, 0, 4, 0, 83, 19145, 'Phase 3', 'Warlock', 'Affliction', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 0, 5, 0, 83, 18809, 'Phase 3', 'Warlock', 'Affliction', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 0, 6, 0, 83, 19133, 'Phase 3', 'Warlock', 'Affliction', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 0, 7, 0, 83, 19131, 'Phase 3', 'Warlock', 'Affliction', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 0, 8, 0, 83, 19374, 'Phase 3', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(9, 0, 9, 0, 83, 19407, 'Phase 3', 'Warlock', 'Affliction', 'Hands', 'Both', 'Ebony Flame Gloves'),
(9, 0, 11, 0, 83, 19434, 'Phase 3', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Band of Dark Dominion'),
(9, 0, 12, 0, 83, 19379, 'Phase 3', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 0, 13, 0, 83, 18820, 'Phase 3', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 0, 14, 0, 83, 19378, 'Phase 3', 'Warlock', 'Affliction', 'Back', 'Both', 'Cloak of the Brood Lord'),
(9, 0, 15, 0, 83, 19356, 'Phase 3', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(9, 0, 17, 0, 83, 13396, 'Phase 3', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 88, 21337, 'Phase 5', 'Warlock', 'Affliction', 'Head', 'Both', 'Doomcaller''s Circlet'),
(9, 0, 1, 0, 88, 21608, 'Phase 5', 'Warlock', 'Affliction', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 0, 2, 0, 88, 21335, 'Phase 5', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Doomcaller''s Mantle'),
(9, 0, 4, 0, 88, 19682, 'Phase 5', 'Warlock', 'Affliction', 'Chest', 'Both', 'Bloodvine Vest'),
(9, 0, 5, 0, 88, 22730, 'Phase 5', 'Warlock', 'Affliction', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 0, 6, 0, 88, 19683, 'Phase 5', 'Warlock', 'Affliction', 'Legs', 'Both', 'Bloodvine Leggings'),
(9, 0, 7, 0, 88, 19684, 'Phase 5', 'Warlock', 'Affliction', 'Feet', 'Both', 'Bloodvine Boots'),
(9, 0, 8, 0, 88, 21186, 'Phase 5', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 0, 9, 0, 88, 21585, 'Phase 5', 'Warlock', 'Affliction', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 0, 10, 0, 88, 21417, 'Phase 5', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ring of Unspoken Names'),
(9, 0, 11, 0, 88, 21709, 'Phase 5', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 0, 12, 0, 88, 19379, 'Phase 5', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 0, 13, 0, 88, 18820, 'Phase 5', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 0, 14, 0, 88, 22731, 'Phase 5', 'Warlock', 'Affliction', 'Back', 'Both', 'Cloak of the Devoured'),
(9, 0, 15, 0, 88, 21622, 'Phase 5', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(9, 0, 16, 0, 88, 21597, 'Phase 5', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(9, 0, 17, 0, 88, 21603, 'Phase 5', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 92, 22506, 'Phase 6', 'Warlock', 'Affliction', 'Head', 'Both', 'Plagueheart Circlet'),
(9, 0, 1, 0, 92, 21608, 'Phase 6', 'Warlock', 'Affliction', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 0, 2, 0, 92, 22507, 'Phase 6', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Plagueheart Shoulderpads'),
(9, 0, 4, 0, 92, 22504, 'Phase 6', 'Warlock', 'Affliction', 'Chest', 'Both', 'Plagueheart Robe'),
(9, 0, 5, 0, 92, 22730, 'Phase 6', 'Warlock', 'Affliction', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 0, 6, 0, 92, 23070, 'Phase 6', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of Polarity'),
(9, 0, 7, 0, 92, 22508, 'Phase 6', 'Warlock', 'Affliction', 'Feet', 'Both', 'Plagueheart Sandals'),
(9, 0, 8, 0, 92, 21186, 'Phase 6', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 0, 9, 0, 92, 21585, 'Phase 6', 'Warlock', 'Affliction', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 0, 10, 0, 92, 23031, 'Phase 6', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Band of the Inevitable'),
(9, 0, 11, 0, 92, 21709, 'Phase 6', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 0, 12, 0, 92, 19379, 'Phase 6', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 0, 13, 0, 92, 23046, 'Phase 6', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(9, 0, 14, 0, 92, 23050, 'Phase 6', 'Warlock', 'Affliction', 'Back', 'Both', 'Cloak of the Necropolis'),
(9, 0, 15, 0, 92, 22807, 'Phase 6', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Wraith Blade'),
(9, 0, 16, 0, 92, 23049, 'Phase 6', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(9, 0, 17, 0, 92, 22820, 'Phase 6', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Wand of Fates');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 120, 24266, 'Pre-Raid', 'Warlock', 'Affliction', 'Head', 'Both', 'Spellstrike Hood'),
(9, 0, 1, 0, 120, 28134, 'Pre-Raid', 'Warlock', 'Affliction', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(9, 0, 2, 0, 120, 21869, 'Pre-Raid', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Frozen Shadoweave Shoulders'),
(9, 0, 4, 0, 120, 21871, 'Pre-Raid', 'Warlock', 'Affliction', 'Chest', 'Both', 'Frozen Shadoweave Robe'),
(9, 0, 5, 0, 120, 24256, 'Pre-Raid', 'Warlock', 'Affliction', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 0, 6, 0, 120, 24262, 'Pre-Raid', 'Warlock', 'Affliction', 'Legs', 'Both', 'Spellstrike Pants'),
(9, 0, 7, 0, 120, 21870, 'Pre-Raid', 'Warlock', 'Affliction', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 0, 8, 0, 120, 24250, 'Pre-Raid', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 0, 9, 0, 120, 24450, 'Pre-Raid', 'Warlock', 'Affliction', 'Hands', 'Both', 'Manaspark Gloves'),
(9, 0, 10, 0, 120, 29172, 'Pre-Raid', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 0, 11, 0, 120, 28227, 'Pre-Raid', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(9, 0, 12, 0, 120, 29370, 'Pre-Raid', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 0, 13, 0, 120, 29132, 'Pre-Raid', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Scryer''s Bloodgem'),
(9, 0, 14, 0, 120, 27981, 'Pre-Raid', 'Warlock', 'Affliction', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(9, 0, 15, 0, 120, 23554, 'Pre-Raid', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Eternium Runed Blade'),
(9, 0, 16, 0, 120, 29273, 'Pre-Raid', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Khadgar''s Knapsack'),
(9, 0, 17, 0, 120, 29350, 'Pre-Raid', 'Warlock', 'Affliction', 'Ranged', 'Both', 'The Black Stalk');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 125, 28963, 'Phase 1', 'Warlock', 'Affliction', 'Head', 'Both', 'Voidheart Crown'),
(9, 0, 1, 0, 125, 28530, 'Phase 1', 'Warlock', 'Affliction', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(9, 0, 2, 0, 125, 28967, 'Phase 1', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 0, 4, 0, 125, 28964, 'Phase 1', 'Warlock', 'Affliction', 'Chest', 'Both', 'Voidheart Robe'),
(9, 0, 5, 0, 125, 24256, 'Phase 1', 'Warlock', 'Affliction', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 0, 6, 0, 125, 30734, 'Phase 1', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of the Seventh Circle'),
(9, 0, 7, 0, 125, 21870, 'Phase 1', 'Warlock', 'Affliction', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 0, 8, 0, 125, 24250, 'Phase 1', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 0, 9, 0, 125, 28968, 'Phase 1', 'Warlock', 'Affliction', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 0, 10, 0, 125, 29172, 'Phase 1', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 0, 11, 0, 125, 28793, 'Phase 1', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Band of Crimson Fury'),
(9, 0, 12, 0, 125, 29370, 'Phase 1', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 0, 13, 0, 125, 27683, 'Phase 1', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 0, 14, 0, 125, 28766, 'Phase 1', 'Warlock', 'Affliction', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 0, 15, 0, 125, 30723, 'Phase 1', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Talon of the Tempest'),
(9, 0, 16, 0, 125, 29272, 'Phase 1', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Orb of the Soul-Eater'),
(9, 0, 17, 0, 125, 28783, 'Phase 1', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 141, 32494, 'Phase 2', 'Warlock', 'Affliction', 'Head', 'Both', 'Destruction Holo-gogs'),
(9, 0, 1, 0, 141, 30015, 'Phase 2', 'Warlock', 'Affliction', 'Neck', 'Both', 'The Sun King''s Talisman'),
(9, 0, 2, 0, 141, 28967, 'Phase 2', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 0, 4, 0, 141, 30107, 'Phase 2', 'Warlock', 'Affliction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 0, 5, 0, 141, 30038, 'Phase 2', 'Warlock', 'Affliction', 'Waist', 'Both', 'Belt of Blasting'),
(9, 0, 6, 0, 141, 30213, 'Phase 2', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of the Corruptor'),
(9, 0, 7, 0, 141, 30037, 'Phase 2', 'Warlock', 'Affliction', 'Feet', 'Both', 'Boots of Blasting'),
(9, 0, 8, 0, 141, 29918, 'Phase 2', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(9, 0, 9, 0, 141, 28968, 'Phase 2', 'Warlock', 'Affliction', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 0, 10, 0, 141, 29302, 'Phase 2', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Band of Eternity'),
(9, 0, 11, 0, 141, 30109, 'Phase 2', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of Endless Coils'),
(9, 0, 12, 0, 141, 29370, 'Phase 2', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 0, 13, 0, 141, 27683, 'Phase 2', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 0, 14, 0, 141, 28766, 'Phase 2', 'Warlock', 'Affliction', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 0, 15, 0, 141, 30095, 'Phase 2', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Fang of the Leviathan'),
(9, 0, 16, 0, 141, 30049, 'Phase 2', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Fathomstone'),
(9, 0, 17, 0, 141, 29982, 'Phase 2', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 156, 31051, 'Phase 3', 'Warlock', 'Affliction', 'Head', 'Both', 'Hood of the Malefic'),
(9, 0, 1, 0, 156, 30015, 'Phase 3', 'Warlock', 'Affliction', 'Neck', 'Both', 'Sun King''s Talisman'),
(9, 0, 2, 0, 156, 31054, 'Phase 3', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 0, 4, 0, 156, 30107, 'Phase 3', 'Warlock', 'Affliction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 0, 5, 0, 156, 30888, 'Phase 3', 'Warlock', 'Affliction', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 0, 6, 0, 156, 31053, 'Phase 3', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 0, 7, 0, 156, 32239, 'Phase 3', 'Warlock', 'Affliction', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 0, 8, 0, 156, 32586, 'Phase 3', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 0, 9, 0, 156, 31050, 'Phase 3', 'Warlock', 'Affliction', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 0, 10, 0, 156, 32527, 'Phase 3', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(9, 0, 11, 0, 156, 32247, 'Phase 3', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 0, 12, 0, 156, 29370, 'Phase 3', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 0, 13, 0, 156, 32483, 'Phase 3', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 0, 14, 0, 156, 32590, 'Phase 3', 'Warlock', 'Affliction', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 0, 15, 0, 156, 32374, 'Phase 3', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 0, 17, 0, 156, 29982, 'Phase 3', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 164, 31051, 'Phase 4', 'Warlock', 'Affliction', 'Head', 'Both', 'Hood of the Malefic'),
(9, 0, 1, 0, 164, 30015, 'Phase 4', 'Warlock', 'Affliction', 'Neck', 'Both', 'Sun King''s Talisman'),
(9, 0, 2, 0, 164, 31054, 'Phase 4', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 0, 4, 0, 164, 30107, 'Phase 4', 'Warlock', 'Affliction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 0, 5, 0, 164, 30888, 'Phase 4', 'Warlock', 'Affliction', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 0, 6, 0, 164, 31053, 'Phase 4', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 0, 7, 0, 164, 32239, 'Phase 4', 'Warlock', 'Affliction', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 0, 8, 0, 164, 32586, 'Phase 4', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 0, 9, 0, 164, 31050, 'Phase 4', 'Warlock', 'Affliction', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 0, 10, 0, 164, 33497, 'Phase 4', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Mana Attuned Band'),
(9, 0, 11, 0, 164, 32247, 'Phase 4', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 0, 12, 0, 164, 33829, 'Phase 4', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Hex Shrunken Head'),
(9, 0, 13, 0, 164, 32483, 'Phase 4', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 0, 14, 0, 164, 32590, 'Phase 4', 'Warlock', 'Affliction', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 0, 15, 0, 164, 32374, 'Phase 4', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 0, 16, 0, 164, 34179, 'Phase 5', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Heart of the Pit'),
(9, 0, 17, 0, 164, 33192, 'Phase 4', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Carved Witch Doctor''s Stick');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 200, 44910, 'Pre-Raid', 'Warlock', 'Affliction', 'Head', 'Both', 'Titan-Forged Hood of Dominance'),
(9, 0, 1, 0, 200, 40680, 'Pre-Raid', 'Warlock', 'Affliction', 'Neck', 'Both', 'Encircling Burnished Gold Chains'),
(9, 0, 2, 0, 200, 34210, 'Pre-Raid', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Amice of the Convoker'),
(9, 0, 4, 0, 200, 39497, 'Pre-Raid', 'Warlock', 'Affliction', 'Chest', 'Both', 'Heroes'' Plagueheart Robe'),
(9, 0, 5, 0, 200, 40696, 'Pre-Raid', 'Warlock', 'Affliction', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(9, 0, 6, 0, 200, 34181, 'Pre-Raid', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of Calamity'),
(9, 0, 7, 0, 200, 44202, 'Pre-Raid', 'Warlock', 'Affliction', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(9, 0, 8, 0, 200, 37361, 'Pre-Raid', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(9, 0, 9, 0, 200, 42113, 'Pre-Raid', 'Warlock', 'Affliction', 'Hands', 'Both', 'Spellweave Gloves'),
(9, 0, 10, 0, 200, 43253, 'Pre-Raid', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Ring of Northern Tears'),
(9, 0, 12, 0, 200, 40682, 'Pre-Raid', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Sundial of the Exiled'),
(9, 0, 14, 0, 200, 41610, 'Pre-Raid', 'Warlock', 'Affliction', 'Back', 'Both', 'Deathchill Cloak'),
(9, 0, 17, 0, 200, 37177, 'Pre-Raid', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Wand of the San''layn');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 224, 40421, 'Phase 1', 'Warlock', 'Affliction', 'Head', 'Both', 'Valorous Plagueheart Circlet'),
(9, 0, 1, 0, 224, 44661, 'Phase 1', 'Warlock', 'Affliction', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(9, 0, 2, 0, 224, 40424, 'Phase 1', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Valorous Plagueheart Shoulderpads'),
(9, 0, 4, 0, 224, 40423, 'Phase 1', 'Warlock', 'Affliction', 'Chest', 'Both', 'Valorous Plagueheart Robe'),
(9, 0, 5, 0, 224, 40561, 'Phase 1', 'Warlock', 'Affliction', 'Waist', 'Both', 'Leash of Heedless Magic'),
(9, 0, 6, 0, 224, 40560, 'Phase 1', 'Warlock', 'Affliction', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(9, 0, 7, 0, 224, 40558, 'Phase 1', 'Warlock', 'Affliction', 'Feet', 'Both', 'Arcanic Tramplers'),
(9, 0, 8, 0, 224, 44008, 'Phase 1', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Unsullied Cuffs'),
(9, 0, 9, 0, 224, 40420, 'Phase 1', 'Warlock', 'Affliction', 'Hands', 'Both', 'Valorous Plagueheart Gloves'),
(9, 0, 10, 0, 224, 40399, 'Phase 1', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(9, 0, 12, 0, 224, 40432, 'Phase 1', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(9, 0, 14, 0, 224, 44005, 'Phase 1', 'Warlock', 'Affliction', 'Back', 'Both', 'Pennant Cloak'),
(9, 0, 15, 0, 224, 40396, 'Phase 1', 'Warlock', 'Affliction', 'MainHand', 'Both', 'The Turning Tide'),
(9, 0, 17, 0, 224, 39712, 'Phase 1', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Gemmed Wand of the Nerubians');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 245, 45497, 'Phase 2', 'Warlock', 'Affliction', 'Head', 'Both', 'Crown of Luminescence'),
(9, 0, 1, 0, 245, 45133, 'Phase 2', 'Warlock', 'Affliction', 'Neck', 'Both', 'Pendant of Fiery Havoc'),
(9, 0, 2, 0, 245, 46068, 'Phase 2', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Amice of Inconceivable Horror'),
(9, 0, 4, 0, 245, 46137, 'Phase 2', 'Warlock', 'Affliction', 'Chest', 'Both', 'Conqueror''s Deathbringer Robe'),
(9, 0, 9, 0, 245, 45665, 'Phase 2', 'Warlock', 'Affliction', 'Hands', 'Both', 'Pharos Gloves'),
(9, 0, 10, 0, 245, 45495, 'Phase 2', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Conductive Seal'),
(9, 0, 14, 0, 245, 45618, 'Phase 2', 'Warlock', 'Affliction', 'Back', 'Both', 'Sunglimmer Cloak'),
(9, 0, 15, 0, 245, 45620, 'Phase 2', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Starshard Edge'),
(9, 0, 17, 0, 245, 45294, 'Phase 2', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 10, 0, 258, 45495, 'Phase 3', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Conductive Seal'),
(9, 0, 17, 0, 258, 45294, 'Phase 3', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 0, 0, 0, 290, 51231, 'Phase 5', 'Warlock', 'Affliction', 'Head', 'Both', 'Sanctified Dark Coven Hood'),
(9, 0, 1, 0, 290, 50182, 'Phase 5', 'Warlock', 'Affliction', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(9, 0, 2, 0, 290, 51234, 'Phase 5', 'Warlock', 'Affliction', 'Shoulders', 'Both', 'Sanctified Dark Coven Shoulderpads'),
(9, 0, 4, 0, 290, 51233, 'Phase 5', 'Warlock', 'Affliction', 'Chest', 'Both', 'Sanctified Dark Coven Robe'),
(9, 0, 5, 0, 290, 50613, 'Phase 5', 'Warlock', 'Affliction', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(9, 0, 6, 0, 290, 50694, 'Phase 5', 'Warlock', 'Affliction', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(9, 0, 7, 0, 290, 50699, 'Phase 5', 'Warlock', 'Affliction', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(9, 0, 8, 0, 290, 54582, 'Phase 5', 'Warlock', 'Affliction', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(9, 0, 9, 0, 290, 51230, 'Phase 5', 'Warlock', 'Affliction', 'Hands', 'Both', 'Sanctified Dark Coven Gloves'),
(9, 0, 10, 0, 290, 50614, 'Phase 5', 'Warlock', 'Affliction', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(9, 0, 11, 0, 290, 50664, 'Phase 5', 'Warlock', 'Affliction', 'Finger2', 'Both', 'Ring of Rapid Ascent'),
(9, 0, 12, 0, 290, 54588, 'Phase 5', 'Warlock', 'Affliction', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(9, 0, 13, 0, 290, 50365, 'Phase 5', 'Warlock', 'Affliction', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(9, 0, 14, 0, 290, 54583, 'Phase 5', 'Warlock', 'Affliction', 'Back', 'Both', 'Cloak of Burning Dusk'),
(9, 0, 15, 0, 290, 50732, 'Phase 5', 'Warlock', 'Affliction', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(9, 0, 16, 0, 290, 50719, 'Phase 5', 'Warlock', 'Affliction', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(9, 0, 17, 0, 290, 50684, 'Phase 5', 'Warlock', 'Affliction', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- Demonology (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 1, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Head', 'Alliance', 'Green Lens'),
(9, 1, 0, 2, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Head', 'Horde', 'Green Lens'),
(9, 1, 1, 1, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 1, 1, 2, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 1, 2, 1, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Shoulders', 'Alliance', 'Felcloth Shoulders'),
(9, 1, 2, 2, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Shoulders', 'Horde', 'Felcloth Shoulders'),
(9, 1, 4, 1, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 1, 4, 2, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Chest', 'Horde', 'Robe of the Void'),
(9, 1, 5, 1, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 1, 5, 2, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 1, 6, 1, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 1, 6, 2, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 1, 7, 1, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 1, 7, 2, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 1, 8, 1, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 1, 8, 2, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 1, 9, 1, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Hands', 'Alliance', 'Hands of Power'),
(9, 1, 9, 2, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Hands', 'Horde', 'Hands of Power'),
(9, 1, 10, 1, 66, 12543, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 1, 10, 2, 66, 12545, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 1, 11, 1, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 1, 11, 2, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 1, 12, 1, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 1, 12, 2, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 1, 13, 1, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket2', 'Alliance', 'Eye of the Beast'),
(9, 1, 13, 2, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket2', 'Horde', 'Eye of the Beast'),
(9, 1, 14, 1, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Back', 'Alliance', 'Archivist Cape'),
(9, 1, 14, 2, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Back', 'Horde', 'Archivist Cape'),
(9, 1, 15, 1, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'MainHand', 'Alliance', 'Witchblade'),
(9, 1, 15, 2, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'MainHand', 'Horde', 'Witchblade'),
(9, 1, 16, 1, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'OffHand', 'Alliance', 'Drakestone'),
(9, 1, 16, 2, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'OffHand', 'Horde', 'Drakestone'),
(9, 1, 17, 1, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 1, 17, 2, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Demonology', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 1, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Head', 'Alliance', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 1, 0, 2, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Head', 'Horde', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 1, 1, 1, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 1, 1, 2, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 1, 2, 1, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Shoulders', 'Alliance', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 1, 2, 2, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Shoulders', 'Horde', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 1, 4, 1, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 1, 4, 2, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Chest', 'Horde', 'Robe of the Void'),
(9, 1, 5, 1, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 1, 5, 2, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 1, 6, 1, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 1, 6, 2, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 1, 7, 1, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 1, 7, 2, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 1, 8, 1, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 1, 8, 2, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 1, 9, 1, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Hands', 'Alliance', 'Felcloth Gloves'),
(9, 1, 9, 2, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Hands', 'Horde', 'Felcloth Gloves'),
(9, 1, 10, 1, 76, 12543, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 1, 10, 2, 76, 12545, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 1, 11, 1, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 1, 11, 2, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 1, 12, 1, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 1, 12, 2, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 1, 13, 1, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket2', 'Alliance', 'Royal Seal of Eldre''Thalas'),
(9, 1, 13, 2, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Trinket2', 'Horde', 'Royal Seal of Eldre''Thalas'),
(9, 1, 14, 1, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Back', 'Alliance', 'Archivist Cape'),
(9, 1, 14, 2, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Back', 'Horde', 'Archivist Cape'),
(9, 1, 15, 1, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'MainHand', 'Alliance', 'Blade of the New Moon'),
(9, 1, 15, 2, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'MainHand', 'Horde', 'Blade of the New Moon'),
(9, 1, 16, 1, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'OffHand', 'Alliance', 'Drakestone'),
(9, 1, 16, 2, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'OffHand', 'Horde', 'Drakestone'),
(9, 1, 17, 1, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 1, 17, 2, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Demonology', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 78, 23310, 'Phase 2', 'Warlock', 'Demonology', 'Head', 'Both', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 1, 1, 0, 78, 18814, 'Phase 2', 'Warlock', 'Demonology', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 1, 2, 0, 78, 23311, 'Phase 2', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 1, 4, 0, 78, 19145, 'Phase 2', 'Warlock', 'Demonology', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 1, 5, 0, 78, 18809, 'Phase 2', 'Warlock', 'Demonology', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 1, 6, 0, 78, 19133, 'Phase 2', 'Warlock', 'Demonology', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 1, 7, 0, 78, 19131, 'Phase 2', 'Warlock', 'Demonology', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 1, 8, 0, 78, 11766, 'Phase 2', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Flameweave Cuffs'),
(9, 1, 9, 0, 78, 18407, 'Phase 2', 'Warlock', 'Demonology', 'Hands', 'Both', 'Felcloth Gloves'),
(9, 1, 10, 0, 78, 19147, 'Phase 2', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Ring of Spell Power'),
(9, 1, 11, 0, 78, 19147, 'Phase 2', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of Spell Power'),
(9, 1, 12, 0, 78, 12930, 'Phase 2', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Briarwood Reed'),
(9, 1, 13, 0, 78, 18820, 'Phase 2', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 1, 14, 0, 78, 13386, 'Phase 2', 'Warlock', 'Demonology', 'Back', 'Both', 'Archivist Cape'),
(9, 1, 15, 0, 78, 17103, 'Phase 2', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Azuresong Mageblade'),
(9, 1, 16, 0, 78, 10796, 'Phase 2', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Drakestone'),
(9, 1, 17, 0, 78, 13396, 'Phase 2', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 83, 19375, 'Phase 3', 'Warlock', 'Demonology', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(9, 1, 1, 0, 83, 18814, 'Phase 3', 'Warlock', 'Demonology', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 1, 2, 0, 83, 19370, 'Phase 3', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(9, 1, 4, 0, 83, 19145, 'Phase 3', 'Warlock', 'Demonology', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 1, 5, 0, 83, 18809, 'Phase 3', 'Warlock', 'Demonology', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 1, 6, 0, 83, 19133, 'Phase 3', 'Warlock', 'Demonology', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 1, 7, 0, 83, 19131, 'Phase 3', 'Warlock', 'Demonology', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 1, 8, 0, 83, 19374, 'Phase 3', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(9, 1, 9, 0, 83, 19407, 'Phase 3', 'Warlock', 'Demonology', 'Hands', 'Both', 'Ebony Flame Gloves'),
(9, 1, 11, 0, 83, 19434, 'Phase 3', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Band of Dark Dominion'),
(9, 1, 12, 0, 83, 19379, 'Phase 3', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 1, 13, 0, 83, 18820, 'Phase 3', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 1, 14, 0, 83, 19378, 'Phase 3', 'Warlock', 'Demonology', 'Back', 'Both', 'Cloak of the Brood Lord'),
(9, 1, 15, 0, 83, 19356, 'Phase 3', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(9, 1, 17, 0, 83, 13396, 'Phase 3', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 88, 21337, 'Phase 5', 'Warlock', 'Demonology', 'Head', 'Both', 'Doomcaller''s Circlet'),
(9, 1, 1, 0, 88, 21608, 'Phase 5', 'Warlock', 'Demonology', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 1, 2, 0, 88, 21335, 'Phase 5', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Doomcaller''s Mantle'),
(9, 1, 4, 0, 88, 19682, 'Phase 5', 'Warlock', 'Demonology', 'Chest', 'Both', 'Bloodvine Vest'),
(9, 1, 5, 0, 88, 22730, 'Phase 5', 'Warlock', 'Demonology', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 1, 6, 0, 88, 19683, 'Phase 5', 'Warlock', 'Demonology', 'Legs', 'Both', 'Bloodvine Leggings'),
(9, 1, 7, 0, 88, 19684, 'Phase 5', 'Warlock', 'Demonology', 'Feet', 'Both', 'Bloodvine Boots'),
(9, 1, 8, 0, 88, 21186, 'Phase 5', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 1, 9, 0, 88, 21585, 'Phase 5', 'Warlock', 'Demonology', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 1, 10, 0, 88, 21417, 'Phase 5', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Ring of Unspoken Names'),
(9, 1, 11, 0, 88, 21709, 'Phase 5', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 1, 12, 0, 88, 19379, 'Phase 5', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 1, 13, 0, 88, 18820, 'Phase 5', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 1, 14, 0, 88, 22731, 'Phase 5', 'Warlock', 'Demonology', 'Back', 'Both', 'Cloak of the Devoured'),
(9, 1, 15, 0, 88, 21622, 'Phase 5', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(9, 1, 16, 0, 88, 21597, 'Phase 5', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(9, 1, 17, 0, 88, 21603, 'Phase 5', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 92, 22506, 'Phase 6', 'Warlock', 'Demonology', 'Head', 'Both', 'Plagueheart Circlet'),
(9, 1, 1, 0, 92, 21608, 'Phase 6', 'Warlock', 'Demonology', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 1, 2, 0, 92, 22507, 'Phase 6', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Plagueheart Shoulderpads'),
(9, 1, 4, 0, 92, 22504, 'Phase 6', 'Warlock', 'Demonology', 'Chest', 'Both', 'Plagueheart Robe'),
(9, 1, 5, 0, 92, 22730, 'Phase 6', 'Warlock', 'Demonology', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 1, 6, 0, 92, 23070, 'Phase 6', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of Polarity'),
(9, 1, 7, 0, 92, 22508, 'Phase 6', 'Warlock', 'Demonology', 'Feet', 'Both', 'Plagueheart Sandals'),
(9, 1, 8, 0, 92, 21186, 'Phase 6', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 1, 9, 0, 92, 21585, 'Phase 6', 'Warlock', 'Demonology', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 1, 10, 0, 92, 23031, 'Phase 6', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Band of the Inevitable'),
(9, 1, 11, 0, 92, 21709, 'Phase 6', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 1, 12, 0, 92, 19379, 'Phase 6', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 1, 13, 0, 92, 23046, 'Phase 6', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(9, 1, 14, 0, 92, 23050, 'Phase 6', 'Warlock', 'Demonology', 'Back', 'Both', 'Cloak of the Necropolis'),
(9, 1, 15, 0, 92, 22807, 'Phase 6', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Wraith Blade'),
(9, 1, 16, 0, 92, 23049, 'Phase 6', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(9, 1, 17, 0, 92, 22820, 'Phase 6', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of Fates');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 120, 24266, 'Pre-Raid', 'Warlock', 'Demonology', 'Head', 'Both', 'Spellstrike Hood'),
(9, 1, 1, 0, 120, 28134, 'Pre-Raid', 'Warlock', 'Demonology', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(9, 1, 2, 0, 120, 21869, 'Pre-Raid', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Frozen Shadoweave Shoulders'),
(9, 1, 4, 0, 120, 21871, 'Pre-Raid', 'Warlock', 'Demonology', 'Chest', 'Both', 'Frozen Shadoweave Robe'),
(9, 1, 5, 0, 120, 24256, 'Pre-Raid', 'Warlock', 'Demonology', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 1, 6, 0, 120, 24262, 'Pre-Raid', 'Warlock', 'Demonology', 'Legs', 'Both', 'Spellstrike Pants'),
(9, 1, 7, 0, 120, 21870, 'Pre-Raid', 'Warlock', 'Demonology', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 1, 8, 0, 120, 24250, 'Pre-Raid', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 1, 9, 0, 120, 24450, 'Pre-Raid', 'Warlock', 'Demonology', 'Hands', 'Both', 'Manaspark Gloves'),
(9, 1, 10, 0, 120, 29172, 'Pre-Raid', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 1, 11, 0, 120, 28227, 'Pre-Raid', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(9, 1, 12, 0, 120, 29370, 'Pre-Raid', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 1, 13, 0, 120, 29132, 'Pre-Raid', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Scryer''s Bloodgem'),
(9, 1, 14, 0, 120, 27981, 'Pre-Raid', 'Warlock', 'Demonology', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(9, 1, 15, 0, 120, 23554, 'Pre-Raid', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Eternium Runed Blade'),
(9, 1, 16, 0, 120, 29273, 'Pre-Raid', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Khadgar''s Knapsack'),
(9, 1, 17, 0, 120, 29350, 'Pre-Raid', 'Warlock', 'Demonology', 'Ranged', 'Both', 'The Black Stalk');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 125, 28963, 'Phase 1', 'Warlock', 'Demonology', 'Head', 'Both', 'Voidheart Crown'),
(9, 1, 1, 0, 125, 28530, 'Phase 1', 'Warlock', 'Demonology', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(9, 1, 2, 0, 125, 28967, 'Phase 1', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 1, 4, 0, 125, 28964, 'Phase 1', 'Warlock', 'Demonology', 'Chest', 'Both', 'Voidheart Robe'),
(9, 1, 5, 0, 125, 24256, 'Phase 1', 'Warlock', 'Demonology', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 1, 6, 0, 125, 30734, 'Phase 1', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of the Seventh Circle'),
(9, 1, 7, 0, 125, 21870, 'Phase 1', 'Warlock', 'Demonology', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 1, 8, 0, 125, 24250, 'Phase 1', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 1, 9, 0, 125, 28968, 'Phase 1', 'Warlock', 'Demonology', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 1, 10, 0, 125, 29172, 'Phase 1', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 1, 11, 0, 125, 28793, 'Phase 1', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Band of Crimson Fury'),
(9, 1, 12, 0, 125, 29370, 'Phase 1', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 1, 13, 0, 125, 27683, 'Phase 1', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 1, 14, 0, 125, 28766, 'Phase 1', 'Warlock', 'Demonology', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 1, 15, 0, 125, 30723, 'Phase 1', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Talon of the Tempest'),
(9, 1, 16, 0, 125, 29272, 'Phase 1', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Orb of the Soul-Eater'),
(9, 1, 17, 0, 125, 28783, 'Phase 1', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 141, 32494, 'Phase 2', 'Warlock', 'Demonology', 'Head', 'Both', 'Destruction Holo-gogs'),
(9, 1, 1, 0, 141, 30015, 'Phase 2', 'Warlock', 'Demonology', 'Neck', 'Both', 'The Sun King''s Talisman'),
(9, 1, 2, 0, 141, 28967, 'Phase 2', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 1, 4, 0, 141, 30107, 'Phase 2', 'Warlock', 'Demonology', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 1, 5, 0, 141, 30038, 'Phase 2', 'Warlock', 'Demonology', 'Waist', 'Both', 'Belt of Blasting'),
(9, 1, 6, 0, 141, 30213, 'Phase 2', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of the Corruptor'),
(9, 1, 7, 0, 141, 30037, 'Phase 2', 'Warlock', 'Demonology', 'Feet', 'Both', 'Boots of Blasting'),
(9, 1, 8, 0, 141, 29918, 'Phase 2', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(9, 1, 9, 0, 141, 28968, 'Phase 2', 'Warlock', 'Demonology', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 1, 10, 0, 141, 29302, 'Phase 2', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Band of Eternity'),
(9, 1, 11, 0, 141, 30109, 'Phase 2', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of Endless Coils'),
(9, 1, 12, 0, 141, 29370, 'Phase 2', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 1, 13, 0, 141, 27683, 'Phase 2', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 1, 14, 0, 141, 28766, 'Phase 2', 'Warlock', 'Demonology', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 1, 15, 0, 141, 30095, 'Phase 2', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Fang of the Leviathan'),
(9, 1, 16, 0, 141, 30049, 'Phase 2', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Fathomstone'),
(9, 1, 17, 0, 141, 29982, 'Phase 2', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 156, 31051, 'Phase 3', 'Warlock', 'Demonology', 'Head', 'Both', 'Hood of the Malefic'),
(9, 1, 1, 0, 156, 30015, 'Phase 3', 'Warlock', 'Demonology', 'Neck', 'Both', 'Sun King''s Talisman'),
(9, 1, 2, 0, 156, 31054, 'Phase 3', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 1, 4, 0, 156, 30107, 'Phase 3', 'Warlock', 'Demonology', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 1, 5, 0, 156, 30888, 'Phase 3', 'Warlock', 'Demonology', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 1, 6, 0, 156, 31053, 'Phase 3', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 1, 7, 0, 156, 32239, 'Phase 3', 'Warlock', 'Demonology', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 1, 8, 0, 156, 32586, 'Phase 3', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 1, 9, 0, 156, 31050, 'Phase 3', 'Warlock', 'Demonology', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 1, 10, 0, 156, 32527, 'Phase 3', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(9, 1, 11, 0, 156, 32247, 'Phase 3', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 1, 12, 0, 156, 29370, 'Phase 3', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 1, 13, 0, 156, 32483, 'Phase 3', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 1, 14, 0, 156, 32590, 'Phase 3', 'Warlock', 'Demonology', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 1, 15, 0, 156, 32374, 'Phase 3', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 1, 17, 0, 156, 29982, 'Phase 3', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 164, 31051, 'Phase 4', 'Warlock', 'Demonology', 'Head', 'Both', 'Hood of the Malefic'),
(9, 1, 1, 0, 164, 30015, 'Phase 4', 'Warlock', 'Demonology', 'Neck', 'Both', 'Sun King''s Talisman'),
(9, 1, 2, 0, 164, 31054, 'Phase 4', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 1, 4, 0, 164, 30107, 'Phase 4', 'Warlock', 'Demonology', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 1, 5, 0, 164, 30888, 'Phase 4', 'Warlock', 'Demonology', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 1, 6, 0, 164, 31053, 'Phase 4', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 1, 7, 0, 164, 32239, 'Phase 4', 'Warlock', 'Demonology', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 1, 8, 0, 164, 32586, 'Phase 4', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 1, 9, 0, 164, 31050, 'Phase 4', 'Warlock', 'Demonology', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 1, 10, 0, 164, 33497, 'Phase 4', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Mana Attuned Band'),
(9, 1, 11, 0, 164, 32247, 'Phase 4', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 1, 12, 0, 164, 33829, 'Phase 4', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Hex Shrunken Head'),
(9, 1, 13, 0, 164, 32483, 'Phase 4', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 1, 14, 0, 164, 32590, 'Phase 4', 'Warlock', 'Demonology', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 1, 15, 0, 164, 32374, 'Phase 4', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 1, 16, 0, 164, 34179, 'Phase 5', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Heart of the Pit'),
(9, 1, 17, 0, 164, 33192, 'Phase 4', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Carved Witch Doctor''s Stick');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 200, 37684, 'Pre-Raid', 'Warlock', 'Demonology', 'Head', 'Both', 'Forgotten Shadow Hood'),
(9, 1, 1, 0, 200, 40680, 'Pre-Raid', 'Warlock', 'Demonology', 'Neck', 'Both', 'Encircling Burnished Gold Chains'),
(9, 1, 2, 0, 200, 37196, 'Pre-Raid', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Runecaster''s Mantle'),
(9, 1, 4, 0, 200, 39497, 'Pre-Raid', 'Warlock', 'Demonology', 'Chest', 'Both', 'Heroes'' Plagueheart Robe'),
(9, 1, 5, 0, 200, 37408, 'Pre-Raid', 'Warlock', 'Demonology', 'Waist', 'Both', 'Girdle of Bane'),
(9, 1, 6, 0, 200, 34170, 'Pre-Raid', 'Warlock', 'Demonology', 'Legs', 'Both', 'Pantaloons of Calming Strife'),
(9, 1, 7, 0, 200, 44202, 'Pre-Raid', 'Warlock', 'Demonology', 'Feet', 'Both', 'Sandals of Crimson Fury'),
(9, 1, 8, 0, 200, 37361, 'Pre-Raid', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(9, 1, 9, 0, 200, 42113, 'Pre-Raid', 'Warlock', 'Demonology', 'Hands', 'Both', 'Spellweave Gloves'),
(9, 1, 10, 0, 200, 40585, 'Pre-Raid', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Signet of the Kirin Tor'),
(9, 1, 12, 0, 200, 40682, 'Pre-Raid', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Sundial of the Exiled'),
(9, 1, 14, 0, 200, 41610, 'Pre-Raid', 'Warlock', 'Demonology', 'Back', 'Both', 'Deathchill Cloak'),
(9, 1, 17, 0, 200, 34348, 'Pre-Raid', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of Cleansing Light');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 224, 40421, 'Phase 1', 'Warlock', 'Demonology', 'Head', 'Both', 'Valorous Plagueheart Circlet'),
(9, 1, 1, 0, 224, 44661, 'Phase 1', 'Warlock', 'Demonology', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(9, 1, 2, 0, 224, 40424, 'Phase 1', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Valorous Plagueheart Shoulderpads'),
(9, 1, 4, 0, 224, 40423, 'Phase 1', 'Warlock', 'Demonology', 'Chest', 'Both', 'Valorous Plagueheart Robe'),
(9, 1, 5, 0, 224, 40561, 'Phase 1', 'Warlock', 'Demonology', 'Waist', 'Both', 'Leash of Heedless Magic'),
(9, 1, 6, 0, 224, 40560, 'Phase 1', 'Warlock', 'Demonology', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(9, 1, 7, 0, 224, 40558, 'Phase 1', 'Warlock', 'Demonology', 'Feet', 'Both', 'Arcanic Tramplers'),
(9, 1, 8, 0, 224, 44008, 'Phase 1', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Unsullied Cuffs'),
(9, 1, 9, 0, 224, 40420, 'Phase 1', 'Warlock', 'Demonology', 'Hands', 'Both', 'Valorous Plagueheart Gloves'),
(9, 1, 10, 0, 224, 40399, 'Phase 1', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(9, 1, 12, 0, 224, 40432, 'Phase 1', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(9, 1, 14, 0, 224, 44005, 'Phase 1', 'Warlock', 'Demonology', 'Back', 'Both', 'Pennant Cloak'),
(9, 1, 15, 0, 224, 40396, 'Phase 1', 'Warlock', 'Demonology', 'MainHand', 'Both', 'The Turning Tide'),
(9, 1, 17, 0, 224, 39426, 'Phase 1', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Wand of the Archlich');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 245, 45497, 'Phase 2', 'Warlock', 'Demonology', 'Head', 'Both', 'Crown of Luminescence'),
(9, 1, 1, 0, 245, 45133, 'Phase 2', 'Warlock', 'Demonology', 'Neck', 'Both', 'Pendant of Fiery Havoc'),
(9, 1, 2, 0, 245, 46068, 'Phase 2', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Amice of Inconceivable Horror'),
(9, 1, 4, 0, 245, 46137, 'Phase 2', 'Warlock', 'Demonology', 'Chest', 'Both', 'Conqueror''s Deathbringer Robe'),
(9, 1, 9, 0, 245, 45665, 'Phase 2', 'Warlock', 'Demonology', 'Hands', 'Both', 'Pharos Gloves'),
(9, 1, 10, 0, 245, 45495, 'Phase 2', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Conductive Seal'),
(9, 1, 14, 0, 245, 45618, 'Phase 2', 'Warlock', 'Demonology', 'Back', 'Both', 'Sunglimmer Cloak'),
(9, 1, 15, 0, 245, 45620, 'Phase 2', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Starshard Edge'),
(9, 1, 17, 0, 245, 45294, 'Phase 2', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 10, 0, 258, 45495, 'Phase 3', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Conductive Seal'),
(9, 1, 17, 0, 258, 45294, 'Phase 3', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 1, 0, 0, 290, 51231, 'Phase 5', 'Warlock', 'Demonology', 'Head', 'Both', 'Sanctified Dark Coven Hood'),
(9, 1, 1, 0, 290, 50182, 'Phase 5', 'Warlock', 'Demonology', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(9, 1, 2, 0, 290, 51234, 'Phase 5', 'Warlock', 'Demonology', 'Shoulders', 'Both', 'Sanctified Dark Coven Shoulderpads'),
(9, 1, 4, 0, 290, 51233, 'Phase 5', 'Warlock', 'Demonology', 'Chest', 'Both', 'Sanctified Dark Coven Robe'),
(9, 1, 5, 0, 290, 50613, 'Phase 5', 'Warlock', 'Demonology', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(9, 1, 6, 0, 290, 50694, 'Phase 5', 'Warlock', 'Demonology', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(9, 1, 7, 0, 290, 50699, 'Phase 5', 'Warlock', 'Demonology', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(9, 1, 8, 0, 290, 54582, 'Phase 5', 'Warlock', 'Demonology', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(9, 1, 9, 0, 290, 51230, 'Phase 5', 'Warlock', 'Demonology', 'Hands', 'Both', 'Sanctified Dark Coven Gloves'),
(9, 1, 10, 0, 290, 50614, 'Phase 5', 'Warlock', 'Demonology', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(9, 1, 11, 0, 290, 50664, 'Phase 5', 'Warlock', 'Demonology', 'Finger2', 'Both', 'Ring of Rapid Ascent'),
(9, 1, 12, 0, 290, 54588, 'Phase 5', 'Warlock', 'Demonology', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(9, 1, 13, 0, 290, 50365, 'Phase 5', 'Warlock', 'Demonology', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(9, 1, 14, 0, 290, 54583, 'Phase 5', 'Warlock', 'Demonology', 'Back', 'Both', 'Cloak of Burning Dusk'),
(9, 1, 15, 0, 290, 50732, 'Phase 5', 'Warlock', 'Demonology', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(9, 1, 16, 0, 290, 50719, 'Phase 5', 'Warlock', 'Demonology', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(9, 1, 17, 0, 290, 50684, 'Phase 5', 'Warlock', 'Demonology', 'Ranged', 'Both', 'Corpse-Impaling Spike');

-- Destruction (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 1, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Head', 'Alliance', 'Green Lens'),
(9, 2, 0, 2, 66, 10504, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Head', 'Horde', 'Green Lens'),
(9, 2, 1, 1, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 2, 1, 2, 66, 18691, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 2, 2, 1, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Shoulders', 'Alliance', 'Felcloth Shoulders'),
(9, 2, 2, 2, 66, 14112, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Shoulders', 'Horde', 'Felcloth Shoulders'),
(9, 2, 4, 1, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 2, 4, 2, 66, 14153, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Chest', 'Horde', 'Robe of the Void'),
(9, 2, 5, 1, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 2, 5, 2, 66, 11662, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 2, 6, 1, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 2, 6, 2, 66, 13170, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 2, 7, 1, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 2, 7, 2, 66, 18735, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 2, 8, 1, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 2, 8, 2, 66, 11766, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 2, 9, 1, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Hands', 'Alliance', 'Hands of Power'),
(9, 2, 9, 2, 66, 13253, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Hands', 'Horde', 'Hands of Power'),
(9, 2, 10, 1, 66, 12543, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 2, 10, 2, 66, 12545, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 2, 11, 1, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 2, 11, 2, 66, 13001, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 2, 12, 1, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 2, 12, 2, 66, 12930, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 2, 13, 1, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket2', 'Alliance', 'Eye of the Beast'),
(9, 2, 13, 2, 66, 13968, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket2', 'Horde', 'Eye of the Beast'),
(9, 2, 14, 1, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Back', 'Alliance', 'Archivist Cape'),
(9, 2, 14, 2, 66, 13386, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Back', 'Horde', 'Archivist Cape'),
(9, 2, 15, 1, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'MainHand', 'Alliance', 'Witchblade'),
(9, 2, 15, 2, 66, 13964, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'MainHand', 'Horde', 'Witchblade'),
(9, 2, 16, 1, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'OffHand', 'Alliance', 'Drakestone'),
(9, 2, 16, 2, 66, 10796, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'OffHand', 'Horde', 'Drakestone'),
(9, 2, 17, 1, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 2, 17, 2, 66, 13396, 'Phase 1 (Pre-Raid)', 'Warlock', 'Destruction', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 1, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Head', 'Alliance', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 2, 0, 2, 76, 23310, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Head', 'Horde', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 2, 1, 1, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Neck', 'Alliance', 'Dark Advisor''s Pendant'),
(9, 2, 1, 2, 76, 18691, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Neck', 'Horde', 'Dark Advisor''s Pendant'),
(9, 2, 2, 1, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Shoulders', 'Alliance', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 2, 2, 2, 76, 23311, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Shoulders', 'Horde', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 2, 4, 1, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Chest', 'Alliance', 'Robe of the Void'),
(9, 2, 4, 2, 76, 14153, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Chest', 'Horde', 'Robe of the Void'),
(9, 2, 5, 1, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Waist', 'Alliance', 'Ban''thok Sash'),
(9, 2, 5, 2, 76, 11662, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Waist', 'Horde', 'Ban''thok Sash'),
(9, 2, 6, 1, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(9, 2, 6, 2, 76, 13170, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Legs', 'Horde', 'Skyshroud Leggings'),
(9, 2, 7, 1, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Feet', 'Alliance', 'Maleki''s Footwraps'),
(9, 2, 7, 2, 76, 18735, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Feet', 'Horde', 'Maleki''s Footwraps'),
(9, 2, 8, 1, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(9, 2, 8, 2, 76, 11766, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(9, 2, 9, 1, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Hands', 'Alliance', 'Felcloth Gloves'),
(9, 2, 9, 2, 76, 18407, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Hands', 'Horde', 'Felcloth Gloves'),
(9, 2, 10, 1, 76, 12543, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(9, 2, 10, 2, 76, 12545, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(9, 2, 11, 1, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(9, 2, 11, 2, 76, 13001, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Finger2', 'Horde', 'Maiden''s Circle'),
(9, 2, 12, 1, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(9, 2, 12, 2, 76, 12930, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket1', 'Horde', 'Briarwood Reed'),
(9, 2, 13, 1, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket2', 'Alliance', 'Royal Seal of Eldre''Thalas'),
(9, 2, 13, 2, 76, 18467, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Trinket2', 'Horde', 'Royal Seal of Eldre''Thalas'),
(9, 2, 14, 1, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Back', 'Alliance', 'Archivist Cape'),
(9, 2, 14, 2, 76, 13386, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Back', 'Horde', 'Archivist Cape'),
(9, 2, 15, 1, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'MainHand', 'Alliance', 'Blade of the New Moon'),
(9, 2, 15, 2, 76, 18372, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'MainHand', 'Horde', 'Blade of the New Moon'),
(9, 2, 16, 1, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'OffHand', 'Alliance', 'Drakestone'),
(9, 2, 16, 2, 76, 10796, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'OffHand', 'Horde', 'Drakestone'),
(9, 2, 17, 1, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Ranged', 'Alliance', 'Skul''s Ghastly Touch'),
(9, 2, 17, 2, 76, 13396, 'Phase 2 (Pre-Raid)', 'Warlock', 'Destruction', 'Ranged', 'Horde', 'Skul''s Ghastly Touch');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 78, 23310, 'Phase 2', 'Warlock', 'Destruction', 'Head', 'Both', 'Lieutenant Commander''s Dreadweave Cowl'),
(9, 2, 1, 0, 78, 18814, 'Phase 2', 'Warlock', 'Destruction', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 2, 2, 0, 78, 23311, 'Phase 2', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Lieutenant Commander''s Dreadweave Spaulders'),
(9, 2, 4, 0, 78, 19145, 'Phase 2', 'Warlock', 'Destruction', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 2, 5, 0, 78, 18809, 'Phase 2', 'Warlock', 'Destruction', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 2, 6, 0, 78, 19133, 'Phase 2', 'Warlock', 'Destruction', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 2, 7, 0, 78, 19131, 'Phase 2', 'Warlock', 'Destruction', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 2, 8, 0, 78, 11766, 'Phase 2', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Flameweave Cuffs'),
(9, 2, 9, 0, 78, 18407, 'Phase 2', 'Warlock', 'Destruction', 'Hands', 'Both', 'Felcloth Gloves'),
(9, 2, 10, 0, 78, 19147, 'Phase 2', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ring of Spell Power'),
(9, 2, 11, 0, 78, 19147, 'Phase 2', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of Spell Power'),
(9, 2, 12, 0, 78, 12930, 'Phase 2', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Briarwood Reed'),
(9, 2, 13, 0, 78, 18820, 'Phase 2', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 2, 14, 0, 78, 13386, 'Phase 2', 'Warlock', 'Destruction', 'Back', 'Both', 'Archivist Cape'),
(9, 2, 15, 0, 78, 17103, 'Phase 2', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Azuresong Mageblade'),
(9, 2, 16, 0, 78, 10796, 'Phase 2', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Drakestone'),
(9, 2, 17, 0, 78, 13396, 'Phase 2', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 83, 19375, 'Phase 3', 'Warlock', 'Destruction', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(9, 2, 1, 0, 83, 18814, 'Phase 3', 'Warlock', 'Destruction', 'Neck', 'Both', 'Choker of the Fire Lord'),
(9, 2, 2, 0, 83, 19370, 'Phase 3', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(9, 2, 4, 0, 83, 19145, 'Phase 3', 'Warlock', 'Destruction', 'Chest', 'Both', 'Robe of Volatile Power'),
(9, 2, 5, 0, 83, 18809, 'Phase 3', 'Warlock', 'Destruction', 'Waist', 'Both', 'Sash of Whispered Secrets'),
(9, 2, 6, 0, 83, 19133, 'Phase 3', 'Warlock', 'Destruction', 'Legs', 'Both', 'Fel Infused Leggings'),
(9, 2, 7, 0, 83, 19131, 'Phase 3', 'Warlock', 'Destruction', 'Feet', 'Both', 'Snowblind Shoes'),
(9, 2, 8, 0, 83, 19374, 'Phase 3', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(9, 2, 9, 0, 83, 19407, 'Phase 3', 'Warlock', 'Destruction', 'Hands', 'Both', 'Ebony Flame Gloves'),
(9, 2, 11, 0, 83, 19434, 'Phase 3', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Band of Dark Dominion'),
(9, 2, 12, 0, 83, 19379, 'Phase 3', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 2, 13, 0, 83, 18820, 'Phase 3', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 2, 14, 0, 83, 19378, 'Phase 3', 'Warlock', 'Destruction', 'Back', 'Both', 'Cloak of the Brood Lord'),
(9, 2, 15, 0, 83, 19356, 'Phase 3', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Staff of the Shadow Flame'),
(9, 2, 17, 0, 83, 13396, 'Phase 3', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Skul''s Ghastly Touch');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 88, 21337, 'Phase 5', 'Warlock', 'Destruction', 'Head', 'Both', 'Doomcaller''s Circlet'),
(9, 2, 1, 0, 88, 21608, 'Phase 5', 'Warlock', 'Destruction', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 2, 2, 0, 88, 21335, 'Phase 5', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Doomcaller''s Mantle'),
(9, 2, 4, 0, 88, 19682, 'Phase 5', 'Warlock', 'Destruction', 'Chest', 'Both', 'Bloodvine Vest'),
(9, 2, 5, 0, 88, 22730, 'Phase 5', 'Warlock', 'Destruction', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 2, 6, 0, 88, 19683, 'Phase 5', 'Warlock', 'Destruction', 'Legs', 'Both', 'Bloodvine Leggings'),
(9, 2, 7, 0, 88, 19684, 'Phase 5', 'Warlock', 'Destruction', 'Feet', 'Both', 'Bloodvine Boots'),
(9, 2, 8, 0, 88, 21186, 'Phase 5', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 2, 9, 0, 88, 21585, 'Phase 5', 'Warlock', 'Destruction', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 2, 10, 0, 88, 21417, 'Phase 5', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ring of Unspoken Names'),
(9, 2, 11, 0, 88, 21709, 'Phase 5', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 2, 12, 0, 88, 19379, 'Phase 5', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 2, 13, 0, 88, 18820, 'Phase 5', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(9, 2, 14, 0, 88, 22731, 'Phase 5', 'Warlock', 'Destruction', 'Back', 'Both', 'Cloak of the Devoured'),
(9, 2, 15, 0, 88, 21622, 'Phase 5', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Sharpened Silithid Femur'),
(9, 2, 16, 0, 88, 21597, 'Phase 5', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(9, 2, 17, 0, 88, 21603, 'Phase 5', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Wand of Qiraji Nobility');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 92, 22506, 'Phase 6', 'Warlock', 'Destruction', 'Head', 'Both', 'Plagueheart Circlet'),
(9, 2, 1, 0, 92, 21608, 'Phase 6', 'Warlock', 'Destruction', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(9, 2, 2, 0, 92, 22507, 'Phase 6', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Plagueheart Shoulderpads'),
(9, 2, 4, 0, 92, 22504, 'Phase 6', 'Warlock', 'Destruction', 'Chest', 'Both', 'Plagueheart Robe'),
(9, 2, 5, 0, 92, 22730, 'Phase 6', 'Warlock', 'Destruction', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(9, 2, 6, 0, 92, 23070, 'Phase 6', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of Polarity'),
(9, 2, 7, 0, 92, 22508, 'Phase 6', 'Warlock', 'Destruction', 'Feet', 'Both', 'Plagueheart Sandals'),
(9, 2, 8, 0, 92, 21186, 'Phase 6', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Rockfury Bracers'),
(9, 2, 9, 0, 92, 21585, 'Phase 6', 'Warlock', 'Destruction', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(9, 2, 10, 0, 92, 23031, 'Phase 6', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Band of the Inevitable'),
(9, 2, 11, 0, 92, 21709, 'Phase 6', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of the Fallen God'),
(9, 2, 12, 0, 92, 19379, 'Phase 6', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(9, 2, 13, 0, 92, 23046, 'Phase 6', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(9, 2, 14, 0, 92, 23050, 'Phase 6', 'Warlock', 'Destruction', 'Back', 'Both', 'Cloak of the Necropolis'),
(9, 2, 15, 0, 92, 22807, 'Phase 6', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Wraith Blade'),
(9, 2, 16, 0, 92, 23049, 'Phase 6', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(9, 2, 17, 0, 92, 22820, 'Phase 6', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Wand of Fates');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 120, 24266, 'Pre-Raid', 'Warlock', 'Destruction', 'Head', 'Both', 'Spellstrike Hood'),
(9, 2, 1, 0, 120, 28134, 'Pre-Raid', 'Warlock', 'Destruction', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(9, 2, 2, 0, 120, 21869, 'Pre-Raid', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Frozen Shadoweave Shoulders'),
(9, 2, 4, 0, 120, 21871, 'Pre-Raid', 'Warlock', 'Destruction', 'Chest', 'Both', 'Frozen Shadoweave Robe'),
(9, 2, 5, 0, 120, 24256, 'Pre-Raid', 'Warlock', 'Destruction', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 2, 6, 0, 120, 24262, 'Pre-Raid', 'Warlock', 'Destruction', 'Legs', 'Both', 'Spellstrike Pants'),
(9, 2, 7, 0, 120, 21870, 'Pre-Raid', 'Warlock', 'Destruction', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 2, 8, 0, 120, 24250, 'Pre-Raid', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 2, 9, 0, 120, 24450, 'Pre-Raid', 'Warlock', 'Destruction', 'Hands', 'Both', 'Manaspark Gloves'),
(9, 2, 10, 0, 120, 29172, 'Pre-Raid', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 2, 11, 0, 120, 28227, 'Pre-Raid', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(9, 2, 12, 0, 120, 29370, 'Pre-Raid', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 2, 13, 0, 120, 29132, 'Pre-Raid', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Scryer''s Bloodgem'),
(9, 2, 14, 0, 120, 27981, 'Pre-Raid', 'Warlock', 'Destruction', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(9, 2, 15, 0, 120, 23554, 'Pre-Raid', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Eternium Runed Blade'),
(9, 2, 16, 0, 120, 29272, 'Pre-Raid', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Orb of the Soul-Eater'),
(9, 2, 17, 0, 120, 29350, 'Pre-Raid', 'Warlock', 'Destruction', 'Ranged', 'Both', 'The Black Stalk');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 125, 28963, 'Phase 1', 'Warlock', 'Destruction', 'Head', 'Both', 'Voidheart Crown'),
(9, 2, 1, 0, 125, 28530, 'Phase 1', 'Warlock', 'Destruction', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(9, 2, 2, 0, 125, 28967, 'Phase 1', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 2, 4, 0, 125, 28964, 'Phase 1', 'Warlock', 'Destruction', 'Chest', 'Both', 'Voidheart Robe'),
(9, 2, 5, 0, 125, 24256, 'Phase 1', 'Warlock', 'Destruction', 'Waist', 'Both', 'Girdle of Ruination'),
(9, 2, 6, 0, 125, 30734, 'Phase 1', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of the Seventh Circle'),
(9, 2, 7, 0, 125, 21870, 'Phase 1', 'Warlock', 'Destruction', 'Feet', 'Both', 'Frozen Shadoweave Boots'),
(9, 2, 8, 0, 125, 24250, 'Phase 1', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Havok'),
(9, 2, 9, 0, 125, 28968, 'Phase 1', 'Warlock', 'Destruction', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 2, 10, 0, 125, 29172, 'Phase 1', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ashyen''s Gift'),
(9, 2, 11, 0, 125, 28793, 'Phase 1', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Band of Crimson Fury'),
(9, 2, 12, 0, 125, 29370, 'Phase 1', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 2, 13, 0, 125, 27683, 'Phase 1', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 2, 14, 0, 125, 28766, 'Phase 1', 'Warlock', 'Destruction', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 2, 15, 0, 125, 30723, 'Phase 1', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Talon of the Tempest'),
(9, 2, 16, 0, 125, 29272, 'Phase 1', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Orb of the Soul-Eater'),
(9, 2, 17, 0, 125, 28783, 'Phase 1', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Eredar Wand of Obliteration');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 141, 32494, 'Phase 2', 'Warlock', 'Destruction', 'Head', 'Both', 'Destruction Holo-gogs'),
(9, 2, 1, 0, 141, 30015, 'Phase 2', 'Warlock', 'Destruction', 'Neck', 'Both', 'The Sun King''s Talisman'),
(9, 2, 2, 0, 141, 28967, 'Phase 2', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Voidheart Mantle'),
(9, 2, 4, 0, 141, 30107, 'Phase 2', 'Warlock', 'Destruction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 2, 5, 0, 141, 30038, 'Phase 2', 'Warlock', 'Destruction', 'Waist', 'Both', 'Belt of Blasting'),
(9, 2, 6, 0, 141, 30213, 'Phase 2', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of the Corruptor'),
(9, 2, 7, 0, 141, 30037, 'Phase 2', 'Warlock', 'Destruction', 'Feet', 'Both', 'Boots of Blasting'),
(9, 2, 8, 0, 141, 29918, 'Phase 2', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Mindstorm Wristbands'),
(9, 2, 9, 0, 141, 28968, 'Phase 2', 'Warlock', 'Destruction', 'Hands', 'Both', 'Voidheart Gloves'),
(9, 2, 10, 0, 141, 29302, 'Phase 2', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Band of Eternity'),
(9, 2, 11, 0, 141, 30109, 'Phase 2', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of Endless Coils'),
(9, 2, 12, 0, 141, 29370, 'Phase 2', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 2, 13, 0, 141, 27683, 'Phase 2', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(9, 2, 14, 0, 141, 28766, 'Phase 2', 'Warlock', 'Destruction', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(9, 2, 15, 0, 141, 30095, 'Phase 2', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Fang of the Leviathan'),
(9, 2, 16, 0, 141, 30049, 'Phase 2', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Fathomstone'),
(9, 2, 17, 0, 141, 29982, 'Phase 2', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 156, 31051, 'Phase 3', 'Warlock', 'Destruction', 'Head', 'Both', 'Hood of the Malefic'),
(9, 2, 1, 0, 156, 32589, 'Phase 3', 'Warlock', 'Destruction', 'Neck', 'Both', 'Hellfire-Encased Pendant'),
(9, 2, 2, 0, 156, 31054, 'Phase 3', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 2, 4, 0, 156, 30107, 'Phase 3', 'Warlock', 'Destruction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 2, 5, 0, 156, 30888, 'Phase 3', 'Warlock', 'Destruction', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 2, 6, 0, 156, 31053, 'Phase 3', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 2, 7, 0, 156, 32239, 'Phase 3', 'Warlock', 'Destruction', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 2, 8, 0, 156, 32586, 'Phase 3', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 2, 9, 0, 156, 31050, 'Phase 3', 'Warlock', 'Destruction', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 2, 10, 0, 156, 32527, 'Phase 3', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ring of Ancient Knowledge'),
(9, 2, 11, 0, 156, 32247, 'Phase 3', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 2, 12, 0, 156, 29370, 'Phase 3', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(9, 2, 13, 0, 156, 32483, 'Phase 3', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 2, 14, 0, 156, 32590, 'Phase 3', 'Warlock', 'Destruction', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 2, 15, 0, 156, 32374, 'Phase 3', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 2, 17, 0, 156, 29982, 'Phase 3', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Wand of the Forgotten Star');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 164, 31051, 'Phase 4', 'Warlock', 'Destruction', 'Head', 'Both', 'Hood of the Malefic'),
(9, 2, 1, 0, 164, 32589, 'Phase 4', 'Warlock', 'Destruction', 'Neck', 'Both', 'Hellfire-Encased Pendant'),
(9, 2, 2, 0, 164, 31054, 'Phase 4', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Mantle of the Malefic'),
(9, 2, 4, 0, 164, 30107, 'Phase 4', 'Warlock', 'Destruction', 'Chest', 'Both', 'Vestments of the Sea-Witch'),
(9, 2, 5, 0, 164, 30888, 'Phase 4', 'Warlock', 'Destruction', 'Waist', 'Both', 'Anetheron''s Noose'),
(9, 2, 6, 0, 164, 31053, 'Phase 4', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of the Malefic'),
(9, 2, 7, 0, 164, 32239, 'Phase 4', 'Warlock', 'Destruction', 'Feet', 'Both', 'Slippers of the Seacaller'),
(9, 2, 8, 0, 164, 32586, 'Phase 4', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Nimble Thought'),
(9, 2, 9, 0, 164, 31050, 'Phase 4', 'Warlock', 'Destruction', 'Hands', 'Both', 'Gloves of the Malefic'),
(9, 2, 10, 0, 164, 33497, 'Phase 4', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Mana Attuned Band'),
(9, 2, 11, 0, 164, 32247, 'Phase 4', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of Captured Storms'),
(9, 2, 12, 0, 164, 33829, 'Phase 4', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Hex Shrunken Head'),
(9, 2, 13, 0, 164, 32483, 'Phase 4', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'The Skull of Gul''dan'),
(9, 2, 14, 0, 164, 32590, 'Phase 4', 'Warlock', 'Destruction', 'Back', 'Both', 'Nethervoid Cloak'),
(9, 2, 15, 0, 164, 32374, 'Phase 4', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Zhar''doom, Greatstaff of the Devourer'),
(9, 2, 16, 0, 164, 34179, 'Phase 5', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Heart of the Pit'),
(9, 2, 17, 0, 164, 33192, 'Phase 4', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Carved Witch Doctor''s Stick');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 200, 42553, 'Pre-Raid', 'Warlock', 'Destruction', 'Head', 'Both', 'Visage Liquification Goggles'),
(9, 2, 1, 0, 200, 39472, 'Pre-Raid', 'Warlock', 'Destruction', 'Neck', 'Both', 'Chain of Latent Energies'),
(9, 2, 2, 0, 200, 34393, 'Pre-Raid', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Shoulderpads of Knowledge''s Pursuit'),
(9, 2, 4, 0, 200, 39497, 'Pre-Raid', 'Warlock', 'Destruction', 'Chest', 'Both', 'Heroes'' Plagueheart Robe'),
(9, 2, 5, 0, 200, 40696, 'Pre-Raid', 'Warlock', 'Destruction', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(9, 2, 6, 0, 200, 34170, 'Pre-Raid', 'Warlock', 'Destruction', 'Legs', 'Both', 'Pantaloons of Calming Strife'),
(9, 2, 7, 0, 200, 40558, 'Pre-Raid', 'Warlock', 'Destruction', 'Feet', 'Both', 'Arcanic Tramplers'),
(9, 2, 8, 0, 200, 37361, 'Pre-Raid', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Cuffs of Winged Levitation'),
(9, 2, 9, 0, 200, 39500, 'Pre-Raid', 'Warlock', 'Destruction', 'Hands', 'Both', 'Heroes'' Plagueheart Gloves'),
(9, 2, 10, 0, 200, 43253, 'Pre-Raid', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Ring of Northern Tears'),
(9, 2, 12, 0, 200, 40682, 'Pre-Raid', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Sundial of the Exiled'),
(9, 2, 14, 0, 200, 41610, 'Pre-Raid', 'Warlock', 'Destruction', 'Back', 'Both', 'Deathchill Cloak'),
(9, 2, 17, 0, 200, 34348, 'Pre-Raid', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Wand of Cleansing Light');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 224, 40421, 'Phase 1', 'Warlock', 'Destruction', 'Head', 'Both', 'Valorous Plagueheart Circlet'),
(9, 2, 1, 0, 224, 44661, 'Phase 1', 'Warlock', 'Destruction', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(9, 2, 2, 0, 224, 40424, 'Phase 1', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Valorous Plagueheart Shoulderpads'),
(9, 2, 4, 0, 224, 40423, 'Phase 1', 'Warlock', 'Destruction', 'Chest', 'Both', 'Valorous Plagueheart Robe'),
(9, 2, 5, 0, 224, 40561, 'Phase 1', 'Warlock', 'Destruction', 'Waist', 'Both', 'Leash of Heedless Magic'),
(9, 2, 6, 0, 224, 40560, 'Phase 1', 'Warlock', 'Destruction', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(9, 2, 7, 0, 224, 40558, 'Phase 1', 'Warlock', 'Destruction', 'Feet', 'Both', 'Arcanic Tramplers'),
(9, 2, 8, 0, 224, 44008, 'Phase 1', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Unsullied Cuffs'),
(9, 2, 9, 0, 224, 40420, 'Phase 1', 'Warlock', 'Destruction', 'Hands', 'Both', 'Valorous Plagueheart Gloves'),
(9, 2, 10, 0, 224, 40399, 'Phase 1', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Signet of Manifested Pain'),
(9, 2, 12, 0, 224, 40432, 'Phase 1', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(9, 2, 14, 0, 224, 44005, 'Phase 1', 'Warlock', 'Destruction', 'Back', 'Both', 'Pennant Cloak'),
(9, 2, 15, 0, 224, 40396, 'Phase 1', 'Warlock', 'Destruction', 'MainHand', 'Both', 'The Turning Tide'),
(9, 2, 17, 0, 224, 39712, 'Phase 1', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Gemmed Wand of the Nerubians');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 245, 45497, 'Phase 2', 'Warlock', 'Destruction', 'Head', 'Both', 'Crown of Luminescence'),
(9, 2, 1, 0, 245, 45133, 'Phase 2', 'Warlock', 'Destruction', 'Neck', 'Both', 'Pendant of Fiery Havoc'),
(9, 2, 2, 0, 245, 46068, 'Phase 2', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Amice of Inconceivable Horror'),
(9, 2, 4, 0, 245, 46137, 'Phase 2', 'Warlock', 'Destruction', 'Chest', 'Both', 'Conqueror''s Deathbringer Robe'),
(9, 2, 9, 0, 245, 45665, 'Phase 2', 'Warlock', 'Destruction', 'Hands', 'Both', 'Pharos Gloves'),
(9, 2, 10, 0, 245, 45495, 'Phase 2', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Conductive Seal'),
(9, 2, 14, 0, 245, 45618, 'Phase 2', 'Warlock', 'Destruction', 'Back', 'Both', 'Sunglimmer Cloak'),
(9, 2, 15, 0, 245, 45620, 'Phase 2', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Starshard Edge'),
(9, 2, 17, 0, 245, 45294, 'Phase 2', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 10, 0, 258, 45495, 'Phase 3', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Conductive Seal'),
(9, 2, 17, 0, 258, 45294, 'Phase 3', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Petrified Ivy Sprig');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(9, 2, 0, 0, 290, 51231, 'Phase 5', 'Warlock', 'Destruction', 'Head', 'Both', 'Sanctified Dark Coven Hood'),
(9, 2, 1, 0, 290, 50182, 'Phase 5', 'Warlock', 'Destruction', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(9, 2, 2, 0, 290, 51234, 'Phase 5', 'Warlock', 'Destruction', 'Shoulders', 'Both', 'Sanctified Dark Coven Shoulderpads'),
(9, 2, 4, 0, 290, 51233, 'Phase 5', 'Warlock', 'Destruction', 'Chest', 'Both', 'Sanctified Dark Coven Robe'),
(9, 2, 5, 0, 290, 50613, 'Phase 5', 'Warlock', 'Destruction', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(9, 2, 6, 0, 290, 50694, 'Phase 5', 'Warlock', 'Destruction', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(9, 2, 7, 0, 290, 50699, 'Phase 5', 'Warlock', 'Destruction', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(9, 2, 8, 0, 290, 54582, 'Phase 5', 'Warlock', 'Destruction', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(9, 2, 9, 0, 290, 51230, 'Phase 5', 'Warlock', 'Destruction', 'Hands', 'Both', 'Sanctified Dark Coven Gloves'),
(9, 2, 10, 0, 290, 50614, 'Phase 5', 'Warlock', 'Destruction', 'Finger1', 'Both', 'Loop of the Endless Labyrinth'),
(9, 2, 11, 0, 290, 50664, 'Phase 5', 'Warlock', 'Destruction', 'Finger2', 'Both', 'Ring of Rapid Ascent'),
(9, 2, 12, 0, 290, 54588, 'Phase 5', 'Warlock', 'Destruction', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(9, 2, 13, 0, 290, 50365, 'Phase 5', 'Warlock', 'Destruction', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(9, 2, 14, 0, 290, 54583, 'Phase 5', 'Warlock', 'Destruction', 'Back', 'Both', 'Cloak of Burning Dusk'),
(9, 2, 15, 0, 290, 50732, 'Phase 5', 'Warlock', 'Destruction', 'MainHand', 'Both', 'Bloodsurge, Kel''Thuzad''s Blade of Agony'),
(9, 2, 16, 0, 290, 50719, 'Phase 5', 'Warlock', 'Destruction', 'OffHand', 'Both', 'Shadow Silk Spindle'),
(9, 2, 17, 0, 290, 50684, 'Phase 5', 'Warlock', 'Destruction', 'Ranged', 'Both', 'Corpse-Impaling Spike');


-- ============================================================
-- Druid (11)
-- ============================================================
-- Balance (tab 0)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 1, 66, 10504, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Head', 'Alliance', 'Green Lens'),
(11, 0, 0, 2, 66, 10504, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Head', 'Horde', 'Green Lens'),
(11, 0, 1, 1, 66, 12103, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Neck', 'Alliance', 'Star of Mystaria'),
(11, 0, 1, 2, 66, 12103, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Neck', 'Horde', 'Star of Mystaria'),
(11, 0, 2, 1, 66, 13013, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Shoulders', 'Alliance', 'Elder Wizard''s Mantle'),
(11, 0, 2, 2, 66, 13013, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Shoulders', 'Horde', 'Elder Wizard''s Mantle'),
(11, 0, 4, 1, 66, 11924, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Chest', 'Alliance', 'Robes of the Royal Crown'),
(11, 0, 4, 2, 66, 11924, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Chest', 'Horde', 'Robes of the Royal Crown'),
(11, 0, 5, 1, 66, 11662, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Waist', 'Alliance', 'Ban''thok Sash'),
(11, 0, 5, 2, 66, 11662, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Waist', 'Horde', 'Ban''thok Sash'),
(11, 0, 6, 1, 66, 13170, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(11, 0, 6, 2, 66, 13170, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Legs', 'Horde', 'Skyshroud Leggings'),
(11, 0, 7, 1, 66, 11822, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Feet', 'Alliance', 'Omnicast Boots'),
(11, 0, 7, 2, 66, 11822, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Feet', 'Horde', 'Omnicast Boots'),
(11, 0, 8, 1, 66, 11766, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(11, 0, 8, 2, 66, 11766, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(11, 0, 9, 1, 66, 13258, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Hands', 'Alliance', 'Slaghide Gauntlets'),
(11, 0, 9, 2, 66, 13258, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Hands', 'Horde', 'Slaghide Gauntlets'),
(11, 0, 10, 1, 66, 12543, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(11, 0, 10, 2, 66, 12545, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(11, 0, 11, 1, 66, 13001, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(11, 0, 11, 2, 66, 13001, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Finger2', 'Horde', 'Maiden''s Circle'),
(11, 0, 12, 1, 66, 12930, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(11, 0, 12, 2, 66, 12930, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Trinket1', 'Horde', 'Briarwood Reed'),
(11, 0, 13, 1, 66, 13968, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Trinket2', 'Alliance', 'Eye of the Beast'),
(11, 0, 13, 2, 66, 13968, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Trinket2', 'Horde', 'Eye of the Beast'),
(11, 0, 14, 1, 66, 13386, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Back', 'Alliance', 'Archivist Cape'),
(11, 0, 14, 2, 66, 13386, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'Back', 'Horde', 'Archivist Cape'),
(11, 0, 15, 1, 66, 13964, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'MainHand', 'Alliance', 'Witchblade'),
(11, 0, 15, 2, 66, 13964, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'MainHand', 'Horde', 'Witchblade'),
(11, 0, 16, 1, 66, 10796, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'OffHand', 'Alliance', 'Drakestone'),
(11, 0, 16, 2, 66, 10796, 'Phase 1 (Pre-Raid)', 'Druid', 'Balance', 'OffHand', 'Horde', 'Drakestone');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 1, 76, 10504, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Head', 'Alliance', 'Green Lens'),
(11, 0, 0, 2, 76, 10504, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Head', 'Horde', 'Green Lens'),
(11, 0, 1, 1, 76, 12103, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Neck', 'Alliance', 'Star of Mystaria'),
(11, 0, 1, 2, 76, 12103, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Neck', 'Horde', 'Star of Mystaria'),
(11, 0, 2, 1, 76, 13013, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Shoulders', 'Alliance', 'Elder Wizard''s Mantle'),
(11, 0, 2, 2, 76, 13013, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Shoulders', 'Horde', 'Elder Wizard''s Mantle'),
(11, 0, 4, 1, 76, 18385, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Chest', 'Alliance', 'Robe of Everlasting Night'),
(11, 0, 4, 2, 76, 18385, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Chest', 'Horde', 'Robe of Everlasting Night'),
(11, 0, 5, 1, 76, 11662, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Waist', 'Alliance', 'Ban''thok Sash'),
(11, 0, 5, 2, 76, 11662, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Waist', 'Horde', 'Ban''thok Sash'),
(11, 0, 6, 1, 76, 13170, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Legs', 'Alliance', 'Skyshroud Leggings'),
(11, 0, 6, 2, 76, 13170, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Legs', 'Horde', 'Skyshroud Leggings'),
(11, 0, 7, 1, 76, 11822, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Feet', 'Alliance', 'Omnicast Boots'),
(11, 0, 7, 2, 76, 11822, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Feet', 'Horde', 'Omnicast Boots'),
(11, 0, 8, 1, 76, 11766, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Wrists', 'Alliance', 'Flameweave Cuffs'),
(11, 0, 8, 2, 76, 11766, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Wrists', 'Horde', 'Flameweave Cuffs'),
(11, 0, 9, 1, 76, 13258, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Hands', 'Alliance', 'Slaghide Gauntlets'),
(11, 0, 9, 2, 76, 13258, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Hands', 'Horde', 'Slaghide Gauntlets'),
(11, 0, 10, 1, 76, 12543, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Finger1', 'Alliance', 'Songstone of Ironforge'),
(11, 0, 10, 2, 76, 12545, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Finger1', 'Horde', 'Eye of Orgrimmar'),
(11, 0, 11, 1, 76, 13001, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Finger2', 'Alliance', 'Maiden''s Circle'),
(11, 0, 11, 2, 76, 13001, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Finger2', 'Horde', 'Maiden''s Circle'),
(11, 0, 12, 1, 76, 12930, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Trinket1', 'Alliance', 'Briarwood Reed'),
(11, 0, 12, 2, 76, 12930, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Trinket1', 'Horde', 'Briarwood Reed'),
(11, 0, 13, 1, 76, 13968, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Trinket2', 'Alliance', 'Eye of the Beast'),
(11, 0, 13, 2, 76, 13968, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Trinket2', 'Horde', 'Eye of the Beast'),
(11, 0, 14, 1, 76, 13386, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Back', 'Alliance', 'Archivist Cape'),
(11, 0, 14, 2, 76, 13386, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'Back', 'Horde', 'Archivist Cape'),
(11, 0, 15, 1, 76, 18534, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'MainHand', 'Alliance', 'Rod of the Ogre Magi'),
(11, 0, 15, 2, 76, 18534, 'Phase 2 (Pre-Raid)', 'Druid', 'Balance', 'MainHand', 'Horde', 'Rod of the Ogre Magi');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 78, 10504, 'Phase 2', 'Druid', 'Balance', 'Head', 'Both', 'Green Lens'),
(11, 0, 1, 0, 78, 18814, 'Phase 2', 'Druid', 'Balance', 'Neck', 'Both', 'Choker of the Fire Lord'),
(11, 0, 2, 0, 78, 16836, 'Phase 2', 'Druid', 'Balance', 'Shoulders', 'Both', 'Cenarion Spaulders'),
(11, 0, 4, 0, 78, 19145, 'Phase 2', 'Druid', 'Balance', 'Chest', 'Both', 'Robe of Volatile Power'),
(11, 0, 5, 0, 78, 19136, 'Phase 2', 'Druid', 'Balance', 'Waist', 'Both', 'Mana Igniting Cord'),
(11, 0, 6, 0, 78, 13170, 'Phase 2', 'Druid', 'Balance', 'Legs', 'Both', 'Skyshroud Leggings'),
(11, 0, 7, 0, 78, 11822, 'Phase 2', 'Druid', 'Balance', 'Feet', 'Both', 'Omnicast Boots'),
(11, 0, 8, 0, 78, 11766, 'Phase 2', 'Druid', 'Balance', 'Wrists', 'Both', 'Flameweave Cuffs'),
(11, 0, 9, 0, 78, 13258, 'Phase 2', 'Druid', 'Balance', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 0, 10, 0, 78, 19147, 'Phase 2', 'Druid', 'Balance', 'Finger1', 'Both', 'Ring of Spell Power'),
(11, 0, 11, 0, 78, 19147, 'Phase 2', 'Druid', 'Balance', 'Finger2', 'Both', 'Ring of Spell Power'),
(11, 0, 12, 0, 78, 12930, 'Phase 2', 'Druid', 'Balance', 'Trinket1', 'Both', 'Briarwood Reed'),
(11, 0, 13, 0, 78, 18820, 'Phase 2', 'Druid', 'Balance', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(11, 0, 14, 0, 78, 13386, 'Phase 2', 'Druid', 'Balance', 'Back', 'Both', 'Archivist Cape'),
(11, 0, 15, 0, 78, 18842, 'Phase 2', 'Druid', 'Balance', 'MainHand', 'Both', 'Staff of Dominance');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 83, 19375, 'Phase 3', 'Druid', 'Balance', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(11, 0, 1, 0, 83, 18814, 'Phase 3', 'Druid', 'Balance', 'Neck', 'Both', 'Choker of the Fire Lord'),
(11, 0, 2, 0, 83, 19370, 'Phase 3', 'Druid', 'Balance', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(11, 0, 4, 0, 83, 19145, 'Phase 3', 'Druid', 'Balance', 'Chest', 'Both', 'Robe of Volatile Power'),
(11, 0, 5, 0, 83, 19400, 'Phase 3', 'Druid', 'Balance', 'Waist', 'Both', 'Firemaw''s Clutch'),
(11, 0, 6, 0, 83, 13170, 'Phase 3', 'Druid', 'Balance', 'Legs', 'Both', 'Skyshroud Leggings'),
(11, 0, 7, 0, 83, 11822, 'Phase 3', 'Druid', 'Balance', 'Feet', 'Both', 'Omnicast Boots'),
(11, 0, 8, 0, 83, 19374, 'Phase 3', 'Druid', 'Balance', 'Wrists', 'Both', 'Bracers of Arcane Accuracy'),
(11, 0, 9, 0, 83, 13258, 'Phase 3', 'Druid', 'Balance', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 0, 11, 0, 83, 19147, 'Phase 3', 'Druid', 'Balance', 'Finger2', 'Both', 'Ring of Spell Power'),
(11, 0, 12, 0, 83, 19379, 'Phase 3', 'Druid', 'Balance', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(11, 0, 13, 0, 83, 18820, 'Phase 3', 'Druid', 'Balance', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(11, 0, 14, 0, 83, 19378, 'Phase 3', 'Druid', 'Balance', 'Back', 'Both', 'Cloak of the Brood Lord'),
(11, 0, 15, 0, 83, 19360, 'Phase 3', 'Druid', 'Balance', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(11, 0, 16, 0, 83, 19308, 'Phase 3', 'Druid', 'Balance', 'OffHand', 'Both', 'Tome of Arcane Domination');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 88, 19375, 'Phase 5', 'Druid', 'Balance', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(11, 0, 1, 0, 88, 21608, 'Phase 5', 'Druid', 'Balance', 'Neck', 'Both', 'Amulet of Vek''nilash'),
(11, 0, 2, 0, 88, 19370, 'Phase 5', 'Druid', 'Balance', 'Shoulders', 'Both', 'Mantle of the Blackwing Cabal'),
(11, 0, 4, 0, 88, 19682, 'Phase 5', 'Druid', 'Balance', 'Chest', 'Both', 'Bloodvine Vest'),
(11, 0, 5, 0, 88, 22730, 'Phase 5', 'Druid', 'Balance', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(11, 0, 6, 0, 88, 19683, 'Phase 5', 'Druid', 'Balance', 'Legs', 'Both', 'Bloodvine Leggings'),
(11, 0, 7, 0, 88, 19684, 'Phase 5', 'Druid', 'Balance', 'Feet', 'Both', 'Bloodvine Boots'),
(11, 0, 8, 0, 88, 21186, 'Phase 5', 'Druid', 'Balance', 'Wrists', 'Both', 'Rockfury Bracers'),
(11, 0, 9, 0, 88, 21585, 'Phase 5', 'Druid', 'Balance', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(11, 0, 11, 0, 88, 21709, 'Phase 5', 'Druid', 'Balance', 'Finger2', 'Both', 'Ring of the Fallen God'),
(11, 0, 12, 0, 88, 19379, 'Phase 5', 'Druid', 'Balance', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(11, 0, 13, 0, 88, 18820, 'Phase 5', 'Druid', 'Balance', 'Trinket2', 'Both', 'Talisman of Ephemeral Power'),
(11, 0, 14, 0, 88, 22731, 'Phase 5', 'Druid', 'Balance', 'Back', 'Both', 'Cloak of the Devoured'),
(11, 0, 15, 0, 88, 19360, 'Phase 5', 'Druid', 'Balance', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(11, 0, 16, 0, 88, 21597, 'Phase 5', 'Druid', 'Balance', 'OffHand', 'Both', 'Royal Scepter of Vek''lor'),
(11, 0, 17, 0, 88, 23197, 'Phase 5', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of the Moon');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 92, 19375, 'Phase 6', 'Druid', 'Balance', 'Head', 'Both', 'Mish''undare, Circlet of the Mind Flayer'),
(11, 0, 1, 0, 92, 23057, 'Phase 6', 'Druid', 'Balance', 'Neck', 'Both', 'Gem of Trapped Innocents'),
(11, 0, 2, 0, 92, 22983, 'Phase 6', 'Druid', 'Balance', 'Shoulders', 'Both', 'Rime Covered Mantle'),
(11, 0, 4, 0, 92, 19682, 'Phase 6', 'Druid', 'Balance', 'Chest', 'Both', 'Bloodvine Vest'),
(11, 0, 5, 0, 92, 22730, 'Phase 6', 'Druid', 'Balance', 'Waist', 'Both', 'Eyestalk Waist Cord'),
(11, 0, 6, 0, 92, 19683, 'Phase 6', 'Druid', 'Balance', 'Legs', 'Both', 'Bloodvine Leggings'),
(11, 0, 7, 0, 92, 19684, 'Phase 6', 'Druid', 'Balance', 'Feet', 'Both', 'Bloodvine Boots'),
(11, 0, 8, 0, 92, 23021, 'Phase 6', 'Druid', 'Balance', 'Wrists', 'Both', 'The Soul Harvester''s Bindings'),
(11, 0, 9, 0, 92, 21585, 'Phase 6', 'Druid', 'Balance', 'Hands', 'Both', 'Dark Storm Gauntlets'),
(11, 0, 10, 0, 92, 23025, 'Phase 6', 'Druid', 'Balance', 'Finger1', 'Both', 'Seal of the Damned'),
(11, 0, 11, 0, 92, 21709, 'Phase 6', 'Druid', 'Balance', 'Finger2', 'Both', 'Ring of the Fallen God'),
(11, 0, 12, 0, 92, 19379, 'Phase 6', 'Druid', 'Balance', 'Trinket1', 'Both', 'Neltharion''s Tear'),
(11, 0, 13, 0, 92, 23046, 'Phase 6', 'Druid', 'Balance', 'Trinket2', 'Both', 'The Restrained Essence of Sapphiron'),
(11, 0, 14, 0, 92, 23050, 'Phase 6', 'Druid', 'Balance', 'Back', 'Both', 'Cloak of the Necropolis'),
(11, 0, 15, 0, 92, 22988, 'Phase 6', 'Druid', 'Balance', 'MainHand', 'Both', 'The End of Dreams'),
(11, 0, 16, 0, 92, 23049, 'Phase 6', 'Druid', 'Balance', 'OffHand', 'Both', 'Sapphiron''s Left Eye'),
(11, 0, 17, 0, 92, 23197, 'Phase 6', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of the Moon');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 120, 24266, 'Pre-Raid', 'Druid', 'Balance', 'Head', 'Both', 'Spellstrike Hood'),
(11, 0, 1, 0, 120, 28134, 'Pre-Raid', 'Druid', 'Balance', 'Neck', 'Both', 'Brooch of Heightened Potential'),
(11, 0, 2, 0, 120, 27796, 'Pre-Raid', 'Druid', 'Balance', 'Shoulders', 'Both', 'Mana-Etched Spaulders'),
(11, 0, 4, 0, 120, 21848, 'Pre-Raid', 'Druid', 'Balance', 'Chest', 'Both', 'Spellfire Robe'),
(11, 0, 5, 0, 120, 21846, 'Pre-Raid', 'Druid', 'Balance', 'Waist', 'Both', 'Spellfire Belt'),
(11, 0, 6, 0, 120, 24262, 'Pre-Raid', 'Druid', 'Balance', 'Legs', 'Both', 'Spellstrike Pants'),
(11, 0, 7, 0, 120, 28406, 'Pre-Raid', 'Druid', 'Balance', 'Feet', 'Both', 'Sigil-Laced Boots'),
(11, 0, 8, 0, 120, 24250, 'Pre-Raid', 'Druid', 'Balance', 'Wrists', 'Both', 'Bracers of Havok'),
(11, 0, 9, 0, 120, 21847, 'Pre-Raid', 'Druid', 'Balance', 'Hands', 'Both', 'Spellfire Gloves'),
(11, 0, 10, 0, 120, 29172, 'Pre-Raid', 'Druid', 'Balance', 'Finger1', 'Both', 'Ashyen''s Gift'),
(11, 0, 11, 0, 120, 28227, 'Pre-Raid', 'Druid', 'Balance', 'Finger2', 'Both', 'Sparking Arcanite Ring'),
(11, 0, 12, 0, 120, 29370, 'Pre-Raid', 'Druid', 'Balance', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(11, 0, 13, 0, 120, 27683, 'Pre-Raid', 'Druid', 'Balance', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(11, 0, 14, 0, 120, 27981, 'Pre-Raid', 'Druid', 'Balance', 'Back', 'Both', 'Sethekk Oracle Cloak'),
(11, 0, 15, 0, 120, 30832, 'Pre-Raid', 'Druid', 'Balance', 'MainHand', 'Both', 'Gavel of Unearthed Secrets'),
(11, 0, 16, 0, 120, 29271, 'Pre-Raid', 'Druid', 'Balance', 'OffHand', 'Both', 'Talisman of Kalecgos'),
(11, 0, 17, 0, 120, 27518, 'Pre-Raid', 'Druid', 'Balance', 'Ranged', 'Both', 'Ivory Idol of the Moongoddess');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 125, 29093, 'Phase 1', 'Druid', 'Balance', 'Head', 'Both', 'Antlers of Malorne'),
(11, 0, 1, 0, 125, 28530, 'Phase 1', 'Druid', 'Balance', 'Neck', 'Both', 'Brooch of Unquenchable Fury'),
(11, 0, 2, 0, 125, 29095, 'Phase 1', 'Druid', 'Balance', 'Shoulders', 'Both', 'Pauldrons of Malorne'),
(11, 0, 4, 0, 125, 21848, 'Phase 1', 'Druid', 'Balance', 'Chest', 'Both', 'Spellfire Robe'),
(11, 0, 5, 0, 125, 21846, 'Phase 1', 'Druid', 'Balance', 'Waist', 'Both', 'Spellfire Belt'),
(11, 0, 6, 0, 125, 24262, 'Phase 1', 'Druid', 'Balance', 'Legs', 'Both', 'Spellstrike Pants'),
(11, 0, 7, 0, 125, 28517, 'Phase 1', 'Druid', 'Balance', 'Feet', 'Both', 'Boots of Foretelling'),
(11, 0, 8, 0, 125, 24250, 'Phase 1', 'Druid', 'Balance', 'Wrists', 'Both', 'Bracers of Havok'),
(11, 0, 9, 0, 125, 21847, 'Phase 1', 'Druid', 'Balance', 'Hands', 'Both', 'Spellfire Gloves'),
(11, 0, 10, 0, 125, 28793, 'Phase 1', 'Druid', 'Balance', 'Finger1', 'Both', 'Band of Crimson Fury'),
(11, 0, 11, 0, 125, 28753, 'Phase 1', 'Druid', 'Balance', 'Finger2', 'Both', 'Ring of Recurrence'),
(11, 0, 12, 0, 125, 29370, 'Phase 1', 'Druid', 'Balance', 'Trinket1', 'Both', 'Icon of the Silver Crescent'),
(11, 0, 13, 0, 125, 27683, 'Phase 1', 'Druid', 'Balance', 'Trinket2', 'Both', 'Quagmirran''s Eye'),
(11, 0, 14, 0, 125, 28766, 'Phase 1', 'Druid', 'Balance', 'Back', 'Both', 'Ruby Drape of the Mysticant'),
(11, 0, 15, 0, 125, 28770, 'Phase 1', 'Druid', 'Balance', 'MainHand', 'Both', 'Nathrezim Mindblade'),
(11, 0, 16, 0, 125, 29271, 'Phase 1', 'Druid', 'Balance', 'OffHand', 'Both', 'Talisman of Kalecgos'),
(11, 0, 17, 0, 125, 27518, 'Phase 1', 'Druid', 'Balance', 'Ranged', 'Both', 'Ivory Idol of the Moongoddess');

-- ilvl 200 (Fresh 80)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 200, 39545, 'Fresh 80', 'Druid', 'Balance', 'Head', 'Both', 'Heroes'' Dreamwalker Cover'),
(11, 0, 1, 0, 200, 44658, 'Fresh 80', 'Druid', 'Balance', 'Neck', 'Both', 'Chain of the Ancient Wyrm'),
(11, 0, 2, 0, 200, 39548, 'Fresh 80', 'Druid', 'Balance', 'Shoulders', 'Both', 'Heroes'' Dreamwalker Mantle'),
(11, 0, 4, 0, 200, 39547, 'Fresh 80', 'Druid', 'Balance', 'Chest', 'Both', 'Heroes'' Dreamwalker Vestments'),
(11, 0, 5, 0, 200, 40696, 'Fresh 80', 'Druid', 'Balance', 'Waist', 'Both', 'Plush Sash of Guzbah'),
(11, 0, 6, 0, 200, 39546, 'Fresh 80', 'Druid', 'Balance', 'Legs', 'Both', 'Heroes'' Dreamwalker Trousers'),
(11, 0, 7, 0, 200, 40519, 'Fresh 80', 'Druid', 'Balance', 'Feet', 'Both', 'Footsteps of Malygos'),
(11, 0, 8, 0, 200, 40740, 'Fresh 80', 'Druid', 'Balance', 'Wrists', 'Both', 'Wraps of the Astral Traveler'),
(11, 0, 9, 0, 200, 39544, 'Fresh 80', 'Druid', 'Balance', 'Hands', 'Both', 'Heroes'' Dreamwalker Gloves'),
(11, 0, 10, 0, 200, 40719, 'Fresh 80', 'Druid', 'Balance', 'Finger1', 'Both', 'Band of Channeled Magic'),
(11, 0, 12, 0, 200, 39229, 'Fresh 80', 'Druid', 'Balance', 'Trinket1', 'Both', 'Embrace of the Spider'),
(11, 0, 14, 0, 200, 40723, 'Fresh 80', 'Druid', 'Balance', 'Back', 'Both', 'Disguise of the Kumiho'),
(11, 0, 15, 0, 200, 39423, 'Fresh 80', 'Druid', 'Balance', 'MainHand', 'Both', 'Hammer of the Astral Plane'),
(11, 0, 17, 0, 200, 40321, 'Fresh 80', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of the Shooting Star');

-- ilvl 213 (Pre-Ulduar)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 213, 40467, 'Pre-Ulduar', 'Druid', 'Balance', 'Head', 'Both', 'Valorous Dreamwalker Cover'),
(11, 0, 1, 0, 213, 44661, 'Pre-Ulduar', 'Druid', 'Balance', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(11, 0, 2, 0, 213, 40470, 'Pre-Ulduar', 'Druid', 'Balance', 'Shoulders', 'Both', 'Valorous Dreamwalker Mantle'),
(11, 0, 4, 0, 213, 40469, 'Pre-Ulduar', 'Druid', 'Balance', 'Chest', 'Both', 'Valorous Dreamwalker Vestments'),
(11, 0, 5, 0, 213, 40561, 'Pre-Ulduar', 'Druid', 'Balance', 'Waist', 'Both', 'Leash of Heedless Magic'),
(11, 0, 6, 0, 213, 40560, 'Pre-Ulduar', 'Druid', 'Balance', 'Legs', 'Both', 'Leggings of the Wanton Spellcaster'),
(11, 0, 7, 0, 213, 40519, 'Pre-Ulduar', 'Druid', 'Balance', 'Feet', 'Both', 'Footsteps of Malygos'),
(11, 0, 8, 0, 213, 40323, 'Pre-Ulduar', 'Druid', 'Balance', 'Wrists', 'Both', 'Esteemed Bindings'),
(11, 0, 9, 0, 213, 40460, 'Pre-Ulduar', 'Druid', 'Balance', 'Hands', 'Both', 'Valorous Dreamwalker Handguards'),
(11, 0, 10, 0, 213, 40080, 'Pre-Ulduar', 'Druid', 'Balance', 'Finger1', 'Both', 'Lost Jewel'),
(11, 0, 12, 0, 213, 40682, 'Pre-Ulduar', 'Druid', 'Balance', 'Trinket1', 'Both', 'Sundial of the Exiled'),
(11, 0, 14, 0, 213, 40405, 'Pre-Ulduar', 'Druid', 'Balance', 'Back', 'Both', 'Cape of the Unworthy Wizard'),
(11, 0, 17, 0, 213, 40321, 'Pre-Ulduar', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of the Shooting Star');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 245, 46191, 'Phase 2', 'Druid', 'Balance', 'Head', 'Both', 'Conqueror''s Nightsong Cover'),
(11, 0, 1, 0, 245, 45933, 'Phase 2', 'Druid', 'Balance', 'Neck', 'Both', 'Pendant of the Shallow Grave'),
(11, 0, 2, 0, 245, 45136, 'Phase 2', 'Druid', 'Balance', 'Shoulders', 'Both', 'Shoulderpads of Dormant Energies'),
(11, 0, 4, 0, 245, 45519, 'Phase 2', 'Druid', 'Balance', 'Chest', 'Both', 'Vestments of the Blind Denizen'),
(11, 0, 5, 0, 245, 45619, 'Phase 2', 'Druid', 'Balance', 'Waist', 'Both', 'Starwatcher''s Binding'),
(11, 0, 6, 0, 245, 46192, 'Phase 2', 'Druid', 'Balance', 'Legs', 'Both', 'Conqueror''s Nightsong Trousers'),
(11, 0, 7, 0, 245, 45537, 'Phase 2', 'Druid', 'Balance', 'Feet', 'Both', 'Treads of the False Oracle'),
(11, 0, 8, 0, 245, 45446, 'Phase 2', 'Druid', 'Balance', 'Wrists', 'Both', 'Grasps of Reason'),
(11, 0, 9, 0, 245, 45665, 'Phase 2', 'Druid', 'Balance', 'Hands', 'Both', 'Pharos Gloves'),
(11, 0, 10, 0, 245, 45495, 'Phase 2', 'Druid', 'Balance', 'Finger1', 'Both', 'Conductive Seal'),
(11, 0, 12, 0, 245, 45466, 'Phase 2', 'Druid', 'Balance', 'Trinket1', 'Both', 'Scale of Fates'),
(11, 0, 14, 0, 245, 45242, 'Phase 2', 'Druid', 'Balance', 'Back', 'Both', 'Drape of Mortal Downfall'),
(11, 0, 17, 0, 245, 40321, 'Phase 2', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of the Shooting Star');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 258, 48171, 'Phase 3', 'Druid', 'Balance', 'Head', 'Both', 'Cover of Triumph'),
(11, 0, 1, 1, 258, 47144, 'Phase 3', 'Druid', 'Balance', 'Neck', 'Alliance', 'Wail of the Val''kyr'),
(11, 0, 1, 2, 258, 47468, 'Phase 3', 'Druid', 'Balance', 'Neck', 'Horde', 'Cry of the Val''kyr'),
(11, 0, 2, 0, 258, 48168, 'Phase 3', 'Druid', 'Balance', 'Shoulders', 'Both', 'Mantle of Triumph'),
(11, 0, 4, 0, 258, 48169, 'Phase 3', 'Druid', 'Balance', 'Chest', 'Both', 'Vestments of Triumph'),
(11, 0, 5, 1, 258, 47084, 'Phase 3', 'Druid', 'Balance', 'Waist', 'Alliance', 'Cord of Biting Cold'),
(11, 0, 5, 2, 258, 47447, 'Phase 3', 'Druid', 'Balance', 'Waist', 'Horde', 'Belt of Biting Cold'),
(11, 0, 6, 1, 258, 47190, 'Phase 3', 'Druid', 'Balance', 'Legs', 'Alliance', 'Legwraps of the Awakening'),
(11, 0, 6, 2, 258, 47479, 'Phase 3', 'Druid', 'Balance', 'Legs', 'Horde', 'Leggings of the Awakening'),
(11, 0, 7, 1, 258, 47097, 'Phase 3', 'Druid', 'Balance', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(11, 0, 7, 2, 258, 47454, 'Phase 3', 'Druid', 'Balance', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(11, 0, 8, 1, 258, 47066, 'Phase 3', 'Druid', 'Balance', 'Wrists', 'Alliance', 'Bracers of the Autumn Willow'),
(11, 0, 8, 2, 258, 47438, 'Phase 3', 'Druid', 'Balance', 'Wrists', 'Horde', 'Bindings of the Autumn Willow'),
(11, 0, 9, 0, 258, 48172, 'Phase 3', 'Druid', 'Balance', 'Hands', 'Both', 'Gloves of Triumph'),
(11, 0, 10, 1, 258, 47237, 'Phase 3', 'Druid', 'Balance', 'Finger1', 'Alliance', 'Band of Deplorable Violence'),
(11, 0, 10, 2, 258, 47489, 'Phase 3', 'Druid', 'Balance', 'Finger1', 'Horde', 'Lurid Manifestation'),
(11, 0, 12, 2, 258, 45518, 'Phase 3', 'Druid', 'Balance', 'Trinket1', 'Horde', 'Flare of the Heavens'),
(11, 0, 14, 1, 258, 47552, 'Phase 3', 'Druid', 'Balance', 'Back', 'Alliance', 'Jaina''s Radiance'),
(11, 0, 14, 2, 258, 47551, 'Phase 3', 'Druid', 'Balance', 'Back', 'Horde', 'Aethas'' Intensity'),
(11, 0, 17, 0, 258, 47670, 'Phase 3', 'Druid', 'Balance', 'Ranged', 'Both', 'Idol of Lunar Fury');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 264, 51290, 'Phase 4', 'Druid', 'Balance', 'Head', 'Both', 'Sanctified Lasherweave Cover'),
(11, 0, 1, 0, 264, 50724, 'Phase 4', 'Druid', 'Balance', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(11, 0, 2, 0, 264, 51292, 'Phase 4', 'Druid', 'Balance', 'Shoulders', 'Both', 'Sanctified Lasherweave Mantle'),
(11, 0, 4, 0, 264, 51294, 'Phase 4', 'Druid', 'Balance', 'Chest', 'Both', 'Sanctified Lasherweave Vestment'),
(11, 0, 5, 0, 264, 50613, 'Phase 4', 'Druid', 'Balance', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(11, 0, 6, 0, 264, 50694, 'Phase 4', 'Druid', 'Balance', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(11, 0, 7, 0, 264, 50699, 'Phase 4', 'Druid', 'Balance', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(11, 0, 8, 0, 264, 50630, 'Phase 4', 'Druid', 'Balance', 'Wrists', 'Both', 'Bracers of Eternal Dreaming'),
(11, 0, 9, 0, 264, 51291, 'Phase 4', 'Druid', 'Balance', 'Hands', 'Both', 'Sanctified Lasherweave Gloves'),
(11, 0, 10, 0, 264, 50398, 'Phase 4', 'Druid', 'Balance', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(11, 0, 12, 0, 264, 50348, 'Phase 4', 'Druid', 'Balance', 'Trinket1', 'Both', 'Dislodged Foreign Object'),
(11, 0, 14, 0, 264, 50628, 'Phase 4', 'Druid', 'Balance', 'Back', 'Both', 'Frostbinder''s Shredded Cape'),
(11, 0, 17, 0, 264, 50719, 'Phase 4', 'Druid', 'Balance', 'Ranged', 'Both', 'Shadow Silk Spindle');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 0, 0, 0, 290, 51290, 'Phase 5', 'Druid', 'Balance', 'Head', 'Both', 'Sanctified Lasherweave Cover'),
(11, 0, 1, 0, 290, 50182, 'Phase 5', 'Druid', 'Balance', 'Neck', 'Both', 'Blood Queen''s Crimson Choker'),
(11, 0, 2, 0, 290, 51292, 'Phase 5', 'Druid', 'Balance', 'Shoulders', 'Both', 'Sanctified Lasherweave Mantle'),
(11, 0, 4, 0, 290, 51294, 'Phase 5', 'Druid', 'Balance', 'Chest', 'Both', 'Sanctified Lasherweave Vestment'),
(11, 0, 5, 0, 290, 50613, 'Phase 5', 'Druid', 'Balance', 'Waist', 'Both', 'Crushing Coldwraith Belt'),
(11, 0, 6, 0, 290, 50694, 'Phase 5', 'Druid', 'Balance', 'Legs', 'Both', 'Plaguebringer''s Stained Pants'),
(11, 0, 7, 0, 290, 50699, 'Phase 5', 'Druid', 'Balance', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(11, 0, 8, 0, 290, 54582, 'Phase 5', 'Druid', 'Balance', 'Wrists', 'Both', 'Bracers of Fiery Night'),
(11, 0, 9, 0, 290, 51291, 'Phase 5', 'Druid', 'Balance', 'Hands', 'Both', 'Sanctified Lasherweave Gloves'),
(11, 0, 10, 0, 290, 50398, 'Phase 5', 'Druid', 'Balance', 'Finger1', 'Both', 'Ashen Band of Endless Destruction'),
(11, 0, 11, 0, 290, 50714, 'Phase 5', 'Druid', 'Balance', 'Finger2', 'Both', 'Valanar''s Other Signet Ring'),
(11, 0, 12, 0, 290, 54588, 'Phase 5', 'Druid', 'Balance', 'Trinket1', 'Both', 'Charred Twilight Scale'),
(11, 0, 13, 0, 290, 50365, 'Phase 5', 'Druid', 'Balance', 'Trinket2', 'Both', 'Phylactery of the Nameless Lich'),
(11, 0, 14, 0, 290, 54583, 'Phase 5', 'Druid', 'Balance', 'Back', 'Both', 'Cloak of Burning Dusk'),
(11, 0, 15, 0, 290, 50734, 'Phase 5', 'Druid', 'Balance', 'MainHand', 'Both', 'Royal Scepter of Terenas II'),
(11, 0, 16, 0, 290, 50719, 'Phase 5', 'Druid', 'Balance', 'OffHand', 'Both', 'Shadow Silk Spindle');

-- Feral Cat (tab 1)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 66, 8345, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 66, 15411, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Mark of Fordring'),
(11, 1, 2, 0, 66, 12927, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(11, 1, 4, 0, 66, 14637, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Cadaverous Armor'),
(11, 1, 5, 0, 66, 13252, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Cloudrunner Girdle'),
(11, 1, 6, 0, 66, 15062, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Devilsaur Leggings'),
(11, 1, 7, 0, 66, 12553, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Swiftwalker Boots'),
(11, 1, 8, 0, 66, 16710, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Shadowcraft Bracers'),
(11, 1, 9, 0, 66, 15063, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(11, 1, 10, 0, 66, 17713, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Blackstone Ring'),
(11, 1, 11, 0, 66, 2246, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Myrmidon''s Signet'),
(11, 1, 12, 0, 66, 13965, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(11, 1, 13, 0, 66, 11815, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Hand of Justice'),
(11, 1, 14, 0, 66, 13340, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cape of the Black Baron'),
(11, 1, 15, 0, 66, 11921, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Impervious Giant');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 76, 8345, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 76, 15411, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Mark of Fordring'),
(11, 1, 2, 0, 76, 12927, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(11, 1, 4, 0, 76, 14637, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Cadaverous Armor'),
(11, 1, 5, 0, 76, 13252, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Cloudrunner Girdle'),
(11, 1, 6, 0, 76, 15062, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Devilsaur Leggings'),
(11, 1, 7, 0, 76, 12553, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Swiftwalker Boots'),
(11, 1, 8, 0, 76, 18375, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Bracers of the Eclipse'),
(11, 1, 9, 0, 76, 15063, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Devilsaur Gauntlets'),
(11, 1, 10, 0, 76, 17713, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Blackstone Ring'),
(11, 1, 11, 0, 76, 18500, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(11, 1, 12, 0, 76, 13965, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(11, 1, 13, 0, 76, 11815, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Hand of Justice'),
(11, 1, 14, 0, 76, 13340, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cape of the Black Baron'),
(11, 1, 15, 0, 76, 18420, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Bonecrusher');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 78, 8345, 'Phase 2', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 78, 18404, 'Phase 2', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Onyxia Tooth Pendant'),
(11, 1, 2, 0, 78, 12927, 'Phase 2', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(11, 1, 4, 0, 78, 14637, 'Phase 2', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Cadaverous Armor'),
(11, 1, 5, 0, 78, 13252, 'Phase 2', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Cloudrunner Girdle'),
(11, 1, 6, 0, 78, 15062, 'Phase 2', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Devilsaur Leggings'),
(11, 1, 7, 0, 78, 12553, 'Phase 2', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Swiftwalker Boots'),
(11, 1, 8, 0, 78, 19146, 'Phase 2', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Wristguards of Stability'),
(11, 1, 9, 0, 78, 18823, 'Phase 2', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Aged Core Leather Gloves'),
(11, 1, 10, 0, 78, 17063, 'Phase 2', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Band of Accuria'),
(11, 1, 11, 0, 78, 18500, 'Phase 2', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Tarnished Elven Ring'),
(11, 1, 12, 0, 78, 13965, 'Phase 2', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Blackhand''s Breadth'),
(11, 1, 13, 0, 78, 11815, 'Phase 2', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Hand of Justice'),
(11, 1, 14, 0, 78, 13340, 'Phase 2', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cape of the Black Baron'),
(11, 1, 15, 0, 78, 18420, 'Phase 2', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Bonecrusher');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 83, 8345, 'Phase 3', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 83, 19377, 'Phase 3', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(11, 1, 2, 0, 83, 12927, 'Phase 3', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Truestrike Shoulders'),
(11, 1, 4, 0, 83, 19405, 'Phase 3', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Malfurion''s Blessed Bulwark'),
(11, 1, 5, 0, 83, 19396, 'Phase 3', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Taut Dragonhide Belt'),
(11, 1, 6, 0, 83, 15062, 'Phase 3', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Devilsaur Leggings'),
(11, 1, 7, 0, 83, 19381, 'Phase 3', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Boots of the Shadow Flame'),
(11, 1, 8, 0, 83, 19146, 'Phase 3', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Wristguards of Stability'),
(11, 1, 9, 0, 83, 18823, 'Phase 3', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Aged Core Leather Gloves'),
(11, 1, 10, 0, 83, 19384, 'Phase 3', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Master Dragonslayer''s Ring'),
(11, 1, 12, 0, 83, 19406, 'Phase 3', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(11, 1, 13, 0, 83, 11815, 'Phase 3', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Hand of Justice'),
(11, 1, 14, 0, 83, 19436, 'Phase 3', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cloak of Draconic Might'),
(11, 1, 15, 0, 83, 19358, 'Phase 3', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Draconic Maul');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 88, 8345, 'Phase 5', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 88, 19377, 'Phase 5', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(11, 1, 2, 0, 88, 21665, 'Phase 5', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Mantle of Wicked Revenge'),
(11, 1, 4, 0, 88, 21680, 'Phase 5', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Vest of Swift Execution'),
(11, 1, 5, 0, 88, 21586, 'Phase 5', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(11, 1, 6, 0, 88, 20665, 'Phase 5', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Abyssal Leather Leggings'),
(11, 1, 7, 0, 88, 21493, 'Phase 5', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Boots of the Vanguard'),
(11, 1, 8, 0, 88, 21602, 'Phase 5', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Qiraji Execution Bracers'),
(11, 1, 10, 0, 88, 21205, 'Phase 5', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Signet Ring of the Bronze Dragonflight'),
(11, 1, 11, 0, 88, 17063, 'Phase 5', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Band of Accuria'),
(11, 1, 12, 0, 88, 19406, 'Phase 5', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(11, 1, 13, 0, 88, 13965, 'Phase 5', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Blackhand''s Breadth'),
(11, 1, 14, 0, 88, 21701, 'Phase 5', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cloak of Concentrated Hatred'),
(11, 1, 15, 0, 88, 21268, 'Phase 5', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Blessed Qiraji War Hammer'),
(11, 1, 16, 0, 88, 13385, 'Phase 5', 'Druid', 'Feral Cat', 'OffHand', 'Both', 'Tome of Knowledge'),
(11, 1, 17, 0, 88, 22397, 'Phase 5', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of Ferocity');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 92, 8345, 'Phase 6', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 92, 19377, 'Phase 6', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Prestor''s Talisman of Connivery'),
(11, 1, 2, 0, 92, 21665, 'Phase 6', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Mantle of Wicked Revenge'),
(11, 1, 4, 0, 92, 23226, 'Phase 6', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Ghoul Skin Tunic'),
(11, 1, 5, 0, 92, 21586, 'Phase 6', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Belt of Never-ending Agony'),
(11, 1, 6, 0, 92, 23071, 'Phase 6', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Leggings of Apocalypse'),
(11, 1, 7, 0, 92, 21493, 'Phase 6', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Boots of the Vanguard'),
(11, 1, 8, 0, 92, 21602, 'Phase 6', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Qiraji Execution Bracers'),
(11, 1, 12, 0, 92, 19406, 'Phase 6', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(11, 1, 13, 0, 92, 23041, 'Phase 6', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Slayer''s Crest'),
(11, 1, 14, 0, 92, 21710, 'Phase 6', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cloak of the Fallen God'),
(11, 1, 15, 0, 92, 22988, 'Phase 6', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'The End of Dreams'),
(11, 1, 16, 0, 92, 13385, 'Phase 6', 'Druid', 'Feral Cat', 'OffHand', 'Both', 'Tome of Knowledge'),
(11, 1, 17, 0, 92, 22397, 'Phase 6', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of Ferocity');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 120, 8345, 'Pre-Raid', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 120, 29381, 'Pre-Raid', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Choker of Vile Intent'),
(11, 1, 2, 0, 120, 27797, 'Pre-Raid', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Wastewalker Shoulderpads'),
(11, 1, 4, 0, 120, 29525, 'Pre-Raid', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Primalstrike Vest'),
(11, 1, 5, 0, 120, 29247, 'Pre-Raid', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(11, 1, 6, 0, 120, 31544, 'Pre-Raid', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Clefthoof Hide Leggings'),
(11, 1, 7, 0, 120, 25686, 'Pre-Raid', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Fel Leather Boots'),
(11, 1, 8, 0, 120, 29246, 'Pre-Raid', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Nightfall Wristguards'),
(11, 1, 9, 0, 120, 29507, 'Pre-Raid', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Windslayer Wraps'),
(11, 1, 10, 0, 120, 30365, 'Pre-Raid', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Overseer''s Signet'),
(11, 1, 11, 0, 120, 31920, 'Pre-Raid', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Shaffar''s Band of Brutality'),
(11, 1, 12, 0, 120, 29383, 'Pre-Raid', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(11, 1, 13, 0, 120, 28034, 'Pre-Raid', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Hourglass of the Unraveller'),
(11, 1, 14, 0, 120, 31255, 'Pre-Raid', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cloak of the Craft'),
(11, 1, 15, 0, 120, 31334, 'Pre-Raid', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Staff of Natural Fury'),
(11, 1, 17, 0, 120, 28372, 'Pre-Raid', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of Feral Shadows');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 125, 8345, 'Phase 1', 'Druid', 'Feral Cat', 'Head', 'Both', 'Wolfshead Helm'),
(11, 1, 1, 0, 125, 29381, 'Phase 1', 'Druid', 'Feral Cat', 'Neck', 'Both', 'Choker of Vile Intent'),
(11, 1, 2, 0, 125, 29100, 'Phase 1', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Mantle of Malorne'),
(11, 1, 4, 0, 125, 29096, 'Phase 1', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Breastplate of Malorne'),
(11, 1, 5, 0, 125, 29247, 'Phase 1', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Girdle of the Deathdealer'),
(11, 1, 6, 0, 125, 28741, 'Phase 1', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Skulker''s Greaves'),
(11, 1, 7, 0, 125, 28545, 'Phase 1', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Edgewalker Longboots'),
(11, 1, 8, 0, 125, 29246, 'Phase 1', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Nightfall Wristguards'),
(11, 1, 9, 0, 125, 28506, 'Phase 1', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Gloves of Dexterous Manipulation'),
(11, 1, 10, 0, 125, 28757, 'Phase 1', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Ring of a Thousand Marks'),
(11, 1, 11, 0, 125, 30834, 'Phase 1', 'Druid', 'Feral Cat', 'Finger2', 'Both', 'Shapeshifter''s Signet'),
(11, 1, 12, 0, 125, 28830, 'Phase 1', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Dragonspine Trophy'),
(11, 1, 13, 0, 125, 29383, 'Phase 1', 'Druid', 'Feral Cat', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(11, 1, 14, 0, 125, 28660, 'Phase 1', 'Druid', 'Feral Cat', 'Back', 'Both', 'Gilded Thorium Cloak'),
(11, 1, 15, 0, 125, 28658, 'Phase 1', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Terestian''s Stranglestaff'),
(11, 1, 17, 0, 125, 29390, 'Phase 1', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Everbloom Idol');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 200, 42550, 'Pre-Raid', 'Druid', 'Feral Cat', 'Head', 'Both', 'Weakness Spectralizers'),
(11, 1, 2, 0, 200, 43481, 'Pre-Raid', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Trollwoven Spaulders'),
(11, 1, 4, 0, 200, 39554, 'Pre-Raid', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Heroes'' Dreamwalker Raiments'),
(11, 1, 5, 0, 200, 40694, 'Pre-Raid', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Jorach''s Crocolisk Skin Belt'),
(11, 1, 6, 0, 200, 37644, 'Pre-Raid', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Gored Hide Legguards'),
(11, 1, 7, 0, 200, 44297, 'Pre-Raid', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Boots of the Neverending Path'),
(11, 1, 8, 0, 200, 44203, 'Pre-Raid', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Dragonfriend Bracers'),
(11, 1, 9, 0, 200, 37409, 'Pre-Raid', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Gilt-Edged Leather Gauntlets'),
(11, 1, 10, 0, 200, 40586, 'Pre-Raid', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Band of the Kirin Tor'),
(11, 1, 12, 0, 200, 44253, 'Pre-Raid', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(11, 1, 14, 0, 200, 43406, 'Pre-Raid', 'Druid', 'Feral Cat', 'Back', 'Both', 'Cloak of the Gushing Wound'),
(11, 1, 15, 0, 200, 41257, 'Pre-Raid', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Titansteel Destroyer'),
(11, 1, 17, 0, 200, 40713, 'Pre-Raid', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of the Ravenous Beast');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 224, 40473, 'Phase 1', 'Druid', 'Feral Cat', 'Head', 'Both', 'Valorous Dreamwalker Headguard'),
(11, 1, 2, 0, 224, 40494, 'Phase 1', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Valorous Dreamwalker Shoulderpads'),
(11, 1, 4, 0, 224, 40539, 'Phase 1', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Chestguard of the Recluse'),
(11, 1, 5, 0, 224, 40205, 'Phase 1', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Stalk-Skin Belt'),
(11, 1, 6, 0, 224, 44011, 'Phase 1', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Leggings of the Honored'),
(11, 1, 7, 0, 224, 40243, 'Phase 1', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Footwraps of Vile Deceit'),
(11, 1, 8, 0, 224, 40738, 'Phase 1', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Wristwraps of the Cutthroat'),
(11, 1, 9, 0, 224, 40541, 'Phase 1', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Frosted Adroit Handguards'),
(11, 1, 10, 0, 224, 40474, 'Phase 1', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Surge Needle Ring'),
(11, 1, 12, 0, 224, 42987, 'Phase 1', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(11, 1, 14, 0, 224, 40403, 'Phase 1', 'Druid', 'Feral Cat', 'Back', 'Both', 'Drape of the Deadly Foe'),
(11, 1, 17, 0, 224, 40713, 'Phase 1', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of the Ravenous Beast');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 245, 46161, 'Phase 2', 'Druid', 'Feral Cat', 'Head', 'Both', 'Conqueror''s Nightsong Headguard'),
(11, 1, 2, 0, 245, 45245, 'Phase 2', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Shoulderpads of the Intruder'),
(11, 1, 4, 0, 245, 45473, 'Phase 2', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Embrace of the Gladiator'),
(11, 1, 5, 0, 245, 46095, 'Phase 2', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(11, 1, 6, 0, 245, 45536, 'Phase 2', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(11, 1, 7, 0, 245, 45564, 'Phase 2', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Footpads of Silence'),
(11, 1, 8, 0, 245, 45869, 'Phase 2', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Fluxing Energy Coils'),
(11, 1, 9, 0, 245, 46158, 'Phase 2', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Conqueror''s Nightsong Handgrips'),
(11, 1, 10, 0, 245, 45608, 'Phase 2', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Brann''s Signet Ring'),
(11, 1, 12, 0, 245, 45931, 'Phase 2', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Mjolnir Runestone'),
(11, 1, 14, 0, 245, 46032, 'Phase 2', 'Druid', 'Feral Cat', 'Back', 'Both', 'Drape of the Faceless General'),
(11, 1, 17, 0, 245, 40713, 'Phase 2', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of the Ravenous Beast');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 258, 48201, 'Phase 3', 'Druid', 'Feral Cat', 'Head', 'Both', 'Headguard of Triumph'),
(11, 1, 2, 0, 258, 48198, 'Phase 3', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Shoulderpads of Triumph'),
(11, 1, 4, 0, 258, 48139, 'Phase 3', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Robe of Triumph'),
(11, 1, 5, 1, 258, 47112, 'Phase 3', 'Druid', 'Feral Cat', 'Waist', 'Alliance', 'Belt of the Merciless Killer'),
(11, 1, 5, 2, 258, 47460, 'Phase 3', 'Druid', 'Feral Cat', 'Waist', 'Horde', 'Belt of the Pitiless Killer'),
(11, 1, 6, 0, 258, 45536, 'Phase 3', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(11, 1, 7, 1, 258, 47077, 'Phase 3', 'Druid', 'Feral Cat', 'Feet', 'Alliance', 'Treads of the Icewalker'),
(11, 1, 7, 2, 258, 47445, 'Phase 3', 'Druid', 'Feral Cat', 'Feet', 'Horde', 'Icewalker Treads'),
(11, 1, 8, 1, 258, 47155, 'Phase 3', 'Druid', 'Feral Cat', 'Wrists', 'Alliance', 'Bracers of Dark Determination'),
(11, 1, 8, 2, 258, 47474, 'Phase 3', 'Druid', 'Feral Cat', 'Wrists', 'Horde', 'Armbands of Dark Determination'),
(11, 1, 9, 0, 258, 48202, 'Phase 3', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Handgrips of Triumph'),
(11, 1, 10, 1, 258, 47075, 'Phase 3', 'Druid', 'Feral Cat', 'Finger1', 'Alliance', 'Ring of Callous Aggression'),
(11, 1, 10, 2, 258, 47443, 'Phase 3', 'Druid', 'Feral Cat', 'Finger1', 'Horde', 'Band of Callous Aggression'),
(11, 1, 12, 1, 258, 47131, 'Phase 3', 'Druid', 'Feral Cat', 'Trinket1', 'Alliance', 'Death''s Verdict'),
(11, 1, 12, 2, 258, 47464, 'Phase 3', 'Druid', 'Feral Cat', 'Trinket1', 'Horde', 'Death''s Choice'),
(11, 1, 14, 1, 258, 47545, 'Phase 3', 'Druid', 'Feral Cat', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(11, 1, 14, 2, 258, 47546, 'Phase 3', 'Druid', 'Feral Cat', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(11, 1, 15, 1, 258, 47130, 'Phase 3', 'Druid', 'Feral Cat', 'MainHand', 'Alliance', 'Lupine Longstaff'),
(11, 1, 15, 2, 258, 47463, 'Phase 3', 'Druid', 'Feral Cat', 'MainHand', 'Horde', 'Twin''s Pact'),
(11, 1, 17, 0, 258, 47668, 'Phase 3', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of Mutilation');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 264, 51296, 'Phase 4', 'Druid', 'Feral Cat', 'Head', 'Both', 'Sanctified Lasherweave Headguard'),
(11, 1, 2, 0, 264, 51299, 'Phase 4', 'Druid', 'Feral Cat', 'Shoulders', 'Both', 'Sanctified Lasherweave Shoulderpads'),
(11, 1, 4, 0, 264, 51298, 'Phase 4', 'Druid', 'Feral Cat', 'Chest', 'Both', 'Sanctified Lasherweave Raiment'),
(11, 1, 5, 0, 264, 50707, 'Phase 4', 'Druid', 'Feral Cat', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(11, 1, 6, 0, 264, 51297, 'Phase 4', 'Druid', 'Feral Cat', 'Legs', 'Both', 'Sanctified Lasherweave Legguards'),
(11, 1, 7, 0, 264, 50607, 'Phase 4', 'Druid', 'Feral Cat', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(11, 1, 8, 0, 264, 50670, 'Phase 4', 'Druid', 'Feral Cat', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(11, 1, 9, 0, 264, 50675, 'Phase 4', 'Druid', 'Feral Cat', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(11, 1, 10, 0, 264, 50402, 'Phase 4', 'Druid', 'Feral Cat', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(11, 1, 12, 0, 264, 50363, 'Phase 4', 'Druid', 'Feral Cat', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(11, 1, 14, 1, 264, 47545, 'Phase 4', 'Druid', 'Feral Cat', 'Back', 'Alliance', 'Vereesa''s Dexterity'),
(11, 1, 14, 2, 264, 47546, 'Phase 4', 'Druid', 'Feral Cat', 'Back', 'Horde', 'Sylvanas'' Cunning'),
(11, 1, 15, 0, 264, 50735, 'Phase 4', 'Druid', 'Feral Cat', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(11, 1, 17, 0, 264, 50456, 'Phase 4', 'Druid', 'Feral Cat', 'Ranged', 'Both', 'Idol of the Crying Moon');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 1, 0, 0, 290, 51296, 'Phase 5', 'Druid', 'FeralCat', 'Head', 'Both', 'Sanctified Lasherweave Headguard'),
(11, 1, 1, 0, 290, 50633, 'Phase 5', 'Druid', 'FeralCat', 'Neck', 'Both', 'Sindragosa''s Cruel Claw'),
(11, 1, 2, 0, 290, 51299, 'Phase 5', 'Druid', 'FeralCat', 'Shoulders', 'Both', 'Sanctified Lasherweave Shoulderpads'),
(11, 1, 4, 0, 290, 51298, 'Phase 5', 'Druid', 'FeralCat', 'Chest', 'Both', 'Sanctified Lasherweave Raiment'),
(11, 1, 5, 0, 290, 50707, 'Phase 5', 'Druid', 'FeralCat', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(11, 1, 6, 0, 290, 51297, 'Phase 5', 'Druid', 'FeralCat', 'Legs', 'Both', 'Sanctified Lasherweave Legguards'),
(11, 1, 7, 0, 290, 50607, 'Phase 5', 'Druid', 'FeralCat', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(11, 1, 8, 0, 290, 50670, 'Phase 5', 'Druid', 'FeralCat', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(11, 1, 9, 0, 290, 50675, 'Phase 5', 'Druid', 'FeralCat', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(11, 1, 10, 0, 290, 50618, 'Phase 5', 'Druid', 'FeralCat', 'Finger1', 'Both', 'Frostbrood Sapphire Ring'),
(11, 1, 11, 0, 290, 50402, 'Phase 5', 'Druid', 'FeralCat', 'Finger2', 'Both', 'Ashen Band of Endless Vengeance'),
(11, 1, 12, 0, 290, 50363, 'Phase 5', 'Druid', 'FeralCat', 'Trinket1', 'Both', 'Deathbringer''s Will'),
(11, 1, 13, 0, 290, 54590, 'Phase 5', 'Druid', 'FeralCat', 'Trinket2', 'Both', 'Sharpened Twilight Scale'),
(11, 1, 14, 0, 290, 50653, 'Phase 5', 'Druid', 'FeralCat', 'Back', 'Both', 'Shadowvault Slayer''s Cloak'),
(11, 1, 15, 0, 290, 50735, 'Phase 5', 'Druid', 'FeralCat', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(11, 1, 17, 0, 290, 50456, 'Phase 5', 'Druid', 'FeralCat', 'Ranged', 'Both', 'Idol of the Crying Moon');

-- Restoration (tab 2)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 66, 13102, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Head', 'Both', 'Cassandra''s Grace'),
(11, 2, 1, 0, 66, 18723, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(11, 2, 2, 0, 66, 15061, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Living Shoulders'),
(11, 2, 4, 0, 66, 13346, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(11, 2, 5, 0, 66, 14553, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Waist', 'Both', 'Sash of Mercy'),
(11, 2, 6, 0, 66, 11841, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Legs', 'Both', 'Senior Designer''s Pantaloons'),
(11, 2, 7, 0, 66, 13954, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Feet', 'Both', 'Verdant Footpads'),
(11, 2, 8, 0, 66, 13208, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bleak Howler Armguards'),
(11, 2, 9, 0, 66, 10787, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Hands', 'Both', 'Atal''ai Gloves'),
(11, 2, 10, 0, 66, 13178, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Finger1', 'Both', 'Rosewine Circle'),
(11, 2, 11, 0, 66, 16058, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Finger2', 'Both', 'Fordring''s Seal'),
(11, 2, 12, 0, 66, 12930, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Briarwood Reed'),
(11, 2, 13, 0, 66, 11819, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Second Wind'),
(11, 2, 14, 0, 66, 13386, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'Back', 'Both', 'Archivist Cape'),
(11, 2, 15, 0, 66, 11923, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'MainHand', 'Both', 'The Hammer of Grace'),
(11, 2, 16, 0, 66, 11928, 'Phase 1 (Pre-Raid)', 'Druid', 'Restoration', 'OffHand', 'Both', 'Thaurissan''s Royal Scepter');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 76, 13102, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Head', 'Both', 'Cassandra''s Grace'),
(11, 2, 1, 0, 76, 18723, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(11, 2, 2, 0, 76, 15061, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Living Shoulders'),
(11, 2, 4, 0, 76, 13346, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(11, 2, 5, 0, 76, 14553, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Waist', 'Both', 'Sash of Mercy'),
(11, 2, 6, 0, 76, 18386, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Legs', 'Both', 'Padre''s Trousers'),
(11, 2, 7, 0, 76, 13954, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Feet', 'Both', 'Verdant Footpads'),
(11, 2, 8, 0, 76, 18525, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bracers of Prosperity'),
(11, 2, 9, 0, 76, 10787, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Hands', 'Both', 'Atal''ai Gloves'),
(11, 2, 10, 0, 76, 13178, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Finger1', 'Both', 'Rosewine Circle'),
(11, 2, 11, 0, 76, 16058, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Finger2', 'Both', 'Fordring''s Seal'),
(11, 2, 12, 0, 76, 18470, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Royal Seal of Eldre''Thalas'),
(11, 2, 13, 0, 76, 18371, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Mindtap Talisman'),
(11, 2, 14, 0, 76, 18510, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'Back', 'Both', 'Hide of the Wild'),
(11, 2, 15, 0, 76, 11923, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'MainHand', 'Both', 'The Hammer of Grace'),
(11, 2, 16, 0, 76, 18523, 'Phase 2 (Pre-Raid)', 'Druid', 'Restoration', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 78, 19132, 'Phase 2', 'Druid', 'Restoration', 'Head', 'Both', 'Crystal Adorned Crown'),
(11, 2, 1, 0, 78, 18723, 'Phase 2', 'Druid', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(11, 2, 2, 0, 78, 18810, 'Phase 2', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(11, 2, 4, 0, 78, 13346, 'Phase 2', 'Druid', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(11, 2, 5, 0, 78, 19162, 'Phase 2', 'Druid', 'Restoration', 'Waist', 'Both', 'Corehound Belt'),
(11, 2, 6, 0, 78, 18875, 'Phase 2', 'Druid', 'Restoration', 'Legs', 'Both', 'Salamander Scale Pants'),
(11, 2, 7, 0, 78, 16829, 'Phase 2', 'Druid', 'Restoration', 'Feet', 'Both', 'Cenarion Boots'),
(11, 2, 8, 0, 78, 18263, 'Phase 2', 'Druid', 'Restoration', 'Wrists', 'Both', 'Flarecore Wraps'),
(11, 2, 9, 0, 78, 10787, 'Phase 2', 'Druid', 'Restoration', 'Hands', 'Both', 'Atal''ai Gloves'),
(11, 2, 10, 0, 78, 19140, 'Phase 2', 'Druid', 'Restoration', 'Finger1', 'Both', 'Cauterizing Band'),
(11, 2, 11, 0, 78, 19140, 'Phase 2', 'Druid', 'Restoration', 'Finger2', 'Both', 'Cauterizing Band'),
(11, 2, 12, 0, 78, 18470, 'Phase 2', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Royal Seal of Eldre''Thalas'),
(11, 2, 13, 0, 78, 17064, 'Phase 2', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Shard of the Scale'),
(11, 2, 14, 0, 78, 18510, 'Phase 2', 'Druid', 'Restoration', 'Back', 'Both', 'Hide of the Wild'),
(11, 2, 15, 0, 78, 17070, 'Phase 2', 'Druid', 'Restoration', 'MainHand', 'Both', 'Fang of the Mystics'),
(11, 2, 16, 0, 78, 18523, 'Phase 2', 'Druid', 'Restoration', 'OffHand', 'Both', 'Brightly Glowing Stone');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 83, 19132, 'Phase 3', 'Druid', 'Restoration', 'Head', 'Both', 'Crystal Adorned Crown'),
(11, 2, 1, 0, 83, 18723, 'Phase 3', 'Druid', 'Restoration', 'Neck', 'Both', 'Animated Chain Necklace'),
(11, 2, 2, 0, 83, 18810, 'Phase 3', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(11, 2, 4, 0, 83, 13346, 'Phase 3', 'Druid', 'Restoration', 'Chest', 'Both', 'Robes of the Exalted'),
(11, 2, 5, 0, 83, 19162, 'Phase 3', 'Druid', 'Restoration', 'Waist', 'Both', 'Corehound Belt'),
(11, 2, 6, 0, 83, 19385, 'Phase 3', 'Druid', 'Restoration', 'Legs', 'Both', 'Empowered Leggings'),
(11, 2, 7, 0, 83, 16898, 'Phase 3', 'Druid', 'Restoration', 'Feet', 'Both', 'Stormrage Boots'),
(11, 2, 8, 0, 83, 16904, 'Phase 3', 'Druid', 'Restoration', 'Wrists', 'Both', 'Stormrage Bracers'),
(11, 2, 9, 0, 83, 16899, 'Phase 3', 'Druid', 'Restoration', 'Hands', 'Both', 'Stormrage Handguards'),
(11, 2, 10, 0, 83, 19140, 'Phase 3', 'Druid', 'Restoration', 'Finger1', 'Both', 'Cauterizing Band'),
(11, 2, 11, 0, 83, 19382, 'Phase 3', 'Druid', 'Restoration', 'Finger2', 'Both', 'Pure Elementium Band'),
(11, 2, 12, 0, 83, 19395, 'Phase 3', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Rejuvenating Gem'),
(11, 2, 13, 0, 83, 17064, 'Phase 3', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Shard of the Scale'),
(11, 2, 14, 0, 83, 19430, 'Phase 3', 'Druid', 'Restoration', 'Back', 'Both', 'Shroud of Pure Thought'),
(11, 2, 15, 0, 83, 19360, 'Phase 3', 'Druid', 'Restoration', 'MainHand', 'Both', 'Lok''amir il Romathis'),
(11, 2, 16, 0, 83, 19312, 'Phase 3', 'Druid', 'Restoration', 'OffHand', 'Both', 'Lei of the Lifegiver');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 88, 20628, 'Phase 5', 'Druid', 'Restoration', 'Head', 'Both', 'Deviate Growth Cap'),
(11, 2, 1, 0, 88, 21712, 'Phase 5', 'Druid', 'Restoration', 'Neck', 'Both', 'Amulet of the Fallen God'),
(11, 2, 2, 0, 88, 18810, 'Phase 5', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Wild Growth Spaulders'),
(11, 2, 4, 0, 88, 21663, 'Phase 5', 'Druid', 'Restoration', 'Chest', 'Both', 'Robes of the Guardian Saint'),
(11, 2, 5, 0, 88, 21582, 'Phase 5', 'Druid', 'Restoration', 'Waist', 'Both', 'Grasp of the Old God'),
(11, 2, 6, 0, 88, 19385, 'Phase 5', 'Druid', 'Restoration', 'Legs', 'Both', 'Empowered Leggings'),
(11, 2, 7, 0, 88, 19437, 'Phase 5', 'Druid', 'Restoration', 'Feet', 'Both', 'Boots of Pure Thought'),
(11, 2, 8, 0, 88, 21604, 'Phase 5', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bracelets of Royal Redemption'),
(11, 2, 9, 0, 88, 21617, 'Phase 5', 'Druid', 'Restoration', 'Hands', 'Both', 'Wasphide Gauntlets'),
(11, 2, 10, 0, 88, 21620, 'Phase 5', 'Druid', 'Restoration', 'Finger1', 'Both', 'Ring of the Martyr'),
(11, 2, 11, 0, 88, 19382, 'Phase 5', 'Druid', 'Restoration', 'Finger2', 'Both', 'Pure Elementium Band'),
(11, 2, 12, 0, 88, 19395, 'Phase 5', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Rejuvenating Gem'),
(11, 2, 13, 0, 88, 17064, 'Phase 5', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Shard of the Scale'),
(11, 2, 14, 0, 88, 21583, 'Phase 5', 'Druid', 'Restoration', 'Back', 'Both', 'Cloak of Clarity'),
(11, 2, 15, 0, 88, 21839, 'Phase 5', 'Druid', 'Restoration', 'MainHand', 'Both', 'Scepter of the False Prophet'),
(11, 2, 16, 0, 88, 21666, 'Phase 5', 'Druid', 'Restoration', 'OffHand', 'Both', 'Sartura''s Might'),
(11, 2, 17, 0, 88, 22399, 'Phase 5', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Health');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 92, 20628, 'Phase 6', 'Druid', 'Restoration', 'Head', 'Both', 'Deviate Growth Cap'),
(11, 2, 1, 0, 92, 21712, 'Phase 6', 'Druid', 'Restoration', 'Neck', 'Both', 'Amulet of the Fallen God'),
(11, 2, 2, 0, 92, 22491, 'Phase 6', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Dreamwalker Spaulders'),
(11, 2, 4, 0, 92, 22488, 'Phase 6', 'Druid', 'Restoration', 'Chest', 'Both', 'Dreamwalker Tunic'),
(11, 2, 5, 0, 92, 21582, 'Phase 6', 'Druid', 'Restoration', 'Waist', 'Both', 'Grasp of the Old God'),
(11, 2, 6, 0, 92, 22489, 'Phase 6', 'Druid', 'Restoration', 'Legs', 'Both', 'Dreamwalker Legguards'),
(11, 2, 7, 0, 92, 22492, 'Phase 6', 'Druid', 'Restoration', 'Feet', 'Both', 'Dreamwalker Boots'),
(11, 2, 8, 0, 92, 21604, 'Phase 6', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bracelets of Royal Redemption'),
(11, 2, 9, 0, 92, 22493, 'Phase 6', 'Druid', 'Restoration', 'Hands', 'Both', 'Dreamwalker Handguards'),
(11, 2, 10, 0, 92, 21620, 'Phase 6', 'Druid', 'Restoration', 'Finger1', 'Both', 'Ring of the Martyr'),
(11, 2, 11, 0, 92, 22939, 'Phase 6', 'Druid', 'Restoration', 'Finger2', 'Both', 'Band of Unanswered Prayers'),
(11, 2, 12, 0, 92, 23047, 'Phase 6', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Eye of the Dead'),
(11, 2, 13, 0, 92, 23027, 'Phase 6', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Warmth of Forgiveness'),
(11, 2, 14, 0, 92, 22960, 'Phase 6', 'Druid', 'Restoration', 'Back', 'Both', 'Cloak of Suturing'),
(11, 2, 15, 0, 92, 23056, 'Phase 6', 'Druid', 'Restoration', 'MainHand', 'Both', 'Hammer of the Twisting Nether'),
(11, 2, 16, 0, 92, 23048, 'Phase 6', 'Druid', 'Restoration', 'OffHand', 'Both', 'Sapphiron''s Right Eye'),
(11, 2, 17, 0, 92, 22399, 'Phase 6', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Health');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 120, 24264, 'Pre-Raid', 'Druid', 'Restoration', 'Head', 'Both', 'Whitemend Hood'),
(11, 2, 1, 0, 120, 30377, 'Pre-Raid', 'Druid', 'Restoration', 'Neck', 'Both', 'Karja''s Medallion'),
(11, 2, 2, 0, 120, 21874, 'Pre-Raid', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(11, 2, 4, 0, 120, 21875, 'Pre-Raid', 'Druid', 'Restoration', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(11, 2, 5, 0, 120, 21873, 'Pre-Raid', 'Druid', 'Restoration', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(11, 2, 6, 0, 120, 24261, 'Pre-Raid', 'Druid', 'Restoration', 'Legs', 'Both', 'Whitemend Pants'),
(11, 2, 7, 0, 120, 27411, 'Pre-Raid', 'Druid', 'Restoration', 'Feet', 'Both', 'Slippers of Serenity'),
(11, 2, 8, 0, 120, 29183, 'Pre-Raid', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bindings of the Timewalker'),
(11, 2, 9, 0, 120, 29506, 'Pre-Raid', 'Druid', 'Restoration', 'Hands', 'Both', 'Gloves of the Living Touch'),
(11, 2, 10, 0, 120, 27780, 'Pre-Raid', 'Druid', 'Restoration', 'Finger1', 'Both', 'Ring of Fabled Hope'),
(11, 2, 11, 0, 120, 31383, 'Pre-Raid', 'Druid', 'Restoration', 'Finger2', 'Both', 'Spiritualist''s Mark of the Sha''tar'),
(11, 2, 12, 0, 120, 29376, 'Pre-Raid', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(11, 2, 13, 0, 120, 30841, 'Pre-Raid', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Lower City Prayerbook'),
(11, 2, 14, 0, 120, 24254, 'Pre-Raid', 'Druid', 'Restoration', 'Back', 'Both', 'White Remedy Cape'),
(11, 2, 15, 0, 120, 23556, 'Pre-Raid', 'Druid', 'Restoration', 'MainHand', 'Both', 'Hand of Eternity'),
(11, 2, 16, 0, 120, 29274, 'Pre-Raid', 'Druid', 'Restoration', 'OffHand', 'Both', 'Tears of Heaven'),
(11, 2, 17, 0, 120, 27886, 'Pre-Raid', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Emerald Queen');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 125, 24264, 'Phase 1', 'Druid', 'Restoration', 'Head', 'Both', 'Whitemend Hood'),
(11, 2, 1, 0, 125, 30726, 'Phase 1', 'Druid', 'Restoration', 'Neck', 'Both', 'Archaic Charm of Presence'),
(11, 2, 2, 0, 125, 21874, 'Phase 1', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Primal Mooncloth Shoulders'),
(11, 2, 4, 0, 125, 21875, 'Phase 1', 'Druid', 'Restoration', 'Chest', 'Both', 'Primal Mooncloth Robe'),
(11, 2, 5, 0, 125, 21873, 'Phase 1', 'Druid', 'Restoration', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(11, 2, 6, 0, 125, 30727, 'Phase 1', 'Druid', 'Restoration', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(11, 2, 7, 0, 125, 30737, 'Phase 1', 'Druid', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(11, 2, 8, 0, 125, 29183, 'Phase 1', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bindings of the Timewalker'),
(11, 2, 9, 0, 125, 28521, 'Phase 1', 'Druid', 'Restoration', 'Hands', 'Both', 'Mitts of the Treemender'),
(11, 2, 10, 0, 125, 28763, 'Phase 1', 'Druid', 'Restoration', 'Finger1', 'Both', 'Jade Ring of the Everliving'),
(11, 2, 11, 0, 125, 30736, 'Phase 1', 'Druid', 'Restoration', 'Finger2', 'Both', 'Ring of Flowing Light'),
(11, 2, 12, 0, 125, 29376, 'Phase 1', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(11, 2, 13, 0, 125, 30841, 'Phase 1', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Lower City Prayerbook'),
(11, 2, 14, 0, 125, 31329, 'Phase 1', 'Druid', 'Restoration', 'Back', 'Both', 'Lifegiving Cloak'),
(11, 2, 15, 0, 125, 28771, 'Phase 1', 'Druid', 'Restoration', 'MainHand', 'Both', 'Light''s Justice'),
(11, 2, 16, 0, 125, 29274, 'Phase 1', 'Druid', 'Restoration', 'OffHand', 'Both', 'Tears of Heaven'),
(11, 2, 17, 0, 125, 27886, 'Phase 1', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Emerald Queen');

-- ilvl 141 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 141, 30219, 'Phase 2', 'Druid', 'Restoration', 'Head', 'Both', 'Nordrassil Headguard'),
(11, 2, 1, 0, 141, 30018, 'Phase 2', 'Druid', 'Restoration', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(11, 2, 2, 0, 141, 30221, 'Phase 2', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Nordrassil Life-Mantle'),
(11, 2, 4, 0, 141, 30216, 'Phase 2', 'Druid', 'Restoration', 'Chest', 'Both', 'Nordrassil Chestguard'),
(11, 2, 5, 0, 141, 21873, 'Phase 2', 'Druid', 'Restoration', 'Waist', 'Both', 'Primal Mooncloth Belt'),
(11, 2, 6, 0, 141, 30727, 'Phase 2', 'Druid', 'Restoration', 'Legs', 'Both', 'Gilded Trousers of Benediction'),
(11, 2, 7, 0, 141, 30737, 'Phase 2', 'Druid', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(11, 2, 8, 0, 141, 30062, 'Phase 2', 'Druid', 'Restoration', 'Wrists', 'Both', 'Grove-Bands of Remulos'),
(11, 2, 9, 0, 141, 28521, 'Phase 2', 'Druid', 'Restoration', 'Hands', 'Both', 'Mitts of the Treemender'),
(11, 2, 10, 0, 141, 30110, 'Phase 2', 'Druid', 'Restoration', 'Finger1', 'Both', 'Coral Band of the Revived'),
(11, 2, 11, 0, 141, 28763, 'Phase 2', 'Druid', 'Restoration', 'Finger2', 'Both', 'Jade Ring of the Everliving'),
(11, 2, 12, 0, 141, 29376, 'Phase 2', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(11, 2, 13, 0, 141, 30841, 'Phase 2', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Lower City Prayerbook'),
(11, 2, 14, 0, 141, 29989, 'Phase 2', 'Druid', 'Restoration', 'Back', 'Both', 'Sunshower Light Cloak'),
(11, 2, 15, 0, 141, 30108, 'Phase 2', 'Druid', 'Restoration', 'MainHand', 'Both', 'Lightfathom Scepter'),
(11, 2, 16, 0, 141, 29274, 'Phase 2', 'Druid', 'Restoration', 'OffHand', 'Both', 'Tears of Heaven'),
(11, 2, 17, 0, 141, 27886, 'Phase 2', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Emerald Queen');

-- ilvl 156 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 156, 31037, 'Phase 3', 'Druid', 'Restoration', 'Head', 'Both', 'Thunderheart Helmet'),
(11, 2, 1, 0, 156, 30018, 'Phase 3', 'Druid', 'Restoration', 'Neck', 'Both', 'Lord Sanguinar''s Claim'),
(11, 2, 2, 0, 156, 32583, 'Phase 3', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Shoulderpads of Renewed Life'),
(11, 2, 4, 0, 156, 31041, 'Phase 3', 'Druid', 'Restoration', 'Chest', 'Both', 'Thunderheart Tunic'),
(11, 2, 5, 0, 156, 30895, 'Phase 3', 'Druid', 'Restoration', 'Waist', 'Both', 'Angelista''s Sash'),
(11, 2, 6, 0, 156, 30912, 'Phase 3', 'Druid', 'Restoration', 'Legs', 'Both', 'Leggings of Eternity'),
(11, 2, 7, 0, 156, 30737, 'Phase 3', 'Druid', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(11, 2, 8, 0, 156, 32584, 'Phase 3', 'Druid', 'Restoration', 'Wrists', 'Both', 'Swiftheal Wraps'),
(11, 2, 9, 0, 156, 32328, 'Phase 3', 'Druid', 'Restoration', 'Hands', 'Both', 'Botanist''s Gloves of Growth'),
(11, 2, 10, 0, 156, 32528, 'Phase 3', 'Druid', 'Restoration', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(11, 2, 11, 0, 156, 32528, 'Phase 3', 'Druid', 'Restoration', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(11, 2, 12, 0, 156, 29376, 'Phase 3', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(11, 2, 13, 0, 156, 32496, 'Phase 3', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Memento of Tyrande'),
(11, 2, 14, 0, 156, 32524, 'Phase 3', 'Druid', 'Restoration', 'Back', 'Both', 'Shroud of the Highborne'),
(11, 2, 15, 0, 156, 32500, 'Phase 3', 'Druid', 'Restoration', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(11, 2, 16, 0, 156, 30911, 'Phase 3', 'Druid', 'Restoration', 'OffHand', 'Both', 'Scepter of Purification'),
(11, 2, 17, 0, 156, 27886, 'Phase 3', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Emerald Queen');

-- ilvl 164 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 164, 31037, 'Phase 4', 'Druid', 'Restoration', 'Head', 'Both', 'Thunderheart Helmet'),
(11, 2, 1, 0, 164, 33281, 'Phase 4', 'Druid', 'Restoration', 'Neck', 'Both', 'Brooch of Nature''s Mercy'),
(11, 2, 2, 0, 164, 32583, 'Phase 4', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Shoulderpads of Renewed Life'),
(11, 2, 4, 0, 164, 31041, 'Phase 4', 'Druid', 'Restoration', 'Chest', 'Both', 'Thunderheart Tunic'),
(11, 2, 5, 0, 164, 30895, 'Phase 4', 'Druid', 'Restoration', 'Waist', 'Both', 'Angelista''s Sash'),
(11, 2, 6, 0, 164, 30912, 'Phase 4', 'Druid', 'Restoration', 'Legs', 'Both', 'Leggings of Eternity'),
(11, 2, 7, 0, 164, 30737, 'Phase 4', 'Druid', 'Restoration', 'Feet', 'Both', 'Gold-Leaf Wildboots'),
(11, 2, 8, 0, 164, 32584, 'Phase 4', 'Druid', 'Restoration', 'Wrists', 'Both', 'Swiftheal Wraps'),
(11, 2, 9, 0, 164, 32328, 'Phase 4', 'Druid', 'Restoration', 'Hands', 'Both', 'Botanist''s Gloves of Growth'),
(11, 2, 10, 0, 164, 32528, 'Phase 4', 'Druid', 'Restoration', 'Finger1', 'Both', 'Blessed Band of Karabor'),
(11, 2, 11, 0, 164, 32528, 'Phase 4', 'Druid', 'Restoration', 'Finger2', 'Both', 'Blessed Band of Karabor'),
(11, 2, 12, 0, 164, 29376, 'Phase 4', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Essence of the Martyr'),
(11, 2, 13, 0, 164, 32496, 'Phase 4', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Memento of Tyrande'),
(11, 2, 14, 0, 164, 32524, 'Phase 4', 'Druid', 'Restoration', 'Back', 'Both', 'Shroud of the Highborne'),
(11, 2, 15, 0, 164, 32500, 'Phase 4', 'Druid', 'Restoration', 'MainHand', 'Both', 'Crystal Spire of Karabor'),
(11, 2, 16, 0, 164, 30911, 'Phase 4', 'Druid', 'Restoration', 'OffHand', 'Both', 'Scepter of Purification'),
(11, 2, 17, 0, 164, 27886, 'Phase 4', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Emerald Queen');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 200, 42554, 'Pre-Raid', 'Druid', 'Restoration', 'Head', 'Both', 'Greensight Gogs'),
(11, 2, 1, 0, 200, 42647, 'Pre-Raid', 'Druid', 'Restoration', 'Neck', 'Both', 'Titanium Spellshock Necklace'),
(11, 2, 2, 0, 200, 37368, 'Pre-Raid', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Silent Spectator Shoulderpads'),
(11, 2, 4, 0, 200, 42102, 'Pre-Raid', 'Druid', 'Restoration', 'Chest', 'Both', 'Spellweave Robe'),
(11, 2, 5, 0, 200, 37643, 'Pre-Raid', 'Druid', 'Restoration', 'Waist', 'Both', 'Sash of Blood Removal'),
(11, 2, 6, 0, 200, 37791, 'Pre-Raid', 'Druid', 'Restoration', 'Legs', 'Both', 'Leggings of the Winged Serpent'),
(11, 2, 7, 0, 200, 43502, 'Pre-Raid', 'Druid', 'Restoration', 'Feet', 'Both', 'Earthgiving Boots'),
(11, 2, 8, 0, 200, 37696, 'Pre-Raid', 'Druid', 'Restoration', 'Wrists', 'Both', 'Plague-Infected Bracers'),
(11, 2, 9, 0, 200, 37230, 'Pre-Raid', 'Druid', 'Restoration', 'Hands', 'Both', 'Grotto Mist Gloves'),
(11, 2, 10, 0, 200, 37694, 'Pre-Raid', 'Druid', 'Restoration', 'Finger1', 'Both', 'Band of Guile'),
(11, 2, 12, 0, 200, 40685, 'Pre-Raid', 'Druid', 'Restoration', 'Trinket1', 'Both', 'The Egg of Mortal Essence'),
(11, 2, 14, 0, 200, 41609, 'Pre-Raid', 'Druid', 'Restoration', 'Back', 'Both', 'Wispcloak'),
(11, 2, 17, 0, 200, 40711, 'Pre-Raid', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Lush Moss');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 224, 44007, 'Phase 1', 'Druid', 'Restoration', 'Head', 'Both', 'Headpiece of Reconciliation'),
(11, 2, 1, 0, 224, 44661, 'Phase 1', 'Druid', 'Restoration', 'Neck', 'Both', 'Wyrmrest Necklace of Power'),
(11, 2, 2, 0, 224, 39719, 'Phase 1', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Mantle of the Locusts'),
(11, 2, 4, 0, 224, 44002, 'Phase 1', 'Druid', 'Restoration', 'Chest', 'Both', 'The Sanctum''s Flowing Vestments'),
(11, 2, 5, 0, 224, 40561, 'Phase 1', 'Druid', 'Restoration', 'Waist', 'Both', 'Leash of Heedless Magic'),
(11, 2, 6, 0, 224, 40379, 'Phase 1', 'Druid', 'Restoration', 'Legs', 'Both', 'Legguards of the Boneyard'),
(11, 2, 7, 0, 224, 40558, 'Phase 1', 'Druid', 'Restoration', 'Feet', 'Both', 'Arcanic Tramplers'),
(11, 2, 8, 0, 224, 44008, 'Phase 1', 'Druid', 'Restoration', 'Wrists', 'Both', 'Unsullied Cuffs'),
(11, 2, 9, 0, 224, 40460, 'Phase 1', 'Druid', 'Restoration', 'Hands', 'Both', 'Valorous Dreamwalker Handguards'),
(11, 2, 10, 0, 224, 40375, 'Phase 1', 'Druid', 'Restoration', 'Finger1', 'Both', 'Ring of Decaying Beauty'),
(11, 2, 12, 0, 224, 40432, 'Phase 1', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Illustration of the Dragon Soul'),
(11, 2, 14, 0, 224, 44005, 'Phase 1', 'Druid', 'Restoration', 'Back', 'Both', 'Pennant Cloak'),
(11, 2, 17, 0, 224, 40342, 'Phase 1', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Awakening');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 245, 46184, 'Phase 2', 'Druid', 'Restoration', 'Head', 'Both', 'Conqueror''s Nightsong Headpiece'),
(11, 2, 1, 0, 245, 45243, 'Phase 2', 'Druid', 'Restoration', 'Neck', 'Both', 'Sapphire Amulet of Renewal'),
(11, 2, 2, 0, 245, 46187, 'Phase 2', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Conqueror''s Nightsong Spaulders'),
(11, 2, 4, 0, 245, 45519, 'Phase 2', 'Druid', 'Restoration', 'Chest', 'Both', 'Vestments of the Blind Denizen'),
(11, 2, 5, 0, 245, 45616, 'Phase 2', 'Druid', 'Restoration', 'Waist', 'Both', 'Star-beaded Clutch'),
(11, 2, 6, 0, 245, 46185, 'Phase 2', 'Druid', 'Restoration', 'Legs', 'Both', 'Conqueror''s Nightsong Leggings'),
(11, 2, 7, 0, 245, 45135, 'Phase 2', 'Druid', 'Restoration', 'Feet', 'Both', 'Boots of Fiery Resolution'),
(11, 2, 8, 0, 245, 45446, 'Phase 2', 'Druid', 'Restoration', 'Wrists', 'Both', 'Grasps of Reason'),
(11, 2, 9, 0, 245, 46183, 'Phase 2', 'Druid', 'Restoration', 'Hands', 'Both', 'Conqueror''s Nightsong Handguards'),
(11, 2, 10, 0, 245, 45495, 'Phase 2', 'Druid', 'Restoration', 'Finger1', 'Both', 'Conductive Seal'),
(11, 2, 12, 0, 245, 45535, 'Phase 2', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Show of Faith'),
(11, 2, 14, 0, 245, 45618, 'Phase 2', 'Druid', 'Restoration', 'Back', 'Both', 'Sunglimmer Cloak'),
(11, 2, 17, 0, 245, 40342, 'Phase 2', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Awakening');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 1, 258, 48141, 'Phase 3', 'Druid', 'Restoration', 'Head', 'Alliance', 'Malfurion''s Headpiece of Triumph'),
(11, 2, 1, 1, 258, 47144, 'Phase 3', 'Druid', 'Restoration', 'Neck', 'Alliance', 'Wail of the Val''kyr'),
(11, 2, 1, 2, 258, 47468, 'Phase 3', 'Druid', 'Restoration', 'Neck', 'Horde', 'Cry of the Val''kyr'),
(11, 2, 2, 1, 258, 48138, 'Phase 3', 'Druid', 'Restoration', 'Shoulders', 'Alliance', 'Malfurion''s Spaulders of Triumph'),
(11, 2, 4, 0, 258, 48169, 'Phase 3', 'Druid', 'Restoration', 'Chest', 'Both', 'Vestments of Triumph'),
(11, 2, 5, 1, 258, 47145, 'Phase 3', 'Druid', 'Restoration', 'Waist', 'Alliance', 'Cord of Pale Thorns'),
(11, 2, 5, 2, 258, 47469, 'Phase 3', 'Druid', 'Restoration', 'Waist', 'Horde', 'Belt of Pale Thorns'),
(11, 2, 6, 0, 258, 48140, 'Phase 3', 'Druid', 'Restoration', 'Legs', 'Both', 'Leggings of Triumph'),
(11, 2, 7, 1, 258, 47097, 'Phase 3', 'Druid', 'Restoration', 'Feet', 'Alliance', 'Boots of the Mourning Widow'),
(11, 2, 7, 2, 258, 47454, 'Phase 3', 'Druid', 'Restoration', 'Feet', 'Horde', 'Sandals of the Mourning Widow'),
(11, 2, 8, 1, 258, 47066, 'Phase 3', 'Druid', 'Restoration', 'Wrists', 'Alliance', 'Bracers of the Autumn Willow'),
(11, 2, 8, 2, 258, 47438, 'Phase 3', 'Druid', 'Restoration', 'Wrists', 'Horde', 'Bindings of the Autumn Willow'),
(11, 2, 9, 0, 258, 48142, 'Phase 3', 'Druid', 'Restoration', 'Hands', 'Both', 'Handguards of Triumph'),
(11, 2, 10, 1, 258, 47224, 'Phase 3', 'Druid', 'Restoration', 'Finger1', 'Alliance', 'Ring of the Darkmender'),
(11, 2, 10, 2, 258, 47439, 'Phase 3', 'Druid', 'Restoration', 'Finger1', 'Horde', 'Circle of the Darkmender'),
(11, 2, 12, 1, 258, 47059, 'Phase 3', 'Druid', 'Restoration', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(11, 2, 12, 2, 258, 47432, 'Phase 3', 'Druid', 'Restoration', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(11, 2, 14, 1, 258, 47553, 'Phase 3', 'Druid', 'Restoration', 'Back', 'Alliance', 'Bolvar''s Devotion'),
(11, 2, 14, 2, 258, 47554, 'Phase 3', 'Druid', 'Restoration', 'Back', 'Horde', 'Lady Liadrin''s Conviction'),
(11, 2, 17, 0, 258, 47671, 'Phase 3', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of Flaring Growth');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 264, 51302, 'Phase 4', 'Druid', 'Restoration', 'Head', 'Both', 'Sanctified Lasherweave Helmet'),
(11, 2, 1, 0, 264, 50609, 'Phase 4', 'Druid', 'Restoration', 'Neck', 'Both', 'Bone Sentinel''s Amulet'),
(11, 2, 2, 0, 264, 51304, 'Phase 4', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Sanctified Lasherweave Pauldrons'),
(11, 2, 4, 0, 264, 50717, 'Phase 4', 'Druid', 'Restoration', 'Chest', 'Both', 'Sanguine Silk Robes'),
(11, 2, 5, 0, 264, 50705, 'Phase 4', 'Druid', 'Restoration', 'Waist', 'Both', 'Professor''s Bloodied Smock'),
(11, 2, 6, 0, 264, 51303, 'Phase 4', 'Druid', 'Restoration', 'Legs', 'Both', 'Sanctified Lasherweave Legplates'),
(11, 2, 7, 0, 264, 50699, 'Phase 4', 'Druid', 'Restoration', 'Feet', 'Both', 'Plague Scientist''s Boots'),
(11, 2, 8, 0, 264, 50630, 'Phase 4', 'Druid', 'Restoration', 'Wrists', 'Both', 'Bracers of Eternal Dreaming'),
(11, 2, 9, 0, 264, 51301, 'Phase 4', 'Druid', 'Restoration', 'Hands', 'Both', 'Sanctified Lasherweave Gauntlets'),
(11, 2, 10, 0, 264, 50636, 'Phase 4', 'Druid', 'Restoration', 'Finger1', 'Both', 'Memory of Malygos'),
(11, 2, 12, 1, 264, 47059, 'Phase 4', 'Druid', 'Restoration', 'Trinket1', 'Alliance', 'Solace of the Defeated'),
(11, 2, 12, 2, 264, 47432, 'Phase 4', 'Druid', 'Restoration', 'Trinket1', 'Horde', 'Solace of the Fallen'),
(11, 2, 14, 0, 264, 50668, 'Phase 4', 'Druid', 'Restoration', 'Back', 'Both', 'Greatcloak of the Turned Champion'),
(11, 2, 15, 0, 264, 46017, 'Phase 4', 'Druid', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(11, 2, 17, 0, 264, 50454, 'Phase 4', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Black Willow');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 2, 0, 0, 290, 51302, 'Phase 5', 'Druid', 'Restoration', 'Head', 'Both', 'Sanctified Lasherweave Helmet'),
(11, 2, 1, 0, 290, 50609, 'Phase 5', 'Druid', 'Restoration', 'Neck', 'Both', 'Bone Sentinel''s Amulet'),
(11, 2, 2, 0, 290, 51304, 'Phase 5', 'Druid', 'Restoration', 'Shoulders', 'Both', 'Sanctified Lasherweave Pauldrons'),
(11, 2, 4, 0, 290, 50717, 'Phase 5', 'Druid', 'Restoration', 'Chest', 'Both', 'Sanguine Silk Robes'),
(11, 2, 5, 0, 290, 50705, 'Phase 5', 'Druid', 'Restoration', 'Waist', 'Both', 'Professor''s Bloodied Smock'),
(11, 2, 6, 0, 290, 51303, 'Phase 5', 'Druid', 'Restoration', 'Legs', 'Both', 'Sanctified Lasherweave Legplates'),
(11, 2, 7, 0, 290, 50665, 'Phase 5', 'Druid', 'Restoration', 'Feet', 'Both', 'Boots Of Unnatural Growth'),
(11, 2, 8, 0, 290, 54584, 'Phase 5', 'Druid', 'Restoration', 'Wrists', 'Both', 'Phaseshifter Bracers'),
(11, 2, 9, 0, 290, 51301, 'Phase 5', 'Druid', 'Restoration', 'Hands', 'Both', 'Sanctified Lasherweave Gauntlets'),
(11, 2, 10, 0, 290, 50400, 'Phase 5', 'Druid', 'Restoration', 'Finger1', 'Both', 'Ashen Band of Endless Wisdom'),
(11, 2, 11, 0, 290, 50636, 'Phase 5', 'Druid', 'Restoration', 'Finger2', 'Both', 'Memory of Malygos'),
(11, 2, 12, 0, 290, 47059, 'Phase 5', 'Druid', 'Restoration', 'Trinket1', 'Both', 'Solace of the Defeated'),
(11, 2, 13, 0, 290, 54589, 'Phase 5', 'Druid', 'Restoration', 'Trinket2', 'Both', 'Glowing Twilight Scale'),
(11, 2, 14, 0, 290, 50668, 'Phase 5', 'Druid', 'Restoration', 'Back', 'Both', 'Greatcloak of the Turned Champion'),
(11, 2, 15, 0, 290, 46017, 'Phase 5', 'Druid', 'Restoration', 'MainHand', 'Both', 'Val''anyr, Hammer of Ancient Kings'),
(11, 2, 16, 0, 290, 50635, 'Phase 5', 'Druid', 'Restoration', 'OffHand', 'Both', 'Sundial of Eternal Dusk'),
(11, 2, 17, 0, 290, 50454, 'Phase 5', 'Druid', 'Restoration', 'Ranged', 'Both', 'Idol of the Black Willow');

-- Feral Bear (tab 10)
-- ilvl 66 (Phase 1 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 66, 14539, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Head', 'Both', 'Bone Ring Helm'),
(11, 10, 1, 0, 66, 13177, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Talisman of Evasion'),
(11, 10, 2, 0, 66, 10783, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Atal''ai Spaulders'),
(11, 10, 4, 0, 66, 15064, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Warbear Harness'),
(11, 10, 5, 0, 66, 13252, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Cloudrunner Girdle'),
(11, 10, 6, 0, 66, 11821, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Warstrife Leggings'),
(11, 10, 7, 0, 66, 16711, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Shadowcraft Boots'),
(11, 10, 8, 0, 66, 12966, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Blackmist Armguards'),
(11, 10, 9, 0, 66, 13258, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 10, 10, 0, 66, 15855, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Ring of Protection'),
(11, 10, 11, 0, 66, 11669, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Finger2', 'Both', 'Naglering'),
(11, 10, 12, 0, 66, 13966, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Mark of Tyranny'),
(11, 10, 13, 0, 66, 11811, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Smoking Heart of the Mountain'),
(11, 10, 14, 0, 66, 12551, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'Back', 'Both', 'Stoneshield Cloak'),
(11, 10, 15, 0, 66, 943, 'Phase 1 (Pre-Raid)', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Warden Staff');

-- ilvl 76 (Phase 2 (Pre-Raid))
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 76, 14539, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Head', 'Both', 'Bone Ring Helm'),
(11, 10, 1, 0, 76, 13177, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Talisman of Evasion'),
(11, 10, 2, 0, 76, 10783, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Atal''ai Spaulders'),
(11, 10, 4, 0, 76, 15064, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Warbear Harness'),
(11, 10, 5, 0, 76, 13252, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Cloudrunner Girdle'),
(11, 10, 6, 0, 76, 11821, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Warstrife Leggings'),
(11, 10, 7, 0, 76, 16711, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Shadowcraft Boots'),
(11, 10, 8, 0, 76, 12966, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Blackmist Armguards'),
(11, 10, 9, 0, 76, 13258, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 10, 10, 0, 76, 15855, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Ring of Protection'),
(11, 10, 11, 0, 76, 11669, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Finger2', 'Both', 'Naglering'),
(11, 10, 12, 0, 76, 13966, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Mark of Tyranny'),
(11, 10, 13, 0, 76, 11811, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Smoking Heart of the Mountain'),
(11, 10, 14, 0, 76, 12551, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'Back', 'Both', 'Stoneshield Cloak'),
(11, 10, 15, 0, 76, 943, 'Phase 2 (Pre-Raid)', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Warden Staff');

-- ilvl 78 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 78, 14539, 'Phase 2', 'Druid', 'Feral Bear', 'Head', 'Both', 'Bone Ring Helm'),
(11, 10, 1, 0, 78, 17065, 'Phase 2', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Medallion of Steadfast Might'),
(11, 10, 2, 0, 78, 19139, 'Phase 2', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Fireguard Shoulders'),
(11, 10, 4, 0, 78, 15064, 'Phase 2', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Warbear Harness'),
(11, 10, 5, 0, 78, 19149, 'Phase 2', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Lava Belt'),
(11, 10, 6, 0, 78, 11821, 'Phase 2', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Warstrife Leggings'),
(11, 10, 7, 0, 78, 16711, 'Phase 2', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Shadowcraft Boots'),
(11, 10, 8, 0, 78, 12966, 'Phase 2', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Blackmist Armguards'),
(11, 10, 9, 0, 78, 13258, 'Phase 2', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 10, 10, 0, 78, 15855, 'Phase 2', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Ring of Protection'),
(11, 10, 11, 0, 78, 18879, 'Phase 2', 'Druid', 'Feral Bear', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(11, 10, 12, 0, 78, 13966, 'Phase 2', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Mark of Tyranny'),
(11, 10, 13, 0, 78, 11811, 'Phase 2', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Smoking Heart of the Mountain'),
(11, 10, 14, 0, 78, 17107, 'Phase 2', 'Druid', 'Feral Bear', 'Back', 'Both', 'Dragon''s Blood Cape'),
(11, 10, 15, 0, 78, 943, 'Phase 2', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Warden Staff');

-- ilvl 83 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 83, 14539, 'Phase 3', 'Druid', 'Feral Bear', 'Head', 'Both', 'Bone Ring Helm'),
(11, 10, 1, 0, 83, 17065, 'Phase 3', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Medallion of Steadfast Might'),
(11, 10, 2, 0, 83, 19389, 'Phase 3', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Taut Dragonhide Shoulderpads'),
(11, 10, 4, 0, 83, 19405, 'Phase 3', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Malfurion''s Blessed Bulwark'),
(11, 10, 5, 0, 83, 19149, 'Phase 3', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Lava Belt'),
(11, 10, 6, 0, 83, 11821, 'Phase 3', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Warstrife Leggings'),
(11, 10, 7, 0, 83, 19381, 'Phase 3', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Boots of the Shadow Flame'),
(11, 10, 8, 0, 83, 12966, 'Phase 3', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Blackmist Armguards'),
(11, 10, 9, 0, 83, 13258, 'Phase 3', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Slaghide Gauntlets'),
(11, 10, 10, 0, 83, 19376, 'Phase 3', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Archimtiros'' Ring of Reckoning'),
(11, 10, 11, 0, 83, 18879, 'Phase 3', 'Druid', 'Feral Bear', 'Finger2', 'Both', 'Heavy Dark Iron Ring'),
(11, 10, 12, 0, 83, 13966, 'Phase 3', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Mark of Tyranny'),
(11, 10, 13, 0, 83, 11811, 'Phase 3', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Smoking Heart of the Mountain'),
(11, 10, 14, 0, 83, 19386, 'Phase 3', 'Druid', 'Feral Bear', 'Back', 'Both', 'Elementium Threaded Cloak'),
(11, 10, 15, 0, 83, 943, 'Phase 3', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Warden Staff');

-- ilvl 88 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 1, 88, 21693, 'Phase 5', 'Druid', 'Feral Bear', 'Head', 'Alliance', 'Guise of the Devourer'),
(11, 10, 0, 2, 88, 21693, 'Phase 5', 'Druid', 'Feral Bear', 'Head', 'Horde', 'Guise of the Devourer'),
(11, 10, 1, 1, 88, 22732, 'Phase 5', 'Druid', 'Feral Bear', 'Neck', 'Alliance', 'Mark of C''Thun'),
(11, 10, 1, 2, 88, 22732, 'Phase 5', 'Druid', 'Feral Bear', 'Neck', 'Horde', 'Mark of C''Thun'),
(11, 10, 2, 1, 88, 20059, 'Phase 5', 'Druid', 'Feral Bear', 'Shoulders', 'Alliance', 'Highlander''s Leather Shoulders'),
(11, 10, 2, 2, 88, 20194, 'Phase 5', 'Druid', 'Feral Bear', 'Shoulders', 'Horde', 'Defiler''s Leather Shoulders'),
(11, 10, 4, 1, 88, 19405, 'Phase 5', 'Druid', 'Feral Bear', 'Chest', 'Alliance', 'Malfurion''s Blessed Bulwark'),
(11, 10, 4, 2, 88, 19405, 'Phase 5', 'Druid', 'Feral Bear', 'Chest', 'Horde', 'Malfurion''s Blessed Bulwark'),
(11, 10, 5, 1, 88, 21675, 'Phase 5', 'Druid', 'Feral Bear', 'Waist', 'Alliance', 'Thick Qirajihide Belt'),
(11, 10, 5, 2, 88, 21675, 'Phase 5', 'Druid', 'Feral Bear', 'Waist', 'Horde', 'Thick Qirajihide Belt'),
(11, 10, 6, 1, 88, 22749, 'Phase 5', 'Druid', 'Feral Bear', 'Legs', 'Alliance', 'Sentinel''s Leather Pants'),
(11, 10, 6, 2, 88, 22740, 'Phase 5', 'Druid', 'Feral Bear', 'Legs', 'Horde', 'Outrider''s Leather Pants'),
(11, 10, 7, 1, 88, 19381, 'Phase 5', 'Druid', 'Feral Bear', 'Feet', 'Alliance', 'Boots of the Shadow Flame'),
(11, 10, 7, 2, 88, 19381, 'Phase 5', 'Druid', 'Feral Bear', 'Feet', 'Horde', 'Boots of the Shadow Flame'),
(11, 10, 8, 1, 88, 21602, 'Phase 5', 'Druid', 'Feral Bear', 'Wrists', 'Alliance', 'Qiraji Execution Bracers'),
(11, 10, 8, 2, 88, 21602, 'Phase 5', 'Druid', 'Feral Bear', 'Wrists', 'Horde', 'Qiraji Execution Bracers'),
(11, 10, 9, 1, 88, 21605, 'Phase 5', 'Druid', 'Feral Bear', 'Hands', 'Alliance', 'Gloves of the Hidden Temple'),
(11, 10, 9, 2, 88, 21605, 'Phase 5', 'Druid', 'Feral Bear', 'Hands', 'Horde', 'Gloves of the Hidden Temple'),
(11, 10, 10, 1, 88, 21601, 'Phase 5', 'Druid', 'Feral Bear', 'Finger1', 'Alliance', 'Ring of Emperor Vek''lor'),
(11, 10, 10, 2, 88, 21601, 'Phase 5', 'Druid', 'Feral Bear', 'Finger1', 'Horde', 'Ring of Emperor Vek''lor'),
(11, 10, 11, 1, 88, 18879, 'Phase 5', 'Druid', 'Feral Bear', 'Finger2', 'Alliance', 'Heavy Dark Iron Ring'),
(11, 10, 11, 2, 88, 18879, 'Phase 5', 'Druid', 'Feral Bear', 'Finger2', 'Horde', 'Heavy Dark Iron Ring'),
(11, 10, 12, 1, 88, 13966, 'Phase 5', 'Druid', 'Feral Bear', 'Trinket1', 'Alliance', 'Mark of Tyranny'),
(11, 10, 12, 2, 88, 13966, 'Phase 5', 'Druid', 'Feral Bear', 'Trinket1', 'Horde', 'Mark of Tyranny'),
(11, 10, 13, 1, 88, 11811, 'Phase 5', 'Druid', 'Feral Bear', 'Trinket2', 'Alliance', 'Smoking Heart of the Mountain'),
(11, 10, 13, 2, 88, 11811, 'Phase 5', 'Druid', 'Feral Bear', 'Trinket2', 'Horde', 'Smoking Heart of the Mountain'),
(11, 10, 14, 1, 88, 19386, 'Phase 5', 'Druid', 'Feral Bear', 'Back', 'Alliance', 'Elementium Threaded Cloak'),
(11, 10, 14, 2, 88, 19386, 'Phase 5', 'Druid', 'Feral Bear', 'Back', 'Horde', 'Elementium Threaded Cloak'),
(11, 10, 15, 1, 88, 943, 'Phase 5', 'Druid', 'Feral Bear', 'MainHand', 'Alliance', 'Warden Staff'),
(11, 10, 15, 2, 88, 943, 'Phase 5', 'Druid', 'Feral Bear', 'MainHand', 'Horde', 'Warden Staff'),
(11, 10, 17, 1, 88, 23198, 'Phase 5', 'Druid', 'Feral Bear', 'Ranged', 'Alliance', 'Idol of Brutality'),
(11, 10, 17, 2, 88, 23198, 'Phase 5', 'Druid', 'Feral Bear', 'Ranged', 'Horde', 'Idol of Brutality');

-- ilvl 92 (Phase 6)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 1, 92, 21693, 'Phase 6', 'Druid', 'Feral Bear', 'Head', 'Alliance', 'Guise of the Devourer'),
(11, 10, 0, 2, 92, 21693, 'Phase 6', 'Druid', 'Feral Bear', 'Head', 'Horde', 'Guise of the Devourer'),
(11, 10, 1, 1, 92, 22732, 'Phase 6', 'Druid', 'Feral Bear', 'Neck', 'Alliance', 'Mark of C''Thun'),
(11, 10, 1, 2, 92, 22732, 'Phase 6', 'Druid', 'Feral Bear', 'Neck', 'Horde', 'Mark of C''Thun'),
(11, 10, 2, 1, 92, 20059, 'Phase 6', 'Druid', 'Feral Bear', 'Shoulders', 'Alliance', 'Highlander''s Leather Shoulders'),
(11, 10, 2, 2, 92, 20194, 'Phase 6', 'Druid', 'Feral Bear', 'Shoulders', 'Horde', 'Defiler''s Leather Shoulders'),
(11, 10, 4, 1, 92, 23226, 'Phase 6', 'Druid', 'Feral Bear', 'Chest', 'Alliance', 'Ghoul Skin Tunic'),
(11, 10, 4, 2, 92, 23226, 'Phase 6', 'Druid', 'Feral Bear', 'Chest', 'Horde', 'Ghoul Skin Tunic'),
(11, 10, 5, 1, 92, 21675, 'Phase 6', 'Druid', 'Feral Bear', 'Waist', 'Alliance', 'Thick Qirajihide Belt'),
(11, 10, 5, 2, 92, 21675, 'Phase 6', 'Druid', 'Feral Bear', 'Waist', 'Horde', 'Thick Qirajihide Belt'),
(11, 10, 6, 1, 92, 22749, 'Phase 6', 'Druid', 'Feral Bear', 'Legs', 'Alliance', 'Sentinel''s Leather Pants'),
(11, 10, 6, 2, 92, 22740, 'Phase 6', 'Druid', 'Feral Bear', 'Legs', 'Horde', 'Outrider''s Leather Pants'),
(11, 10, 7, 1, 92, 19381, 'Phase 6', 'Druid', 'Feral Bear', 'Feet', 'Alliance', 'Boots of the Shadow Flame'),
(11, 10, 7, 2, 92, 19381, 'Phase 6', 'Druid', 'Feral Bear', 'Feet', 'Horde', 'Boots of the Shadow Flame'),
(11, 10, 8, 1, 92, 22663, 'Phase 6', 'Druid', 'Feral Bear', 'Wrists', 'Alliance', 'Polar Bracers'),
(11, 10, 8, 2, 92, 22663, 'Phase 6', 'Druid', 'Feral Bear', 'Wrists', 'Horde', 'Polar Bracers'),
(11, 10, 9, 1, 92, 21605, 'Phase 6', 'Druid', 'Feral Bear', 'Hands', 'Alliance', 'Gloves of the Hidden Temple'),
(11, 10, 9, 2, 92, 21605, 'Phase 6', 'Druid', 'Feral Bear', 'Hands', 'Horde', 'Gloves of the Hidden Temple'),
(11, 10, 10, 1, 92, 21601, 'Phase 6', 'Druid', 'Feral Bear', 'Finger1', 'Alliance', 'Ring of Emperor Vek''lor'),
(11, 10, 10, 2, 92, 21601, 'Phase 6', 'Druid', 'Feral Bear', 'Finger1', 'Horde', 'Ring of Emperor Vek''lor'),
(11, 10, 11, 1, 92, 23018, 'Phase 6', 'Druid', 'Feral Bear', 'Finger2', 'Alliance', 'Signet of the Fallen Defender'),
(11, 10, 11, 2, 92, 23018, 'Phase 6', 'Druid', 'Feral Bear', 'Finger2', 'Horde', 'Signet of the Fallen Defender'),
(11, 10, 12, 1, 92, 13966, 'Phase 6', 'Druid', 'Feral Bear', 'Trinket1', 'Alliance', 'Mark of Tyranny'),
(11, 10, 12, 2, 92, 13966, 'Phase 6', 'Druid', 'Feral Bear', 'Trinket1', 'Horde', 'Mark of Tyranny'),
(11, 10, 13, 1, 92, 11811, 'Phase 6', 'Druid', 'Feral Bear', 'Trinket2', 'Alliance', 'Smoking Heart of the Mountain'),
(11, 10, 13, 2, 92, 11811, 'Phase 6', 'Druid', 'Feral Bear', 'Trinket2', 'Horde', 'Smoking Heart of the Mountain'),
(11, 10, 14, 1, 92, 22938, 'Phase 6', 'Druid', 'Feral Bear', 'Back', 'Alliance', 'Cryptfiend Silk Cloak'),
(11, 10, 14, 2, 92, 22938, 'Phase 6', 'Druid', 'Feral Bear', 'Back', 'Horde', 'Cryptfiend Silk Cloak'),
(11, 10, 15, 1, 92, 943, 'Phase 6', 'Druid', 'Feral Bear', 'MainHand', 'Alliance', 'Warden Staff'),
(11, 10, 15, 2, 92, 943, 'Phase 6', 'Druid', 'Feral Bear', 'MainHand', 'Horde', 'Warden Staff'),
(11, 10, 17, 1, 92, 23198, 'Phase 6', 'Druid', 'Feral Bear', 'Ranged', 'Alliance', 'Idol of Brutality'),
(11, 10, 17, 2, 92, 23198, 'Phase 6', 'Druid', 'Feral Bear', 'Ranged', 'Horde', 'Idol of Brutality');

-- ilvl 120 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 120, 28182, 'Pre-Raid', 'Druid', 'Feral Bear', 'Head', 'Both', 'Helm of the Claw'),
(11, 10, 1, 0, 120, 27779, 'Pre-Raid', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Bone Chain Necklace'),
(11, 10, 2, 0, 120, 27434, 'Pre-Raid', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Mantle of Perenolde'),
(11, 10, 4, 0, 120, 25689, 'Pre-Raid', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Heavy Clefthoof Vest'),
(11, 10, 5, 0, 120, 30942, 'Pre-Raid', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Manimal''s Cinch'),
(11, 10, 6, 0, 120, 31544, 'Pre-Raid', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Clefthoof Hide Leggings'),
(11, 10, 7, 0, 120, 25691, 'Pre-Raid', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Heavy Clefthoof Boots'),
(11, 10, 8, 0, 120, 29263, 'Pre-Raid', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Forestheart Bracers'),
(11, 10, 9, 0, 120, 30341, 'Pre-Raid', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Flesh Handler''s Gauntlets'),
(11, 10, 10, 0, 120, 30834, 'Pre-Raid', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Shapeshifter''s Signet'),
(11, 10, 12, 0, 120, 19406, 'Pre-Raid', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Drake Fang Talisman'),
(11, 10, 13, 0, 120, 29383, 'Pre-Raid', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Bloodlust Brooch'),
(11, 10, 14, 0, 120, 24258, 'Pre-Raid', 'Druid', 'Feral Bear', 'Back', 'Both', 'Resolute Cape'),
(11, 10, 15, 0, 120, 29171, 'Pre-Raid', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Earthwarden'),
(11, 10, 17, 0, 120, 28064, 'Pre-Raid', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of the Wild');

-- ilvl 125 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 125, 29098, 'Phase 1', 'Druid', 'Feral Bear', 'Head', 'Both', 'Stag-Helm of Malorne'),
(11, 10, 1, 0, 125, 28509, 'Phase 1', 'Druid', 'Feral Bear', 'Neck', 'Both', 'Worgen Claw Necklace'),
(11, 10, 2, 0, 125, 29100, 'Phase 1', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Mantle of Malorne'),
(11, 10, 4, 0, 125, 29096, 'Phase 1', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Breastplate of Malorne'),
(11, 10, 5, 0, 125, 29264, 'Phase 1', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Tree-Mender''s Belt'),
(11, 10, 6, 0, 125, 29099, 'Phase 1', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Greaves of Malorne'),
(11, 10, 7, 0, 125, 28545, 'Phase 1', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Edgewalker Longboots'),
(11, 10, 8, 0, 125, 29263, 'Phase 1', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Forestheart Bracers'),
(11, 10, 9, 0, 125, 29097, 'Phase 1', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Gauntlets of Malorne'),
(11, 10, 10, 0, 125, 30834, 'Phase 1', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Shapeshifter''s Signet'),
(11, 10, 11, 0, 125, 29279, 'Phase 1', 'Druid', 'Feral Bear', 'Finger2', 'Both', 'Violet Signet of the Great Protector'),
(11, 10, 12, 0, 125, 29383, 'Phase 1', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Bloodlust Brooch'),
(11, 10, 13, 0, 125, 28830, 'Phase 1', 'Druid', 'Feral Bear', 'Trinket2', 'Both', 'Dragonspine Trophy'),
(11, 10, 14, 0, 125, 28660, 'Phase 1', 'Druid', 'Feral Bear', 'Back', 'Both', 'Gilded Thorium Cloak'),
(11, 10, 15, 0, 125, 28658, 'Phase 1', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Terestian''s Stranglestaff'),
(11, 10, 17, 0, 125, 23198, 'Phase 1', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of Brutality');

-- ilvl 200 (Pre-Raid)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 200, 42550, 'Pre-Raid', 'Druid', 'Feral Bear', 'Head', 'Both', 'Weakness Spectralizers'),
(11, 10, 2, 0, 200, 43481, 'Pre-Raid', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Trollwoven Spaulders'),
(11, 10, 4, 0, 200, 39554, 'Pre-Raid', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Heroes'' Dreamwalker Raiments'),
(11, 10, 5, 0, 200, 40694, 'Pre-Raid', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Jorach''s Crocolisk Skin Belt'),
(11, 10, 6, 0, 200, 37644, 'Pre-Raid', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Gored Hide Legguards'),
(11, 10, 7, 0, 200, 44297, 'Pre-Raid', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Boots of the Neverending Path'),
(11, 10, 8, 0, 200, 44203, 'Pre-Raid', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Dragonfriend Bracers'),
(11, 10, 9, 0, 200, 37409, 'Pre-Raid', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Gilt-Edged Leather Gauntlets'),
(11, 10, 10, 0, 200, 40586, 'Pre-Raid', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Band of the Kirin Tor'),
(11, 10, 12, 0, 200, 42987, 'Pre-Raid', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(11, 10, 14, 0, 200, 43406, 'Pre-Raid', 'Druid', 'Feral Bear', 'Back', 'Both', 'Cloak of the Gushing Wound'),
(11, 10, 17, 0, 200, 40713, 'Pre-Raid', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of the Ravenous Beast');

-- ilvl 224 (Phase 1)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 224, 40329, 'Phase 1', 'Druid', 'Feral Bear', 'Head', 'Both', 'Hood of the Exodus'),
(11, 10, 2, 0, 224, 40494, 'Phase 1', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Valorous Dreamwalker Shoulderpads'),
(11, 10, 4, 0, 224, 40471, 'Phase 1', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Valorous Dreamwalker Raiments'),
(11, 10, 5, 0, 224, 43591, 'Phase 1', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Polar Cord'),
(11, 10, 6, 0, 224, 44011, 'Phase 1', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Leggings of the Honored'),
(11, 10, 7, 0, 224, 40243, 'Phase 1', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Footwraps of Vile Deceit'),
(11, 10, 8, 0, 224, 40186, 'Phase 1', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Thrusting Bands'),
(11, 10, 9, 0, 224, 40472, 'Phase 1', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Valorous Dreamwalker Handgrips'),
(11, 10, 10, 0, 224, 40370, 'Phase 1', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Gatekeeper'),
(11, 10, 12, 0, 224, 44253, 'Phase 1', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Darkmoon Card: Greatness'),
(11, 10, 14, 0, 224, 40252, 'Phase 1', 'Druid', 'Feral Bear', 'Back', 'Both', 'Cloak of the Shadowed Sun'),
(11, 10, 17, 0, 224, 38365, 'Phase 1', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of Perspicacious Attacks');

-- ilvl 245 (Phase 2)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 245, 41678, 'Phase 2', 'Druid', 'Feral Bear', 'Head', 'Both', 'Furious Gladiator''s Dragonhide Helm'),
(11, 10, 2, 0, 245, 45245, 'Phase 2', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Shoulderpads of the Intruder'),
(11, 10, 4, 0, 245, 45473, 'Phase 2', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Embrace of the Gladiator'),
(11, 10, 5, 0, 245, 46095, 'Phase 2', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Soul-Devouring Cinch'),
(11, 10, 6, 0, 245, 45536, 'Phase 2', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Legguards of Cunning Deception'),
(11, 10, 7, 0, 245, 45232, 'Phase 2', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Runed Ironhide Boots'),
(11, 10, 8, 0, 245, 45611, 'Phase 2', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Solar Bindings'),
(11, 10, 9, 0, 245, 46043, 'Phase 2', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Gloves of the Endless Dark'),
(11, 10, 10, 0, 245, 45471, 'Phase 2', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Fate''s Clutch'),
(11, 10, 12, 0, 245, 45158, 'Phase 2', 'Druid', 'Feral Bear', 'Trinket1', 'Both', 'Heart of Iron'),
(11, 10, 14, 0, 245, 45496, 'Phase 2', 'Druid', 'Feral Bear', 'Back', 'Both', 'Titanskin Cloak'),
(11, 10, 17, 0, 245, 45509, 'Phase 2', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of the Corruptor');

-- ilvl 258 (Phase 3)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 258, 48201, 'Phase 3', 'Druid', 'Feral Bear', 'Head', 'Both', 'Headguard of Triumph'),
(11, 10, 2, 0, 258, 45245, 'Phase 3', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Shoulderpads of the Intruder'),
(11, 10, 4, 1, 258, 47004, 'Phase 3', 'Druid', 'Feral Bear', 'Chest', 'Alliance', 'Cuirass of Calamitous Fate'),
(11, 10, 4, 2, 258, 47431, 'Phase 3', 'Druid', 'Feral Bear', 'Chest', 'Horde', 'Vest of Calamitous Fate'),
(11, 10, 5, 1, 258, 47112, 'Phase 3', 'Druid', 'Feral Bear', 'Waist', 'Alliance', 'Belt of the Merciless Killer'),
(11, 10, 5, 2, 258, 47460, 'Phase 3', 'Druid', 'Feral Bear', 'Waist', 'Horde', 'Belt of the Pitiless Killer'),
(11, 10, 6, 1, 258, 46975, 'Phase 3', 'Druid', 'Feral Bear', 'Legs', 'Alliance', 'Leggings of the Broken Beast'),
(11, 10, 6, 2, 258, 47420, 'Phase 3', 'Druid', 'Feral Bear', 'Legs', 'Horde', 'Legwraps of the Broken Beast'),
(11, 10, 7, 1, 258, 47077, 'Phase 3', 'Druid', 'Feral Bear', 'Feet', 'Alliance', 'Treads of the Icewalker'),
(11, 10, 7, 2, 258, 47445, 'Phase 3', 'Druid', 'Feral Bear', 'Feet', 'Horde', 'Icewalker Treads'),
(11, 10, 8, 1, 258, 47155, 'Phase 3', 'Druid', 'Feral Bear', 'Wrists', 'Alliance', 'Bracers of Dark Determination'),
(11, 10, 8, 2, 258, 47474, 'Phase 3', 'Druid', 'Feral Bear', 'Wrists', 'Horde', 'Armbands of Dark Determination'),
(11, 10, 9, 0, 258, 48202, 'Phase 3', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Handgrips of Triumph'),
(11, 10, 10, 1, 258, 47955, 'Phase 3', 'Druid', 'Feral Bear', 'Finger1', 'Alliance', 'Loop of the Twin Val''kyr'),
(11, 10, 10, 2, 258, 48027, 'Phase 3', 'Druid', 'Feral Bear', 'Finger1', 'Horde', 'Band of the Twin Val''kyr'),
(11, 10, 12, 1, 258, 47088, 'Phase 3', 'Druid', 'Feral Bear', 'Trinket1', 'Alliance', 'Satrina''s Impeding Scarab'),
(11, 10, 12, 2, 258, 47451, 'Phase 3', 'Druid', 'Feral Bear', 'Trinket1', 'Horde', 'Juggernaut''s Vitality'),
(11, 10, 14, 1, 258, 47549, 'Phase 3', 'Druid', 'Feral Bear', 'Back', 'Alliance', 'Magni''s Resolution'),
(11, 10, 14, 2, 258, 47550, 'Phase 3', 'Druid', 'Feral Bear', 'Back', 'Horde', 'Cairne''s Endurance'),
(11, 10, 15, 0, 258, 48523, 'Phase 3', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Relentless Gladiator''s Greatstaff'),
(11, 10, 17, 0, 258, 45509, 'Phase 3', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of the Corruptor');

-- ilvl 264 (Phase 4)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 264, 51296, 'Phase 4', 'Druid', 'Feral Bear', 'Head', 'Both', 'Sanctified Lasherweave Headguard'),
(11, 10, 2, 0, 264, 51299, 'Phase 4', 'Druid', 'Feral Bear', 'Shoulders', 'Both', 'Sanctified Lasherweave Shoulderpads'),
(11, 10, 4, 0, 264, 51298, 'Phase 4', 'Druid', 'Feral Bear', 'Chest', 'Both', 'Sanctified Lasherweave Raiment'),
(11, 10, 5, 0, 264, 50707, 'Phase 4', 'Druid', 'Feral Bear', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(11, 10, 6, 0, 264, 51297, 'Phase 4', 'Druid', 'Feral Bear', 'Legs', 'Both', 'Sanctified Lasherweave Legguards'),
(11, 10, 7, 0, 264, 50607, 'Phase 4', 'Druid', 'Feral Bear', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(11, 10, 8, 0, 264, 50670, 'Phase 4', 'Druid', 'Feral Bear', 'Wrists', 'Both', 'Toskk''s Maximized Wristguards'),
(11, 10, 9, 0, 264, 50675, 'Phase 4', 'Druid', 'Feral Bear', 'Hands', 'Both', 'Aldriana''s Gloves of Secrecy'),
(11, 10, 10, 0, 264, 50402, 'Phase 4', 'Druid', 'Feral Bear', 'Finger1', 'Both', 'Ashen Band of Endless Vengeance'),
(11, 10, 12, 1, 264, 47088, 'Phase 4', 'Druid', 'Feral Bear', 'Trinket1', 'Alliance', 'Satrina''s Impeding Scarab'),
(11, 10, 12, 2, 264, 47451, 'Phase 4', 'Druid', 'Feral Bear', 'Trinket1', 'Horde', 'Juggernaut''s Vitality'),
(11, 10, 14, 0, 264, 50466, 'Phase 4', 'Druid', 'Feral Bear', 'Back', 'Both', 'Sentinel''s Winter Cloak'),
(11, 10, 15, 0, 264, 50735, 'Phase 4', 'Druid', 'Feral Bear', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(11, 10, 17, 0, 264, 50456, 'Phase 4', 'Druid', 'Feral Bear', 'Ranged', 'Both', 'Idol of the Crying Moon');

-- ilvl 290 (Phase 5)
INSERT INTO `playerbots_bis_gear` VALUES
(11, 10, 0, 0, 290, 51296, 'Phase 5', 'Druid', 'FeralBear', 'Head', 'Both', 'Sanctified Lasherweave Headguard'),
(11, 10, 1, 0, 290, 50682, 'Phase 5', 'Druid', 'FeralBear', 'Neck', 'Both', 'Bile-Encrusted Medallion'),
(11, 10, 2, 0, 290, 51299, 'Phase 5', 'Druid', 'FeralBear', 'Shoulders', 'Both', 'Sanctified Lasherweave Shoulderpads'),
(11, 10, 4, 0, 290, 50656, 'Phase 5', 'Druid', 'FeralBear', 'Chest', 'Both', 'Ikfirus''s Sack of Wonder'),
(11, 10, 5, 0, 290, 50707, 'Phase 5', 'Druid', 'FeralBear', 'Waist', 'Both', 'Astrylian''s Sutured Cinch'),
(11, 10, 6, 0, 290, 51297, 'Phase 5', 'Druid', 'FeralBear', 'Legs', 'Both', 'Sanctified Lasherweave Legguards'),
(11, 10, 7, 0, 290, 50607, 'Phase 5', 'Druid', 'FeralBear', 'Feet', 'Both', 'Frostbitten Fur Boots'),
(11, 10, 8, 0, 290, 54580, 'Phase 5', 'Druid', 'FeralBear', 'Wrists', 'Both', 'Umbrage Armbands'),
(11, 10, 9, 0, 290, 51295, 'Phase 5', 'Druid', 'FeralBear', 'Hands', 'Both', 'Sanctified Lasherweave Handgrips'),
(11, 10, 10, 0, 290, 50622, 'Phase 5', 'Druid', 'FeralBear', 'Finger1', 'Both', 'Devium''s Eternally Cold Ring'),
(11, 10, 11, 0, 290, 50404, 'Phase 5', 'Druid', 'FeralBear', 'Finger2', 'Both', 'Ashen Band of Endless Courage'),
(11, 10, 12, 0, 290, 50364, 'Phase 5', 'Druid', 'FeralBear', 'Trinket1', 'Both', 'Sindragosa''s Flawless Fang'),
(11, 10, 13, 0, 290, 50356, 'Phase 5', 'Druid', 'FeralBear', 'Trinket2', 'Both', 'Corroded Skeleton Key'),
(11, 10, 14, 0, 290, 50466, 'Phase 5', 'Druid', 'FeralBear', 'Back', 'Both', 'Sentinel''s Winter Cloak'),
(11, 10, 15, 0, 290, 50735, 'Phase 5', 'Druid', 'FeralBear', 'MainHand', 'Both', 'Oathbinder, Charge of the Ranger-General'),
(11, 10, 17, 0, 290, 50456, 'Phase 5', 'Druid', 'FeralBear', 'Ranged', 'Both', 'Idol of the Crying Moon');

