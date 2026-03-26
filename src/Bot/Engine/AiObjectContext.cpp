/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AiObjectContext.h"

#include "AzjolNerubTriggerContext.h"
#include "ChatTriggerContext.h"
#include "CullingOfStratholmeTriggerContext.h"
#include "DKAiObjectContext.h"
#include "DrakTharonKeepTriggerContext.h"
#include "DruidAiObjectContext.h"
#include "ForgeOfSoulsTriggerContext.h"
#include "GundrakTriggerContext.h"
#include "HallsOfLightningTriggerContext.h"
#include "HallsOfStoneTriggerContext.h"
#include "HunterAiObjectContext.h"
#include "MageAiObjectContext.h"
#include "NexusTriggerContext.h"
#include "OculusTriggerContext.h"
#include "OldKingdomTriggerContext.h"
#include "PaladinAiObjectContext.h"
#include "PitOfSaronTriggerContext.h"
#include "Playerbots.h"
#include "PriestAiObjectContext.h"
#include "RogueAiObjectContext.h"
#include "ShamanAiObjectContext.h"
#include "StrategyContext.h"
#include "TrialOfTheChampionTriggerContext.h"
#include "TriggerContext.h"
#include "UtgardeKeepTriggerContext.h"
#include "UtgardePinnacleTriggerContext.h"
#include "ValueContext.h"
#include "VioletHoldTriggerContext.h"
#include "WarlockAiObjectContext.h"
#include "WarriorAiObjectContext.h"
#include "WorldPacketTriggerContext.h"
#include "Ai/Dungeon/DungeonStrategyContext.h"
#include "Ai/Raid/RaidStrategyContext.h"
#include "Ai/Raid/Aq20/RaidAq20TriggerContext.h"
#include "Ai/Raid/MoltenCore/RaidMcTriggerContext.h"
#include "Ai/Raid/BlackwingLair/RaidBwlTriggerContext.h"
#include "Ai/Raid/Karazhan/RaidKarazhanTriggerContext.h"
#include "Ai/Raid/Magtheridon/RaidMagtheridonTriggerContext.h"
#include "Ai/Raid/GruulsLair/RaidGruulsLairTriggerContext.h"
#include "Ai/Raid/SerpentshrineCavern/RaidSSCTriggerContext.h"
#include "Ai/Raid/GruulsLair/RaidGruulsLairTriggerContext.h"
#include "Ai/Raid/Magtheridon/RaidMagtheridonTriggerContext.h"
#include "Ai/Raid/Naxxramas/RaidNaxxTriggerContext.h"
#include "Ai/Raid/SerpentshrineCavern/RaidSSCTriggerContext.h"
#include "Ai/Raid/TempestKeep/RaidTempestKeepTriggerContext.h"
#include "Ai/Raid/EyeOfEternity/RaidEoETriggerContext.h"
#include "Ai/Raid/VaultOfArchavon/RaidVoATriggerContext.h"
#include "Ai/Raid/ObsidianSanctum/RaidOsTriggerContext.h"
#include "Ai/Raid/Ulduar/RaidUlduarTriggerContext.h"
#include "Ai/Raid/Onyxia/RaidOnyxiaTriggerContext.h"
#include "Ai/Raid/Icecrown/RaidIccTriggerContext.h"

SharedNamedObjectContextList<Strategy> AiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> AiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> AiObjectContext::sharedValueContexts;

AiObjectContext::AiObjectContext(PlayerbotAI* botAI, SharedNamedObjectContextList<Strategy>& sharedStrategyContext,
                                 SharedNamedObjectContextList<Trigger>& sharedTriggerContext,
                                 SharedNamedObjectContextList<UntypedValue>& sharedValueContext)
    : PlayerbotAIAware(botAI),
      strategyContexts(sharedStrategyContext),
      triggerContexts(sharedTriggerContext),
      valueContexts(sharedValueContext)
{
}

void AiObjectContext::BuildAllSharedContexts()
{
    AiObjectContext::BuildSharedContexts();
    PriestAiObjectContext::BuildSharedContexts();
    MageAiObjectContext::BuildSharedContexts();
    WarlockAiObjectContext::BuildSharedContexts();
    WarriorAiObjectContext::BuildSharedContexts();
    ShamanAiObjectContext::BuildSharedContexts();
    PaladinAiObjectContext::BuildSharedContexts();
    DruidAiObjectContext::BuildSharedContexts();
    HunterAiObjectContext::BuildSharedContexts();
    RogueAiObjectContext::BuildSharedContexts();
    DKAiObjectContext::BuildSharedContexts();
}

void AiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void AiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    strategyContexts.Add(new StrategyContext());
    strategyContexts.Add(new MovementStrategyContext());
    strategyContexts.Add(new AssistStrategyContext());
    strategyContexts.Add(new QuestStrategyContext());
    strategyContexts.Add(new DungeonStrategyContext());
    strategyContexts.Add(new RaidStrategyContext());
}

void AiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());
    triggerContexts.Add(new RaidAq20TriggerContext());
    triggerContexts.Add(new RaidMcTriggerContext());
    triggerContexts.Add(new RaidBwlTriggerContext());
    triggerContexts.Add(new RaidKarazhanTriggerContext());
    triggerContexts.Add(new RaidGruulsLairTriggerContext());
    triggerContexts.Add(new RaidMagtheridonTriggerContext());
    triggerContexts.Add(new RaidNaxxTriggerContext());
    triggerContexts.Add(new RaidSSCTriggerContext());
    triggerContexts.Add(new RaidTempestKeepTriggerContext());
    triggerContexts.Add(new RaidOsTriggerContext());
    triggerContexts.Add(new RaidEoETriggerContext());
    triggerContexts.Add(new RaidVoATriggerContext());
    triggerContexts.Add(new RaidUlduarTriggerContext());
    triggerContexts.Add(new RaidOnyxiaTriggerContext());
    triggerContexts.Add(new RaidIccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUKTriggerContext());
    triggerContexts.Add(new WotlkDungeonNexTriggerContext());
    triggerContexts.Add(new WotlkDungeonANTriggerContext());
    triggerContexts.Add(new WotlkDungeonOKTriggerContext());
    triggerContexts.Add(new WotlkDungeonDTKTriggerContext());
    triggerContexts.Add(new WotlkDungeonVHTriggerContext());
    triggerContexts.Add(new WotlkDungeonGDTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonHoLTriggerContext());
    triggerContexts.Add(new WotlkDungeonOccTriggerContext());
    triggerContexts.Add(new WotlkDungeonUPTriggerContext());
    triggerContexts.Add(new WotlkDungeonCoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonFoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonPoSTriggerContext());
    triggerContexts.Add(new WotlkDungeonToCTriggerContext());
}

void AiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new ValueContext());
}

std::vector<std::string> AiObjectContext::Save()
{
    std::vector<std::string> result;

    std::set<std::string> names = valueContexts.GetCreated();
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        std::string const data = value->Save();
        if (data == "?")
            continue;

        std::string const name = *i;
        std::ostringstream out;
        out << name;

        out << ">" << data;
        result.push_back(out.str());
    }

    return result;
}

void AiObjectContext::Load(std::vector<std::string> data)
{
    for (std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        std::string const row = *i;
        std::vector<std::string> parts = split(row, '>');
        if (parts.size() != 2)
            continue;

        std::string const name = parts[0];
        std::string const text = parts[1];

        UntypedValue* value = GetUntypedValue(name);
        if (!value)
            continue;

        value->Load(text);
    }
}

Strategy* AiObjectContext::GetStrategy(std::string const name)
{
    return strategyContexts.GetContextObject(name, botAI);
}

std::set<std::string> AiObjectContext::GetSiblingStrategy(std::string const name)
{
    return strategyContexts.GetSiblings(name);
}

Trigger* AiObjectContext::GetTrigger(std::string const name) { return triggerContexts.GetContextObject(name, botAI); }

UntypedValue* AiObjectContext::GetUntypedValue(std::string const name)
{
    return valueContexts.GetContextObject(name, botAI);
}

std::set<std::string> AiObjectContext::GetValues() { return valueContexts.GetCreated(); }

std::set<std::string> AiObjectContext::GetSupportedStrategies() { return strategyContexts.supports(); }

std::string const AiObjectContext::FormatValues()
{
    std::ostringstream out;
    std::set<std::string> names = valueContexts.GetCreated();
    for (std::set<std::string>::iterator i = names.begin(); i != names.end(); ++i, out << "|")
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        std::string const text = value->Format();
        if (text == "?")
            continue;

        out << "{" << *i << "=" << text << "}";
    }

    return out.str();
}
