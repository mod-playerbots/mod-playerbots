/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H
#define _PLAYERBOT_RAIDHYJALSUMMITMULTIPLIERS_H

#include "Multiplier.h"

class HyjalSummitTimeBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    HyjalSummitTimeBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hyjal summit time bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

// Rage Winterchill

class RageWinterchillDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    RageWinterchillDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

class RageWinterchillMeleeControlAvoidanceMultiplier : public Multiplier
{
public:
    RageWinterchillMeleeControlAvoidanceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill melee control avoidance multiplier") {}
    virtual float GetValue(Action* action);
};

// Anetheron

class AnetheronDisableTankActionsMultiplier : public Multiplier
{
public:
    AnetheronDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class AnetheronDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AnetheronDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

class AnetheronControlMisdirectionMultiplier : public Multiplier
{
public:
    AnetheronControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron control misdirection multiplier") {}
    virtual float GetValue(Action* action);
};

// Kaz'rogal

class KazrogalLowManaBotStayAwayFromGroupMultiplier : public Multiplier
{
public:
    KazrogalLowManaBotStayAwayFromGroupMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal low mana bot stay away from group multiplier") {}
    virtual float GetValue(Action* action);
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public Multiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal keep aspect of the viper active multiplier") {}
    virtual float GetValue(Action* action);
};

class KazrogalControlMovementMultiplier : public Multiplier
{
public:
    KazrogalControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal control movement multiplier") {}
    virtual float GetValue(Action* action);
};

// Azgalor

class AzgalorDisableTankActionsMultiplier : public Multiplier
{
public:
    AzgalorDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor disable tank actions multiplier") {}
    virtual float GetValue(Action* action);
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public Multiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor doomed bot prioritize positioning multiplier") {}
    virtual float GetValue(Action* action);
};

class AzgalorMeleeDpsControlAvoidanceMultiplier : public Multiplier
{
public:
    AzgalorMeleeDpsControlAvoidanceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor melee dps control avoidance multiplier") {}
    virtual float GetValue(Action* action);
};

// Archimonde

class ArchimondeDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    ArchimondeDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "archimonde disable combat formation move multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
