/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"
#include "Playerbots.h"

// These combat strategies are used to set the corresponding totems on the bar, and cast the totem when it's missing.
// There are special cases for Totem of Wrath, Windfury Totem, Wrath of Air totem, and Cleansing totem - these totems
// aren't learned at level 30, and have fallbacks in order to prevent the trigger from continuously firing.

// Earth Totems
StrengthOfEarthTotemStrategy::StrengthOfEarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StrengthOfEarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set strength of earth totem", { NextAction("set strength of earth totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no earth totem", { NextAction("strength of earth totem", 55.0f) }));
}

StoneclawTotemStrategy::StoneclawTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StoneclawTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set stoneskin totem", { NextAction("set stoneskin totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no earth totem", { NextAction("stoneskin totem", 55.0f) }));
}

EarthTotemStrategy::EarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set tremor totem", { NextAction("set tremor totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no earth totem", { NextAction("tremor totem", 55.0f) }));
}

EarthbindTotemStrategy::EarthbindTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthbindTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set earthbind totem", { NextAction("set earthbind totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no earth totem", { NextAction("earthbind totem", 55.0f) }));
}

// Fire Totems
SearingTotemStrategy::SearingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void SearingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set searing totem", { NextAction("set searing totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no fire totem", { NextAction("searing totem", 55.0f) }));
}

MagmaTotemStrategy::MagmaTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void MagmaTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set magma totem", { NextAction("set magma totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no fire totem", { NextAction("magma totem", 55.0f) }));
}

FlametongueTotemStrategy::FlametongueTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FlametongueTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set flametongue totem", { NextAction("set flametongue totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no fire totem", { NextAction("flametongue totem", 55.0f) }));
}

TotemOfWrathStrategy::TotemOfWrathStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void TotemOfWrathStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Totem of Wrath yet, set Flametongue Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(30706))
    {
        triggers.push_back(new TriggerNode("set totem of wrath", { NextAction("set totem of wrath", 60.0f) }));
    }
    else if (bot->HasSpell(8227))
    {
        triggers.push_back(new TriggerNode("set flametongue totem", { NextAction("set flametongue totem", 60.0f) }));
    }
    triggers.push_back(new TriggerNode("no fire totem", { NextAction("totem of wrath", 55.0f) }));
}

FrostResistanceTotemStrategy::FrostResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FrostResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set frost resistance totem", { NextAction("set frost resistance totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no fire totem", { NextAction("frost resistance totem", 55.0f) }));
}

// Water Totems
HealingStreamTotemStrategy::HealingStreamTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void HealingStreamTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set healing stream totem", { NextAction("set healing stream totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no water totem", { NextAction("healing stream totem", 55.0f) }));
}

ManaSpringTotemStrategy::ManaSpringTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void ManaSpringTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set mana spring totem", { NextAction("set mana spring totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no water totem", { NextAction("mana spring totem", 55.0f) }));
}

CleansingTotemStrategy::CleansingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void CleansingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Cleansing Totem yet, set Mana Spring Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8170))
    {
        triggers.push_back(new TriggerNode("set cleansing totem", { NextAction("set cleansing totem", 60.0f) }));
    }
    else if (bot->HasSpell(5675))
    {
        triggers.push_back(new TriggerNode("set mana spring totem", { NextAction("set mana spring totem", 60.0f) }));
    }
    triggers.push_back(new TriggerNode("no water totem", { NextAction("cleansing totem", 55.0f) }));
}

FireResistanceTotemStrategy::FireResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FireResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set fire resistance totem", { NextAction("set fire resistance totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no water totem", { NextAction("fire resistance totem", 55.0f) }));
}

// Air Totems
WrathOfAirTotemStrategy::WrathOfAirTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WrathOfAirTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Wrath of Air Totem yet, set Grounding Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(3738))
    {
        triggers.push_back(new TriggerNode("set wrath of air totem", { NextAction("set wrath of air totem", 60.0f) }));
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(new TriggerNode("set grounding totem", { NextAction("set grounding totem", 60.0f) }));
    }
    triggers.push_back(
        new TriggerNode("no air totem", { NextAction("wrath of air totem", 55.0f) }));
}

WindfuryTotemStrategy::WindfuryTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WindfuryTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Windfury Totem yet, set Grounding Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8512))
    {
        triggers.push_back(new TriggerNode("set windfury totem", { NextAction("set windfury totem", 60.0f) }));
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(new TriggerNode("set grounding totem", { NextAction("set grounding totem", 60.0f) }));
    }
    triggers.push_back(new TriggerNode("no air totem", { NextAction("windfury totem", 55.0f) }));
}

NatureResistanceTotemStrategy::NatureResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void NatureResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set nature resistance totem", { NextAction("set nature resistance totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no air totem", { NextAction("nature resistance totem", 55.0f) }));
}

GroundingTotemStrategy::GroundingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void GroundingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode("set grounding totem", { NextAction("set grounding totem", 60.0f) }));
    triggers.push_back(new TriggerNode("no air totem", { NextAction("grounding totem", 55.0f) }));
}
