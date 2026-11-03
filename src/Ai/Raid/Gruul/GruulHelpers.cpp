/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <limits>
#include <list>
#include <mutex>
#include <unordered_map>

using namespace EncounterHelpers;

namespace GruulHelpers
{

namespace
{

std::mutex gruulStateMutex;
std::unordered_map<uint32, GruulInstanceState> gruulStates;

} // end anonymous namespace (Shared)

// High King Maulgar

bool IsMaulgarTank(Player* bot)
{
    // Note: IsMainTank() is not necessarily a tank (by either strategy or spec). It can be anybody
    // with the main tank flag. Raid strategies will have problems with non-tank main tanks so this
    // assumes you are using a real tank for your main tank.
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    // Although passing true for indexLivingOnly means a death will swap the Blindeye tank to Olm,
    // this is intended since Blindeye dies first and Olm is more important to control anyway.
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsBlindeyeTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

bool UpdateGruulClassRoles(PlayerbotAI* botAI)
{
    GruulInstanceState& state = GruulState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAssignments(botAI, GRUUL_MAP_ID, {
        { KROSH_MAGE_TANK_ROLE, state.kroshMageTank },
        { KIGGLER_MOONKIN_TANK_ROLE, state.kigglerMoonkinTank },
    });
}

bool UpdateMaulgarClassRoleAnnouncement(PlayerbotAI* botAI)
{
    GruulInstanceState& state = GruulState(botAI->GetBot()->GetInstanceId());
    return UpdateClassRoleAnnouncement(
        botAI, Id(GruulNpcs::NPC_HIGH_KING_MAULGAR), MAULGAR_SPAWN_POSITION,
        MAULGAR_CLASS_ROLE_ANNOUNCE_DISTANCE, MAULGAR_CLASS_ROLE_REARM_DISTANCE, {
            { KROSH_MAGE_TANK_ROLE, state.kroshMageTank },
            { KIGGLER_MOONKIN_TANK_ROLE, state.kigglerMoonkinTank },
        });
}

Player* GetKroshMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->kroshMageTank) : nullptr;
}

bool IsKroshMageTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_MAGE)
        return false;

    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->kroshMageTank);
}

Player* GetKigglerMoonkinTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state ? GetClassRolePlayer(bot, state->kigglerMoonkinTank) : nullptr;
}

bool IsKigglerMoonkinTank(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_DRUID)
        return false;

    GruulInstanceState const* state = FindGruulState(bot->GetInstanceId());
    return state && IsClassRolePlayer(bot, state->kigglerMoonkinTank);
}

GuidVector FindNearbyWildFelStalkerGuids(Player* bot)
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return {};

    std::list<Creature*> creatureList;
    bot->GetCreatureListWithEntryInGrid(
        creatureList, Id(GruulNpcs::NPC_WILD_FEL_STALKER), WILD_FEL_STALKER_SEARCH_RADIUS);

    GuidVector guids;
    guids.reserve(creatureList.size());
    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            guids.push_back(creature->GetGUID());
    }

    std::sort(guids.begin(), guids.end(), [](ObjectGuid const& lhs, ObjectGuid const& rhs)
    {
        return lhs.GetCounter() < rhs.GetCounter();
    });

    return guids;
}

std::vector<Unit*> GetNearbyWildFelStalkers(PlayerbotAI* botAI)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& guids = AI_VALUE_REF(GuidVector, "high king maulgar wild fel stalkers");

    std::vector<Unit*> felStalkers;
    felStalkers.reserve(guids.size());
    for (ObjectGuid const& guid : guids)
    {
        Unit* felStalker = botAI->GetUnit(guid);
        if (felStalker && felStalker->IsAlive())
            felStalkers.push_back(felStalker);
    }

    return felStalkers;
}

// Gruul the Dragonkiller

namespace
{
std::vector<Position> const& GetCaveInPositions(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()
        ->GetValue<std::vector<Position>>("gruul the dragonkiller cave in")->RefGet();
}
}

bool GetNearestCaveInPosition(PlayerbotAI* botAI, Position& pool)
{
    Player* bot = botAI->GetBot();
    bool found = false;
    float nearestDistance = std::numeric_limits<float>::max();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        float const distance = bot->GetExactDist2d(position);
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            pool = position;
            found = true;
        }
    }

    return found;
}

bool IsNearCaveIn(PlayerbotAI* botAI, float radius)
{
    Player* bot = botAI->GetBot();
    for (Position const& position : GetCaveInPositions(botAI))
    {
        if (bot->GetExactDist2d(position) < radius)
            return true;
    }

    return false;
}

bool IsInCaveIn(PlayerbotAI* botAI)
{
    return IsNearCaveIn(botAI, CAVE_IN_RADIUS);
}

bool HasGroundSlam(Player* bot)
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_1)) ||
        bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_2));
}

// Shared encounter state

GruulInstanceState& GruulState(uint32 instanceId)
{
    std::lock_guard lock(gruulStateMutex);
    return gruulStates[instanceId];
}

GruulInstanceState* FindGruulState(uint32 instanceId)
{
    std::lock_guard lock(gruulStateMutex);
    auto it = gruulStates.find(instanceId);
    return it != gruulStates.end() ? &it->second : nullptr;
}

}
