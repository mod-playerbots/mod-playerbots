/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPActions.h"
#include "SWPEncounter_Kalec.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "SWPData.h"
#include <algorithm>
#include <string>
#include <map>

using namespace SwpHelpers;

bool KalecgosAnnounceBossHealthAction::Execute(Event /*event*/)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    auto const stateItr = kalecgosEncounterStates.find(bot->GetInstanceId());
    if (stateItr == kalecgosEncounterStates.end())
        return false;

    KalecgosEncounterState& state = stateItr->second;
    std::string text;

    if (!IsInSpectralRealm(bot))
    {
        if (state.surfaceHealthAnnounced)
            return false;

        state.surfaceHealthAnnounced = true;

        text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "kalecgos_below_twenty_percent_health",
            "Kalecgos's health is at 20%!",
            {});
    }
    else
    {
        if (state.spectralHealthAnnounced)
            return false;

        Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
        if (!sathrovarr)
            return false;

        state.spectralHealthAnnounced = true;

        std::string const sathrovarrHealth = std::to_string(
            static_cast<uint32>(sathrovarr->GetHealthPct()));

        text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "sathrovarr_health_when_kalecgos_below_twenty_percent_health",
            "Sathrovarr's health is at %sathrovarrHealth%! "
            "Don't forget that we need to defeat them at about the same time!",
            std::map<std::string, std::string>{
                {"%sathrovarrHealth", sathrovarrHealth}});
    }

    return botAI->SayToRaid(text);
}

bool KalecgosTankPositionBossAction::Execute(Event event)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    if (AI_VALUE(Unit*, "current target") != kalecgos)
        return Attack(kalecgos);

    Position const& position = KALECGOS_TANK_POSITION;
    float const distToPosition = bot->GetExactDist2d(position);

    if (distToPosition > 3.0f && bot->IsWithinMeleeRange(kalecgos))
    {
        float const posX = position.GetPositionX();
        float const posY = position.GetPositionY();
        float const botX = bot->GetPositionX();
        float const botY = bot->GetPositionY();

        float const toPosX = posX - botX;
        float const toPosY = posY - botY;
        float const toBossX = kalecgos->GetPositionX() - botX;
        float const toBossY = kalecgos->GetPositionY() - botY;
        bool const backwards = kalecgos->GetVictim() == bot &&
            (toPosX * toBossX + toPosY * toBossY) < 0.0f;

        float const maxMoveDist = backwards ? 2.25f : 3.5f;
        float const moveDist = std::min(maxMoveDist, distToPosition);
        float const moveX = botX + (toPosX / distToPosition) * moveDist;
        float const moveY = botY + (toPosY / distToPosition) * moveDist;

        return MoveTo(
            SWP_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
            false, false, MovementPriority::MOVEMENT_COMBAT, true, backwards);
    }

    if (GetKalecgosDesignatedTank(bot) == bot && kalecgos->GetVictim() != bot)
        return botAI->DoSpecificAction("taunt spell", event, true);

    return false;
}

bool KalecgosEnterSpectralRiftAction::Execute(Event /*event*/)
{
    // Special conditions for tanks only
    if (PlayerbotAI::IsTank(bot) && !ShouldTankEnter())
        return false;

    constexpr float searchRadius = 75.0f;
    GameObject* rift = bot->FindNearestGameObject(
        Id(SwpObjects::GO_SPECTRAL_RIFT), searchRadius, true);
    if (!rift)
        return false;

    if (rift->IsAtInteractDistance(*bot, rift->GetInteractionDistance()))
    {
        rift->Use(bot);
        return true;
    }

    float const targetDist = rift->GetInteractionDistance() - 0.5f;
    float const angle = rift->GetAngle(bot);
    float const destX = rift->GetPositionX() + std::cos(angle) * targetDist;
    float const destY = rift->GetPositionY() + std::sin(angle) * targetDist;

    return MoveTo(
        SWP_MAP_ID, destX, destY, rift->GetPositionZ(), false, false,
        false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

bool KalecgosEnterSpectralRiftAction::ShouldTankEnter()
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return false;

    Player* surfaceTank = GetKalecgosDesignatedTank(bot);
    if (!surfaceTank)
        return false;

    // The current tank cannot enter a portal until the next tank takes over. If the designated
    // tank is still this bot, nobody has taken over yet.
    if (surfaceTank == bot)
        return false;

    Position const& position = KALECGOS_TANK_POSITION;
    if (surfaceTank->GetExactDist2d(position) > 3.0f || kalecgos->GetVictim() != surfaceTank)
        return false;

    return true;
}

bool KalecgosDisperseRangedAction::Execute(Event /*event*/)
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

        return MoveInside(
            SWP_MAP_ID, initialPos.GetPositionX(), initialPos.GetPositionY(),
            initialPos.GetPositionZ(), initialRangedRadius, MovementPriority::MOVEMENT_COMBAT);
    }

    if (Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos"))
    {
        constexpr float safeDistFromDragon = 20.0f;
        constexpr uint32 minInterval = 0;
        if (bot->GetExactDist2d(kalecgos) < safeDistFromDragon)
            return FleePosition(kalecgos->GetPosition(), safeDistFromDragon, minInterval);
    }

    constexpr float safeDistFromPlayer = 6.0f;
    if (Player* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistFromPlayer))
    {
        constexpr uint32 minInterval = 1000;
        return FleePosition(nearestPlayer->GetPosition(), safeDistFromPlayer, minInterval);
    }

    return false;
}

bool KalecgosRemoveArcaneBuffetAction::Execute(Event /*event*/)
{
    switch (bot->getClass())
    {
        case CLASS_MAGE:
            return botAI->CanCastSpell("ice block", bot) &&
                botAI->CastSpell("ice block", bot);

        case CLASS_PALADIN:
            return botAI->CanCastSpell("divine shield", bot) &&
                botAI->CastSpell("divine shield", bot);

        case CLASS_ROGUE:
            return botAI->CanCastSpell("cloak of shadows", bot) &&
                botAI->CastSpell("cloak of shadows", bot);

        default:
            return false;
    }
}

bool KalecgosSathrovarrTankStandWithKalecAction::Execute(Event /*event*/)
{
    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr)
        return false;

    constexpr float searchRadius = 20.0f;
    Unit* kalec = bot->FindNearestCreature(Id(SwpNpcs::NPC_KALECGOS_HUMANOID), searchRadius);
    if (!kalec || sathrovarr->GetVictim() != kalec)
        return false;

    Position const position = kalec->GetPosition();
    if (bot->GetExactDist2d(position) <= 3.0f)
        return false;

    return MoveTo(
        SWP_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
        false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool KalecgosReturnToSpectralRealmGroundAction::Execute(Event /*event*/)
{
    return bot->TeleportTo(
        SWP_MAP_ID, bot->GetPositionX(), bot->GetPositionY(),
        KALECGOS_SPECTRAL_REALM_Z, bot->GetOrientation());
}
