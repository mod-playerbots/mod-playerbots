/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Trigger.h"

#include "AiObjectContext.h"
#include "Event.h"

Trigger::Trigger(PlayerbotAI* botAI, std::string const name, int32 checkInterval)
    : AiNamedObject(botAI, name),
      checkInterval(checkInterval == 1 ? 1 : (checkInterval < 100 ? checkInterval * 1000 : checkInterval)),
      lastCheckTime(0)
{
}

Event Trigger::Check()
{
    if (IsActive())
    {
        Event event(getName());
        return event;
    }

    Event event;
    return event;
}

Value<Unit*>* Trigger::GetTargetValue() { return context->GetValue<Unit*>(GetTargetName()); }

Unit* Trigger::GetTarget() { return GetTargetValue()->Get(); }

bool Trigger::needCheck(uint32 now)
{
    if (checkInterval < 2)
        return true;

    if (!lastCheckTime || now - lastCheckTime >= uint32(checkInterval))
    {
        lastCheckTime = now;
        return true;
    }

    return false;
}
