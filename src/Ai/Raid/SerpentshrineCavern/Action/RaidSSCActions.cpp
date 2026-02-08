#include "RaidSSCActions.h"
#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "Corpse.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"

using namespace SerpentShrineCavernHelpers;

// General

bool SerpentShrineCavernEraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    const uint32 instanceId = bot->GetMap()->GetInstanceId();
    const ObjectGuid guid = bot->GetGUID();

    bool erased = false;
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
    {
        if (hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0)
            erased = true;
        if (hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0)
            erased = true;
        if (hydrossNatureDpsWaitTimer.erase(instanceId) > 0)
            erased = true;
        if (hydrossFrostDpsWaitTimer.erase(instanceId) > 0)
            erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "the lurker below"))
    {
        if (lurkerRangedPositions.erase(guid) > 0)
            erased = true;
        if (lurkerSpoutTimer.erase(instanceId) > 0)
            erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "fathom-lord karathress"))
    {
        if (karathressDpsWaitTimer.erase(instanceId) > 0)
            erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "morogrim tidewalker"))
    {
        if (tidewalkerTankStep.erase(guid) > 0)
            erased = true;
        if (tidewalkerRangedStep.erase(guid) > 0)
            erased = true;
    }
    if (!AI_VALUE2(Unit*, "find target", "lady vashj"))
    {
        if (vashjRangedPositions.erase(guid) > 0)
            erased = true;
        if (hasReachedVashjRangedPosition.erase(guid) > 0)
            erased = true;
    }

    return erased;
}

// Trash Mobs

// Move out of toxic pool left behind by some colossi upon death
bool UnderbogColossusEscapeToxicPoolAction::Execute(Event /*event*/)
{
    Aura* aura = bot->GetAura(SPELL_TOXIC_POOL);
    if (!aura)
        return false;

    DynamicObject* dynObj = aura->GetDynobjOwner();
    if (!dynObj)
        return false;

    float radius = dynObj->GetRadius();
    if (radius <= 0.0f)
    {
        const SpellInfo* sInfo = sSpellMgr->GetSpellInfo(dynObj->GetSpellId());
        if (sInfo)
        {
            for (int e = 0; e < MAX_SPELL_EFFECTS; ++e)
            {
                auto const& eff = sInfo->Effects[e];
                if (eff.Effect == SPELL_EFFECT_SCHOOL_DAMAGE ||
                    (eff.Effect == SPELL_EFFECT_APPLY_AURA &&
                     eff.ApplyAuraName == SPELL_AURA_PERIODIC_DAMAGE))
                {
                    radius = eff.CalcRadius();
                    break;
                }
            }
        }
    }

    if (radius <= 0.0f)
        return false;

    constexpr float bufferDist = 3.0f;
    constexpr float centerThreshold = 1.0f;

    float dx = bot->GetPositionX() - dynObj->GetPositionX();
    float dy = bot->GetPositionY() - dynObj->GetPositionY();

    float distToObj = bot->GetExactDist2d(dynObj->GetPositionX(), dynObj->GetPositionY());
    const float insideThresh = radius + centerThreshold;

    if (distToObj > insideThresh)
        return false;

    float safeDist = radius + bufferDist;
    float moveX, moveY;

    if (distToObj == 0.0f)
    {
        float angle = frand(0.0f, static_cast<float>(M_PI * 2.0));
        moveX = dynObj->GetPositionX() + std::cos(angle) * safeDist;
        moveY = dynObj->GetPositionY() + std::sin(angle) * safeDist;
    }
    else
    {
        float invDist = 1.0f / distToObj;
        moveX = dynObj->GetPositionX() + (dx * invDist) * safeDist;
        moveY = dynObj->GetPositionY() + (dy * invDist) * safeDist;
    }

    botAI->Reset();
    return MoveTo(SSC_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false,
                  true, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool GreyheartTidecallerMarkWaterElementalTotemAction::Execute(Event /*event*/)
{
    if (Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_WATER_ELEMENTAL_TOTEM))
        MarkTargetWithSkull(bot, totem);

    return false;
}

// Hydross the Unstable <Duke of Currents>

// (1) When tanking, move to designated tanking spot on frost side
// (2) 1 second after 100% Mark of Hydross, move to nature tank's spot to hand off boss
// (3) When Hydross is in nature form, move back to frost tank spot and wait for transition
bool HydrossTheUnstablePositionFrostTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (!hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfHydrossAt100Percent(bot))
    {
        MarkTargetWithSquare(bot, hydross);
        SetRtiTarget(botAI, "square", hydross);

        if (bot->GetTarget() != hydross->GetGUID())
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            const Position& position = HYDROSS_FROST_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfHydrossAt100Percent(bot) &&
        hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToNaturePhaseTimer.find(hydross->GetMap()->GetInstanceId());

        if (it != hydrossChangeToNaturePhaseTimer.end() && (now - it->second) >= 1)
        {
            const Position& position = HYDROSS_NATURE_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                botAI->Reset();
                return true;
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HYDROSS_FROST_TANK_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// (1) When tanking, move to designated tanking spot on nature side
// (2) 1 second after 100% Mark of Corruption, move to frost tank's spot to hand off boss
// (3) When Hydross is in frost form, move back to nature tank spot and wait for transition
bool HydrossTheUnstablePositionNatureTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (hydross->HasAura(SPELL_CORRUPTION) && !HasMarkOfCorruptionAt100Percent(bot))
    {
        MarkTargetWithTriangle(bot, hydross);
        SetRtiTarget(botAI, "triangle", hydross);

        if (bot->GetTarget() != hydross->GetGUID())
            return Attack(hydross);

        if (hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
        {
            const Position& position = HYDROSS_NATURE_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
    }

    if (hydross->HasAura(SPELL_CORRUPTION) && HasMarkOfCorruptionAt100Percent(bot) &&
        hydross->GetVictim() == bot && bot->IsWithinMeleeRange(hydross))
    {
        const time_t now = std::time(nullptr);
        auto it = hydrossChangeToFrostPhaseTimer.find(hydross->GetMap()->GetInstanceId());

        if (it != hydrossChangeToFrostPhaseTimer.end() && (now - it->second) >= 1)
        {
            const Position& position = HYDROSS_FROST_TANK_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
            else
            {
                botAI->Reset();
                return true;
            }
        }
    }

    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        const Position& position = HYDROSS_NATURE_TANK_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HydrossTheUnstablePrioritizeElementalAddsAction::Execute(Event /*event*/)
{
    Unit* waterElemental = GetFirstAliveUnitByEntry(botAI, NPC_PURE_SPAWN_OF_HYDROSS);
    if (waterElemental)
    {
        if (IsMechanicTrackerBot(botAI, bot, SSC_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, waterElemental);

        SetRtiTarget(botAI, "skull", waterElemental);

        if (bot->GetTarget() != waterElemental->GetGUID())
            return Attack(waterElemental);
    }
    else if (Unit* natureElemental = GetFirstAliveUnitByEntry(botAI, NPC_TAINTED_SPAWN_OF_HYDROSS))
    {
        if (IsMechanicTrackerBot(botAI, bot, SSC_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, natureElemental);

        SetRtiTarget(botAI, "skull", natureElemental);

        if (bot->GetTarget() != natureElemental->GetGUID())
            return Attack(natureElemental);
    }

    return false;
}

// To mitigate the effect of Water Tomb
bool HydrossTheUnstableFrostPhaseSpreadOutAction::Execute(Event /*event*/)
{
    if (!AI_VALUE2(Unit*, "find target", "hydross the unstable"))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            constexpr float safeDistance = 6.0f;
            constexpr uint32 minInterval = 1000;
            if (bot->GetExactDist2d(member) < safeDistance)
                return FleePosition(member->GetPosition(), safeDistance, minInterval);
        }
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (TryMisdirectToFrostTank(hydross, group))
            return true;

        if (TryMisdirectToNatureTank(hydross, group))
            return true;
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToFrostTank(
    Unit* hydross, Group* group)
{
    Player* frostTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            frostTank = member;
            break;
        }
    }

    if (HasNoMarkOfHydross(bot) && !hydross->HasAura(SPELL_CORRUPTION) && frostTank)
    {
        if (botAI->CanCastSpell("misdirection", frostTank))
            return botAI->CastSpell("misdirection", frostTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableMisdirectBossToTankAction::TryMisdirectToNatureTank(
    Unit* hydross, Group* group)
{
    Player* natureTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsAssistTankOfIndex(member, 0, true))
        {
            natureTank = member;
            break;
        }
    }

    if (HasNoMarkOfCorruption(bot) && hydross->HasAura(SPELL_CORRUPTION) && natureTank)
    {
        if (botAI->CanCastSpell("misdirection", natureTank))
            return botAI->CastSpell("misdirection", natureTank);

        if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", hydross))
            return botAI->CastSpell("steady shot", hydross);
    }

    return false;
}

bool HydrossTheUnstableStopDpsUponPhaseChangeAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    constexpr uint8 phaseStartStopSeconds = 5;
    constexpr uint8 phaseEndStopSeconds = 1;

    bool shouldStopDps = false;

    // 1 second after 100% Mark of Hydross, stop DPS
    auto itNature = hydrossChangeToNaturePhaseTimer.find(instanceId);
    if (itNature != hydrossChangeToNaturePhaseTimer.end() &&
        (now - itNature->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into nature phase
    auto itNatureDps = hydrossNatureDpsWaitTimer.find(instanceId);
    if (itNatureDps != hydrossNatureDpsWaitTimer.end() &&
        (now - itNatureDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    // 1 second after 100% Mark of Corruption, stop DPS
    auto itFrost = hydrossChangeToFrostPhaseTimer.find(instanceId);
    if (itFrost != hydrossChangeToFrostPhaseTimer.end() &&
        (now - itFrost->second) >= phaseEndStopSeconds)
        shouldStopDps = true;

    // Keep DPS stopped for 5 seconds after transition into frost phase
    auto itFrostDps = hydrossFrostDpsWaitTimer.find(instanceId);
    if (itFrostDps != hydrossFrostDpsWaitTimer.end() &&
        (now - itFrostDps->second) < phaseStartStopSeconds)
        shouldStopDps = true;

    if (shouldStopDps)
    {
        botAI->Reset();
        return true;
    }

    return false;
}

bool HydrossTheUnstableManageTimersAction::Execute(Event /*event*/)
{
    Unit* hydross = AI_VALUE2(Unit*, "find target", "hydross the unstable");
    if (!hydross)
        return false;

    const uint32 instanceId = hydross->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    bool changed = false;
    if (!hydross->HasAura(SPELL_CORRUPTION))
    {
        if (hydrossFrostDpsWaitTimer.try_emplace(instanceId, now).second)
            changed = true;
        if (hydrossNatureDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
        if (hydrossChangeToFrostPhaseTimer.erase(instanceId) > 0)
            changed = true;
        if (HasMarkOfHydrossAt100Percent(bot))
        {
            if (hydrossChangeToNaturePhaseTimer.try_emplace(instanceId, now).second)
                changed = true;
        }
    }
    else
    {
        if (hydrossNatureDpsWaitTimer.try_emplace(instanceId, now).second)
            changed = true;
        if (hydrossFrostDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
        if (hydrossChangeToNaturePhaseTimer.erase(instanceId) > 0)
            changed = true;
        if (HasMarkOfCorruptionAt100Percent(bot))
        {
            if (hydrossChangeToFrostPhaseTimer.try_emplace(instanceId, now).second)
                changed = true;
        }
    }

    return changed;
}

// The Lurker Below

// Run around behind Lurker during Spout
bool TheLurkerBelowRunAroundBehindBossAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    float radius = frand(20.0f, 21.0f);
    float botAngle = std::atan2(
        bot->GetPositionY() - lurker->GetPositionY(), bot->GetPositionX() - lurker->GetPositionX());
    float relativeAngle = Position::NormalizeOrientation(botAngle - lurker->GetOrientation());
    constexpr float safeArc = M_PI / 2.0f;

    if (std::fabs(Position::NormalizeOrientation(relativeAngle - M_PI)) > safeArc / 2.0f)
    {
        float tangentAngle = botAngle + (relativeAngle > M_PI ? -0.1f : 0.1f);
        float moveX = lurker->GetPositionX() + radius * std::cos(tangentAngle);
        float moveY = lurker->GetPositionY() + radius * std::sin(tangentAngle);
        botAI->Reset();
        return MoveTo(SSC_MAP_ID, moveX, moveY, lurker->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }
    else
    {
        float behindAngle = lurker->GetOrientation() + M_PI + frand(-0.5f, 0.5f) * safeArc;
        float targetX = lurker->GetPositionX() + radius * std::cos(behindAngle);
        float targetY = lurker->GetPositionY() + radius * std::sin(behindAngle);
        if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
        {
            botAI->Reset();
            return MoveTo(SSC_MAP_ID, targetX, targetY, lurker->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

bool TheLurkerBelowPositionMainTankAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    if (bot->GetTarget() != lurker->GetGUID())
        return Attack(lurker);

    const Position& position = LURKER_MAIN_TANK_POSITION;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 0.2f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Assign ranged positions within a 120-degree arc behind Lurker
bool TheLurkerBelowSpreadRangedInArcAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    std::vector<Player*> rangedMembers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRanged(member))
                continue;

            rangedMembers.push_back(member);
        }
    }

    if (rangedMembers.empty())
        return false;

    const ObjectGuid guid = bot->GetGUID();

    auto it = lurkerRangedPositions.find(guid);
    if (it == lurkerRangedPositions.end())
    {
        size_t count = rangedMembers.size();
        auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
        size_t botIndex = (findIt != rangedMembers.end()) ?
            std::distance(rangedMembers.begin(), findIt) : 0;

        constexpr float arcSpan = 2.0f * M_PI / 3.0f;
        constexpr float arcCenter = 2.262f;
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle = (count == 1) ? arcCenter :
            (arcStart + arcSpan * static_cast<float>(botIndex) / static_cast<float>(count - 1));
        float radius = 28.0f;

        float targetX = lurker->GetPositionX() + radius * std::sin(angle);
        float targetY = lurker->GetPositionY() + radius * std::cos(angle);

        lurkerRangedPositions.try_emplace(guid, Position(targetX, targetY, lurker->GetPositionZ()));
        it = lurkerRangedPositions.find(guid);
    }

    if (it == lurkerRangedPositions.end())
        return false;

    const Position& position = it->second;
    if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
    {
        return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                      position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// During the submerge phase, if there are >= 3 tanks in the raid,
// the first 3 will each pick up 1 Guardian
bool TheLurkerBelowTanksPickUpAddsAction::Execute(Event /*event*/)
{
    Player* mainTank = nullptr;
    Player* firstAssistTank = nullptr;
    Player* secondAssistTank = nullptr;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!mainTank && botAI->IsMainTank(member))
                mainTank = member;
            else if (!firstAssistTank && botAI->IsAssistTankOfIndex(member, 0, true))
                firstAssistTank = member;
            else if (!secondAssistTank && botAI->IsAssistTankOfIndex(member, 1, true))
                secondAssistTank = member;
        }
    }

    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return false;

    std::vector<Unit*> guardians;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (auto guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_COILFANG_GUARDIAN)
            guardians.push_back(unit);
    }

    if (guardians.size() < 3)
        return false;

    std::vector<Player*> tanks = { mainTank, firstAssistTank, secondAssistTank };
    std::vector<uint8> rtiIndices =
    {
        RtiTargetValue::starIndex,
        RtiTargetValue::circleIndex,
        RtiTargetValue::diamondIndex
    };
    std::vector<std::string> rtiNames = { "star", "circle", "diamond" };

    for (size_t i = 0; i < 3; ++i)
    {
        Player* tank = tanks[i];
        Unit* guardian = guardians[i];
        if (bot == tank)
        {
            MarkTargetWithIcon(bot, guardian, rtiIndices[i]);
            SetRtiTarget(botAI, rtiNames[i], guardian);
            if (bot->GetTarget() != guardian->GetGUID())
                return Attack(guardian);
        }
    }

    return false;
}

bool TheLurkerBelowManageSpoutTimerAction::Execute(Event /*event*/)
{
    Unit* lurker = AI_VALUE2(Unit*, "find target", "the lurker below");
    if (!lurker)
        return false;

    const uint32 instanceId = lurker->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    auto it = lurkerSpoutTimer.find(instanceId);
    if (it != lurkerSpoutTimer.end() && it->second <= now)
    {
        lurkerSpoutTimer.erase(it);
        it = lurkerSpoutTimer.end();
    }

    const time_t spoutCastTime = 20;
    if (IsLurkerCastingSpout(lurker) && it == lurkerSpoutTimer.end())
        lurkerSpoutTimer.try_emplace(instanceId, now + spoutCastTime);

    return false;
}

// Leotheras the Blind

bool LeotherasTheBlindTargetSpellbindersAction::Execute(Event /*event*/)
{
    if (Unit* spellbinder = GetFirstAliveUnitByEntry(botAI, NPC_GREYHEART_SPELLBINDER))
        MarkTargetWithSkull(bot, spellbinder);

    return false;
}

// Warlock tank action--see GetLeotherasDemonFormTank in RaidSSCHelpers.cpp
// Use tank strategy for Demon Form and DPS strategy for Human Form
bool LeotherasTheBlindDemonFormTankAttackBossAction::Execute(Event /*event*/)
{
    Unit* innerDemon = nullptr;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (creature && creature->GetEntry() == NPC_INNER_DEMON
            && creature->GetSummonerGUID() == bot->GetGUID())
        {
            innerDemon = creature;
            break;
        }
    }

    if (innerDemon)
        return false;

    if (Unit* leotherasDemon = GetActiveLeotherasDemon(botAI))
    {
        MarkTargetWithSquare(bot, leotherasDemon);
        SetRtiTarget(botAI, "square", leotherasDemon);

        if (botAI->CanCastSpell("searing pain", leotherasDemon))
            return botAI->CastSpell("searing pain", leotherasDemon);
    }

    return false;
}

// Stop melee tanks from attacking upon transformation so they don't take aggro
// Applies only if there is a Warlock tank present
bool LeotherasTheBlindMeleeTanksDontAttackDemonFormAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    botAI->Reset();
    return true;
}

// Intent is to keep enough distance from Leotheras and spread to prepare for Whirlwind
// And stay away from the Warlock tank to avoid Chaos Blasts
bool LeotherasTheBlindPositionRangedAction::Execute(Event /*event*/)
{
    constexpr float safeDistFromBoss = 15.0f;
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (leotherasHuman && bot->GetExactDist2d(leotherasHuman) < safeDistFromBoss &&
        leotherasHuman->GetVictim() != bot)
    {
        constexpr uint32 minInterval = 500;
        return FleePosition(leotherasHuman->GetPosition(), safeDistFromBoss, minInterval);
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (GetActiveLeotherasDemon(botAI))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            constexpr uint32 minInterval = 0;
            if (GetLeotherasDemonFormTank(bot) == member)
            {
                constexpr float safeDistFromTank = 10.0f;
                if (bot->GetExactDist2d(member) < safeDistFromTank)
                    return FleePosition(member->GetPosition(), safeDistFromTank, minInterval);
            }
            else
            {
                constexpr float safeDistFromMember = 6.0f;
                if (bot->GetExactDist2d(member) < safeDistFromMember)
                    return FleePosition(member->GetPosition(), safeDistFromMember, minInterval);
            }
        }
    }

    return false;
}

bool LeotherasTheBlindRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    if (Unit* leotherasHuman = GetLeotherasHuman(botAI))
    {
        float currentDistance = bot->GetExactDist2d(leotherasHuman);
        constexpr float safeDistance = 25.0f;
        if (currentDistance < safeDistance)
        {
            botAI->Reset();
            return MoveAway(leotherasHuman, safeDistance - currentDistance);
        }
    }

    return false;
}

// This method is likely unnecessary unless the player does not use a Warlock tank
// If a melee tank is used, other melee needs to run away after too many Chaos Blast stacks
bool LeotherasTheBlindMeleeDpsRunAwayFromBossAction::Execute(Event /*event*/)
{
    if (botAI->CanCastSpell("cloak of shadows", bot))
        return botAI->CastSpell("cloak of shadows", bot);

    Unit* leotheras = GetPhase2LeotherasDemon(botAI);
    if (!leotheras)
        return false;

    Unit* demonVictim = leotheras->GetVictim();
    if (!demonVictim)
        return false;

    float currentDistance = bot->GetExactDist2d(demonVictim);
    constexpr float safeDistance = 10.0f;
    if (currentDistance < safeDistance)
    {
        botAI->Reset();
        if (demonVictim != bot)
            return MoveAway(demonVictim, safeDistance - currentDistance);
    }

    return false;
}

// Hardcoded actions for healers and bear tanks to kill Inner Demons
bool LeotherasTheBlindDestroyInnerDemonAction::Execute(Event /*event*/)
{
    Unit* innerDemon = nullptr;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    for (auto const& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (creature && creature->GetEntry() == NPC_INNER_DEMON
            && creature->GetSummonerGUID() == bot->GetGUID())
        {
            innerDemon = creature;
            break;
        }
    }

    if (innerDemon)
    {
        if (botAI->IsTank(bot) && bot->getClass() == CLASS_DRUID)
            return HandleFeralTankStrategy(innerDemon);

        if (botAI->IsHeal(bot))
            return HandleHealerStrategy(innerDemon);

        // Roles without a strategy need to affirmatively attack their Inner Demons
        // Because DPS assist is disabled via multipliers
        if (bot->GetTarget() != innerDemon->GetGUID())
            return Attack(innerDemon);
    }

    return false;
}

// At 50% nerfed damage, bears have trouble killing their Inner Demons without a specific strategy
// Warrior and Paladin tanks have no trouble in my experience (Prot Warriors have high DPS, and
// Prot Paladins have an advantage in that Inner Demons are weak to Holy)
bool LeotherasTheBlindDestroyInnerDemonAction::HandleFeralTankStrategy(Unit* innerDemon)
{
    if (bot->HasAura(SPELL_DIRE_BEAR_FORM))
        bot->RemoveAura(SPELL_DIRE_BEAR_FORM);

    if (bot->HasAura(SPELL_BEAR_FORM))
        bot->RemoveAura(SPELL_BEAR_FORM);

    bool casted = false;
    if (!bot->HasAura(SPELL_CAT_FORM) && botAI->CanCastSpell("cat form", bot))
    {
        if (botAI->CastSpell("cat form", bot))
            casted = true;
    }
    if (botAI->CanCastSpell("berserk", bot))
    {
        if (botAI->CastSpell("berserk", bot))
            casted = true;
    }
    if (bot->GetPower(POWER_ENERGY) < 30 && botAI->CanCastSpell("tiger's fury", bot))
    {
        if (botAI->CastSpell("tiger's fury", bot))
            casted = true;
    }
    if (bot->GetComboPoints() >= 4 && botAI->CanCastSpell("ferocious bite", innerDemon))
    {
        if (botAI->CastSpell("ferocious bite", innerDemon))
            casted = true;
    }
    if (bot->GetComboPoints() == 0 && innerDemon->GetHealthPct() > 25.0f &&
        botAI->CanCastSpell("rake", innerDemon))
    {
        if (botAI->CastSpell("rake", innerDemon))
            casted = true;
    }
    if (botAI->CanCastSpell("mangle (cat)", innerDemon))
    {
        if (botAI->CastSpell("mangle (cat)", innerDemon))
            casted = true;
    }

    return casted;
}

bool LeotherasTheBlindDestroyInnerDemonAction::HandleHealerStrategy(Unit* innerDemon)
{
    if (bot->getClass() == CLASS_DRUID)
    {
        if (bot->HasAura(SPELL_TREE_OF_LIFE))
            bot->RemoveAura(SPELL_TREE_OF_LIFE);

        bool casted = false;
        if (botAI->CanCastSpell("barkskin", bot))
        {
            if (botAI->CastSpell("barkskin", bot))
                casted = true;
        }
        if (botAI->CanCastSpell("wrath", innerDemon))
        {
            if (botAI->CastSpell("wrath", innerDemon))
                casted = true;
        }

        return casted;
    }
    else if (bot->getClass() == CLASS_PALADIN)
    {
        bool casted = false;
        if (botAI->CanCastSpell("avenging wrath", bot))
        {
            if (botAI->CastSpell("avenging wrath", bot))
                casted = true;
        }
        if (botAI->CanCastSpell("consecration", bot))
        {
            if (botAI->CastSpell("consecration", bot))
                casted = true;
        }
        if (botAI->CanCastSpell("exorcism", innerDemon))
        {
            if (botAI->CastSpell("exorcism", innerDemon))
                casted = true;
        }
        if (botAI->CanCastSpell("hammer of wrath", innerDemon))
        {
            if (botAI->CastSpell("hammer of wrath", innerDemon))
                casted = true;
        }
        if (botAI->CanCastSpell("holy shock", innerDemon))
        {
            if (botAI->CastSpell("holy shock", innerDemon))
                casted = true;
        }
        if (botAI->CanCastSpell("judgment of light", innerDemon))
        {
            if (botAI->CastSpell("judgment of light", innerDemon))
                casted = true;
        }

        return casted;
    }
    else if (bot->getClass() == CLASS_PRIEST)
    {
        if (botAI->CanCastSpell("smite", innerDemon))
            return botAI->CastSpell("smite", innerDemon);
    }
    else if (bot->getClass() == CLASS_SHAMAN)
    {
        bool casted = false;
        if (botAI->CanCastSpell("earth shock", innerDemon))
        {
            if (botAI->CastSpell("earth shock", innerDemon))
                casted = true;
        }
        if (botAI->CanCastSpell("chain lightning", innerDemon))
        {
            if (botAI->CastSpell("chain lightning", innerDemon))
                casted = true;
        }
        if (botAI->CanCastSpell("lightning bolt", innerDemon))
        {
            if (botAI->CastSpell("lightning bolt", innerDemon))
                casted = true;
        }

        return casted;
    }

    return false;
}

// Everybody except the Warlock tank should focus on Leotheras in Phase 3
bool LeotherasTheBlindFinalPhaseAssignDpsPriorityAction::Execute(Event /*event*/)
{
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    if (!leotherasHuman)
        return false;

    MarkTargetWithStar(bot, leotherasHuman);
    SetRtiTarget(botAI, "star", leotherasHuman);

    if (bot->GetTarget() != leotherasHuman->GetGUID())
        return Attack(leotherasHuman);

    Unit* leotherasDemon = GetPhase3LeotherasDemon(botAI);
    if (leotherasDemon)
    {
        if (leotherasHuman->GetVictim() == bot)
        {
            Unit* demonTarget = leotherasDemon->GetVictim();
            if (demonTarget && leotherasHuman->GetExactDist2d(demonTarget) < 20.0f)
            {
                float angle = atan2(bot->GetPositionY() - demonTarget->GetPositionY(),
                                    bot->GetPositionX() - demonTarget->GetPositionX());
                float targetX = bot->GetPositionX() + 25.0f * std::cos(angle);
                float targetY = bot->GetPositionY() + 25.0f * std::sin(angle);

                return MoveTo(SSC_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                              false, false, MovementPriority::MOVEMENT_FORCED, true, false);
            }
        }
    }

    return false;
}

// Misdirect to Warlock tank or to main tank if there is no Warlock tank
bool LeotherasTheBlindMisdirectBossToDemonFormTankAction::Execute(Event /*event*/)
{
    Unit* leotherasDemon = GetActiveLeotherasDemon(botAI);
    if (!leotherasDemon)
        return false;

    Player* demonFormTank = GetLeotherasDemonFormTank(bot);
    Player* targetTank = demonFormTank;

    if (!targetTank)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsMainTank(member))
                {
                    targetTank = member;
                    break;
                }
            }
        }
    }

    if (!targetTank)
        return false;

    if (botAI->CanCastSpell("misdirection", targetTank))
        return botAI->CastSpell("misdirection", targetTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", leotherasDemon))
        return botAI->CastSpell("steady shot", leotherasDemon);

    return false;
}

// This does not pause DPS after a Whirlwind, which is also an aggro wipe
bool LeotherasTheBlindManageDpsWaitTimersAction::Execute(Event /*event*/)
{
    Unit* leotheras = AI_VALUE2(Unit*, "find target", "leotheras the blind");
    if (!leotheras)
        return false;

    const uint32 instanceId = leotheras->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);

    bool changed = false;
    // Encounter start/reset: clear all timers
    if (leotheras->HasAura(SPELL_LEOTHERAS_BANISHED))
    {
        if (leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
        if (leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
        if (leotherasFinalPhaseDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
    }

    // Human Phase
    Unit* leotherasHuman = GetLeotherasHuman(botAI);
    Unit* leotherasPhase3Demon = GetPhase3LeotherasDemon(botAI);
    if (leotherasHuman && !leotherasPhase3Demon)
    {
        if (leotherasHumanFormDpsWaitTimer.try_emplace(instanceId, now).second)
            changed = true;
        if (leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
    }
    // Demon Phase
    else if (Unit* leotherasPhase2Demon = GetPhase2LeotherasDemon(botAI))
    {
        if (leotherasDemonFormDpsWaitTimer.try_emplace(instanceId, now).second)
            changed = true;
        if (leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
    }
    // Final Phase (<15% HP)
    else if (leotherasHuman && leotherasPhase3Demon)
    {
        if (leotherasFinalPhaseDpsWaitTimer.try_emplace(instanceId, now).second)
            changed = true;
        if (leotherasHumanFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
        if (leotherasDemonFormDpsWaitTimer.erase(instanceId) > 0)
            changed = true;
    }

    return changed;
}

// Fathom-Lord Karathress
// Note: 4 tanks are required for the full strategy, and having at least 2
// is crucial to separate Caribdis from the others

// Karathress is tanked near his starting position
bool FathomLordKarathressMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    MarkTargetWithTriangle(bot, karathress);
    SetRtiTarget(botAI, "triangle", karathress);

    if (bot->GetTarget() != karathress->GetGUID())
        return Attack(karathress);

    if (karathress->GetVictim() == bot && bot->IsWithinMeleeRange(karathress))
    {
        const Position& position = KARATHRESS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis is pulled far to the West in the corner
// Best to use a Warrior or Druid tank for interrupts
bool FathomLordKarathressFirstAssistTankPositionCaribdisAction::Execute(Event /*event*/)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    MarkTargetWithDiamond(bot, caribdis);
    SetRtiTarget(botAI, "diamond", caribdis);

    if (bot->GetTarget() != caribdis->GetGUID())
        return Attack(caribdis);

    if (caribdis->GetVictim() == bot)
    {
        const Position& position = CARIBDIS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Sharkkis is pulled North to the other side of the ramp
bool FathomLordKarathressSecondAssistTankPositionSharkkisAction::Execute(Event /*event*/)
{
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (!sharkkis)
        return false;

    MarkTargetWithStar(bot, sharkkis);
    SetRtiTarget(botAI, "star", sharkkis);

    if (bot->GetTarget() != sharkkis->GetGUID())
        return Attack(sharkkis);

    if (sharkkis->GetVictim() == bot && bot->IsWithinMeleeRange(sharkkis))
    {
        const Position& position = SHARKKIS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Tidalvess is pulled Northwest near the pillar
bool FathomLordKarathressThirdAssistTankPositionTidalvessAction::Execute(Event /*event*/)
{
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (!tidalvess)
        return false;

    MarkTargetWithCircle(bot, tidalvess);
    SetRtiTarget(botAI, "circle", tidalvess);

    if (bot->GetTarget() != tidalvess->GetGUID())
        return Attack(tidalvess);

    if (tidalvess->GetVictim() == bot && bot->IsWithinMeleeRange(tidalvess))
    {
        const Position& position = TIDALVESS_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 3.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                          false, true, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Caribdis's tank spot is far away so a dedicated healer is needed
// Use the assistant flag to select the healer
bool FathomLordKarathressPositionCaribdisTankHealerAction::Execute(Event /*event*/)
{
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (!caribdis)
        return false;

    const Position& position = CARIBDIS_HEALER_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 3.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                      false, true, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Misdirect priority: (1) Caribdis tank, (2) Tidalvess tank, (3) Sharkkis tank
bool FathomLordKarathressMisdirectBossesToTanksAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
            GET_PLAYERBOT_AI(member))
            hunters.push_back(member);

        if (hunters.size() >= 3)
            break;
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1)
        return false;

    Unit* bossTarget = nullptr;
    Player* tankTarget = nullptr;
    if (hunterIndex == 0)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 0, false))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 1)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 2, false))
            {
                tankTarget = member;
                break;
            }
        }
    }
    else if (hunterIndex == 2)
    {
        bossTarget = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsAssistTankOfIndex(member, 1, false))
            {
                tankTarget = member;
                break;
            }
        }
    }

    if (!bossTarget || !tankTarget)
        return false;

    if (botAI->CanCastSpell("misdirection", tankTarget))
        return botAI->CastSpell("misdirection", tankTarget);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", bossTarget))
        return botAI->CastSpell("steady shot", bossTarget);

    return false;
}

// Kill order is non-standard because bots handle Cyclones poorly and need more time
// to get her down than real players (standard is ranged DPS help with Sharkkis first)
bool FathomLordKarathressAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Spitfire Totems for melee dps
    Unit* totem = GetFirstAliveUnitByEntry(botAI, NPC_SPITFIRE_TOTEM);
    if (totem && botAI->IsMelee(bot) && botAI->IsDps(bot))
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
            return Attack(totem);

        // Direct movement order due to path between Sharkkis and totem sometimes being screwy
        if (!bot->IsWithinMeleeRange(totem))
        {
            return MoveTo(SSC_MAP_ID, totem->GetPositionX(), totem->GetPositionY(),
                          totem->GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        return false;
    }

    // Target priority 2: Tidalvess for all dps
    Unit* tidalvess = AI_VALUE2(Unit*, "find target", "fathom-guard tidalvess");
    if (tidalvess)
    {
        MarkTargetWithCircle(bot, tidalvess);
        SetRtiTarget(botAI, "circle", tidalvess);

        if (bot->GetTarget() != tidalvess->GetGUID())
            return Attack(tidalvess);

        return false;
    }

    // Target priority 3: Caribdis for ranged dps
    Unit* caribdis = AI_VALUE2(Unit*, "find target", "fathom-guard caribdis");
    if (botAI->IsRangedDps(bot) && caribdis)
    {
        MarkTargetWithDiamond(bot, caribdis);
        SetRtiTarget(botAI, "diamond", caribdis);

        const Position& position = CARIBDIS_RANGED_DPS_POSITION;
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveInside(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                              position.GetPositionZ(), 8.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        if (bot->GetTarget() != caribdis->GetGUID())
            return Attack(caribdis);

        return false;
    }

    // Target priority 4: Sharkkis for melee dps and, after Caribdis is down, ranged dps also
    Unit* sharkkis = AI_VALUE2(Unit*, "find target", "fathom-guard sharkkis");
    if (sharkkis)
    {
        MarkTargetWithStar(bot, sharkkis);
        SetRtiTarget(botAI, "star", sharkkis);

        if (bot->GetTarget() != sharkkis->GetGUID())
            return Attack(sharkkis);

        return false;
    }

    // Target priority 5: Sharkkis pets for all dps
    Unit* fathomSporebat = AI_VALUE2(Unit*, "find target", "fathom sporebat");
    if (fathomSporebat && botAI->IsMelee(bot))
    {
        MarkTargetWithCross(bot, fathomSporebat);
        SetRtiTarget(botAI, "cross", fathomSporebat);

        if (bot->GetTarget() != fathomSporebat->GetGUID())
            return Attack(fathomSporebat);

        return false;
    }

    Unit* fathomLurker = AI_VALUE2(Unit*, "find target", "fathom lurker");
    if (fathomLurker && botAI->IsMelee(bot))
    {
        MarkTargetWithSquare(bot, fathomLurker);
        SetRtiTarget(botAI, "square", fathomLurker);

        if (bot->GetTarget() != fathomLurker->GetGUID())
            return Attack(fathomLurker);

        return false;
    }

    // Target priority 6: Karathress for all dps
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (karathress)
    {
        MarkTargetWithTriangle(bot, karathress);
        SetRtiTarget(botAI, "triangle", karathress);

        if (bot->GetTarget() != karathress->GetGUID())
            return Attack(karathress);
    }

    return false;
}

bool FathomLordKarathressManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* karathress = AI_VALUE2(Unit*, "find target", "fathom-lord karathress");
    if (!karathress)
        return false;

    karathressDpsWaitTimer.try_emplace(
        karathress->GetMap()->GetInstanceId(), std::time(nullptr));

    return false;
}

// Morogrim Tidewalker

bool MorogrimTidewalkerMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", tidewalker))
        return botAI->CastSpell("steady shot", tidewalker);

    return false;
}

// Separate tanking positions are used for phase 1 and phase 2 to address the
// Water Globule mechanic in phase 2
bool MorogrimTidewalkerMoveBossToTankPositionAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    if (bot->GetTarget() != tidewalker->GetGUID())
        return Attack(tidewalker);

    if (tidewalker->GetVictim() == bot && bot->IsWithinMeleeRange(tidewalker))
    {
        if (tidewalker->GetHealthPct() > 26.0f)
            return MoveToPhase1TankPosition(tidewalker);
        else
            return MoveToPhase2TankPosition(tidewalker);
    }

    return false;
}

// Phase 1: tank position is up against the Northeast pillar
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase1TankPosition(Unit* tidewalker)
{
    const Position& phase1 = TIDEWALKER_PHASE_1_TANK_POSITION;
    float distToPhase1 = bot->GetExactDist2d(phase1.GetPositionX(), phase1.GetPositionY());
    if (distToPhase1 > 1.0f)
    {
        float dX = phase1.GetPositionX() - bot->GetPositionX();
        float dY = phase1.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(5.0f, distToPhase1);
        float moveX = bot->GetPositionX() + (dX / distToPhase1) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPhase1) * moveDist;

        return MoveTo(SSC_MAP_ID, moveX, moveY, phase1.GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
    }

    return false;
}

// Phase 2: move in two steps to get around the pillar and back up into the Northeast corner
bool MorogrimTidewalkerMoveBossToTankPositionAction::MoveToPhase2TankPosition(Unit* tidewalker)
{
    const Position& phase2 = TIDEWALKER_PHASE_2_TANK_POSITION;
    const Position& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerTankStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerTankStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
        else
            tidewalkerTankStep.try_emplace(bot->GetGUID(), 1);
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(5.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Ranged stack behind the boss in the Northeast corner in phase 2
// No corresponding method for melee since they will do so anyway
bool MorogrimTidewalkerPhase2RepositionRangedAction::Execute(Event /*event*/)
{
    Unit* tidewalker = AI_VALUE2(Unit*, "find target", "morogrim tidewalker");
    if (!tidewalker)
        return false;

    const Position& phase2 = TIDEWALKER_PHASE_2_RANGED_POSITION;
    const Position& transition = TIDEWALKER_PHASE_TRANSITION_WAYPOINT;

    auto itStep = tidewalkerRangedStep.find(bot->GetGUID());
    uint8 step = (itStep != tidewalkerRangedStep.end()) ? itStep->second : 0;

    if (step == 0)
    {
        float distToTransition =
            bot->GetExactDist2d(transition.GetPositionX(), transition.GetPositionY());

        if (distToTransition > 2.0f)
        {
            float dX = transition.GetPositionX() - bot->GetPositionX();
            float dY = transition.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToTransition);
            float moveX = bot->GetPositionX() + (dX / distToTransition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToTransition) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, transition.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        else
        {
            tidewalkerRangedStep.try_emplace(bot->GetGUID(), 1);
            step = 1;
        }
    }

    if (step == 1)
    {
        float distToPhase2 =
            bot->GetExactDist2d(phase2.GetPositionX(), phase2.GetPositionY());

        if (distToPhase2 > 1.0f)
        {
            float dX = phase2.GetPositionX() - bot->GetPositionX();
            float dY = phase2.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPhase2);
            float moveX = bot->GetPositionX() + (dX / distToPhase2) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPhase2) * moveDist;

            return MoveTo(SSC_MAP_ID, moveX, moveY, phase2.GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Lady Vashj <Coilfang Matron>

bool LadyVashjMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    if (bot->GetTarget() != vashj->GetGUID())
        return Attack(vashj);

    if (vashj->GetVictim() == bot && bot->IsWithinMeleeRange(vashj))
    {
        // Phase 1: Position Vashj in the center of the platform
        if (IsLadyVashjInPhase1(botAI))
        {
            const Position& position = VASHJ_PLATFORM_CENTER_POSITION;
            float distToPosition =
                bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

            if (distToPosition > 2.0f)
            {
                float dX = position.GetPositionX() - bot->GetPositionX();
                float dY = position.GetPositionY() - bot->GetPositionY();
                float moveDist = std::min(5.0f, distToPosition);
                float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
                float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

                return MoveTo(SSC_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false,
                              false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
            }
        }
        // Phase 3: No fixed position, but move Vashj away from Enchanted Elementals
        else if (IsLadyVashjInPhase3(botAI))
        {
            Unit* enchanted = AI_VALUE2(Unit*, "find target", "enchanted elemental");
            if (enchanted)
            {
                float currentDistance = bot->GetExactDist2d(enchanted);
                constexpr float safeDistance = 10.0f;
                if (currentDistance < safeDistance)
                    return MoveAway(enchanted, safeDistance - currentDistance);
            }
        }
    }

    return false;
}

// Semicircle around center of the room (to allow escape paths by Static Charged bots)
bool LadyVashjPhase1SpreadRangedInArcAction::Execute(Event /*event*/)
{
    std::vector<Player*> spreadMembers;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && GET_PLAYERBOT_AI(member))
            {
                if (botAI->IsRanged(member))
                    spreadMembers.push_back(member);
            }
        }
    }

    const ObjectGuid guid = bot->GetGUID();

    auto itPos = vashjRangedPositions.find(guid);
    auto itReached = hasReachedVashjRangedPosition.find(guid);
    if (itPos == vashjRangedPositions.end())
    {
        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        size_t botIndex = (it != spreadMembers.end()) ?
            std::distance(spreadMembers.begin(), it) : 0;
        size_t count = spreadMembers.size();
        if (count == 0)
            return false;

        const Position& center = VASHJ_PLATFORM_CENTER_POSITION;
        constexpr float minRadius = 20.0f;
        constexpr float maxRadius = 30.0f;

        constexpr float arcCenter = M_PI / 2.0f; // North
        constexpr float arcSpan = M_PI; // 180°
        constexpr float arcStart = arcCenter - arcSpan / 2.0f;

        float angle;
        if (count == 1)
            angle = arcCenter;
        else
            angle = arcStart + (static_cast<float>(botIndex) / (count - 1)) * arcSpan;

        float radius = frand(minRadius, maxRadius);
        float targetX = center.GetPositionX() + radius * std::cos(angle);
        float targetY = center.GetPositionY() + radius * std::sin(angle);

        auto res = vashjRangedPositions.try_emplace(guid, Position(targetX, targetY, center.GetPositionZ()));
        itPos = res.first;
        hasReachedVashjRangedPosition.try_emplace(guid, false);
        itReached = hasReachedVashjRangedPosition.find(guid);
    }

    if (itPos == vashjRangedPositions.end())
        return false;

    Position position = itPos->second;
    if (itReached == hasReachedVashjRangedPosition.end() || !(itReached->second))
    {
        if (bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY()) > 2.0f)
        {
            return MoveTo(SSC_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        if (itReached != hasReachedVashjRangedPosition.end())
            itReached->second = true;
    }

    return false;
}

// For absorbing Shock Burst
bool LadyVashjSetGroundingTotemInMainTankGroupAction::Execute(Event /*event*/)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (!mainTank)
        return false;

    if (bot->GetExactDist2d(mainTank) > 25.0f)
    {
        return MoveInside(SSC_MAP_ID, mainTank->GetPositionX(), mainTank->GetPositionY(),
                          mainTank->GetPositionZ(), 20.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    if (!botAI->HasStrategy("grounding", BotState::BOT_STATE_COMBAT))
        botAI->ChangeStrategy("+grounding", BotState::BOT_STATE_COMBAT);

    if (!bot->HasAura(SPELL_GROUNDING_TOTEM_EFFECT) &&
        botAI->CanCastSpell("grounding totem", bot))
        return botAI->CastSpell("grounding totem", bot);

    return false;
}

bool LadyVashjMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", vashj))
        return botAI->CastSpell("steady shot", vashj);

    return false;
}

bool LadyVashjStaticChargeMoveAwayFromGroupAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // If the main tank has Static Charge, other group members should move away
    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member) &&
            member->HasAura(SPELL_STATIC_CHARGE))
        {
            mainTank = member;
            break;
        }
    }

    if (mainTank && bot != mainTank)
    {
        float currentDistance = bot->GetExactDist2d(mainTank);
        constexpr float safeDistance = 11.0f;
        if (currentDistance < safeDistance)
            return MoveAway(mainTank, safeDistance - currentDistance);
    }

    // If any other bot has Static Charge, it should move away from other group members
    if (!botAI->IsMainTank(bot) && bot->HasAura(SPELL_STATIC_CHARGE))
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float currentDistance = bot->GetExactDist2d(member);
            constexpr float safeDistance = 11.0f;
            if (currentDistance < safeDistance)
                return MoveFromGroup(safeDistance);
        }
    }

    return false;
}

bool LadyVashjAssignPhase2AndPhase3DpsPriorityAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    const Position& center = VASHJ_PLATFORM_CENTER_POSITION;
    float platformZ = center.GetPositionZ();
    if (bot->GetPositionZ() - platformZ > 2.0f)
    {
        // This block is needed to prevent bots from floating into the air to attack sporebats
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->TeleportTo(SSC_MAP_ID, bot->GetPositionX(), bot->GetPositionY(),
                        platformZ, bot->GetOrientation());
        return true;
    }

    auto const& attackers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
    Unit* target = nullptr;
    Unit* enchanted = nullptr;
    Unit* elite = nullptr;
    Unit* strider = nullptr;
    Unit* sporebat = nullptr;

    // Search and attack radius are intended to keep bots from going down the stairs
    const float maxSearchRange =
        botAI->IsRanged(bot) ? 60.0f : 55.0f;
    const float maxPursueRange = maxSearchRange - 5.0f;

    for (auto guid : attackers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!IsValidLadyVashjCombatNpc(unit, botAI))
            continue;

        float distFromCenter = unit->GetExactDist2d(center.GetPositionX(), center.GetPositionY());
        if (IsLadyVashjInPhase2(botAI) && distFromCenter > maxSearchRange)
            continue;

        switch (unit->GetEntry())
        {
            case NPC_ENCHANTED_ELEMENTAL:
                if (!enchanted || vashj->GetExactDist2d(unit) < vashj->GetExactDist2d(enchanted))
                    enchanted = unit;
                break;

            case NPC_COILFANG_ELITE:
                if (!elite || unit->GetHealthPct() < elite->GetHealthPct())
                    elite = unit;
                break;

            case NPC_COILFANG_STRIDER:
                if (!strider || unit->GetHealthPct() < strider->GetHealthPct())
                    strider = unit;
                break;

            case NPC_TOXIC_SPOREBAT:
                if (!sporebat || bot->GetExactDist2d(unit) < bot->GetExactDist2d(sporebat))
                    sporebat = unit;
                break;

            case NPC_LADY_VASHJ:
                vashj = unit;
                break;

            default:
                break;
        }
    }

    std::vector<Unit*> targets;
    if (IsLadyVashjInPhase2(botAI))
    {
        if (botAI->IsRanged(bot))
        {
            // Hunters and Mages prioritize Enchanted Elementals,
            // while other ranged DPS prioritize Striders
            if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_MAGE)
                targets = { enchanted, strider, elite };
            else
                targets = { strider, elite, enchanted };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { enchanted, elite };
        else if (botAI->IsTank(bot))
        {
            if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsAssistTankOfIndex(bot, 0, true))
                targets = { strider, elite, enchanted };
            else
                targets = { elite, strider, enchanted };
        }
        else
            targets = { enchanted, elite, strider };
    }

    if (IsLadyVashjInPhase3(botAI))
    {
        if (botAI->IsTank(bot))
        {
            if (botAI->IsMainTank(bot))
            {
                MarkTargetWithDiamond(bot, vashj);
                SetRtiTarget(botAI, "diamond", vashj);
                targets = { vashj };
            }
            else if (botAI->IsAssistTankOfIndex(bot, 0, true))
            {
                if (botAI->HasCheat(BotCheatMask::raid))
                    targets = { strider, elite, enchanted, vashj };
            }
            else
                targets = { elite, strider, enchanted, vashj };
        }
        else if (botAI->IsRanged(bot))
        {
            // Hunters are assigned to kill Sporebats in Phase 3
            if (bot->getClass() == CLASS_HUNTER)
                targets = { sporebat, enchanted, strider, elite, vashj };
            else
                targets = { enchanted, strider, elite, vashj };
        }
        else if (botAI->IsMelee(bot) && botAI->IsDps(bot))
            targets = { enchanted, elite, vashj };
        else
            targets = { enchanted, elite, strider, vashj };
    }

    for (Unit* candidate : targets)
    {
        if (candidate && bot->GetExactDist2d(candidate) <= maxPursueRange)
        {
            target = candidate;
            break;
        }
    }

    Unit* currentTarget = context->GetValue<Unit*>("current target")->Get();

    if (currentTarget && !IsValidLadyVashjCombatNpc(currentTarget, botAI))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());
        currentTarget = nullptr;
    }

    if (target && currentTarget != target && bot->GetTarget() != target->GetGUID())
        return Attack(target);

    // If bots have wandered too far from the center, move them back
    if (bot->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) > 55.0f)
    {
        Player* designatedLooter = GetDesignatedCoreLooter(bot->GetGroup(), botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(bot->GetGroup(), botAI);
        // A bot will not move back to the middle if (1) there is a Tainted Elemental, and
        // (2) the bot is either the designated looter or the first core passer
        if (Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental"))
        {
            if ((designatedLooter && designatedLooter == bot) ||
                (firstCorePasser && firstCorePasser == bot))
                return false;
        }

        return MoveInside(SSC_MAP_ID, center.GetPositionX(), center.GetPositionY(),
                          center.GetPositionZ(), 40.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool LadyVashjMisdirectStriderToFirstAssistTankAction::Execute(Event /*event*/)
{
    // Striders are not tankable without a cheat to block Fear so there is
    // no point in misdirecting if raid cheats are not enabled
    if (!botAI->HasCheat(BotCheatMask::raid))
        return false;

    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    if (!strider)
        return false;

    Player* firstAssistTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsAssistTankOfIndex(member, 0, true))
            {
                firstAssistTank = member;
                break;
            }
        }
    }

    if (!firstAssistTank || strider->GetVictim() == firstAssistTank)
        return false;

    if (botAI->CanCastSpell("misdirection", firstAssistTank))
        return botAI->CastSpell("misdirection", firstAssistTank);

    if (bot->HasAura(SPELL_MISDIRECTION) && botAI->CanCastSpell("steady shot", strider))
        return botAI->CastSpell("steady shot", strider);

    return false;
}

bool LadyVashjTankAttackAndMoveAwayStriderAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Unit* strider = GetFirstAliveUnitByEntry(botAI, NPC_COILFANG_STRIDER);
    if (!strider)
        return false;

    // Raid cheat automatically applies Fear Ward to tanks to make Strider tankable
    // This simulates the real-life strategy where the Strider can be meleed by
    // players wearing an Ogre Suit (due to the extended combat reach)
    if (botAI->HasCheat(BotCheatMask::raid) && botAI->IsTank(bot))
    {
        if (!bot->HasAura(SPELL_FEAR_WARD))
            bot->AddAura(SPELL_FEAR_WARD, bot);

        if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
            bot->GetTarget() != strider->GetGUID())
            return Attack(strider);

        if (strider->GetVictim() == bot)
        {
            float currentDistance = bot->GetExactDist2d(vashj);
            constexpr float safeDistance = 28.0f;

            if (currentDistance < safeDistance)
                return MoveAway(vashj, safeDistance - currentDistance);
        }

        return false;
    }

    // Don't move away if raid cheats are enabled, or in any case if the bot is a tank
    if (!botAI->HasCheat(BotCheatMask::raid) || !botAI->IsTank(bot))
    {
        float currentDistance = bot->GetExactDist2d(strider);
        constexpr float safeDistance = 20.0f;
        if (currentDistance < safeDistance)
            return MoveAway(strider, safeDistance - currentDistance);
    }

    // Try to root/slow the Strider if it is not tankable (poor man's kiting strategy)
    if (!botAI->HasCheat(BotCheatMask::raid))
    {
        if (!strider->HasAura(SPELL_HEAVY_NETHERWEAVE_NET))
        {
            Item* net = bot->GetItemByEntry(ITEM_HEAVY_NETHERWEAVE_NET);
            if (net && botAI->HasItemInInventory(ITEM_HEAVY_NETHERWEAVE_NET) &&
                botAI->CanCastSpell("heavy netherweave net", strider))
                return botAI->CastSpell("heavy netherweave net", strider);
        }

        if (!botAI->HasAura("frost shock", strider) && bot->getClass() == CLASS_SHAMAN &&
            botAI->CanCastSpell("frost shock", strider))
            return botAI->CastSpell("frost shock", strider);

        if (!strider->HasAura(SPELL_CURSE_OF_EXHAUSTION) && bot->getClass() == CLASS_WARLOCK &&
            botAI->CanCastSpell("curse of exhaustion", strider))
            return botAI->CastSpell("curse of exhaustion", strider);

        if (!strider->HasAura(SPELL_SLOW) && bot->getClass() == CLASS_MAGE &&
            botAI->CanCastSpell("slow", strider))
            return botAI->CastSpell("slow", strider);
    }

    return false;
}

// If cheats are enabled, the first returned melee DPS bot will teleport to Tainted Elementals
// Such bot will recover HP and remove the Poison Bolt debuff while attacking the elemental
bool LadyVashjTeleportToTaintedElementalAction::Execute(Event /*event*/)
{
    Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental");
    if (!tainted)
        return false;

    if (bot->GetExactDist2d(tainted) >= 10.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->TeleportTo(SSC_MAP_ID, tainted->GetPositionX(), tainted->GetPositionY(),
                        tainted->GetPositionZ(), tainted->GetOrientation());
    }

    if (bot->GetTarget() != tainted->GetGUID())
    {
        MarkTargetWithStar(bot, tainted);
        SetRtiTarget(botAI, "star", tainted);
        return Attack(tainted);
    }

    if (bot->GetExactDist2d(tainted) < 5.0f)
    {
        bot->SetFullHealth();
        bot->RemoveAura(SPELL_POISON_BOLT);
    }

    return false;
}

bool LadyVashjLootTaintedCoreAction::Execute(Event)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    auto const& corpses = context->GetValue<GuidVector>("nearest corpses")->Get();
    const float maxLootRange = sPlayerbotAIConfig.lootDistance;

    for (auto const& guid : corpses)
    {
        LootObject loot(bot, guid);
        if (!loot.IsLootPossible(bot))
            continue;

        WorldObject* object = loot.GetWorldObject(bot);
        if (!object)
            continue;

        Creature* creature = object->ToCreature();
        if (!creature || creature->GetEntry() != NPC_TAINTED_ELEMENTAL || creature->IsAlive())
            continue;

        context->GetValue<LootObject>("loot target")->Set(loot);

        float dist = bot->GetDistance(object);
        if (dist > maxLootRange)
            return MoveTo(object, 2.0f, MovementPriority::MOVEMENT_FORCED);

        OpenLootAction open(botAI);
        if (!open.Execute(Event()))
            return false;

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                    return true;
            }
        }

        const ObjectGuid botGuid = bot->GetGUID();
        const ObjectGuid corpseGuid = guid;
        constexpr uint8 coreIndex = 0;

        botAI->AddTimedEvent([botGuid, corpseGuid, coreIndex, vashj]()
        {
            Player* receiver = botGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(botGuid);
            if (!receiver)
                return;

            if (Group* group = receiver->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                        return;
                }
            }

            receiver->SetLootGUID(corpseGuid);

            WorldPacket* packet = new WorldPacket(CMSG_AUTOSTORE_LOOT_ITEM, 1);
            *packet << coreIndex;
            receiver->GetSession()->QueuePacket(packet);

            const uint32 instanceId = vashj->GetMap()->GetInstanceId();
            const time_t now = std::time(nullptr);
            lastCoreInInventoryTime.insert_or_assign(instanceId, now);
        }, 600);

        return true;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
    if (!designatedLooter)
        return false;

    Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
    Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
    Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);
    Player* fourthCorePasser = GetFourthTaintedCorePasser(group, botAI);
    const uint32 instanceId = vashj->GetMap()->GetInstanceId();

    Unit* closestTrigger = nullptr;
    if (Unit* tainted = AI_VALUE2(Unit*, "find target", "tainted elemental"))
    {
        closestTrigger = GetNearestActiveShieldGeneratorTriggerByEntry(tainted);
        if (closestTrigger)
            nearestTriggerGuid.insert_or_assign(instanceId, closestTrigger->GetGUID());
    }

    auto itSnap = nearestTriggerGuid.find(instanceId);
    if (itSnap != nearestTriggerGuid.end() && !itSnap->second.IsEmpty())
    {
        Unit* snapUnit = botAI->GetUnit(itSnap->second);
        if (snapUnit)
            closestTrigger = snapUnit;
        else
            nearestTriggerGuid.erase(instanceId);
    }

    if (!firstCorePasser || !secondCorePasser || !thirdCorePasser ||
        !fourthCorePasser || !closestTrigger)
        return false;

    // Not gated behind CheatMask because the auto application of Fear Ward is necessary
    // to address an issue with bot movement, which is that bots cannot be rooted and
    // therefore will move when feared while holding the Tainted Core
    if (!bot->HasAura(SPELL_FEAR_WARD))
        bot->AddAura(SPELL_FEAR_WARD, bot);

    Item* item = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (!item || !botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Passer order: HealAssistantOfIndex 0, 1, 2, then RangedDpsAssistantOfIndex 0
        if (bot == firstCorePasser)
        {
            if (LineUpFirstCorePasser(designatedLooter, closestTrigger))
                return true;
        }
        else if (bot == secondCorePasser)
        {
            if (LineUpSecondCorePasser(firstCorePasser, closestTrigger))
                return true;
        }
        else if (bot == thirdCorePasser)
        {
            if (LineUpThirdCorePasser(designatedLooter, firstCorePasser,
                                      secondCorePasser, closestTrigger))
                return true;
        }
        else if (bot == fourthCorePasser)
        {
            if (LineUpFourthCorePasser(firstCorePasser, secondCorePasser,
                                       thirdCorePasser, closestTrigger))
                return true;
        }
    }
    else if (item && botAI->HasItemInInventory(ITEM_TAINTED_CORE))
    {
        // Designated core looter logic
        // Applicable only if cheat mode is on and thus looter is a bot
        if (bot == designatedLooter)
        {
            if (IsFirstCorePasserInIntendedPosition(
                designatedLooter, firstCorePasser, closestTrigger))
            {
                const time_t now = std::time(nullptr);
                auto it = lastImbueAttempt.find(instanceId);
                if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                {
                    lastImbueAttempt.insert_or_assign(instanceId, now);
                    lastCoreInInventoryTime.insert_or_assign(instanceId, now);
                    botAI->ImbueItem(item, firstCorePasser);
                    intendedLineup.erase(bot->GetGUID());
                    ScheduleTransferCoreAfterImbue(botAI, bot, firstCorePasser);
                    return true;
                }
            }
        }
        // First core passer: receive core from looter at the top of the stairs,
        // pass to second core passer
        else if (bot == firstCorePasser)
        {
            const time_t now = std::time(nullptr);
            auto it = lastImbueAttempt.find(instanceId);
            if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
            {
                lastImbueAttempt.insert_or_assign(instanceId, now);
                lastCoreInInventoryTime.insert_or_assign(instanceId, now);
                botAI->ImbueItem(item, secondCorePasser);
                intendedLineup.erase(bot->GetGUID());
                ScheduleTransferCoreAfterImbue(botAI, bot, secondCorePasser);
                return true;
            }
        }
        // Second core passer: if closest usable generator is within passing distance
        // of the first passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == secondCorePasser)
        {
            if (!UseCoreOnNearestGenerator(instanceId))
            {
                if (IsThirdCorePasserInIntendedPosition(
                    secondCorePasser, thirdCorePasser, closestTrigger))
                {
                    const time_t now = std::time(nullptr);
                    auto it = lastImbueAttempt.find(instanceId);
                    if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                    {
                        lastImbueAttempt.insert_or_assign(instanceId, now);
                        lastCoreInInventoryTime.insert_or_assign(instanceId, now);
                        botAI->ImbueItem(item, thirdCorePasser);
                        intendedLineup.erase(bot->GetGUID());
                        ScheduleTransferCoreAfterImbue(botAI, bot, thirdCorePasser);
                        return true;
                    }
                }
            }
        }
        // Third core passer: if closest usable generator is within passing distance
        // of the second passer, move to the generator; otherwise, move as close as
        // possible to the generator while staying in passing range
        else if (bot == thirdCorePasser)
        {
            if (!UseCoreOnNearestGenerator(instanceId))
            {
                if (IsFourthCorePasserInIntendedPosition(
                    thirdCorePasser, fourthCorePasser, closestTrigger))
                {
                    const time_t now = std::time(nullptr);
                    auto it = lastImbueAttempt.find(instanceId);
                    if (it == lastImbueAttempt.end() || (now - it->second) >= 2)
                    {
                        lastImbueAttempt.insert_or_assign(instanceId, now);
                        lastCoreInInventoryTime.insert_or_assign(instanceId, now);
                        botAI->ImbueItem(item, fourthCorePasser);
                        intendedLineup.erase(bot->GetGUID());
                        ScheduleTransferCoreAfterImbue(botAI, bot, fourthCorePasser);
                        return true;
                    }
                }
            }
        }
        // Fourth core passer: the fourth passer is rarely needed and no more than
        // four ever should be, so it should use the Core on the nearest generator
        else if (bot == fourthCorePasser)
            UseCoreOnNearestGenerator(instanceId);
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFirstCorePasser(
    Player* designatedLooter, Unit* closestTrigger)
{
    const float centerX = VASHJ_PLATFORM_CENTER_POSITION.GetPositionX();
    const float centerY = VASHJ_PLATFORM_CENTER_POSITION.GetPositionY();
    constexpr float radius = 57.5f;

    float mx = designatedLooter->GetPositionX();
    float my = designatedLooter->GetPositionY();
    float angle = atan2(my - centerY, mx - centerX);

    float targetX = centerX + radius * std::cos(angle);
    float targetY = centerY + radius * std::sin(angle);
    constexpr float targetZ = 41.097f;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, targetZ));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, targetX, targetY, targetZ, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpSecondCorePasser(
    Player* firstCorePasser, Unit* closestTrigger)
{
    float fx = firstCorePasser->GetPositionX();
    float fy = firstCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - fx;
    float dy = closestTrigger->GetPositionY() - fy;
    float distToTrigger = firstCorePasser->GetExactDist2d(closestTrigger);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    // Target is on a line between firstCorePasser and closestTrigger
    float targetX, targetY, targetZ;
    // If firstCorePasser is within thresholdDist of closestTrigger,
    // go to nearTriggerDist short of closestTrigger
    constexpr float thresholdDist = 40.0f;
    constexpr float nearTriggerDist = 1.5f;
    // If firstCorePasser is not thresholdDist yards from closestTrigger,
    // go to farDistance from firstCorePasser
    constexpr float farDistance = 38.0f;

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = fx + dx * moveDist;
        targetY = fy + dy * moveDist;
    }
    else
    {
        targetX = fx + dx * farDistance;
        targetY = fy + dy * farDistance;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, VASHJ_PLATFORM_Z));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, targetX, targetY, VASHJ_PLATFORM_Z, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool LadyVashjPassTheTaintedCoreAction::LineUpThirdCorePasser(
    Player* designatedLooter, Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    // Wait to move until it is clear that a third passer is needed
    bool needThird =
        (IsFirstCorePasserInIntendedPosition(designatedLooter, firstCorePasser, closestTrigger) &&
         firstCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needThird)
        return false;

    float sx = secondCorePasser->GetPositionX();
    float sy = secondCorePasser->GetPositionY();

    float dx = closestTrigger->GetPositionX() - sx;
    float dy = closestTrigger->GetPositionY() - sy;
    float distToTrigger = secondCorePasser->GetExactDist2d(closestTrigger);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    float targetX, targetY, targetZ;
    constexpr float thresholdDist = 40.0f;
    constexpr float nearTriggerDist = 1.5f;
    constexpr float farDistance = 38.0f;

    if (distToTrigger <= thresholdDist)
    {
        float moveDist = std::max(distToTrigger - nearTriggerDist, 0.0f);
        targetX = sx + dx * moveDist;
        targetY = sy + dy * moveDist;
    }
    else
    {
        targetX = sx + dx * farDistance;
        targetY = sy + dy * farDistance;
    }

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, VASHJ_PLATFORM_Z));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, targetX, targetY, VASHJ_PLATFORM_Z, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::LineUpFourthCorePasser(
    Player* firstCorePasser, Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger)
{
    // Wait to move until it is clear that a fourth passer is needed
    bool needFourth =
        (IsSecondCorePasserInIntendedPosition(firstCorePasser, secondCorePasser, closestTrigger) &&
         secondCorePasser->GetExactDist2d(closestTrigger) > 42.0f) ||
        (IsThirdCorePasserInIntendedPosition(secondCorePasser, thirdCorePasser, closestTrigger) &&
         thirdCorePasser->GetExactDist2d(closestTrigger) > 4.0f);

    if (!needFourth)
        return false;

    float sx = thirdCorePasser->GetPositionX();
    float sy = thirdCorePasser->GetPositionY();

    float tx = closestTrigger->GetPositionX();
    float ty = closestTrigger->GetPositionY();

    float dx = tx - sx;
    float dy = ty - sy;
    float distToTrigger = thirdCorePasser->GetExactDist2d(closestTrigger);

    if (distToTrigger == 0.0f)
        return false;

    dx /= distToTrigger; dy /= distToTrigger;

    constexpr float nearTriggerDist = 1.5f;
    float targetX = tx - dx * nearTriggerDist;
    float targetY = ty - dy * nearTriggerDist;

    intendedLineup.insert_or_assign(bot->GetGUID(), Position(targetX, targetY, VASHJ_PLATFORM_Z));

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(SSC_MAP_ID, targetX, targetY, VASHJ_PLATFORM_Z, false, false, false, true,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

// The next four functions check if the respective passer is <= 2 yards of their intended
// position and are used to determine when the prior bot in the chain can pass the core
bool LadyVashjPassTheTaintedCoreAction::IsFirstCorePasserInIntendedPosition(
    Player* designatedLooter, Player* firstCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(firstCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = firstCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsSecondCorePasserInIntendedPosition(
    Player* firstCorePasser, Player* secondCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(secondCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = secondCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsThirdCorePasserInIntendedPosition(
    Player* secondCorePasser, Player* thirdCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(thirdCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = thirdCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                       itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

bool LadyVashjPassTheTaintedCoreAction::IsFourthCorePasserInIntendedPosition(
    Player* thirdCorePasser, Player* fourthCorePasser, Unit* closestTrigger)
{
    auto itSnap = intendedLineup.find(fourthCorePasser->GetGUID());
    if (itSnap != intendedLineup.end())
    {
        float dist2d = fourthCorePasser->GetExactDist2d(itSnap->second.GetPositionX(),
                                                        itSnap->second.GetPositionY());
        return dist2d <= 2.0f;
    }

    return false;
}

// ImbueItem() is inconsistent in causing the receiver bot to receive the core and the giver
// bot to remove the core, so ScheduleTransferCoreAfterImbue() creates the core on the receiver
// and removes it from the giver, with ImbueItem() called primarily for the throwing animation
void LadyVashjPassTheTaintedCoreAction::ScheduleTransferCoreAfterImbue(
    PlayerbotAI* botAI, Player* giver, Player* receiver)
{
    if (!receiver || !giver)
        return;

    constexpr uint32 delayMs = 1500;
    const ObjectGuid receiverGuid = receiver->GetGUID();
    const ObjectGuid giverGuid = giver->GetGUID();

    botAI->AddTimedEvent([receiverGuid, giverGuid]()
    {
        Player* receiverPlayer =
            receiverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(receiverGuid);
        Player* giverPlayer =
            giverGuid.IsEmpty() ? nullptr : ObjectAccessor::FindPlayer(giverGuid);

        if (!receiverPlayer)
            return;

        if (!receiverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
        {
            ItemPosCountVec dest;
            uint32 count = 1;
            int canStore =
                receiverPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_TAINTED_CORE, count);

            if (canStore == EQUIP_ERR_OK)
            {
                receiverPlayer->StoreNewItem(dest, ITEM_TAINTED_CORE, true,
                    Item::GenerateItemRandomPropertyId(ITEM_TAINTED_CORE));
            }
        }

        if (giverPlayer)
        {
            Item* item = giverPlayer->GetItemByEntry(ITEM_TAINTED_CORE);
            if (item && giverPlayer->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                giverPlayer->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
        }
    }, delayMs);
}

bool LadyVashjPassTheTaintedCoreAction::UseCoreOnNearestGenerator(const uint32 instanceId)
{
    auto const& generators =
        GetAllGeneratorInfosByDbGuids(bot->GetMap(), SHIELD_GENERATOR_DB_GUIDS);
    const GeneratorInfo* nearestGen = GetNearestGeneratorToBot(bot, generators);
    if (!nearestGen)
        return false;

    GameObject* generator = botAI->GetGameObject(nearestGen->guid);
    if (!generator)
        return false;

    if (bot->GetExactDist2d(generator) > 4.5f)
        return false;

    Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE);
    if (!core)
        return false;

    if (bot->CanUseItem(core) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    const uint8 bagIndex = core->GetBagSlot();
    const uint8 slot = core->GetSlot();
    constexpr uint8 cast_count = 0;
    uint32 spellId = 0;

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (core->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = core->GetTemplate()->Spells[i].SpellId;
            break;
        }
    }

    const ObjectGuid item_guid = core->GetGUID();
    constexpr uint32 glyphIndex = 0;
    constexpr uint8 castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex;
    packet << slot;
    packet << cast_count;
    packet << spellId;
    packet << item_guid;
    packet << glyphIndex;
    packet << castFlags;
    packet << (uint32)TARGET_FLAG_GAMEOBJECT;
    packet << generator->GetGUID().WriteAsPacked();

    bot->GetSession()->HandleUseItemOpcode(packet);
    nearestTriggerGuid.erase(instanceId);
    lastImbueAttempt.erase(instanceId);
    lastCoreInInventoryTime.erase(instanceId);
    return true;
}

// Fallback for residual cores to be destroyed in Phase 3 in case
// ScheduleTransferCoreAfterImbue() fails to remove the core from the giver
bool LadyVashjDestroyTaintedCoreAction::Execute(Event /*event*/)
{
    if (Item* core = bot->GetItemByEntry(ITEM_TAINTED_CORE))
    {
        bot->DestroyItem(core->GetBagSlot(), core->GetSlot(), true);
        return true;
    }

    return false;
}

// This needs to be separate from the general map erasing logic because
// Bots may end up out of combat during the Vashj encounter
bool LadyVashjEraseCorePassingTrackersAction::Execute(Event /*event*/)
{
    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    if (!vashj)
        return false;

    const uint32 instanceId = vashj->GetMap()->GetInstanceId();

    bool erased = false;
    if (nearestTriggerGuid.erase(instanceId) > 0)
        erased = true;
    if (lastImbueAttempt.erase(instanceId) > 0)
        erased = true;
    if (lastCoreInInventoryTime.erase(instanceId) > 0)
        erased = true;
    if (intendedLineup.erase(bot->GetGUID()) > 0)
        erased = true;

    return erased;
}

// The standard "avoid aoe" strategy does work for Toxic Spores, but this method
// provides more buffer distance and limits the area in which bots can move
// so that they do not go down the stairs
bool LadyVashjAvoidToxicSporesAction::Execute(Event /*event*/)
{
    auto const& spores = GetAllSporeDropTriggers(botAI, bot);
    if (spores.empty())
        return false;

    constexpr float hazardRadius = 7.0f;
    bool inDanger = false;
    for (Unit* spore : spores)
    {
        if (bot->GetExactDist2d(spore) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& vashjCenter = VASHJ_PLATFORM_CENTER_POSITION;
    constexpr float maxRadius = 60.0f;

    Position safestPos = FindSafestNearbyPosition(spores, vashjCenter, maxRadius, hazardRadius);

    Unit* vashj = AI_VALUE2(Unit*, "find target", "lady vashj");
    bool backwards = (vashj && vashj->GetVictim() == bot);
    return MoveTo(SSC_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  safestPos.GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, backwards);
}

Position LadyVashjAvoidToxicSporesAction::FindSafestNearbyPosition(
    const std::vector<Unit*>& spores, const Position& vashjCenter,
    float maxRadius, float hazardRadius)
{
    constexpr float searchStep = M_PI / 8.0f;
    constexpr float minDistance = 2.0f;
    constexpr float maxDistance = 40.0f;
    constexpr float distanceStep = 1.0f;

    Position bestPos;
    float minMoveDistance = std::numeric_limits<float>::max();
    bool foundSafe = false;

    for (float distance = minDistance;
         distance <= maxDistance; distance += distanceStep)
    {
        for (float angle = 0.0f; angle < 2 * M_PI; angle += searchStep)
        {
            float x = bot->GetPositionX() + distance * std::cos(angle);
            float y = bot->GetPositionY() + distance * std::sin(angle);
            float z = bot->GetPositionZ();

            if (vashjCenter.GetExactDist2d(x, y) > maxRadius)
                continue;

            bool isSafe = true;
            for (Unit* spore : spores)
            {
                if (spore->GetExactDist2d(x, y) < hazardRadius)
                {
                    isSafe = false;
                    break;
                }
            }

            if (!isSafe)
                continue;

            Position testPos(x, y, z);

            bool pathSafe =
                IsPathSafeFromSpores(bot->GetPosition(), testPos, spores, hazardRadius);
            if (pathSafe || !foundSafe)
            {
                float moveDistance = bot->GetExactDist2d(x, y);

                if (pathSafe && (!foundSafe || moveDistance < minMoveDistance))
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                    foundSafe = true;
                }
                else if (!foundSafe && moveDistance < minMoveDistance)
                {
                    bestPos = testPos;
                    minMoveDistance = moveDistance;
                }
            }
        }

        if (foundSafe)
            break;
    }

    return bestPos;
}

bool LadyVashjAvoidToxicSporesAction::IsPathSafeFromSpores(const Position& start,
    const Position& end, const std::vector<Unit*>& spores, float hazardRadius)
{
    constexpr uint8 numChecks = 10;
    float dx = end.GetPositionX() - start.GetPositionX();
    float dy = end.GetPositionY() - start.GetPositionY();

    for (uint8 i = 1; i <= numChecks; ++i)
    {
        float ratio = static_cast<float>(i) / numChecks;
        float checkX = start.GetPositionX() + dx * ratio;
        float checkY = start.GetPositionY() + dy * ratio;

        for (Unit* spore : spores)
        {
            float distToSpore = spore->GetExactDist2d(checkX, checkY);
            if (distToSpore < hazardRadius)
                return false;
        }
    }

    return true;
}

// When Toxic Sporebats spit poison, they summon "Spore Drop Trigger" NPCs
// that create the toxic pools
std::vector<Unit*> LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(
    PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> sporeDropTriggers;
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        constexpr float maxSearchRadius = 40.0f;
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->GetEntry() == NPC_SPORE_DROP_TRIGGER &&
            bot->GetExactDist2d(unit) < maxSearchRadius)
            sporeDropTriggers.push_back(unit);
    }

    return sporeDropTriggers;
}

bool LadyVashjUseFreeActionAbilitiesAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    auto const& spores =
        LadyVashjAvoidToxicSporesAction::GetAllSporeDropTriggers(botAI, bot);
    constexpr float toxicSporeRadius = 6.0f;

    // If Rogues are Entangled and either have Static Charge or
    // are near a spore, use Cloak of Shadows
    if (bot->getClass() == CLASS_ROGUE && bot->HasAura(SPELL_ENTANGLE))
    {
        bool nearSpore = false;
        for (Unit* spore : spores)
        {
            if (bot->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearSpore = true;
                break;
            }
        }
        if (bot->HasAura(SPELL_STATIC_CHARGE) || nearSpore)
        {
            if (botAI->CanCastSpell("cloak of shadows", bot))
                return botAI->CastSpell("cloak of shadows", bot);
        }
    }

    // The remainder of the logic is for Paladins to use Hand of Freedom
    Player* mainTankToxic = nullptr;
    Player* anyToxic = nullptr;
    Player* mainTankStatic = nullptr;
    Player* anyStatic = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->HasAura(SPELL_ENTANGLE) ||
            !botAI->IsMelee(member))
            continue;

        bool nearToxicSpore = false;
        for (Unit* spore : spores)
        {
            if (member->GetExactDist2d(spore) < toxicSporeRadius)
            {
                nearToxicSpore = true;
                break;
            }
        }

        if (nearToxicSpore)
        {
            if (botAI->IsMainTank(member))
                mainTankToxic = member;

            if (!anyToxic)
                anyToxic = member;
        }

        if (member->HasAura(SPELL_STATIC_CHARGE))
        {
            if (botAI->IsMainTank(member))
                mainTankStatic = member;

            if (!anyStatic)
                anyStatic = member;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        // Priority 1: Entangled in Toxic Spores (prefer main tank)
        Player* toxicTarget = mainTankToxic ? mainTankToxic : anyToxic;
        if (toxicTarget)
        {
            if (botAI->CanCastSpell("hand of freedom", toxicTarget))
                return botAI->CastSpell("hand of freedom", toxicTarget);
        }

        // Priority 2: Entangled with Static Charge (prefer main tank)
        Player* staticTarget = mainTankStatic ? mainTankStatic : anyStatic;
        if (staticTarget)
        {
            if (botAI->CanCastSpell("hand of freedom", staticTarget))
                return botAI->CastSpell("hand of freedom", staticTarget);
        }
    }

    return false;
}
