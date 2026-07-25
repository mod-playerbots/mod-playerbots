/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
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
