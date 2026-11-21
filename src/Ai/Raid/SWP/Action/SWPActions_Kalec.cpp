/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "EncounterHelpers.h"
#include "LastMovementValue.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "SWPEncounter_Kalec.h"
#include "SWPShared.h"
#include "ThreatHelpers.h"
#include <cmath>
#include <map>
#include <string>

using namespace SwpHelpers;
using namespace EncounterHelpers;

bool KalecgosAnnounceBossHealthAction::Execute(Event /*event*/)
{
    KalecgosCall const call = GetDueKalecgosCall(bot);
    if (call == KalecgosCall::None)
        return false;

    Unit* boss = GetKalecgosSideBoss(bot);
    if (!boss)
        return false;

    KalecgosEncounterState& state = KalecgosState(bot->GetInstanceId());
    bool const isRealmSide = IsInSpectralRealm(bot);
    std::string const bossName = isRealmSide ? "Sathrovarr" : "Kalecgos";
    std::string const otherName = isRealmSide ? "Kalecgos" : "Sathrovarr";
    PlayerbotTextMgr& textMgr = PlayerbotTextMgr::instance();
    std::string text;

    switch (call)
    {
        case KalecgosCall::FirstUnder:
        {
            state.healthCall = KalecgosHealthCall::FirstUnder;
            state.firstLowBoss = isRealmSide ? KalecgosBoss::Sathrovarr : KalecgosBoss::Kalecgos;
            text = textMgr.GetBotTextOrDefault(
                "kalecgos_health_call_first_under", "%boss is at 15% health!",
                std::map<std::string, std::string>{ { "%boss", bossName } });
            break;
        }
        case KalecgosCall::OtherHealth:
        {
            // The hold is decided on the rounded health, so the call never says 20% with no hold.
            long const healthPct = std::lround(boss->GetHealthPct());
            std::map<std::string, std::string> const placeholders = {
                { "%boss", bossName },
                { "%health", std::to_string(healthPct) },
                { "%low", otherName },
            };

            if (healthPct >= KALECGOS_HEALTH_GAP_HOLD_PCT)
            {
                state.healthCall = KalecgosHealthCall::GapHold;
                text = textMgr.GetBotTextOrDefault(
                    "kalecgos_health_call_hold",
                    "%boss is at %health%. Stop attacking %low until %boss reaches 15%!",
                    placeholders);
            }
            else
            {
                state.healthCall = KalecgosHealthCall::Together;
                text = textMgr.GetBotTextOrDefault(
                    "kalecgos_health_call_together",
                    "%boss is at %health%. Keep attacking both together!", placeholders);
            }
            break;
        }
        case KalecgosCall::GapHoldRelease:
        {
            state.healthCall = KalecgosHealthCall::Together;
            text = textMgr.GetBotTextOrDefault(
                "kalecgos_health_call_release", "%boss has reached 15%. Attack %low again!",
                std::map<std::string, std::string>{ { "%boss", bossName }, { "%low", otherName } });
            break;
        }
        case KalecgosCall::EndHold:
        {
            state.endHoldCalled = true;
            text = textMgr.GetBotTextOrDefault(
                "kalecgos_end_hold",
                "Kalecgos is at 5%. Stop attacking him until Sathrovarr is banished!", {});
            break;
        }
        case KalecgosCall::SathrovarrBanished:
        {
            state.sathrovarrBanishCalled = true;
            text = textMgr.GetBotTextOrDefault(
                "kalecgos_sathrovarr_banished", "Sathrovarr is banished. Finish Kalecgos!", {});
            break;
        }
        default:
            return false;
    }

    botAI->SayToRaid(text);
    return true;
}

bool KalecgosUpdateRaidAssignmentsAction::Execute(Event /*event*/)
{
    return UpdateKalecgosEncounterState(bot);
}

bool KalecgosStopAttackingForHoldAction::Execute(Event /*event*/)
{
    return StopAttackingAndChanneling(bot);
}

bool KalecgosSelectTargetForHoldAction::isUseful()
{
    return botAI->GetState() == BOT_STATE_NON_COMBAT;
}

bool KalecgosSelectTargetForHoldAction::Execute(Event /*event*/)
{
    return SelectTarget(botAI, AI_VALUE(Unit*, "dps target"));
}

bool KalecgosSetPetStanceAction::Execute(Event /*event*/)
{
    bool const isHeld = GetKalecgosDpsHoldWindow(bot) != KalecgosDpsHold::None;
    return SetMinionStancesForHold(bot, isHeld, _previousReactStates);
}

bool KalecgosSetPetStanceAction::ResetPetStance()
{
    bool const restored = SetMinionStancesForHold(bot, false, _previousReactStates);
    _previousReactStates.clear();
    return restored;
}

bool KalecgosSurfaceTankPositionDragonAction::Execute(Event event)
{
    Unit* kalecgos = GetSwpBoss(bot, SwpBosses::DATA_KALECGOS);
    if (!kalecgos)
        return false;

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    // This action also runs for a tank that does not have aggro on Kalecgos, so whether to move
    // backwards is kicked to GetStepToPosition(), which makes backwards movement contingent on
    // the bot being Kalecgos's victim.
    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (GetStepToPosition(
            bot, KALECGOS_TANK_POSITION, arrivalDist, kalecgos, moveX, moveY, backwards))
    {
        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    if (GetKalecgosDesignatedTank(bot) != bot || kalecgos->GetVictim() == bot)
        return false;

    return botAI->DoSpecificAction("taunt spell", event, true);
}

// FYI, there is a (Blizzlike) delay of 1s after a player clicks a rift before the next can click.
bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    // Special conditions for tanks only
    if (PlayerbotAI::IsTank(bot) && !ShouldTankEnter())
        return false;

    GameObject* rift = botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift"));
    if (!rift)
        return false;

    if (rift->IsAtInteractDistance(*bot, rift->GetInteractionDistance()))
    {
        // A use fails while exhausted, but still blocks the rift for everyone for 1s.
        if (IsExhausted(bot))
            return false;

        rift->Use(bot);
        return true;
    }

    float const targetDist = rift->GetInteractionDistance() - 0.5f;
    float const angle = rift->GetAngle(bot);
    float const destX = rift->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = rift->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SWP_MAP_ID, destX, destY, rift->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KalecgosEnterSpectralRiftAction::ShouldTankEnter()
{
    Unit* kalecgos = GetSwpBoss(bot, SwpBosses::DATA_KALECGOS);
    if (!kalecgos)
        return false;

    // The current tank cannot enter a portal until the next tank takes over. If the designated
    // tank is still this bot, nobody has taken over yet.
    Player* surfaceTank = GetKalecgosDesignatedTank(bot);
    if (!surfaceTank || surfaceTank == bot)
        return false;

    Position const& position = KALECGOS_TANK_POSITION;
    return surfaceTank->GetExactDist2d(position) <= 3.0f && kalecgos->GetVictim() == surfaceTank;
}

bool KalecgosSpreadRangedAction::Execute(Event /*event*/)
{
    if (!_initialRangedPositionReached)
    {
        Position const& initialPos = KALECGOS_INITIAL_RANGED_POSITION;
        constexpr float initialRangedRadius = 10.0f;

        if (bot->GetExactDist2d(initialPos) <= initialRangedRadius)
        {
            _initialRangedPositionReached = true;
            return false;
        }

        // The point on the circle MoveInside() would head for, so the ranged still fan out
        float const angle = GetFollowAngle();
        Position const rimPosition(
            initialPos.GetPositionX() + std::cos(angle) * initialRangedRadius,
            initialPos.GetPositionY() + std::sin(angle) * initialRangedRadius,
            initialPos.GetPositionZ());

        constexpr float arrivalDist = 0.0f;
        float moveX;
        float moveY;
        bool backwards;
        if (!GetStepToPosition(bot, rimPosition, arrivalDist, nullptr, moveX, moveY, backwards))
            return false;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
            MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (Unit* kalecgos = GetSwpBoss(bot, SwpBosses::DATA_KALECGOS))
    {
        constexpr float safeDistFromDragon = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(kalecgos) < safeDistFromDragon)
            return FleePosition(kalecgos->GetPosition(), safeDistFromDragon, minInterval);
    }

    Player* nearestPlayer = GetNearestPlayerInRadius(bot, KALECGOS_RANGED_SPREAD_CLEARANCE);
    if (!nearestPlayer)
        return false;

    return FleePosition(nearestPlayer->GetPosition(), KALECGOS_RANGED_SPREAD_CLEARANCE);
}

bool KalecgosSathrovarrTankStandWithKalecAction::Execute(Event /*event*/)
{
    Unit* sathrovarr = FindThreatTarget(bot, "sathrovarr the corruptor");
    if (!sathrovarr)
        return false;

    constexpr float searchRadius = 20.0f;
    Creature* kalec = bot->FindNearestCreature(Id(SwpNpcs::NPC_KALECGOS_HUMANOID), searchRadius);
    if (!kalec || sathrovarr->GetVictim() != kalec)
        return false;

    constexpr float arrivalDist = 3.0f;
    float moveX;
    float moveY;
    bool backwards;
    if (!GetStepToPosition(
            bot, kalec->GetPosition(), arrivalDist, nullptr, moveX, moveY, backwards))
    {
        return false;
    }

    return MoveTo(
        SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KalecgosReturnToSpectralRealmGroundAction::Execute(Event /*event*/)
{
    // Temporary, to find what lifts bots off the realm floor (which is flat and meshed).
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    LastMovement const& lastMovement = AI_VALUE(LastMovement&, "last movement");
    LOG_INFO(
        "playerbots",
        "Kalecgos realm gravity: {} at ({:.1f}, {:.1f}, {:.1f}), engine {}, target {}, motion {}, "
        "last move to ({:.1f}, {:.1f}, {:.1f})",
        bot->GetName(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        static_cast<uint32>(botAI->GetState()), currentTarget ? currentTarget->GetName() : "none",
        static_cast<uint32>(bot->GetMotionMaster()->GetCurrentMovementGeneratorType()),
        lastMovement.lastMoveToX, lastMovement.lastMoveToY, lastMovement.lastMoveToZ);

    // Temporary: whether a spline is running and where to, the motion slots, and time in the realm.
    MotionMaster* motionMaster = bot->GetMotionMaster();
    float splineX = 0.0f;
    float splineY = 0.0f;
    float splineZ = 0.0f;
    bool const isSplineRunning = motionMaster->GetDestination(splineX, splineY, splineZ);
    Aura const* realm = bot->GetAura(Id(SwpSpells::SPELL_SPECTRAL_REALM));
    int32 const msInRealm = realm ? realm->GetMaxDuration() - realm->GetDuration() : -1;
    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    bool const isBlasted = state && state->blastedPlayerGuid == bot->GetGUID();
    LOG_INFO(
        "playerbots",
        "Kalecgos realm gravity: {} spline running {} to ({:.1f}, {:.1f}, {:.1f}), slots {} "
        "(active {}, controlled {}), {} ms in realm, blasted by current rift {}",
        bot->GetName(), isSplineRunning, splineX, splineY, splineZ, motionMaster->size(),
        static_cast<uint32>(motionMaster->GetMotionSlotType(MOTION_SLOT_ACTIVE)),
        static_cast<uint32>(motionMaster->GetMotionSlotType(MOTION_SLOT_CONTROLLED)), msInRealm,
        isBlasted);

    bot->NearTeleportTo(
        bot->GetPositionX(), bot->GetPositionY(), SPECTRAL_REALM_Z, bot->GetOrientation());

    constexpr float zTolerance = 1.0f;
    return std::fabs(bot->GetPositionZ() - SPECTRAL_REALM_Z) <= zTolerance;
}
