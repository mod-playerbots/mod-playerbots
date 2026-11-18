/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_FactionChampionsBall.h"

#include "Playerbots.h"
#include "RaidDirectTargeting.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsBall.h"
#include "ToCHelpers_FactionChampionsDefence.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
constexpr float STAND_TRIGGER = 3.0f;
}  // namespace

// While our breakable CC holds its champion, until the DK has stopped swinging at it
bool FactionChampionsIceTargetTrigger::IsActive()
{
    Unit* champion = FactionChampionsIceTarget(botAI);
    if (!champion)
        return false;

    if (FactionChampionsCcProtected(botAI, champion))
    {
        Pet* pet = bot->GetPet();
        return bot->GetVictim() || !bot->GetTarget().IsEmpty() || AI_VALUE(Unit*, "current target") ||
               (pet && pet->GetVictim());
    }

    return AI_VALUE(Unit*, "current target") != champion || RaidPetOffPick(botAI, champion);
}

bool FactionChampionsChainsOfIceTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsIceChains(botAI, spellId) != nullptr;
}

bool FactionChampionsIceReachTrigger::IsActive()
{
    Position stand;
    return botAI->CanMove() && FactionChampionsIceReach(botAI, stand);
}

// Not inside a Bladestorm or Hellfire, the dodge walks out first
bool FactionChampionsCycloneStandTrigger::IsActive()
{
    Position spot;
    return FactionChampionsCycloneStand(botAI, spot) && botAI->CanMove() && !FactionChampionsInAoe(botAI) &&
           bot->GetExactDist2d(&spot) > STAND_TRIGGER;
}

bool FactionChampionsStepOffTrigger::IsActive()
{
    Position spot;
    return botAI->CanMove() && !FactionChampionsInAoe(botAI) && FactionChampionsStepOff(botAI, spot);
}

bool FactionChampionsChasedTrigger::IsActive()
{
    Position spot;
    return botAI->CanMove() && FactionChampionsChased(botAI, spot);
}

bool FactionChampionsKiteTrigger::IsActive()
{
    Position spot;
    return botAI->CanMove() && FactionChampionsKite(botAI, spot);
}

bool FactionChampionsSpeedBoostTrigger::IsActive()
{
    return botAI->CanMove() && FactionChampionsSpeedBoost(botAI);
}

bool FactionChampionsOpenerTrigger::IsActive()
{
    uint32 spellId = 0;
    return FactionChampionsOpener(botAI, spellId) != nullptr;
}

bool FactionChampionsEscapeTrigger::IsActive()
{
    FactionChampionsEscapeCast cast;
    return FactionChampionsEscape(botAI, cast);
}

bool FactionChampionsDemonicCircleTrigger::IsActive() { return FactionChampionsCircleDue(botAI); }
