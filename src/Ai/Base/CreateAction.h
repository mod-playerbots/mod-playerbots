#pragma once

#include <memory>
#include <type_traits>

#include "Action.h"
#include "ActionFactoryRegistry.h"

template <typename TAction>
std::unique_ptr<Action> CreateAction(PlayerbotAI* botAI)
{
    static_assert(std::is_base_of<Action, TAction>::value == true, "TAction must derive from Action.");

    ActionFactoryRegistry::Register(std::type_index(typeid(TAction)), &CreateAction<TAction>);

    return std::unique_ptr<Action>(new TAction(botAI));
}
