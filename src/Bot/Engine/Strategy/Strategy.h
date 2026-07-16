/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STRATEGY_H
#define PLAYERBOTS_STRATEGY_H

#include "Action.h"
#include "Multiplier.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "PlayerbotAIAware.h"
#include "Trigger.h"

enum class TargetValueExclusionType : uint8
{
    None = 0,
    Tank,
    Dps,
    Attacker
};

enum StrategyType : uint32
{
    STRATEGY_TYPE_GENERIC = 0,
    STRATEGY_TYPE_COMBAT = 1,
    STRATEGY_TYPE_NONCOMBAT = 2,
    STRATEGY_TYPE_TANK = 4,
    STRATEGY_TYPE_DPS = 8,
    STRATEGY_TYPE_HEAL = 16,
    STRATEGY_TYPE_RANGED = 32,
    STRATEGY_TYPE_MELEE = 64
};

// enum ActionPriority
// {
//     ACTION_IDLE = 0,
//     ACTION_DEFAULT = 5,
//     ACTION_NORMAL = 10,
//     ACTION_HIGH = 20,
//     ACTION_MOVE = 30,
//     ACTION_INTERRUPT = 40,
//     ACTION_DISPEL = 50,
//     ACTION_RAID = 60,
//     ACTION_LIGHT_HEAL = 10,
//     ACTION_MEDIUM_HEAL = 20,
//     ACTION_CRITICAL_HEAL = 30,
//     ACTION_EMERGENCY = 90
// };

static float ACTION_IDLE = 0.0f;
static float ACTION_BG = 1.0f;
static float ACTION_DEFAULT = 5.0f;
static float ACTION_NORMAL = 10.0f;
static float ACTION_HIGH = 20.0f;
static float ACTION_MOVE = 30.0f;
static float ACTION_INTERRUPT = 40.0f;
static float ACTION_DISPEL = 50.0f;
static float ACTION_RAID = 60.0f;
static float ACTION_LIGHT_HEAL = 10.0f;
static float ACTION_MEDIUM_HEAL = 20.0f;
static float ACTION_CRITICAL_HEAL = 30.0f;
static float ACTION_EMERGENCY = 90.0f;

class Strategy : public PlayerbotAIAware
{
public:
    Strategy(PlayerbotAI* botAI);
    virtual ~Strategy() {}

    virtual std::vector<NextAction> getDefaultActions() { return {}; }
    virtual void InitTriggers([[maybe_unused]] std::vector<TriggerNode*>& triggers) {}
    virtual void InitMultipliers([[maybe_unused]] std::vector<Multiplier*>& multipliers) {}
    virtual void AppendTargetExclusions([[maybe_unused]] GuidSet& exclusions,
                                        [[maybe_unused]] TargetValueExclusionType type) {}
    virtual bool HasTargetExclusions() const { return false; }
    virtual std::string const getName() = 0;
    virtual uint32 GetType() const { return STRATEGY_TYPE_GENERIC; }
    virtual ActionNode* GetAction(std::string const name);
    void Update() {}
    void Reset() {}

public:
    NamedObjectFactoryList<ActionNode> actionNodeFactories;
};

#endif
