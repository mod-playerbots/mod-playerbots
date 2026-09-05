/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HYJALMULTIPLIERS_H
#define PLAYERBOTS_HYJALMULTIPLIERS_H

#include "Multiplier.h"

class HyjalSummitTimeBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    HyjalSummitTimeBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hyjal summit time bloodlust and heroism multiplier") {}
    float GetValue(Action* action) override;
};

// Rage Winterchill

class RageWinterchillDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    RageWinterchillDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill disable combat formation move multiplier") {}
    float GetValue(Action* action) override;
};

class RageWinterchillMeleeControlAvoidanceMultiplier : public Multiplier
{
public:
    RageWinterchillMeleeControlAvoidanceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "rage winterchill melee control avoidance multiplier") {}
    float GetValue(Action* action) override;
};

// Anetheron

class AnetheronDisableTankActionsMultiplier : public Multiplier
{
public:
    AnetheronDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class AnetheronDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    AnetheronDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron disable combat formation move multiplier") {}
    float GetValue(Action* action) override;
};

class AnetheronControlMisdirectionMultiplier : public Multiplier
{
public:
    AnetheronControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "anetheron control misdirection multiplier") {}
    float GetValue(Action* action) override;
};

// Kaz'rogal

class KazrogalLowManaBotStayAwayFromGroupMultiplier : public Multiplier
{
public:
    KazrogalLowManaBotStayAwayFromGroupMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal low mana bot stay away from group multiplier") {}
    float GetValue(Action* action) override;
};

class KazrogalKeepAspectOfTheViperActiveMultiplier : public Multiplier
{
public:
    KazrogalKeepAspectOfTheViperActiveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal keep aspect of the viper active multiplier") {}
    float GetValue(Action* action) override;
};

class KazrogalControlMovementMultiplier : public Multiplier
{
public:
    KazrogalControlMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "kaz'rogal control movement multiplier") {}
    float GetValue(Action* action) override;
};

// Azgalor

class AzgalorDisableTankActionsMultiplier : public Multiplier
{
public:
    AzgalorDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor disable tank actions multiplier") {}
    float GetValue(Action* action) override;
};

class AzgalorDoomedBotPrioritizePositioningMultiplier : public Multiplier
{
public:
    AzgalorDoomedBotPrioritizePositioningMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor doomed bot prioritize positioning multiplier") {}
    float GetValue(Action* action) override;
};

class AzgalorMeleeDpsControlAvoidanceMultiplier : public Multiplier
{
public:
    AzgalorMeleeDpsControlAvoidanceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "azgalor melee dps control avoidance multiplier") {}
    float GetValue(Action* action) override;
};

// Archimonde

class ArchimondeDisableCombatFormationMoveMultiplier : public Multiplier
{
public:
    ArchimondeDisableCombatFormationMoveMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "archimonde disable combat formation move multiplier") {}
    float GetValue(Action* action) override;
};

#endif
