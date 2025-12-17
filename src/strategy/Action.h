/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "AiObject.h"
#include "Common.h"
#include "Event.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class NextAction
{
public:
    NextAction(std::string const name, float relevance = 0.0f)
        : relevance(relevance), name(name) {}                                  // name after relevance - whipowill
    NextAction(NextAction const& o) : relevance(o.relevance), name(o.name) {}  // name after relevance - whipowill

    std::string const getName() { return name; }
    float getRelevance() { return relevance; }

    static std::vector<NextAction*> clone(std::vector<NextAction*>& actions);
    static std::vector<NextAction*> merge(std::vector<NextAction*> what, std::vector<NextAction*> with)
    {
        what.reserve(what.size() + with.size());
        what.insert(
            what.end(),
            std::make_move_iterator(with.begin()),
            std::make_move_iterator(with.end())
        );

        return what;
    };

    // static NextAction** array(uint32 nil, ...);
    static void destroy(std::vector<NextAction*> actions)
    {    
        for (NextAction* action : actions)
            delete action;
    }

private:
    float relevance;
    const std::string name;
};

class Action : public AiNamedObject
{
public:
    enum class ActionThreatType
    {
        None = 0,
        Single = 1,
        Aoe = 2
    };

    Action(PlayerbotAI* botAI, std::string const name = "action")
        : AiNamedObject(botAI, name), verbose(false) {}  // verbose after ainamedobject - whipowill
    virtual ~Action(void) {}

    virtual bool Execute([[maybe_unused]] Event event) { return true; }
    virtual bool isPossible() { return true; }
    virtual bool isUseful() { return true; }
    virtual std::vector<NextAction*> getPrerequisites() { return {}; }
    virtual std::vector<NextAction*> getAlternatives() { return {}; }
    virtual std::vector<NextAction*> getContinuers() { return {}; }
    virtual ActionThreatType getThreatType() { return ActionThreatType::None; }
    void Update() {}
    void Reset() {}
    virtual Unit* GetTarget();
    virtual Value<Unit*>* GetTargetValue();
    virtual std::string const GetTargetName() { return "self target"; }
    void MakeVerbose() { verbose = true; }
    void setRelevance(uint32 relevance1) { relevance = relevance1; };
    virtual float getRelevance() { return relevance; }

protected:
    bool verbose;
    float relevance = 0;
};

class ActionNode
{
public:
    ActionNode(
        std::string name,
        std::vector<NextAction*> prerequisites = {},
        std::vector<NextAction*> alternatives = {},
        std::vector<NextAction*> continuers = {}
    ) :
    name(std::move(name)),
    action(nullptr),
    continuers(continuers),
    alternatives(alternatives),
    prerequisites(prerequisites)
    {}

    virtual ~ActionNode()
    {
        NextAction::destroy(prerequisites);
        NextAction::destroy(alternatives);
        NextAction::destroy(continuers);
    }

    Action* getAction() { return action; }
    void setAction(Action* action) { this->action = action; }
    const std::string getName() { return name; }

    std::vector<NextAction*> getContinuers()
    {
        return NextAction::merge(NextAction::clone(continuers), action->getContinuers());
    }
    std::vector<NextAction*> getAlternatives()
    {
        return NextAction::merge(NextAction::clone(alternatives), action->getAlternatives());
    }
    std::vector<NextAction*> getPrerequisites()
    {
        return NextAction::merge(NextAction::clone(prerequisites), action->getPrerequisites());
    }

private:
    const std::string name;
    Action* action;
    std::vector<NextAction*> continuers;
    std::vector<NextAction*> alternatives;
    std::vector<NextAction*> prerequisites;
};

class ActionBasket
{
public:
    ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event);

    virtual ~ActionBasket(void) {}

    float getRelevance() { return relevance; }
    ActionNode* getAction() { return action; }
    Event getEvent() { return event; }
    bool isSkipPrerequisites() { return skipPrerequisites; }
    void AmendRelevance(float k) { relevance *= k; }
    void setRelevance(float relevance) { this->relevance = relevance; }
    bool isExpired(uint32_t msecs);

private:
    ActionNode* action;
    float relevance;
    bool skipPrerequisites;
    Event event;
    uint32_t created;
};
