#pragma once

#include <memory>
#include <type_traits>

#include "Action.h"
#include "ActionFactoryRegistry.h"
#include "RegisterActionFactoryOnce.h"

template <typename TAction>
std::unique_ptr<Action> CreateAction(PlayerbotAI* botAI)
{
    static_assert(std::is_base_of<Action, TAction>::value == true, "TAction must derive from Action.");

    RegisterActionFactoryOnce<TAction>(botAI);

    return std::unique_ptr<Action>(new TAction(botAI));
}
