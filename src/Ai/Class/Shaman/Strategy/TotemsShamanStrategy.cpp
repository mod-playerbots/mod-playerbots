/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TotemsShamanStrategy.h"
#include "CreateNextAction.h"
#include "ShamanActions.h"

// These combat strategies are used to set the corresponding totems on the bar, and cast the totem when it's missing.
// There are special cases for Totem of Wrath, Windfury Totem, Wrath of Air totem, and Cleansing totem - these totems
// aren't learned at level 30, and have fallbacks in order to prevent the trigger from continuously firing.

// Earth Totems
StrengthOfEarthTotemStrategy::StrengthOfEarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StrengthOfEarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set strength of earth totem",
            {
                CreateNextAction<SetStrengthOfEarthTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no earth totem",
            {
                CreateNextAction<CastStrengthOfEarthTotemAction>(55.0f)
            }
        )
    );
}

StoneclawTotemStrategy::StoneclawTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void StoneclawTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set stoneskin totem",
            {
                CreateNextAction<SetStoneskinTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no earth totem",
            {
                CreateNextAction<CastStoneskinTotemAction>(55.0f)
            }
        )
    );
}

EarthTotemStrategy::EarthTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set tremor totem",
            {
                CreateNextAction<SetTremorTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no earth totem",
            {
                CreateNextAction<CastTremorTotemAction>(55.0f)
            }
        )
    );
}

EarthbindTotemStrategy::EarthbindTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void EarthbindTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set earthbind totem",
            {
                CreateNextAction<SetEarthbindTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no earth totem",
            {
                CreateNextAction<CastEarthbindTotemAction>(55.0f)
            }
        )
    );
}

// Fire Totems
SearingTotemStrategy::SearingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void SearingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set searing totem",
            {
                CreateNextAction<SetSearingTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no fire totem",
            {
                CreateNextAction<CastSearingTotemAction>(55.0f)
            }
        )
    );
}

MagmaTotemStrategy::MagmaTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void MagmaTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set magma totem",
            {
                CreateNextAction<SetMagmaTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no fire totem",
            {
                CreateNextAction<CastMagmaTotemAction>(55.0f)
            }
        )
    );
}

FlametongueTotemStrategy::FlametongueTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FlametongueTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set flametongue totem",
            {
                CreateNextAction<SetFlametongueTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no fire totem",
            {
                CreateNextAction<CastFlametongueTotemAction>(55.0f)
            }
        )
    );
}

TotemOfWrathStrategy::TotemOfWrathStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void TotemOfWrathStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Totem of Wrath yet, set Flametongue Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(30706))
    {
        triggers.push_back(
            new TriggerNode(
                "set totem of wrath",
                {
                    CreateNextAction<SetTotemOfWrathAction>(60.0f)
                }
            )
        );
    }
    else if (bot->HasSpell(8227))
    {
        triggers.push_back(
            new TriggerNode(
                "set flametongue totem",
                {
                    CreateNextAction<SetFlametongueTotemAction>(60.0f)
                }
            )
        );
    }
    triggers.push_back(
        new TriggerNode(
            "no fire totem",
            {
                CreateNextAction<CastTotemOfWrathAction>(55.0f)
            }
        )
    );
}

FrostResistanceTotemStrategy::FrostResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FrostResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set frost resistance totem",
            {
                CreateNextAction<SetFrostResistanceTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no fire totem",
            {
                CreateNextAction<CastFrostResistanceTotemAction>(55.0f)
            }
        )
    );
}

// Water Totems
HealingStreamTotemStrategy::HealingStreamTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void HealingStreamTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set healing stream totem",
            {
                CreateNextAction<SetHealingStreamTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no water totem",
            {
                CreateNextAction<CastHealingStreamTotemAction>(55.0f)
            }
        )
    );
}

ManaSpringTotemStrategy::ManaSpringTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void ManaSpringTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set mana spring totem",
            {
                CreateNextAction<SetManaSpringTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no water totem",
            {
                CreateNextAction<CastManaSpringTotemAction>(55.0f)
            }
        )
    );
}

CleansingTotemStrategy::CleansingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void CleansingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Cleansing Totem yet, set Mana Spring Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8170))
    {
        triggers.push_back(
            new TriggerNode(
                "set cleansing totem",
                {
                    CreateNextAction<SetCleansingTotemAction>(60.0f)
                }
            )
        );
    }
    else if (bot->HasSpell(5675))
    {
        triggers.push_back(
            new TriggerNode(
                "set mana spring totem",
                {
                    CreateNextAction<SetManaSpringTotemAction>(60.0f)
                }
            )
        );
    }
    triggers.push_back(
        new TriggerNode(
            "no water totem",
            {
                CreateNextAction<CastCleansingTotemAction>(55.0f)
            }
        )
    );
}

FireResistanceTotemStrategy::FireResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void FireResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set fire resistance totem",
            {
                CreateNextAction<SetFireResistanceTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no water totem",
            {
                CreateNextAction<CastFireResistanceTotemAction>(55.0f)
            }
        )
    );
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
        triggers.push_back(
            new TriggerNode(
                "set wrath of air totem",
                {
                    CreateNextAction<SetWrathOfAirTotemAction>(60.0f)
                }
            )
        );
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(
            new TriggerNode(
                "set grounding totem",
                {
                    CreateNextAction<SetGroundingTotemAction>(60.0f)
                }
            )
        );
    }
    triggers.push_back(
        new TriggerNode(
            "no air totem",
            {
                CreateNextAction<CastWrathOfAirTotemAction>(55.0f)
            }
        )
    );
}

WindfuryTotemStrategy::WindfuryTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void WindfuryTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    // If the bot hasn't learned Windfury Totem yet, set Grounding Totem instead.
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(8512))
    {
        triggers.push_back(
            new TriggerNode(
                "set windfury totem",
                {
                    CreateNextAction<SetWindfuryTotemAction>(60.0f)
                }
            )
        );
    }
    else if (bot->HasSpell(8177))
    {
        triggers.push_back(
            new TriggerNode(
                "set grounding totem",
                {
                    CreateNextAction<SetGroundingTotemAction>(60.0f)
                }
            )
        );
    }
    triggers.push_back(
        new TriggerNode(
            "no air totem",
            {
                CreateNextAction<CastWindfuryTotemAction>(55.0f)
            }
        )
    );
}

NatureResistanceTotemStrategy::NatureResistanceTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void NatureResistanceTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set nature resistance totem",
            {
                CreateNextAction<SetNatureResistanceTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no air totem",
            {
                CreateNextAction<CastNatureResistanceTotemAction>(55.0f)
            }
        )
    );
}

GroundingTotemStrategy::GroundingTotemStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI) {}
void GroundingTotemStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);
    triggers.push_back(
        new TriggerNode(
            "set grounding totem",
            {
                CreateNextAction<SetGroundingTotemAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no air totem",
            {
                CreateNextAction<SetGroundingTotemAction>(55.0f)
            }
        )
    );
}
