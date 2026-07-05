/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetHomeAction.h"

#include "Event.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool SetHomeAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();

    ObjectGuid selection = bot->GetTarget();
    bool isRpgAction = AI_VALUE(GuidPosition, "rpg target") == selection;

    if (!isRpgAction)
    {
        if (master)
            selection = master->GetTarget();
        else
            return false;
    }

    if (Unit* unit = botAI->GetUnit(selection))
        if (unit->HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER))
        {
            Creature* creature = botAI->GetCreature(selection);
            bot->GetSession()->SendBindPoint(creature);
            botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "set_home_success", "This inn is my new home", {}));
            return true;
        }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : npcs)
    {
        Creature* unit = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_INNKEEPER);
        if (!unit)
            continue;

        bot->GetSession()->SendBindPoint(unit);
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "set_home_success", "This inn is my new home", {}));
        return true;
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "set_home_no_innkeeper_error", "Can't find any innkeeper around", {}));
    return false;
}
