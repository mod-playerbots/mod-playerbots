#pragma once

#include <memory>

class Action;
class PlayerbotAI;

struct NextAction
{
    using Factory = std::unique_ptr<Action>(*)(PlayerbotAI* botAI);

    float weight;
    Factory factory;
};
