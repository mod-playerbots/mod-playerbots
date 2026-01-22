/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "AiObject.h"
#include "Event.h"
#include "Value.h"
#include "NextAction.h"

class PlayerbotAI;
class Unit;

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
    virtual bool isRPG() { return false; }
    virtual std::vector<NextAction> getPrerequisites() { return {}; }
    virtual std::vector<NextAction> getAlternatives() { return {}; }
    virtual std::vector<NextAction> getContinuers() { return {}; }
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
