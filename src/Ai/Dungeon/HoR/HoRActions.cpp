/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRActions.h"
#include "ChooseTargetActions.h"
#include "Event.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include <cmath>

bool HorEscapeActive(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();

    // Cheap O(1) guard first: this is called from a multiplier for every
    // candidate action on every tick, so never do a grid search outside HoR.
    if (bot->GetMapId() != 668)
        return false;

    // Roleplay intro: the Lich King stands on the altar but is not in combat.
    // Chase event: he uses SetInCombatWithZone(), pulling every player in.
    Unit* lichKing = bot->FindNearestCreature(NPC_HOR_LICH_KING, 150.0f);
    return lichKing && lichKing->IsAlive() && lichKing->IsInCombat();
}

Unit* HorFindPriorityMob(PlayerbotAI* botAI, bool forTank)
{
    static constexpr uint32 dpsPrio[]  = { NPC_HOR_WITCH_DOCTOR, NPC_HOR_ABOMINATION, NPC_HOR_RAGING_GHOUL };
    static constexpr uint32 tankPrio[] = { NPC_HOR_ABOMINATION, NPC_HOR_WITCH_DOCTOR, NPC_HOR_RAGING_GHOUL };

    Player* bot = botAI->GetBot();
    for (uint32 entry : (forTank ? tankPrio : dpsPrio))
    {
        if (Unit* mob = bot->FindNearestCreature(entry, 60.0f))
        {
            if (mob->IsAlive() && mob->IsHostileTo(bot))
                return mob;
        }
    }
    return nullptr;
}

bool HorEscapeAttackAction::Execute(Event /*event*/)
{
    Unit* target = HorFindPriorityMob(botAI, false);
    if (!target)
        return false;

    // Skull-mark the ranged caster so the whole group burns the interrupt
    // target first (bots follow the skull via their normal assist logic).
    if (target->GetEntry() == NPC_HOR_WITCH_DOCTOR)
    {
        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentSkull = group->GetTargetIcon(7);
            if (currentSkull.IsEmpty() || currentSkull != target->GetGUID())
                group->SetTargetIcon(7, bot->GetGUID(), target->GetGUID());
        }
    }

    return Attack(target);
}

bool HorEscapeAttackAction::isUseful()
{
    return HorFindPriorityMob(botAI, false) != nullptr;
}

bool HorEscapeTankAction::Execute(Event /*event*/)
{
    Unit* abomination = bot->FindNearestCreature(NPC_HOR_ABOMINATION, 60.0f);
    if (abomination && abomination->IsAlive())
    {
        // Pull the abomination first so it sticks to the tank.
        if (bot->GetTarget() != abomination->GetGUID())
            Attack(abomination);

        // Position: stand on the far side of the abomination (relative to the
        // group center) so its cleave cone faces away from the group.
        float cx = 0.0f, cy = 0.0f;
        uint32 count = 0;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                    continue;
                cx += member->GetPositionX();
                cy += member->GetPositionY();
                ++count;
            }
        }

        if (count > 0)
        {
            cx /= count;
            cy /= count;
            float dx = abomination->GetPositionX() - cx;
            float dy = abomination->GetPositionY() - cy;
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 1.0f)
            {
                dx /= len;
                dy /= len;
                float tx = abomination->GetPositionX() + dx * 2.0f;
                float ty = abomination->GetPositionY() + dy * 2.0f;
                if (bot->GetExactDist2d(tx, ty) > 3.0f)
                    return MoveTo(bot->GetMapId(), tx, ty, abomination->GetPositionZ());
            }
        }
        return true;
    }

    // No abomination up: tanks join the priority kill (Witch Doctor first).
    Unit* target = HorFindPriorityMob(botAI, false);
    if (target)
        return Attack(target);

    return false;
}

bool HorEscapeTankAction::isUseful()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* abomination = bot->FindNearestCreature(NPC_HOR_ABOMINATION, 60.0f);
    return (abomination && abomination->IsAlive()) || HorFindPriorityMob(botAI, false);
}

bool HorEscapeFollowAction::Execute(Event /*event*/)
{
    // Follow the real player (master) - in a bot group the master is the
    // summoner. Falls back to the group leader when there is no master.
    Unit* target = botAI->GetMaster();
    if (!target)
    {
        if (Group* group = bot->GetGroup())
        {
            Player* leader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
            if (leader && leader != bot)
                target = leader;
        }
    }

    if (target && target->IsInWorld() && target->GetMapId() == bot->GetMapId())
        return MoveNear(target, sPlayerbotAIConfig.FollowDistance);

    return false;
}

bool HorEscapeFollowAction::isUseful()
{
    // Only useful when there is nothing left to fight nearby.
    return HorFindPriorityMob(botAI, false) == nullptr;
}
