/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FishValues.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"
#include "Map.h"
#include "Spell.h"
#include "FishingAction.h"

bool CanFishValue::Calculate()
{
    int32 SkillFishing = bot->GetSkillValue(SKILL_FISHING);

    if (SkillFishing == 0)
        return false;

    if (bot->isSwimming())
        return false;

    if (bot->IsInCombat())
        return false;

    return true;
}

bool CanUseFishingBobberValue::Calculate()
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
    for (auto const& guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
        {
            if (go->GetEntry() != FISHING_BOBBER)
                continue;
            if (go->GetOwnerGUID() != bot->GetGUID())
                continue;

            if (go->getLootState() == GO_READY)
                return true;

            // Not ready yet → delay next check
            time_t bobberActiveTime = go->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
            if (bobberActiveTime > time(0))
                botAI->SetNextCheckDelay((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
            else
                botAI->SetNextCheckDelay(1000);

            return false;
        }
    }
    return false;
}
