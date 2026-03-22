/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidHyjalSummitTriggers.h"
#include "RaidHyjalSummitHelpers.h"
#include "RaidHyjalSummitActions.h"
#include "AiFactory.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace HyjalSummitHelpers;

// General

bool HyjalSummitBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
}

// Rage Winterchill

bool RageWinterchillPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* winterchill = AI_VALUE2(Unit*, "find target", "rage winterchill");
    return winterchill && winterchill->GetHealthPct() > 95.0f;
}

bool RageWinterchillBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "rage winterchill");
}

bool RageWinterchillBossCastsDeathAndDecayTrigger::IsActive()
{
    return botAI->IsRanged(bot) &&
           AI_VALUE2(Unit*, "find target", "rage winterchill");
}

// Anetheron

bool AnetheronPullingBossOrInfernalTrigger::IsActive()
{
    return bot->getClass() == CLASS_HUNTER &&
           AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "anetheron");
}

bool AnetheronBossCastsCarrionSwarmTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    return GetInfernoTarget(anetheron) != bot;
}

bool AnetheronBotIsTargetedByInfernalTrigger::IsActive()
{
    Unit* anetheron = AI_VALUE2(Unit*, "find target", "anetheron");
    if (!anetheron)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    return GetInfernoTarget(anetheron) == bot;
}

bool AnetheronInfernalsNeedToBeKeptAwayFromRaidTrigger::IsActive()
{
    return botAI->IsAssistTankOfIndex(bot, 0, true) &&
           AI_VALUE2(Unit*, "find target", "towering infernal");
}

bool AnetheronInfernalsContinueToSpawnTrigger::IsActive()
{
    return !botAI->IsTank(bot) &&
           AI_VALUE2(Unit*, "find target", "anetheron");
}

// Kaz'rogal

bool KazrogalPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kazrogal = AI_VALUE2(Unit*, "find target", "kaz'rogal");
    return kazrogal && kazrogal->GetHealthPct() > 95.0f;
}

bool KazrogalBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "kaz'rogal");
}

bool KazrogalBossEngagedByAssistTanksTrigger::IsActive()
{
    if (!botAI->IsAssistTank(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    return bot->GetPower(POWER_MANA) > 3000;
}

bool KazrogalLowManaBotsNeedEscapePathTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
        return false;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        return true;
    }
    else if (bot->GetPower(POWER_MANA) > 4000)
    {
        isBelowManaThreshold.erase(bot->GetGUID());
        if (botAI->IsMelee(bot))
            return false;
        else
            return true;
    }

    return false;
}

bool KazrogalBotIsLowOnManaTrigger::IsActive()
{
    if (bot->getClass() == CLASS_WARRIOR ||
        bot->getClass() == CLASS_ROGUE ||
        bot->getClass() == CLASS_DEATH_KNIGHT)
        return false;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);
    if (bot->getClass() == CLASS_DRUID && tab == DRUID_TAB_FERAL)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (botAI->HasAnyAuraOf(bot, "ice block", "divine shield", nullptr))
        return false;

    if (isBelowManaThreshold.count(bot->GetGUID()) ||
        bot->GetPower(POWER_MANA) <= 3200)
        return true;

    return false;
}

bool KazrogalMarkDealsShadowDamageTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PALADIN &&
        bot->getClass() != CLASS_WARLOCK)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kaz'rogal"))
        return false;

    if (bot->getClass() == CLASS_PALADIN &&
        (botAI->HasAura("shadow resistance aura", bot) ||
         botAI->HasAura("prayer of shadow protection", bot) ||
         botAI->HasAura("shadow protection", bot)))
        return false;

    return bot->HasAura(
        static_cast<uint32>(HyjalSummitSpells::SPELL_MARK_OF_KAZROGAL));
}

// Azgalor

bool AzgalorPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    return azgalor && azgalor->GetHealthPct() > 95.0f;
}

bool AzgalorBossEngagedByMainTankTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "azgalor");
}

bool AzgalorMainTankIsPositioningBossTrigger::IsActive()
{
    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor || azgalor->GetHealthPct() < 85.0f ||
        azgalor->GetVictim() == bot)
        return false;

    if (botAI->IsMainTank(bot))
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (!GET_PLAYERBOT_AI(mainTank))
    {
        if (botAI->IsMelee(bot) && azgalor->GetHealthPct() > 95.0f)
            return true;
        else
            return false;
    }
    else if (botAI->IsMelee(bot))
    {
        return GetAzgalorTankStep(botAI, bot) < 2;
    }
    else
    {
        return GetAzgalorTankStep(botAI, bot) < 1;
    }
}

// Spread to mitigate Rain of Fire, but GTFO if Rain of Fire is on the bot
bool AzgalorBossCastsRainOfFireTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* azgalor = AI_VALUE2(Unit*, "find target", "azgalor");
    if (!azgalor ||
        bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOM)))
        return false;

    if (azgalor->GetHealthPct() < 85.0f)
        return true;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && GET_PLAYERBOT_AI(mainTank))
        return GetAzgalorTankStep(botAI, bot) == 2;
    else
        return true;
}

bool AzgalorBotIsDoomedTrigger::IsActive()
{
    return bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOM));
}

bool AzgalorDoomguardsMustBeControlledTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, true) &&
        !botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    // Exclude second assist tank also, unless first assist tank has Doom
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    if (firstAssistTank &&
        !firstAssistTank->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOM)) &&
        botAI->IsAssistTankOfIndex(bot, 1, true))
        return false;

    return AI_VALUE2(Unit*, "find target", "lesser doomguard") ||
           AnyGroupMemberHasDoom(bot);
}

bool AzgalorDoomguardsContinueToSpawnTrigger::IsActive()
{
    return botAI->IsDps(bot) && AI_VALUE2(Unit*, "find target", "azgalor");
}

// Archimonde

bool ArchimondePullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 95.0f;
}

bool ArchimondeBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 90.0f;
}

bool ArchimondeBossCastsFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST &&
        bot->getClass() != CLASS_SHAMAN)
        return false;

    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    return archimonde && archimonde->GetHealthPct() > 10.0f;
}

bool ArchimondeBossCastsAirBurstTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() <= 10.0f ||
        archimonde->GetVictim() == bot)
        return false;

    return !botAI->IsMainTank(bot);
}

bool ArchimondeBossSummonedDoomfireTrigger::IsActive()
{
    Unit* archimonde = AI_VALUE2(Unit*, "find target", "archimonde");
    if (!archimonde || archimonde->GetHealthPct() <= 10.0f)
        return false;

    if (bot->GetExactDist2d(archimonde) <= 0.0f)
        return false;

    // If I don't make an exception, bots actually refuse to enter the
    // Doomfire even when feared
    return !bot->HasAura(
        static_cast<uint32>(HyjalSummitSpells::SPELL_ARCHIMONDE_FEAR));
}

bool ArchimondeBotStoodInDoomfireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_ROGUE &&
        bot->getClass() != CLASS_PALADIN)
        return false;

    return bot->GetHealthPct() < 40.0f &&
           (bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOMFIRE)) ||
            bot->HasAura(static_cast<uint32>(HyjalSummitSpells::SPELL_DOOMFIRE_DOT)));
}
