/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LfgTriggers.h"

#include "Playerbots.h"

bool LfgProposalActiveTrigger::IsActive() { return AI_VALUE(uint32, "lfg proposal"); }

bool UnknownDungeonTrigger::IsActive()
{
    return botAI->HasActivePlayerMaster() && botAI->GetMaster() && botAI->GetMaster()->IsInWorld() &&
           botAI->GetMaster()->GetMap()->IsDungeon() && bot->GetMapId() == botAI->GetMaster()->GetMapId();
}
