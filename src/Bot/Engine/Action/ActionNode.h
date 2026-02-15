#pragma once

#include "Action.h"
#include "NextAction.h"

class ActionNode
{
public:
    ActionNode(
        std::vector<NextAction> prerequisites = {},
        std::vector<NextAction> alternatives = {},
        std::vector<NextAction> continuers = {}
    ) :
    action(nullptr),
    continuers(continuers),
    alternatives(alternatives),
    prerequisites(prerequisites)
    {}

    virtual ~ActionNode() = default;

    Action& getAction() const
    {
        return *this->action;
    }

    void setAction(std::unique_ptr<Action> action)
    {
        this->action = std::move(action);
    }

    static std::vector<NextAction> MergeNextActions(std::vector<NextAction> what, std::vector<NextAction> with)
    {
        std::vector<NextAction> result = {};

        for (NextAction const& nextAction : what)
        {
            result.push_back(nextAction);
        }

        for (NextAction const& nextAction : with)
        {
            result.push_back(nextAction);
        }

        return result;
    }

    std::vector<NextAction> getContinuers()
    {
        return ActionNode::MergeNextActions(this->continuers, this->action->getContinuers());
    }
    std::vector<NextAction> getAlternatives()
    {
        return ActionNode::MergeNextActions(this->alternatives, this->action->getAlternatives());
    }
    std::vector<NextAction> getPrerequisites()
    {
        return ActionNode::MergeNextActions(this->prerequisites, this->action->getPrerequisites());
    }

private:
    std::unique_ptr<Action> action;
    std::vector<NextAction> continuers;
    std::vector<NextAction> alternatives;
    std::vector<NextAction> prerequisites;
};
