/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraHelpers.h"
#include "Playerbots.h"

namespace KaraHelpers
{

// General

bool IsSafePosition(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
{
    for (Unit* hazard : hazards)
    {
        float dist = hazard->GetDistance2d(x, y);
        if (dist < hazardRadius)
            return false;
    }

    return true;
}

// Attumen the Huntsman

Position const ATTUMEN_TANK_POSITION = { -11123.762f, -1926.619f, 49.215f };
std::unordered_map<uint32, time_t> attumenDpsWaitTimer;

Unit* GetAttumenMounted(Player* bot)
{
    constexpr uint32 searchRadius = 50.0f;
    return bot->FindNearestCreature(
        static_cast<uint32>(KaraNpcs::NPC_ATTUMEN_THE_HUNTSMAN), searchRadius, true);
}

// Maiden of Virtue

Position const MAIDEN_OF_VIRTUE_TANK_POSITION = { -10945.881f, -2103.782f, 92.712f };
std::array<Position, 8> const MAIDEN_OF_VIRTUE_RANGED_POSITIONS =
{{
    { -10959.242f, -2119.617f, 92.180f }, // SE (opposite entrance)
    { -10944.495f, -2123.857f, 92.180f }, // E
    { -10966.017f, -2105.288f, 92.175f }, // S
    { -10931.178f, -2116.580f, 92.179f }, // NE
    { -10960.912f, -2090.437f, 92.179f }, // SW
    { -10925.828f, -2102.425f, 92.180f }, // N
    { -10947.590f, -2082.815f, 92.180f }, // W
    { -10933.089f, -2088.502f, 92.180f }, // NW (entrance)
}};

// The Big Bad Wolf

Position const BIG_BAD_WOLF_TANK_POSITION = { -10913.391f, -1773.508f, 90.477f };
std::array<Position, 4> const BIG_BAD_WOLF_RUN_POSITIONS =
{{
    { -10875.456f, -1779.036f, 90.477f },
    { -10872.281f, -1751.638f, 90.477f },
    { -10910.492f, -1747.401f, 90.477f },
    { -10913.391f, -1773.508f, 90.477f },
}};

// Wizard of Oz

std::array<const char*, 5> const& GetOzTargets()
{
    static std::array<const char*, 5> const targets =
    {
        "dorothee",
        "tito",
        "roar",
        "strawman",
        "tinhead",
    };

    return targets;
}

// The Curator

Position const THE_CURATOR_TANK_POSITION = { -11139.463f, -1884.645f, 165.765f };

// Shade of Aran

bool IsAranCastingArcaneExplosion(Unit* aran)
{
    return aran && aran->HasUnitState(UNIT_STATE_CASTING) && aran->FindCurrentSpellBySpellId(
        static_cast<uint32>(KaraSpells::SPELL_ARCANE_EXPLOSION));
}

bool IsFlameWreathActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    Spell* currentSpell = aran->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (currentSpell && currentSpell->m_spellInfo && currentSpell->m_spellInfo->Id ==
            static_cast<uint32>(KaraSpells::SPELL_FLAME_WREATH_CAST))
    {
        return true;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;

        if (member->HasAura(static_cast<uint32>(KaraSpells::SPELL_FLAME_WREATH_AURA)))
            return true;
    }

    return false;
}

// Netherspite

std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;

bool IsBanishPhase(Unit* netherspite)
{
    return netherspite && netherspite->HasAura(
        static_cast<uint32>(KaraSpells::SPELL_NETHERSPITE_BANISHED));
}

// Red beam blockers: tank bots, no Nether Exhaustion Red
std::vector<Player*> GetRedBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> redBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !PlayerbotAI::IsTank(member) ||
            !GET_PLAYERBOT_AI(member))
        {
            continue;
        }

        if (!member->HasAura(static_cast<uint32>(KaraSpells::SPELL_NETHER_EXHAUSTION_RED)))
            redBlockers.push_back(member);
    }

    return redBlockers;
}

// Blue beam blockers: DPS bots, excluding Warrior/Rogue/DK
// no Nether Exhaustion Blue and <25 stacks of Blue Beam debuff
std::vector<Player*> GetBlueBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> blueBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsDps(member) || member->getPowerType() != POWER_MANA)
        {
            continue;
        }

        if (member->HasAura(static_cast<uint32>(KaraSpells::SPELL_NETHER_EXHAUSTION_BLUE)))
            continue;

        Aura* blueBuff = member->GetAura(
            static_cast<uint32>(KaraSpells::SPELL_BLUE_BEAM_DEBUFF));
        if (!blueBuff || blueBuff->GetStackAmount() < 25)
            blueBlockers.push_back(member);
    }

    return blueBlockers;
}

// Green beam blockers:
// (1) Prioritize Rogues and non-tank Warrior and DK bots, no Nether Exhaustion Green
// (2) Then assign Healer bots, no Nether Exhaustion Green and <25 stacks of Green Beam debuff
std::vector<Player*> GetGreenBlockers(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return {};

    std::vector<Player*> greenBlockers;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsDps(member) || member->getPowerType() == POWER_MANA)
        {
            continue;
        }

        if (!member->HasAura(static_cast<uint32>(KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN)))
            greenBlockers.push_back(member);
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            !PlayerbotAI::IsHeal(member))
        {
            continue;
        }

        Aura* greenBuff = member->GetAura(
            static_cast<uint32>(KaraSpells::SPELL_GREEN_BEAM_DEBUFF));
        if (!member->HasAura(static_cast<uint32>(KaraSpells::SPELL_NETHER_EXHAUSTION_GREEN)) &&
            (!greenBuff || greenBuff->GetStackAmount() < 25))
        {
            greenBlockers.push_back(member);
        }
    }

    return greenBlockers;
}

std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(Player* bot)
{
    static ObjectGuid currentRedBlocker;
    static ObjectGuid currentGreenBlocker;
    static ObjectGuid currentBlueBlocker;

    Player* redBlocker = nullptr;
    auto redBlockers = GetRedBlockers(bot);
    if (!redBlockers.empty())
    {
        auto it = std::find_if(redBlockers.begin(), redBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentRedBlocker;
        });

        if (it != redBlockers.end())
            redBlocker = *it;
        else
            redBlocker = redBlockers.front();

        currentRedBlocker = redBlocker ? redBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentRedBlocker = ObjectGuid::Empty;
        redBlocker = nullptr;
    }

    Player* greenBlocker = nullptr;
    auto greenBlockers = GetGreenBlockers(bot);
    if (!greenBlockers.empty())
    {
        auto it = std::find_if(greenBlockers.begin(), greenBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentGreenBlocker;
        });

        if (it != greenBlockers.end())
            greenBlocker = *it;
        else
            greenBlocker = greenBlockers.front();

        currentGreenBlocker = greenBlocker ? greenBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentGreenBlocker = ObjectGuid::Empty;
        greenBlocker = nullptr;
    }

    Player* blueBlocker = nullptr;
    auto blueBlockers = GetBlueBlockers(bot);
    if (!blueBlockers.empty())
    {
        auto it = std::find_if(blueBlockers.begin(), blueBlockers.end(), [](Player* player)
        {
            return player && player->GetGUID() == currentBlueBlocker;
        });

        if (it != blueBlockers.end())
            blueBlocker = *it;
        else
            blueBlocker = blueBlockers.front();

        currentBlueBlocker = blueBlocker ? blueBlocker->GetGUID() : ObjectGuid::Empty;
    }
    else
    {
        currentBlueBlocker = ObjectGuid::Empty;
        blueBlocker = nullptr;
    }

    return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
}

std::vector<Unit*> GetAllVoidZones(Player* bot)
{
    std::vector<Unit*> voidZones;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 30.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(KaraNpcs::NPC_VOID_ZONE), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            voidZones.push_back(creature);
    }

    return voidZones;
}

bool FindBeamPosition(
    Unit* netherspite, Unit* portal, std::vector<Unit*> const& voidZones,
    float idealDistance, Position& outPos)
{
    float bossX = netherspite->GetPositionX();
    float bossY = netherspite->GetPositionY();
    float portalX = portal->GetPositionX();
    float portalY = portal->GetPositionY();

    float dx = portalX - bossX;
    float dy = portalY - bossY;
    float length = netherspite->GetExactDist2d(portalX, portalY);
    if (length == 0.0f)
        return false;

    constexpr float voidZoneRadius = 4.0f;
    constexpr float searchMinDist = 18.0f;
    constexpr float searchStep = 0.5f;
    constexpr uint8 numSteps = 24;

    dx /= length;
    dy /= length;

    float bestDist = std::numeric_limits<float>::max();
    bool found = false;

    for (uint8 i = 0; i <= numSteps; ++i)
    {
        float const dist = searchMinDist + i * searchStep;
        float candidateX = bossX + dx * dist;
        float candidateY = bossY + dy * dist;
        float candidateZ = netherspite->GetPositionZ();
        if (!IsSafePosition(candidateX, candidateY, voidZones, voidZoneRadius))
            continue;

        float distToIdeal = fabs(dist - idealDistance);
        if (!found || distToIdeal < bestDist)
        {
            bestDist = distToIdeal;
            outPos = Position(candidateX, candidateY, candidateZ);
            found = true;
        }
    }

    return found;
}

// Prince Malchezaar

std::vector<Unit*> GetSpawnedInfernals(Player* bot)
{
    std::vector<Unit*> infernals;
    std::list<Creature*> creatureList;
    constexpr float searchRadius = 100.0f;

    bot->GetCreatureListWithEntryInGrid(
        creatureList, static_cast<uint32>(KaraNpcs::NPC_NETHERSPITE_INFERNAL), searchRadius);

    for (Creature* creature : creatureList)
    {
        if (creature && creature->IsAlive())
            infernals.push_back(creature);
    }

    return infernals;
}

bool IsStraightPathSafe(
    float sx, float sy, float tx, float ty, std::vector<Unit*> const& hazards, float hazardRadius)
{
    float const totalDistX = tx - sx;
    float const totalDistY = ty - sy;
    float const totalDist = sqrt(totalDistX * totalDistX + totalDistY * totalDistY);
    if (totalDist == 0.0f)
        return true;

    constexpr float stepSize = 0.5f;
    int const numSteps = static_cast<int>(totalDist / stepSize);
    for (int i = 0; i <= numSteps; ++i)
    {
        float const checkDist = i * stepSize;
        float const t = (totalDist > 0.0f) ? checkDist / totalDist : 0.0f;
        float checkX = sx + totalDistX * t;
        float checkY = sy + totalDistY * t;
        for (Unit* hazard : hazards)
        {
            float const hx = checkX - hazard->GetPositionX();
            float const hy = checkY - hazard->GetPositionY();
            if ((hx*hx + hy*hy) < hazardRadius * hazardRadius)
                return false;
        }
    }

    return true;
}

bool TryFindSafePositionWithSafePath(
    Player* bot, float originX, float originY, float centerX, float centerY,
    std::vector<Unit*> const& hazards, float safeDistance, float maxSampleDist,
    float& outX, float& outY)
{
    constexpr uint8 numAngles = 64;
    constexpr float stepSize = 0.5f;

    // Attempt to find a safe path, but take any path to a safe position if no safe path is found
    for (bool requireSafePath : { true, false })
    {
        float bestMoveDistSq = std::numeric_limits<float>::max();
        bool found = false;

        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2.0f * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            int const numSteps = static_cast<int>(maxSampleDist / stepSize);
            for (int j = 1; j <= numSteps; ++j)
            {
                float const dist = j * stepSize;
                float destX = centerX + dx * dist;
                float destY = centerY + dy * dist;
                float destZ = bot->GetPositionZ();
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                        bot, centerX, centerY, destZ, destX, destY, destZ, true))
                {
                    continue;
                }

                if (!IsSafePosition(destX, destY, hazards, safeDistance))
                    continue;

                if (requireSafePath)
                {
                    if (!IsStraightPathSafe(originX, originY, destX, destY, hazards, safeDistance))
                        continue;
                }

                float const toDestX = destX - originX;
                float const toDestY = destY - originY;
                float const moveDistSq = toDestX*toDestX + toDestY*toDestY;
                if (moveDistSq < bestMoveDistSq)
                {
                    bestMoveDistSq = moveDistSq;
                    outX = destX;
                    outY = destY;
                    found = true;
                }
            }
        }

        if (found)
            return true;
    }

    return false;
}

// Nightbane

Position const TERRACE_DOME_CENTER = { -11126.015f, -1925.271f, 91.473f };
Position const TERRACE_EAST_END    = { -11115.958f, -1972.058f, 91.457f };
Position const TERRACE_WEST_END    = { -11077.521f, -1913.315f, 91.471f };
std::array<Position, 2> const NIGHTBANE_FLIGHT_STACK_POSITIONS =
{{
    { -11156.233f, -1888.353f, 91.473f },  // primary
    { -11149.115f, -1897.154f, 91.473f },  // backup in case of charred earth
}};
std::array<Position, 2> const NIGHTBANE_RAIN_OF_BONES_POSITIONS =
{{
    { -11166.516f, -1901.405f, 91.473f },  // primary
    { -11158.752f, -1909.394f, 91.473f },  // backup in case of charred earth
}};
Position const NIGHTBANE_TELEPORT_POSITION = { -11159.555f, -1893.526f, 91.473f };
std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;

}
