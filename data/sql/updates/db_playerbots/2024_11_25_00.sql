UPDATE `updates_include`
SET `path` = '$/data/sql/updates/db_playerbots'
WHERE `state` = 'RELEASED';

UPDATE `updates_include`
SET `path` = '$/data/sql/custom/db_playerbots'
WHERE `state` = 'CUSTOM';

UPDATE `updates_include`
SET `path` = '$/data/sql/archive/db_playerbots'
WHERE `state` = 'ARCHIVED';
