/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinBuffStrategies.h"
#include "CreateNextAction.h"
#include "PaladinActions.h"

void PaladinBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blessing of wisdom on party",
            {
                CreateNextAction<CastBlessingOfWisdomOnPartyAction>(11.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "blessing of kings on party",
            {
                CreateNextAction<CastBlessingOfKingsOnPartyAction>(10.5f)
            }
        )
    );
}

void PaladinBuffHealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blessing of sanctuary on party",
            {
                CreateNextAction<CastBlessingOfSanctuaryOnPartyAction>(11.0f)
            }
        )
    );
}

void PaladinBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blessing of might on party",
            {
                CreateNextAction<CastBlessingOfMightOnPartyAction>(11.0f)
            }
        )
    );
}

void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "shadow resistance aura",
            {
                CreateNextAction<CastShadowResistanceAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "frost resistance aura",
            {
                CreateNextAction<CastFrostResistanceAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "fire resistance aura",
            {
                CreateNextAction<CastFireResistanceAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "devotion aura",
            {
                CreateNextAction<CastDevotionAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "retribution aura",
            {
                CreateNextAction<CastRetributionAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinBuffCastStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "concentration aura",
            {
                CreateNextAction<CastConcentrationAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "crusader aura",
            {
                CreateNextAction<CastCrusaderAuraAction>(ACTION_NORMAL)
            }
        )
    );
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "righteous fury",
            {
                CreateNextAction<CastRighteousFuryAction>(ACTION_HIGH + 8.0f)
            }
        )
    );
}

void PaladinBuffStatsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // First Sanctuary (prio > Kings)
    triggers.push_back(
        new TriggerNode(
            "blessing of sanctuary on party",
            {
                CreateNextAction<CastBlessingOfSanctuaryOnPartyAction>(12.0f)
            }
        )
    );

    // After Kings
    triggers.push_back(
        new TriggerNode(
            "blessing of kings on party",
            {
                CreateNextAction<CastBlessingOfKingsOnPartyAction>(11.0f)
            }
        )
    );
}
