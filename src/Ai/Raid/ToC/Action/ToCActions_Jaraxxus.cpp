/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_Jaraxxus.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidDefensive.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCHelpers_Jaraxxus.h"
#include "Unit.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
// how long a latched flame spot is trusted while the walk to it is in flight
constexpr uint32 FLAME_LATCH_MS = 3000;
// standing still this long past the issue means something stopped the walk short
constexpr uint32 FLAME_STALL_MS = 500;
// a gap this long between ticks is a new flame, nothing latched carries over
constexpr uint32 FLAME_WINDOW_GAP_MS = 2000;
constexpr uint32 PORTAL_LATCH_MS = 3000;
constexpr uint32 PORTAL_STALL_MS = 500;
constexpr float PORTAL_ARRIVE = 1.5f;
// a latched exit this little inside the portal's clearance still counts as clear
constexpr float PORTAL_EXIT_SLACK = 0.5f;
// a booking this close to a spot is the walk to it
constexpr float SAME_DESTINATION = 1.0f;

// A walk waits behind any booking of equal or higher priority until it runs out
void DropForeignBooking(LastMovement& last, Position const& own, MovementPriority upTo)
{
    if (last.msTime && last.priority <= upTo &&
        last.lastMoveShort.GetExactDist2d(own.GetPositionX(), own.GetPositionY()) >= SAME_DESTINATION)
        last.clear();
}

}  // namespace

bool JaraxxusInterruptFelFireballAction::Execute(Event /*event*/)
{
    Unit* jaraxxus = GetJaraxxus(botAI);
    if (!jaraxxus)
        return false;

    char const* interrupt = JaraxxusReadyInterrupt(bot, jaraxxus);
    return interrupt && botAI->CastSpell(interrupt, jaraxxus);
}

void JaraxxusAvoidLegionFlameAction::ClearLatches()
{
    _hasSpot = false;
    _hasHeading = false;
}

bool JaraxxusAvoidLegionFlameAction::IssueLeg(Position const& spot, uint32 now)
{
    // our last leg still holds the FORCED lock and a lock only yields to a higher priority, so step
    // it down for the replacement and put it back if nothing went out
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    bool const replacing =
        last.priority == MovementPriority::MOVEMENT_FORCED &&
        last.lastMoveShort.GetExactDist2d(_lastIssued.GetPositionX(), _lastIssued.GetPositionY()) < SAME_DESTINATION;
    if (replacing)
        last.priority = MovementPriority::MOVEMENT_COMBAT;

    MoveOutcome const outcome =
        TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED);

    if (outcome == MoveOutcome::Issued)
    {
        _legStart = bot->GetPosition();
        _lastIssued = spot;
        _spot = spot;
        _hasSpot = true;
        _issuedMs = now;
        return true;
    }

    if (replacing)
        last.priority = MovementPriority::MOVEMENT_FORCED;

    // already walking there
    if (outcome == MoveOutcome::Duplicate)
    {
        _spot = spot;
        _hasSpot = true;
        _issuedMs = now;
        return true;
    }

    _hasSpot = false;
    return false;
}

bool JaraxxusAvoidLegionFlameAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();
    if (!_lastTickMs || getMSTimeDiff(_lastTickMs, now) > FLAME_WINDOW_GAP_MS)
        ClearLatches();
    _lastTickMs = now;

    JaraxxusFlameRole const role = GetJaraxxusFlameRole(bot);
    if (role == JaraxxusFlameRole::None)
    {
        ClearLatches();
        return false;
    }

    // a running cast pins the feet, the spline never launches while it lasts
    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);

    if (role != JaraxxusFlameRole::Carrier)
        _hasHeading = false;
    else if (!_hasHeading)
    {
        // a dodge picked before the flame landed on us isn't a leg away from the raid
        _hasSpot = false;
        _heading = GetLegionFlameCarrierHeading(bot);
        _hasHeading = true;
    }

    // every MoveTo clears the motion master, so re-deriving under a walk in flight would restart it
    // each tick
    if (_hasSpot)
    {
        uint32 const age = getMSTimeDiff(_issuedMs, now);
        bool const arrived = bot->GetExactDist2d(&_spot) <= JARAXXUS_FLAME_ARRIVE;
        bool const stalled = !arrived && !bot->isMoving() && age > FLAME_STALL_MS;

        if (!arrived && !stalled && age < FLAME_LATCH_MS && IsLegionFlameSpotClear(botAI, _spot))
            return true;

        _hasSpot = false;

        if (arrived && role == JaraxxusFlameRole::Dodge)
            return false;

        // the next leg carries straight on, never back over the trail
        if (role == JaraxxusFlameRole::Carrier)
            _heading = _legStart.GetAngle(&_spot);

        // MoveTo refuses the same point for 5 s, moving or not
        if (stalled)
            AI_VALUE(LastMovement&, "last movement").clear();
    }

    Position spot;
    if (!DeriveLegionFlameSpot(bot, role, _heading, spot))
        return false;

    return IssueLeg(spot, now);
}

bool JaraxxusBreakPinnedCastAction::Execute(Event /*event*/)
{
    for (Player* caster : GetJaraxxusPinnedCasters(bot))
        if (PlayerbotAI* casterAI = GET_PLAYERBOT_AI(caster))
            casterAI->RequestSpellInterrupt();

    return false;
}

bool JaraxxusLeavePortalAction::Execute(Event /*event*/)
{
    Unit* portal = GetJaraxxusPortalToClear(botAI);
    if (!portal || bot->GetExactDist2d(portal) >= JARAXXUS_PORTAL_TRIGGER)
    {
        _hasSpot = false;
        return false;
    }

    // every MoveTo restarts the walk, so one in flight to a spot still clear is left alone
    uint32 const now = getMSTime();
    if (_hasSpot)
    {
        uint32 const age = getMSTimeDiff(_issuedMs, now);
        bool const arrived = bot->GetExactDist2d(&_spot) <= PORTAL_ARRIVE;
        bool const stalled = !arrived && !bot->isMoving() && age > PORTAL_STALL_MS;
        if (!arrived && !stalled && age < PORTAL_LATCH_MS &&
            _spot.GetExactDist2d(portal) >= JARAXXUS_PORTAL_CLEAR - PORTAL_EXIT_SLACK &&
            IsLegionFlameSpotClear(botAI, _spot))
            return true;

        _hasSpot = false;
        // MoveTo refuses the same point for 5 s, moving or not
        if (stalled)
            AI_VALUE(LastMovement&, "last movement").clear();
    }

    Position spot;
    if (!DeriveJaraxxusPortalExit(bot, portal, spot))
        return false;

    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);

    // the hold blocks every other mover of this priority, a reach melee walk already booked included
    DropForeignBooking(AI_VALUE(LastMovement&, "last movement"), spot, MovementPriority::MOVEMENT_COMBAT);
    MoveOutcome const outcome = TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(),
                                          spot.GetPositionZ(), false, false, false, false,
                                          MovementPriority::MOVEMENT_COMBAT);
    if (outcome != MoveOutcome::Issued && outcome != MoveOutcome::Duplicate)
        return false;

    _spot = spot;
    _hasSpot = true;
    _issuedMs = now;
    return true;
}

bool JaraxxusCastRaidCoverAction::Execute(Event /*event*/)
{
    RaidPredictedHit hit;
    RaidCover const cover =
        GetRaidCover(botAI, TOC_DEFENSIVE_SCOPE, GetJaraxxusBurningInferno(botAI, hit) ? &hit : nullptr);
    uint32 const spellId = RaidCoverSpellId(botAI, cover);
    if (!spellId)
        return false;

    // a channel fails any other cast's check, and a moving bot breaks its own channel
    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    if (IsRaidCoverChannel(cover) && bot->isMoving())
        bot->StopMoving();

    if (!botAI->CastSpell(spellId, bot))
        return false;

    FinishRaidCover(botAI, cover);
    return true;
}

bool JaraxxusCastPersonalDefensiveAction::Execute(Event /*event*/)
{
    RaidPredictedHit hit;
    RaidPersonalPick const pick =
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, GetJaraxxusPersonalHit(botAI, hit) ? &hit : nullptr);
    if (!pick)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    return pick.consumable ? botAI->DoSpecificAction(pick.consumable, Event(), true)
                           : botAI->CastSpell(pick.spellId, bot);
}

bool JaraxxusMainTankHoldBossAction::Execute(Event /*event*/)
{
    Unit* jaraxxus = GetJaraxxus(botAI);
    if (!jaraxxus)
        return false;

    if (AI_VALUE(Unit*, "current target") != jaraxxus)
        return Attack(jaraxxus);

    float const toCenter = bot->GetExactDist2d(ARENA_CENTER);
    if (jaraxxus->GetVictim() != bot || toCenter <= TOC_DRAG_DONE)
        return false;

    // only the step about to be walked: a tank-carried trail runs along the whole way back for 60 s
    float const scale = std::min(TOC_DRAG_STEP, toCenter) / toCenter;
    Position const step(bot->GetPositionX() + (ARENA_CENTER.GetPositionX() - bot->GetPositionX()) * scale,
                        bot->GetPositionY() + (ARENA_CENTER.GetPositionY() - bot->GetPositionY()) * scale,
                        bot->GetPositionZ());
    if (!LegionFlameCrossesPath(bot, step))
        return DragBossToAnchor(jaraxxus, ARENA_CENTER);

    Position detour;
    if (!DeriveLegionFlameDetour(bot, ARENA_CENTER, TOC_DRAG_STEP, detour))
        return false;

    return MoveTo(bot->GetMapId(), detour.GetPositionX(), detour.GetPositionY(), detour.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
}

bool JaraxxusAssistTankHoldAction::HoldAdd(uint8 index)
{
    Unit* add = GetJaraxxusAssistTankAdd(botAI, index);
    if (!add)
        return false;

    bool attacked = false;
    if (AI_VALUE(Unit*, "current target") != add)
        attacked = Attack(add);

    // the add is this tank's alone, so it comes back off anyone else, tanks too: Fel Streak resets the
    // Infernal's threat onto whoever it charged, and a taunt copies the top threat back
    Unit* victim = add->GetVictim();
    if (victim && victim != bot && victim->ToPlayer() && CastClassTaunt(botAI, add))
        return true;

    return attacked;
}

bool JaraxxusAssistTankHoldAddAction::Execute(Event /*event*/) { return HoldAdd(0); }

bool JaraxxusAssistTankHoldSecondAddAction::Execute(Event /*event*/) { return HoldAdd(1); }

bool JaraxxusRemoveNetherPowerAction::Execute(Event /*event*/)
{
    Unit* jaraxxus = GetJaraxxus(botAI);
    if (!jaraxxus || !JaraxxusHasNetherPower(jaraxxus))
        return false;

    static std::vector<std::string> const dispels = {"spellsteal", "purge", "dispel magic"};
    for (std::string const& dispel : dispels)
    {
        if (botAI->CanCastSpell(dispel, jaraxxus))
            return botAI->CastSpell(dispel, jaraxxus);
    }

    return false;
}

bool JaraxxusDpsTargetAction::Execute(Event /*event*/) { return AttackDirect(GetJaraxxusDpsTarget(botAI)); }

bool JaraxxusHealIncinerateTargetAction::Execute(Event /*event*/)
{
    Unit* target = GetIncinerateFleshTarget(botAI);
    if (!target)
        return false;

    // the kiss punishes anything still casting on its 0.5 s check
    bool const kissed = HasMistressKiss(bot);
    return CastRaidThroughputHeal(botAI, target, [this, kissed](std::string const& heal)
                                 { return kissed && JaraxxusSpellHasCastTime(botAI, heal); });
}
