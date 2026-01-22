
#pragma once

#include "ActionNode.h"
#include "Action.h"
#include "Event.h"

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
