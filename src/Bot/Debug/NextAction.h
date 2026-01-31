#pragma once

#include <memory>

class Action;

struct NextAction
{
    using Factory = std::unique_ptr<Action>(*)();

    float weight;
    Factory factory;
};
