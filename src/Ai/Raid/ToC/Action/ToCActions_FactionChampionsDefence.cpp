/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_FactionChampionsDefence.h"

#include <algorithm>
#include <vector>

#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "RaidDefensive.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"

using namespace TrialOfTheCrusaderHelpers;
using namespace EncounterHelpers;

namespace
{
constexpr float AOE_ARRIVE = 1.5f;
constexpr float GRIP_ARRIVE = 2.0f;
// a walk already headed here, so a reissue would only restart its spline
constexpr float GRIP_SAME_STAND = 0.5f;
// how long a latched spot is trusted while the walk to it is in flight
constexpr uint32 AOE_LATCH_MS = 3000;
// standing still this long past the issue means something stopped the walk short
constexpr uint32 AOE_STALL_MS = 500;
// the wall grip hands back the DK's own spot once it's at its stand
constexpr float WALL_AT_STAND = 0.5f;
// a CC walk stops this far inside the spell's range, the champion keeps moving
constexpr float CC_WALK_INSIDE = 3.0f;
// the champion moved less than this since the walk went out, so it still heads the right way
constexpr float CC_SAME_SPOT = 3.0f;

bool IsClearOf(Position const& spot, std::vector<HazardCircle> const& hazards)
{
    for (HazardCircle const& hazard : hazards)
        if (spot.GetExactDist2d(hazard.first) < hazard.second)
            return false;

    return true;
}

// FindNearestPositionClearOfHazards answers Position() when nothing is clear
bool IsNoSpot(Position const& spot) { return !spot.GetPositionX() && !spot.GetPositionY(); }

// a FORCED leg keeps its lock for its walk time, the same window MoveTo's gate waits out
bool ForcedLegHolds(LastMovement const& last)
{
    return last.priority == MovementPriority::MOVEMENT_FORCED && last.lastdelayTime + last.msTime > getMSTime();
}

// CastSpell refuses any cast-time spell while the bot walks. A FORCED leg is a dodge, let it finish.
bool StandForCast(Player* bot, LastMovement& last, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!bot->isMoving() || !info || !info->CalcCastTime(bot))
        return true;

    if (ForcedLegHolds(last))
        return false;

    bot->StopMoving();
    last.clear();
    return true;
}
}  // namespace

bool FactionChampionsAvoidAoeAction::IssueLeg(Position const& spot, uint32 now)
{
    // our last leg still holds the FORCED lock and a lock only yields to a higher priority, so step
    // it down for the replacement and put it back if nothing went out
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    bool const replacing =
        last.priority == MovementPriority::MOVEMENT_FORCED &&
        last.lastMoveShort.GetExactDist2d(_lastIssued.GetPositionX(), _lastIssued.GetPositionY()) < 1.0f;
    if (replacing)
        last.priority = MovementPriority::MOVEMENT_COMBAT;

    MoveOutcome const outcome =
        TryMoveTo(bot->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ(), false, false,
                  false, false, MovementPriority::MOVEMENT_FORCED);

    if (outcome == MoveOutcome::Issued)
    {
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

bool FactionChampionsAvoidAoeAction::Execute(Event /*event*/)
{
    if (!FactionChampionsInAoe(botAI))
    {
        _hasSpot = false;
        return false;
    }

    // a running cast pins the feet, the spline never launches while it lasts
    if (bot->IsMovementPreventedByCasting())
        bot->InterruptNonMeleeSpells(true);

    std::vector<HazardCircle> const clearances = FactionChampionsAoeClearances(botAI);
    uint32 const now = getMSTime();

    // every MoveTo clears the motion master, so re-deriving under a walk in flight would restart it
    // each tick. Still re-check the spot, a bladestorming warrior keeps walking
    if (_hasSpot)
    {
        uint32 const age = getMSTimeDiff(_issuedMs, now);
        bool const arrived = bot->GetExactDist2d(&_spot) <= AOE_ARRIVE;
        bool const stalled = !arrived && !bot->isMoving() && age > AOE_STALL_MS;

        if (!arrived && !stalled && age < AOE_LATCH_MS && IsClearOf(_spot, clearances))
            return true;

        _hasSpot = false;

        // MoveTo refuses the same point for 5 s, moving or not
        if (stalled)
            AI_VALUE(LastMovement&, "last movement").clear();
    }

    Position const spot = FactionChampionsAoeDodgeSpot(botAI);
    if (IsNoSpot(spot))
        return false;

    return IssueLeg(spot, now);
}

bool FactionChampionsDeathGripAction::StillWalking(Position const& stand, bool& walked)
{
    if (bot->GetExactDist2d(&stand) <= GRIP_ARRIVE)
    {
        DropWalk();
        return false;
    }

    Position const* walking = WalkInFlight();
    if (walking && walking->GetExactDist2d(&stand) < GRIP_SAME_STAND)
    {
        walked = true;
        return true;
    }

    // a booking left by a walk elsewhere refuses this one at the same priority for its whole walk time
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (last.priority != MovementPriority::MOVEMENT_FORCED &&
        stand.GetExactDist2d(last.lastMoveToX, last.lastMoveToY) >= GRIP_SAME_STAND)
        last.clear();

    walked = WalkTo(stand);
    return true;
}

bool FactionChampionsDeathGripAction::Execute(Event /*event*/)
{
    Position stand;
    Unit* target = FactionChampionsGripTarget(botAI, stand);
    if (!target)
    {
        DropWalk();
        return false;
    }

    bool walked = false;
    if (StillWalking(stand, walked))
        return walked;

    return botAI->CastSpell("death grip", target);
}

bool FactionChampionsChainGripAction::Execute(Event /*event*/)
{
    Position stand;
    bool walkOnly = false;
    Unit* target = FactionChampionsChainGripTarget(botAI, stand, walkOnly);
    if (!target)
    {
        DropWalk();
        return false;
    }

    bool walked = false;
    if (StillWalking(stand, walked))
        return walked;

    if (walkOnly || !botAI->CastSpell("death grip", target))
        return false;

    FactionChampionsTrackChainGrip(botAI, target);
    return true;
}

bool FactionChampionsChainRequestAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    return FactionChampionsRequestChain(botAI, requester);
}

bool FactionChampionsWallGripAction::Execute(Event /*event*/)
{
    Position stand;
    uint32 spellId = 0;
    bool gripped = false;
    Unit* champion = FactionChampionsWallGrip(botAI, stand, spellId, &gripped);
    if (!champion)
    {
        DropWalk();
        return false;
    }

    if (spellId)
    {
        DropWalk();
        if (!botAI->CastSpell(spellId, champion))
            return false;

        if (!gripped)
            FactionChampionsTrackWallGrip(botAI, champion);

        return true;
    }

    // waiting on the cooldown or the landing, the DK fights from where it stands meanwhile
    if (bot->GetExactDist2d(&stand) <= WALL_AT_STAND)
    {
        DropWalk();
        return false;
    }

    Position const* walking = WalkInFlight();
    if (walking && walking->GetExactDist2d(&stand) < GRIP_SAME_STAND)
        return true;

    return WalkTo(stand, MovementPriority::MOVEMENT_FORCED);
}

bool FactionChampionsFreedomAction::Execute(Event /*event*/)
{
    uint32 spellId = 0;
    Unit* member = FactionChampionsFreedomTarget(botAI, spellId);
    return member && botAI->CastSpell(spellId, member);
}

// A queued basket can pop ticks after its trigger fired, once the root wore off or another paladin took it
bool FactionChampionsFreedomAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsFreedomTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsChannelInterruptAction::Execute(Event /*event*/)
{
    char const* spell = nullptr;
    Unit* warlock = FactionChampionsChannelInterruptTarget(botAI, spell);
    return warlock && spell && botAI->CastSpell(spell, warlock);
}

// A queued basket can pop ticks after its trigger fired, once the channel ended
bool FactionChampionsChannelInterruptAction::isUseful()
{
    char const* spell = nullptr;
    return FactionChampionsChannelInterruptTarget(botAI, spell) != nullptr;
}

bool FactionChampionsSnareAction::Execute(Event /*event*/)
{
    uint32 spellId = 0;
    Unit* champion = FactionChampionsSnareTarget(botAI, spellId);
    return champion && botAI->CastSpell(spellId, champion);
}

// A queued basket can pop ticks after its trigger fired, once the champion is slowed or off its victim
bool FactionChampionsSnareAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsSnareTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsCrowdControlAction::Execute(Event /*event*/)
{
    uint32 spellId = 0;
    Unit* champion = FactionChampionsCcTarget(botAI, spellId);
    SpellInfo const* info = champion ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info)
    {
        DropWalk();
        return false;
    }

    float const range = info->GetMaxRange(false, bot);
    if (!bot->IsWithinCombatRange(champion, range) || !bot->IsWithinLOSInMap(champion))
    {
        // straight in, stopping a little inside the range
        float const away = bot->GetExactDist2d(champion);
        float const keep = std::max(0.0f, range - CC_WALK_INSIDE);
        float const share = away > 0.0f ? keep / away : 0.0f;
        Position const spot(champion->GetPositionX() + (bot->GetPositionX() - champion->GetPositionX()) * share,
                            champion->GetPositionY() + (bot->GetPositionY() - champion->GetPositionY()) * share,
                            bot->GetPositionZ());
        Position const* walking = WalkInFlight();
        if (walking && walking->GetExactDist2d(&spot) < CC_SAME_SPOT)
            return true;

        return WalkTo(spot);
    }

    DropWalk();
    if (!StandForCast(bot, AI_VALUE(LastMovement&, "last movement"), spellId))
        return false;

    return botAI->CastSpell(spellId, champion);
}

// A queued basket can pop ticks after its trigger fired, once the cast landed or another bot took it
bool FactionChampionsCrowdControlAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsCcTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsPeelAction::Execute(Event /*event*/)
{
    uint32 spellId = 0;
    Unit* champion = FactionChampionsPeelTarget(botAI, spellId);
    if (!champion || !StandForCast(bot, AI_VALUE(LastMovement&, "last movement"), spellId))
        return false;

    return botAI->CastSpell(spellId, champion);
}

// A queued basket can pop ticks after its trigger fired, once the champion left its victim or is held
bool FactionChampionsPeelAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsPeelTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsProtectAction::Execute(Event /*event*/)
{
    constexpr uint32 SPELL_DIVINE_SACRIFICE = 64205;
    uint32 spellId = 0;
    Unit* member = FactionChampionsProtectTarget(botAI, spellId);
    if (!member)
        return false;

    if (spellId != SPELL_DIVINE_SACRIFICE)
        return botAI->CastSpell(spellId, member);

    // self cast; only Divine Guardian's raid wide cut is wanted, the redirect would drain the paladin
    if (!botAI->CastSpell(spellId, bot))
        return false;

    FinishRaidCover(botAI, RaidCover::DivineSacrifice);
    return true;
}

// A queued basket can pop ticks after its trigger fired, once the champions moved on or someone covered it
bool FactionChampionsProtectAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsProtectTarget(botAI, spellId) != nullptr;
}

bool FactionChampionsPersonalDefensiveAction::Execute(Event /*event*/)
{
    RaidPredictedHit hit;
    RaidPersonalPick const pick =
        GetRaidPersonalDefensive(botAI, TOC_DEFENSIVE_SCOPE, FactionChampionsPersonalHit(botAI, hit) ? &hit : nullptr);
    if (!pick)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        bot->InterruptNonMeleeSpells(false);

    return pick.consumable ? botAI->DoSpecificAction(pick.consumable, Event(), true)
                           : botAI->CastSpell(pick.spellId, bot);
}

bool FactionChampionsBattleResAction::Execute(Event /*event*/)
{
    uint32 spellId = 0;
    Unit* corpse = FactionChampionsBattleResTarget(botAI, spellId);
    if (!corpse)
        return false;

    if (!StandForCast(bot, AI_VALUE(LastMovement&, "last movement"), spellId))
        return false;

    return botAI->CastSpell(spellId, corpse);
}

// A queued basket can pop ticks after its trigger fired, once someone else raised the corpse
bool FactionChampionsBattleResAction::isUseful()
{
    uint32 spellId = 0;
    return FactionChampionsBattleResTarget(botAI, spellId) != nullptr;
}

// Rebirth won't go out in cat or bear form
std::vector<NextAction> FactionChampionsBattleResAction::getPrerequisites() { return {NextAction("caster form")}; }

bool FactionChampionsMassDispelAction::Execute(Event /*event*/)
{
    Unit* target = FactionChampionsMassDispelTarget(botAI);
    if (!target)
        return false;

    // CastSpell refuses any cast-time spell while the bot walks. A FORCED leg is a dodge, let it finish
    if (bot->isMoving())
    {
        LastMovement& last = AI_VALUE(LastMovement&, "last movement");
        if (ForcedLegHolds(last))
            return false;

        bot->StopMoving();
        last.clear();
    }

    return botAI->CastSpell("mass dispel", target);
}

// A queued basket can pop ticks after its trigger fired, once the shield is gone
bool FactionChampionsMassDispelAction::isUseful() { return FactionChampionsMassDispelTarget(botAI) != nullptr; }

bool FactionChampionsDispelCcAction::Execute(Event /*event*/)
{
    char const* spell = nullptr;
    Unit* member = FactionChampionsDispelCcTarget(botAI, spell);
    return member && spell && botAI->CastSpell(spell, member);
}

// A queued basket can pop ticks after its trigger fired, once the CC is off or another bot took it
bool FactionChampionsDispelCcAction::isUseful()
{
    char const* spell = nullptr;
    return FactionChampionsDispelCcTarget(botAI, spell) != nullptr;
}

bool FactionChampionsKillTotemAction::Execute(Event /*event*/)
{
    return AttackDirect(FactionChampionsTotemTarget(botAI));
}

bool FactionChampionsPhysicalSwitchAction::Execute(Event /*event*/)
{
    Unit* target = FactionChampionsPhysicalSwitchTarget(botAI);
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool FactionChampionsPurgeKillTargetAction::Execute(Event /*event*/)
{
    char const* spell = nullptr;
    Unit* killTarget = FactionChampionsPurgeTarget(botAI, spell);
    return killTarget && spell && botAI->CastSpell(spell, killTarget);
}

// A queued basket can pop ticks after its trigger fired, once the buff is gone
bool FactionChampionsPurgeKillTargetAction::isUseful()
{
    char const* spell = nullptr;
    return FactionChampionsPurgeTarget(botAI, spell) != nullptr;
}
