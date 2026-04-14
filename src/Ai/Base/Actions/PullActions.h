/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PULLACTIONS_H
#define _PLAYERBOT_PULLACTIONS_H

#include "GenericSpellActions.h"
#include "ReachTargetActions.h"

class PullRequestAction : public Action
{
public:
    PullRequestAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

    bool Execute(Event event) override;

protected:
    virtual Unit* GetPullTarget(Event event) = 0;
};

class PullMyTargetAction : public PullRequestAction
{
public:
    PullMyTargetAction(PlayerbotAI* botAI) : PullRequestAction(botAI, "pull my target") {}

private:
    Unit* GetPullTarget(Event event) override;
};

class PullRtiTargetAction : public PullRequestAction
{
public:
    PullRtiTargetAction(PlayerbotAI* botAI) : PullRequestAction(botAI, "pull rti target") {}

private:
    Unit* GetPullTarget(Event event) override;
};

class PullStartAction : public Action
{
public:
    PullStartAction(PlayerbotAI* botAI, std::string const name = "pull start") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class PullAction : public CastSpellAction
{
public:
    PullAction(PlayerbotAI* botAI, std::string const name = "pull action");

    bool Execute(Event event) override;
    bool isPossible() override;
    std::vector<NextAction> getPrerequisites() override;
    Unit* GetTarget() override;

private:
    void InitPullAction();
};

class PullEndAction : public Action
{
public:
    PullEndAction(PlayerbotAI* botAI, std::string const name = "pull end") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class ReachPullAction : public ReachTargetAction
{
public:
    ReachPullAction(PlayerbotAI* botAI) : ReachTargetAction(botAI, "reach pull", botAI->GetRange("spell")) {}

    bool Execute(Event event) override;
    bool isUseful() override;
    Unit* GetTarget() override;
};

class ReturnToPullPositionAction : public MovementAction
{
public:
    ReturnToPullPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, "return to pull position") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
