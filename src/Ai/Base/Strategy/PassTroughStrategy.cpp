/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PassTroughStrategy.h"
#include "NextAction.h"

void PassTroughStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    for (std::vector<PassthroughStrategySupportedActionsStruct>::iterator i = this->supported.begin(); i != this->supported.end(); i++)
    {
        triggers.push_back(
            new TriggerNode(
                i->name,
                {
                    NextAction{
                        .weight = relevance,
                        .factory = i->factory
                    },
                }
            )
        );
    }
}
