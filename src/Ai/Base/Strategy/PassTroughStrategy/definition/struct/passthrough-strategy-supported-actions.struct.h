#pragma once

#include <string>

#include "NextAction.h"

struct PassthroughStrategySupportedActionsStruct
{
    std::string name;
    NextAction::Factory factory;
};
