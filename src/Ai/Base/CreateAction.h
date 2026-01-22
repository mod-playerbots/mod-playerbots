#pragma once

#include <memory>
#include <type_traits>

#include "Action.h"

template <typename TAction>
std::unique_ptr<TAction> CreateAction()
{
    static_assert(std::is_base_of<Action, TAction>::value == true, "TAction must derive from Action.");

    return std::unique_ptr<TAction>(new TAction());
}
