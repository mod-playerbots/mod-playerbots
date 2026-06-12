CREATE TABLE IF NOT EXISTS `playerbots_account_ownership` (
    `account_id` INT UNSIGNED NOT NULL,
    `owner_realm_id` INT UNSIGNED NOT NULL,
    `claimed_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`account_id`),
    KEY `idx_owner_realm_id` (`owner_realm_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
COLLATE=utf8mb4_unicode_ci
COMMENT='Shared random bot account ownership by realm';
