/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Gormok.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <vector>

#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "PaladinHelper.h"
#include "Playerbots.h"
#include "RaidTankDefensive.h"
#include "ToCData.h"
#include "ToCHelpers_Gormok.h"
#include "ToCHelpers_Icehowl.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "Unit.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
// A carrier aims this far inside the melee range, since it stops short and he drifts with the drag
constexpr float CARRIER_AIM_INSET = 3.0f;
// A latched carrier spot still counts while it sits this far inside the range
constexpr float CARRIER_KEEP_INSET = 1.0f;
// Room past the clearance: a caster on his centre has all of it to walk
constexpr float STOMP_SWEEP_RADIUS = 30.0f;

bool ClearsBombs(Position const& spot, std::vector<Position> const& bombs, float clearance)
{
    return std::none_of(bombs.begin(), bombs.end(),
                        [&spot, clearance](Position const& bomb) { return bomb.GetExactDist2d(spot) < clearance; });
}
}

bool GormokTankHoldBossAction::Execute(Event /*event*/)
{
    Unit* gormok = GetEngagedBeast(botAI, NorthrendBeast::Gormok);
    if (!gormok)
        return false;

    if (AI_VALUE(Unit*, "current target") != gormok)
        return Attack(gormok);

    // Only take him back off a non-tank. The swap tank taunts on its own node, and a hold that taunted
    // back whenever it lost him would undo every swap.
    Unit* victim = gormok->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    if (victim && !(victimPlayer && IsBeastsTank(victimPlayer)) && CastClassTaunt(botAI, gormok))
        return true;

    // Every Icehowl charge line crosses the centre, so no drag toward it while one is latched
    Position chargeStart;
    Position chargeEnd;
    if (IcehowlChargeLatched(botAI, chargeStart, chargeEnd))
        return false;

    // Keep the boss near the centre of the arena so ranged can spread and melee have room
    return DragBossToAnchor(gormok, ARENA_CENTER);
}

bool GormokSwapTankHoldAction::Execute(Event /*event*/)
{
    return AttackDirect(GetEngagedBeast(botAI, NorthrendBeast::Gormok));
}

bool GormokTankSwapTauntAction::Execute(Event /*event*/)
{
    Unit* gormok = GetGormokSwapTauntTarget(botAI);
    if (!gormok)
        return false;

    if (CastClassTaunt(botAI, gormok))
        return true;

    // Taunt on cooldown or resisted: build threat on him meanwhile
    if (AI_VALUE(Unit*, "current target") != gormok)
        return Attack(gormok);

    return false;
}

bool GormokTankDefensiveAction::Execute(Event /*event*/)
{
    uint32 const defensive = NextTankDefensive(botAI, bot);
    return defensive && botAI->CastSpell(defensive, bot);
}

bool GormokHandOfProtectionAction::Execute(Event /*event*/)
{
    Player* target = GetGormokImpaleResetTarget(botAI);
    uint32 const spellId = target ? AI_VALUE2(uint32, "spell id", "hand of protection") : 0;
    return spellId && botAI->CastSpell(spellId, target);
}

bool GormokCancelHandOfProtectionAction::Execute(Event /*event*/)
{
    if (!GormokHandOfProtectionToCancel(botAI))
        return false;

    for (uint32 rank : {ai::paladin::SPELL_HAND_OF_PROTECTION, ai::paladin::SPELL_HAND_OF_PROTECTION_RANK_2,
                        ai::paladin::SPELL_HAND_OF_PROTECTION_RANK_3})
        bot->RemoveAurasDueToSpell(rank, ObjectGuid::Empty, 0, AURA_REMOVE_BY_CANCEL);

    return true;
}

bool GormokBringSnoboldToMeleeAction::Execute(Event /*event*/)
{
    Unit* gormok = GetGormokForSnoboldCarrier(botAI);
    if (!gormok || bot->GetExactDist2d(gormok) <= GORMOK_CARRIER_MELEE_RANGE)
    {
        DropWalk();
        return false;
    }

    if (Position const* spot = WalkInFlight())
        if (gormok->GetExactDist2d(spot) <= GORMOK_CARRIER_MELEE_RANGE - CARRIER_KEEP_INSET)
            return true;

    float const angle = gormok->GetAngle(bot);
    float const reach = GORMOK_CARRIER_MELEE_RANGE - CARRIER_AIM_INSET;
    Position const spot(gormok->GetPositionX() + reach * std::cos(angle),
                        gormok->GetPositionY() + reach * std::sin(angle), gormok->GetPositionZ());
    return WalkTo(spot);
}

bool GormokLeaveStompRangeAction::Execute(Event /*event*/)
{
    Unit* gormok = GetGormokStompThreat(botAI);
    if (!gormok)
    {
        DropWalk();
        return false;
    }

    // Without the bombs a caster walks back into one it just dodged, and the two trade it until impact
    std::vector<Position> young;
    std::vector<Position> old;
    GetFireBombs(botAI, young, old);

    if (Position const* spot = WalkInFlight())
        if (gormok->GetExactDist2d(spot) >= GORMOK_STOMP_TRIGGER &&
            (!_spotClearsBombs || ClearsBombs(*spot, young, FIRE_BOMB_CLEARANCE)))
            return true;

    // Ties go toward his tank, so healers stay in range of whoever they're keeping up
    Position tank;
    Position const* preferNear = nullptr;
    if (Unit* victim = gormok->GetVictim())
    {
        tank = victim->GetPosition();
        preferNear = &tank;
    }

    HazardCircle const stomp(gormok->GetPosition(), GORMOK_STOMP_CLEARANCE);
    std::vector<HazardCircle> hazards = { stomp };
    for (Position const& bomb : young)
        hazards.emplace_back(bomb, FIRE_BOMB_CLEARANCE);

    HazardSweepCache sweep;
    auto const findSpot = [this, &preferNear, &sweep](std::vector<HazardCircle> const& circles)
    {
        return FindNearestPositionClearOfHazards(bot, circles, STOMP_SWEEP_RADIUS, 2.0f,
                                                 static_cast<float>(M_PI) / 8.0f, preferNear, {}, &sweep);
    };

    Position spot = findSpot(hazards);
    _spotClearsBombs = spot != Position();
    if (!_spotClearsBombs && hazards.size() > 1)
        spot = findSpot({ stomp });

    if (spot == Position())
        return false;

    return WalkTo(spot);
}

bool GormokDodgeFireBombAction::Execute(Event /*event*/)
{
    if (!IsInFireBombImpact(botAI))
    {
        DropWalk();
        return false;
    }

    if (!botAI->CanMove())
        return false;

    std::vector<Position> young;
    std::vector<Position> old;
    GetFireBombs(botAI, young, old);

    Position const* inFlight = WalkInFlight();
    if (inFlight && ClearsBombs(*inFlight, young, FIRE_BOMB_TIGHT_CLEARANCE))
        return true;

    // WalkTo refuses under a pinning cast anyway, so skip the sweep. The hit isn't lethal, let the cast
    // finish.
    if (bot->IsMovementPreventedByCasting())
        return false;

    Position anchor;
    Position const* preferNear = GetFireBombDodgeAnchor(botAI, anchor) ? &anchor : nullptr;

    // Most important first, so each fallback drops the tail: young bombs, then the pulses of landed
    // ones, then Gormok's stomp for a caster, whom the stomp walk would pull straight back.
    std::vector<HazardCircle> circles;
    for (Position const& bomb : young)
        circles.emplace_back(bomb, FIRE_BOMB_CLEARANCE);
    for (Position const& bomb : old)
        circles.emplace_back(bomb, FIRE_BOMB_PULSE_CLEARANCE);

    size_t const youngCount = young.size();
    size_t const bombCount = circles.size();
    if (Unit* gormok = GetGormokForStompCaster(botAI))
        circles.emplace_back(gormok->GetPosition(), GORMOK_STOMP_CLEARANCE);

    HazardSweepCache sweep;
    auto const findSpot = [this, &preferNear, &sweep](std::vector<HazardCircle> const& hazards)
    {
        return FindNearestPositionClearOfHazards(bot, hazards, FIRE_BOMB_SWEEP_RADIUS, 2.0f,
                                                 static_cast<float>(M_PI) / 8.0f, preferNear, {}, &sweep);
    };

    Position spot = findSpot(circles);
    if (spot == Position() && circles.size() > bombCount)
    {
        circles.resize(bombCount);
        spot = findSpot(circles);
    }

    if (spot == Position() && bombCount > youngCount)
    {
        circles.resize(youngCount);
        spot = findSpot(circles);
    }

    if (spot == Position())
    {
        circles.resize(youngCount);
        for (HazardCircle& circle : circles)
            circle.second = FIRE_BOMB_TIGHT_CLEARANCE;
        spot = findSpot(circles);
    }

    if (spot == Position())
        return false;

    // Our own walk still holds the FORCED booking, and a booking only yields to a higher priority
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    bool const lowered = inFlight && last.priority == MovementPriority::MOVEMENT_FORCED;
    if (lowered)
        last.priority = MovementPriority::MOVEMENT_COMBAT;

    if (!WalkTo(spot, MovementPriority::MOVEMENT_FORCED))
    {
        if (lowered)
            last.priority = MovementPriority::MOVEMENT_FORCED;

        return false;
    }

    return true;
}
