/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinBuffStrategies.h"

#include "Playerbots.h"

void PaladinBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("blessing of wisdom on party",
        { NextAction("blessing of wisdom on party", 11.0f) }));

    triggers.push_back(new TriggerNode("blessing of kings on party",
        { NextAction("blessing of kings on party", 10.5f) }));
}

void PaladinBuffHealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of sanctuary on party",
                        { NextAction("blessing of sanctuary on party", 11.0f) }));
}

void PaladinBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of might on party",
                        { NextAction("blessing of might on party", 11.0f) }));
}

void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("shadow resistance aura",
                        { NextAction("shadow resistance aura", ACTION_NORMAL) }));
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("frost resistance aura",
                        { NextAction("frost resistance aura", ACTION_NORMAL) }));
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "fire resistance aura", { NextAction("fire resistance aura", ACTION_NORMAL) }));
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("devotion aura",
                                       { NextAction("devotion aura", ACTION_NORMAL) }));
}

void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "retribution aura", { NextAction("retribution aura", ACTION_NORMAL) }));
}

void PaladinBuffCastStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "concentration aura", { NextAction("concentration aura", ACTION_NORMAL) }));
}

void PaladinBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "crusader aura", { NextAction("crusader aura", ACTION_NORMAL) }));
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "righteous fury", { NextAction("righteous fury", ACTION_HIGH + 8) }));
}

void PaladinBuffStatsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // First Sanctuary (prio > Kings)
    triggers.push_back(
        new TriggerNode("blessing of sanctuary on party",
                        { NextAction("blessing of sanctuary on party", 12.0f) }));

    // After Kings
    triggers.push_back(
        new TriggerNode("blessing of kings on party",
                        { NextAction("blessing of kings on party", 11.0f) }));
}
