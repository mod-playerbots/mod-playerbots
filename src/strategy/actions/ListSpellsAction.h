/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_LISTSPELLSACTION_H
#define _PLAYERBOT_LISTSPELLSACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class ListSpellsAction : public InventoryAction
{
public:
    ListSpellsAction(PlayerbotAI* botAI, std::string const name = "spells") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;
    virtual std::vector<std::pair<uint32, std::string>> GetSpellList(std::string filter = "");

    // Initialize static caches used by GetSpellList (skillSpells and vendorItems).
    // Intended to be called from a WorldScript at server startup to avoid
    // synchronous database queries from MapUpdater worker threads.
    static void InitStaticCaches();

private:
    static std::map<uint32, SkillLineAbilityEntry const*> skillSpells;
    static std::set<uint32> vendorItems;
};

#endif
