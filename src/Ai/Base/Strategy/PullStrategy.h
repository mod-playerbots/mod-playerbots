/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PULLSTRATEGY_H
#define _PLAYERBOT_PULLSTRATEGY_H

#include "Strategy.h"

class Action;
class Multiplier;
class Unit;

class PlayerbotAI;

class PullStrategy : public Strategy
{
public:
    PullStrategy(PlayerbotAI* botAI, std::string const action, std::string const preAction = "");

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "pull"; }
    std::vector<NextAction> getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_NONCOMBAT; }

    static PullStrategy* Get(PlayerbotAI* botAI);
    static uint8 GetMaxPullTime() { return 15; }

    time_t GetPullStartTime() const { return pullStartTime; }
    bool IsPullPendingToStart() const { return pendingToStart; }
    bool HasPullStarted() const { return pullStartTime > 0; }

    bool CanDoPullAction(Unit* target);
    Unit* GetTarget() const;
    bool HasTarget() const;

    virtual std::string GetPullActionName() const;
    std::string GetSpellName() const;
    float GetRange() const;
    virtual std::string GetPreActionName() const;

    void RequestPull(Unit* target, bool resetTime = true);
    void OnPullStarted();
    void OnPullEnded();

    ReactStates GetPetReactState() const { return petReactState; }
    void SetPetReactState(ReactStates reactState) { petReactState = reactState; }

private:
    void SetTarget(Unit* target);

private:
    std::string const action;
    std::string const preAction;
    bool pendingToStart = false;
    time_t pullStartTime = 0;
    ReactStates petReactState = REACT_DEFENSIVE;
};

class PullMultiplier : public Multiplier
{
public:
    PullMultiplier(PlayerbotAI* botAI);

    float GetValue(Action* action) override;
};

class PossibleAddsStrategy : public Strategy
{
public:
    PossibleAddsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "adds"; }
};

class PullBackStrategy : public Strategy
{
public:
    PullBackStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "pull back"; }
};

#endif
