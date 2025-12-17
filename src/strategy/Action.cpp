/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "Action.h"

#include "Playerbots.h"
#include "Timer.h"

// uint32 NextAction::size(NextAction** actions)
// {
//     if (!actions)
//         return 0;

//     uint32 size = 0;
//     for (size = 0; actions[size];)
//         ++size;

//     return size;
// }

std::vector<NextAction*> NextAction::clone(std::vector<NextAction*>& actions)
{
    return actions;
}

// std::vector<NextAction*> NextAction::merge(std::vector<NextAction*>& left, std::vector<NextAction*>& right)
// {
//     return left.insert(left.end(), right.begin(), right.end());
// }

// NextAction** NextAction::array(uint32 nil, ...)
// {
//     va_list vl;
//     va_start(vl, nil);

//     uint32 size = 0;
//     NextAction* cur = nullptr;
//     do
//     {
//         cur = va_arg(vl, NextAction*);
//         ++size;
//     } while (cur);

//     va_end(vl);

//     NextAction** res = new NextAction*[size];
//     va_start(vl, nil);
//     for (uint32 i = 0; i < size; i++)
//         res[i] = va_arg(vl, NextAction*);
//     va_end(vl);

//     return res;
// }

// void NextAction::destroy(std::vector<NextAction*> actions)
// {
//     for (uint32_t i = 0; actions.size(); ++i)
//         delete actions[i];

// }

Value<Unit*>* Action::GetTargetValue() { return context->GetValue<Unit*>(GetTargetName()); }

Unit* Action::GetTarget() { return GetTargetValue()->Get(); }

ActionBasket::ActionBasket(ActionNode* action, float relevance, bool skipPrerequisites, Event event)
    : action(action), relevance(relevance), skipPrerequisites(skipPrerequisites), event(event), created(getMSTime())
{
}

bool ActionBasket::isExpired(uint32_t msecs) { return getMSTime() - created >= msecs; }
