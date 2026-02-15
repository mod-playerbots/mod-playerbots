-- Temporarily disables innodb_strict_mode for the session to allow the script to complete even if legacy table definitions contain InnoDB-incompatible attributes
SET SESSION innodb_strict_mode = 0;	

-- Change the tables to InnoDB
ALTER TABLE playerbots_guild_names ENGINE=InnoDB;
ALTER TABLE playerbots_names ENGINE=InnoDB;

-- Re-enables innodb_strict_mode 
SET SESSION innodb_strict_mode = 1;
