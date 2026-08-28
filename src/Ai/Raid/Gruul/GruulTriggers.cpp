/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "InstanceScript.h"
#include "Playerbots.h"

using namespace GruulHelpers;

// General

bool GruulsLairNoEncounterInProgress::IsActive()
{
    if (bot->GetMapId() != GRUUL_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// High King Maulgar

bool HighKingMaulgarThreeOgresNeedMeleeTanksTrigger::IsActive()
{
    if (IsBlindeyeTank(bot))
        return AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsOlmTank(bot))
        return AI_VALUE2(Unit*, "find target", "olm the summoner");

    return IsMaulgarTank(bot) && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarKroshNeedsMageTankTrigger::IsActive()
{
    return IsKroshMageTank(bot) && AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarKigglerNeedsMoonkinTankTrigger::IsActive()
{
    return IsKigglerMoonkinTank(bot) && AI_VALUE2(Unit*, "find target", "kiggler the crazed");
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return false;

    if (IsMaulgarTank(bot))
        return false;

    if (IsOlmTank(bot))
        return !AI_VALUE2(Unit*, "find target", "olm the summoner");

    if (IsBlindeyeTank(bot))
        return !AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (IsKroshMageTank(bot))
        return !AI_VALUE2(Unit*, "find target", "krosh firehand");

    if (IsKigglerMoonkinTank(bot))
        return !AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return true;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar || !maulgar->HasAura(Id(GruulSpells::SPELL_WHIRLWIND)))
        return false;

    return !IsMaulgarTank(bot);
}

bool HighKingMaulgarKroshCastsBlastWaveTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot) || IsKroshMageTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "krosh firehand");
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK && AI_VALUE2(Unit*, "find target", "wild fel stalker");
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > BLINDEYE_PULL_COMPLETE_HP_PERCENT;
}

bool HighKingMaulgarBossCastsIntimidatingRoarTrigger::IsActive()
{
    return bot->getClass() == CLASS_PRIEST && AI_VALUE2(Unit*, "find target", "high king maulgar");
}

// Gruul the Dragonkiller

bool GruulTheDragonkillerShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerRangedShouldSpreadTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return HasGroundSlam(bot);
}
