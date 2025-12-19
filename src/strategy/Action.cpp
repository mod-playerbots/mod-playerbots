/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Action.h"

#include "Playerbots.h"
#include "Timer.h"

Value<Unit*>* Action::GetTargetValue() { return context->GetValue<Unit*>(GetTargetName()); }

Unit* Action::GetTarget() { return GetTargetValue()->Get(); }

ActionBasket::ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event)
    : action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event), created(getMSTime())
{
}

bool ActionBasket::isExpired(uint32_t msecs) { return getMSTime() - created >= msecs; }
