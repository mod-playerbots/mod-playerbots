#include "GruulTriggers.h"
#include "GruulHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "high king maulgar");
}

bool HighKingMaulgarOlmEngagedByFirstAssistTankTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, false) &&
           AI_VALUE2(Unit*, "find target", "olm the summoner");
}

bool HighKingMaulgarBlindeyeEngagedBySecondAssistTankTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 1, false) &&
           AI_VALUE2(Unit*, "find target", "blindeye the seer");
}

bool HighKingMaulgarKroshEngagedByMageTankTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE &&
           AI_VALUE2(Unit*, "find target", "krosh firehand") &&
           GetKroshMageTank(bot) == bot;
}

bool HighKingMaulgarKigglerEngagedByMoonkinTankTrigger::IsActive()
{
    return bot->getClass() == CLASS_DRUID &&
           AI_VALUE2(Unit*, "find target", "kiggler the crazed") &&
           GetKigglerMoonkinTank(bot) == bot;
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) || botAI->IsMainTank(bot))
        return false;

    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar)
        return false;

    if (botAI->IsAssistTankOfIndex(bot, 0, false))
        return !AI_VALUE2(Unit*, "find target", "olm the summoner");

    if (botAI->IsAssistTankOfIndex(bot, 1, false))
        return !AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (bot->getClass() == CLASS_MAGE && GetKroshMageTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "krosh firehand");

    if (bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(bot) == bot)
        return !AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return true;
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar ||
        !maulgar->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_WHIRLWIND)))
    {
        return false;
    }

    return !botAI->IsMainTank(bot);
}

bool HighKingMaulgarKroshCastsBlastWaveTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "krosh firehand"))
        return false;

    return !botAI->IsTank(bot) && GetKroshMageTank(bot) != bot;
}

bool HighKingMaulgarWildFelStalkerSpawnedTrigger::IsActive()
{
    return bot->getClass() == CLASS_WARLOCK &&
           AI_VALUE2(Unit*, "find target", "wild fel stalker");
}

bool HighKingMaulgarPullingOgreCouncilTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    return blindeye && blindeye->GetHealthPct() > 80.0f;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossEngagedByTanksTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerBossEngagedByRangedTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_GROUND_SLAM_1)) ||
           bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_GROUND_SLAM_2));
}
