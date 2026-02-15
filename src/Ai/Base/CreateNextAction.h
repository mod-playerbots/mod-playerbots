#pragma once

#include "NextAction.h"
#include "CreateAction.h"

template<typename TAction>
NextAction CreateNextAction(float weight)
{
    return NextAction{ weight, &CreateAction<TAction> };
}
