// NextAction.h
#pragma once

#include <memory>

#include "Action.h"

struct NextAction
{
    using Factory = std::unique_ptr<Action>(*)();

    float weight;
    Factory factory;
};
