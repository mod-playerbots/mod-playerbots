/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GatheringLevelingTriggers.h"

#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"

#include <list>

namespace
{
    constexpr uint32 GATHER_SKILL_MAX = 450;

    bool IsGatheringSkill(uint32 skillId)
    {
        return skillId == SKILL_MINING || skillId == SKILL_HERBALISM || skillId == SKILL_SKINNING;
    }
}  // namespace

bool NeedGatheringLevelingTrigger::IsActive()
{
    if (!sPlayerbotAIConfig.gatherLevelingEnabled)
        return false;

    // With no profession the bot counts as "not maxed": the first node seen
    // pushes it to obtain one. With a profession it must be below max level.
    bool hasProfession = bot->HasSkill(SKILL_MINING) || bot->HasSkill(SKILL_HERBALISM) ||
                         bot->HasSkill(SKILL_SKINNING);
    bool professionNotMaxed = true;
    if (hasProfession)
    {
        professionNotMaxed = false;
        for (uint32 skillId : {uint32(SKILL_MINING), uint32(SKILL_HERBALISM), uint32(SKILL_SKINNING)})
            if (bot->HasSkill(skillId) && bot->GetSkillValue(skillId) > 0 &&
                bot->GetMaxSkillValue(skillId) < GATHER_SKILL_MAX)
            {
                professionNotMaxed = true;
                break;
            }
    }

    return professionNotMaxed && SeesBoostNode();
}

bool NeedGatheringLevelingTrigger::SeesBoostNode()
{
    std::list<GameObject*> targets;
    AnyGameObjectInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.grindDistance);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, sPlayerbotAIConfig.reactDistance);

    for (GameObject* go : targets)
    {
        if (!go || !go->isSpawned() || go->GetGoState() != GO_STATE_READY)
            continue;

        LockEntry const* lockInfo = sLockStore.LookupEntry(go->GetGOInfo()->GetLockId());
        if (!lockInfo)
            continue;

        for (uint8 i = 0; i < 8; ++i)
        {
            if (lockInfo->Type[i] != LOCK_KEY_SKILL)
                continue;

            uint32 skillId = SkillByLockType(LockType(lockInfo->Index[i]));
            if (!IsGatheringSkill(skillId))
                continue;

            uint32 reqSkill = std::max(2u, lockInfo->Skill[i]);
            uint32 botSkill = bot->HasSkill(skillId) ? bot->GetSkillValue(skillId) : 0;
            if (reqSkill > botSkill)
                return true;
        }
    }

    return false;
}
