/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLHelpers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Group.h"

#include <algorithm>
#include <vector>

namespace BlackwingLairHelpers
{
    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot)
    {
        constexpr float suppressionDeviceInteractionDistance = 15.0f;
        return go &&
               go->GetEntry() == static_cast<uint32>(BlackwingLairGameObjects::GO_SUPPRESSION_DEVICE) &&
               go->GetDistance(bot) < suppressionDeviceInteractionDistance &&
               go->GetGoState() == GO_STATE_READY;
    }

    bool AreRazorgoreEggsAlive(PlayerbotAI* botAI)
    {
        GuidVector gos = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
        for (auto const& guid : gos)
        {
            const GameObject* go = botAI->GetGameObject(guid);
            if (go && go->GetEntry() == static_cast<uint32>(BlackwingLairGameObjects::GO_BLACK_DRAGON_EGG))
                return true;
        }
        return false;
    }

    bool IsRazorgoreOffTank(Player* bot)
    {
        return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
    }

    bool IsNonBABotNearPosition(const Player* bot, Position const& position, float distance)
    {
        const Group* group = bot->GetGroup();
        if (!group)
            return false;

        for (const GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            const Player* p = gref->GetSource();
            if (!p || p == bot || !p->IsAlive() || p->GetMapId() != bot->GetMapId())
                continue;

            if (p->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BURNING_ADRENALINE)))
                continue;

            if (p->GetDistance2d(position.GetPositionX(), position.GetPositionY()) < distance)
                return true;
        }

        return false;
    }

    Creature* FindNearestInCombat(const Player* bot, BlackwingLairNPCs entry, float range)
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, static_cast<uint32>(entry), range);

        Creature* nearest = nullptr;
        float nearestDist = range;
        for (Creature* creature : creatures)
        {
            if (!creature->IsAlive() || !creature->IsInCombat())
                continue;

            float dist = bot->GetDistance2d(creature);
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearest = creature;
            }
        }
        return nearest;
    }

    // Melee split as evenly as possible across the live warlocks instead of
    // piling the nearest. A bot claims a warlock by its ordinal among the
    // group's alive dps, both lists GUID-ordered, so every bot derives the
    // same split without shared state.
    Creature* FindAssignedWarlock(Player* bot, float range)
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures,
            static_cast<uint32>(BlackwingLairNPCs::NPC_BLACKWING_WARLOCK), range);

        std::vector<Creature*> warlocks;
        for (Creature* creature : creatures)
            if (creature->IsAlive() && creature->IsInCombat())
                warlocks.push_back(creature);

        if (warlocks.empty())
            return nullptr;

        std::sort(warlocks.begin(), warlocks.end(),
            [](Creature const* a, Creature const* b) { return a->GetGUID() < b->GetGUID(); });

        uint32 ordinal = 0;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* member = gref->GetSource();
                if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                    continue;
                if (!PlayerbotAI::IsDps(member) || !(member->GetGUID() < bot->GetGUID()))
                    continue;
                ++ordinal;
            }
        }

        return warlocks[ordinal % warlocks.size()];
    }

    // Steady-state fast path shared by the warlock pack trigger, action and
    // multiplier: the bot's own target proving a live warlock is in the fight
    // makes their grid searches redundant.
    bool IsTargetingLiveWarlock(PlayerbotAI* botAI)
    {
        Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        return current && current->IsAlive() && current->IsInCombat() &&
               current->GetEntry() == static_cast<uint32>(BlackwingLairNPCs::NPC_BLACKWING_WARLOCK);
    }
}
