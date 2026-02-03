-- Temporarily disables innodb_strict_mode for the session to allow the script to complete even if legacy table definitions contain InnoDB-incompatible attributes
SET SESSION innodb_strict_mode = 0;	

-- Change the tables to InnoDB
ALTER TABLE playerbots_dungeon_suggestion_abbrevation ENGINE=InnoDB;
ALTER TABLE playerbots_dungeon_suggestion_definition ENGINE=InnoDB;
ALTER TABLE playerbots_dungeon_suggestion_strategy ENGINE=InnoDB;
ALTER TABLE playerbots_equip_cache ENGINE=InnoDB;
ALTER TABLE playerbots_item_info_cache ENGINE=InnoDB;
ALTER TABLE playerbots_rarity_cache ENGINE=InnoDB;
ALTER TABLE playerbots_rnditem_cache ENGINE=InnoDB;
ALTER TABLE playerbots_tele_cache ENGINE=InnoDB;
ALTER TABLE playerbots_travelnode ENGINE=InnoDB;
ALTER TABLE playerbots_travelnode_link ENGINE=InnoDB;
ALTER TABLE playerbots_travelnode_path ENGINE=InnoDB;

-- Re-enables innodb_strict_mode 
SET SESSION innodb_strict_mode = 1;
