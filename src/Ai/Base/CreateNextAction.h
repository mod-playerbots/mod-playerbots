#pragma once

#include "NextAction.h"
#include "CreateAction.h"

template<typename TAction>
NextAction CreateNextAction(float weight, PlayerbotAI* botAI)
{
    return NextAction{ weight, &CreateAction<TAction>(botAI) };
}
