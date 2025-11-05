-- Add shortName column to existing account_links table (if it doesn't exist)
-- Check if column exists before adding it
SET @col_exists = 0;
SELECT COUNT(*) INTO @col_exists
FROM INFORMATION_SCHEMA.COLUMNS
WHERE TABLE_SCHEMA = DATABASE()
AND TABLE_NAME = 'playerbots_account_links'
AND COLUMN_NAME = 'short_name';

SET @sql = IF(@col_exists = 0,
    'ALTER TABLE playerbots_account_links ADD COLUMN `short_name` varchar(32) NOT NULL DEFAULT ''LegacyLink'' COMMENT ''Friendly name for this connection (same for both users)''',
    'SELECT ''Column short_name already exists'' AS msg');

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Add index for shortName lookups (if it doesn't exist)
SET @index_exists = 0;
SELECT COUNT(*) INTO @index_exists
FROM INFORMATION_SCHEMA.STATISTICS
WHERE TABLE_SCHEMA = DATABASE()
AND TABLE_NAME = 'playerbots_account_links'
AND INDEX_NAME = 'idx_short_name';

SET @sql = IF(@index_exists = 0,
    'CREATE INDEX `idx_short_name` ON playerbots_account_links (`account_id`, `short_name`)',
    'SELECT ''Index idx_short_name already exists'' AS msg');

PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Invite codes table
CREATE TABLE IF NOT EXISTS `playerbots_invite_codes` (
    `id` INT AUTO_INCREMENT PRIMARY KEY,
    `code` varchar(16) NOT NULL,
    `account_id` INT NOT NULL,
    `created_at` INT NOT NULL,
    `expires_at` INT NOT NULL,
    `status` TINYINT NOT NULL DEFAULT 1,
    KEY `idx_account_id` (`account_id`),
    KEY `idx_expires_at` (`expires_at`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- Drop legacy tables that are no longer needed
DROP TABLE IF EXISTS playerbots_account_keys;
