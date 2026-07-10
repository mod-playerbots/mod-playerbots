/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "MagTriggers.h"
#include "MagHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace MagtheridonHelpers;

bool MagtheridonFirstThreeChannelersEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && !IsMagtheridonActive(magtheridon);
}

bool MagtheridonNWChannelerEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, false))
        return false;

    return AI_VALUE2(Unit*, "find target", "magtheridon") &&
           GetChanneler(bot, NORTHWEST_CHANNELER);
}

bool MagtheridonNEChannelerEngagedBySecondAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "magtheridon") &&
           GetChanneler(bot, NORTHEAST_CHANNELER);
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return GetChanneler(bot, WEST_CHANNELER) || GetChanneler(bot, EAST_CHANNELER);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || !AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return !botAI->IsMainTank(bot) &&
           !(botAI->IsAssistTankOfIndex(bot, 0, false) && channelerDiamond) &&
           !(botAI->IsAssistTankOfIndex(bot, 1, true) && channelerTriangle);
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK &&
           AI_VALUE2(Unit*, "find target", "burning abyssal");
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    // Include an assist tank that pulls aggro
    return botAI->IsMainTank(bot) || magtheridon->GetVictim() == bot;
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || !IsMagtheridonActive(magtheridon))
        return false;

    constexpr uint8 dpsWaitSeconds = 6;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it == dpsWaitTimer.end() || (time(nullptr) - it->second) < dpsWaitSeconds)
        return false;

    return magtheridon->GetVictim() != bot;
}

bool MagtheridonStandingInDebrisTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "magtheridon"))
        return false;

    return IsPositionInActiveDebris(
        bot->GetMap()->GetInstanceId(), bot->GetPositionX(), bot->GetPositionY());
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    return magtheridon && IsMagtheridonActive(magtheridon) && IsCubeClicker(bot);
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    return IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr) &&
           AI_VALUE2(Unit*, "find target", "magtheridon");
}

bool MagtheridonBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat() && bot->GetMapId() == MAGTHERIDON_MAP_ID &&
           !AI_VALUE2(Unit*, "find target", "magtheridon") &&
           !AI_VALUE2(Unit*, "find target", "hellfire channeler");
}
