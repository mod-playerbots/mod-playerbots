/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidZulAmanHelpers.h"
#include "Group.h"
#include "Playerbots.h"

namespace ZulAmanHelpers
{
    // General
    Position FindSafestNearbyPosition(Player* bot,
        const std::vector<Unit*>& hazards, const Position& safeZoneCenter,
        float safeZoneRadius, float hazardRadius, bool requireSafePath)
    {
        constexpr float searchStep = M_PI / 8.0f;
        constexpr float distanceStep = 1.0f;

        Position bestPos;
        float minMoveDistance = std::numeric_limits<float>::max();
        bool foundSafe = false;

        for (float distance = 0.0f;
             distance <= safeZoneRadius; distance += distanceStep)
        {
            for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
            {
                float x = bot->GetPositionX() + distance * std::cos(angle);
                float y = bot->GetPositionY() + distance * std::sin(angle);

                if (safeZoneCenter.GetExactDist2d(x, y) > safeZoneRadius)
                    continue;

                if (!IsPositionSafeFromHazards(x, y, hazards, hazardRadius))
                    continue;

                Position testPos(x, y, bot->GetPositionZ());

                bool pathSafe = true;
                if (requireSafePath)
                {
                    pathSafe =
                        IsPathSafeFromHazards(bot->GetPosition(), testPos, hazards, hazardRadius);
                    if (!pathSafe)
                        continue;
                }

                float moveDistance = bot->GetExactDist2d(x, y);
                if (!foundSafe || moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = pathSafe;
                }
            }

            if (foundSafe)
                break;
        }

        return bestPos;
    }

    bool IsPathSafeFromHazards(const Position& start, const Position& end,
        const std::vector<Unit*>& hazards, float hazardRadius)
    {
        constexpr uint8 numChecks = 10;
        float dx = end.GetPositionX() - start.GetPositionX();
        float dy = end.GetPositionY() - start.GetPositionY();

        for (uint8 i = 1; i <= numChecks; ++i)
        {
            float ratio = static_cast<float>(i) / numChecks;
            float checkX = start.GetPositionX() + dx * ratio;
            float checkY = start.GetPositionY() + dy * ratio;

            if (!IsPositionSafeFromHazards(checkX, checkY, hazards, hazardRadius))
                return false;
        }

        return true;
    }

    bool IsPositionSafeFromHazards(
        float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
    {
        for (Unit* hazard : hazards)
        {
            if (hazard->GetDistance2d(x, y) < hazardRadius)
                return false;
        }

        return true;
    }

    std::vector<Unit*> GetAllHazardTriggers(Player* bot, uint32 entry, float searchRadius)
    {
        std::vector<Unit*> triggers;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, entry, searchRadius);

        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
                triggers.push_back(creature);
        }

        return triggers;
    }

    // Akil'zon <Eagle Avatar>
    const Position AKILZON_TANK_POSITION = { 378.369f, 1407.718f, 74.797f };
    std::unordered_map<uint32, time_t> akilzonStormTimer;

    bool IsInStormWindow(time_t start, time_t now)
    {
        time_t elapsed = now - start;
        uint32 seconds = elapsed % 60;
        return elapsed >= 55 && (seconds >= 55 || seconds < 10);
    }

    Player* GetElectricalStormTarget(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member &&
                member->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_ELECTRICAL_STORM)))
                return member;
        }

        return nullptr;
    }

    // Nalorakk <Bear Avatar>
    const Position NALORAKK_TANK_POSITION = { -80.208f, 1324.530f, 40.942f };

    // Jan'alai <Dragonhawk Avatar>
    const Position JANALAI_TANK_POSITION = { -33.873f, 1149.571f, 19.146f };

    bool HasFireBombNearby(Player* bot)
    {
        constexpr float searchRadius = 30.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(
            creatureList, static_cast<uint32>(ZulAmanNPCs::NPC_FIRE_BOMB), searchRadius);

        for (Creature* creature : creatureList)
        {
            if (creature && creature->IsAlive())
                return true;
        }

        return false;
    }

    std::pair<Unit*, Unit*> GetAmanishiHatcherPair(PlayerbotAI* botAI)
    {
        Unit* lowest = nullptr;
        Unit* highest = nullptr;

        for (auto const& guid :
             botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit &&
                unit->GetEntry() == static_cast<uint32>(ZulAmanNPCs::NPC_AMANISHI_HATCHER))
            {
                if (!lowest || unit->GetGUID().GetCounter() < lowest->GetGUID().GetCounter())
                    lowest = unit;

                if (!highest || unit->GetGUID().GetCounter() > highest->GetGUID().GetCounter())
                    highest = unit;
            }
        }

        return {lowest, highest};
    }

    // Halazzi <Lynx Avatar>
    const Position HALAZZI_TANK_POSITION = { 370.733f, 1131.202f, 6.516f };

    // Zul'jin
    const Position ZULJIN_TANK_POSITION = { 120.210f, 705.564f, 45.111f };
}
