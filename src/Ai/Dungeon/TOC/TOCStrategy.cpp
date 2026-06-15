/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TOCStrategy.h"

void WotlkDungeonToCStrategy::InitTriggers(std::vector<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode("toc lance",
        { NextAction("toc lance", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("toc ue lance",
        { NextAction("toc ue lance", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("toc mount near",
        { NextAction("toc mount", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("toc mounted",
        { NextAction("toc mounted", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("toc eadric",
        { NextAction("toc eadric", ACTION_RAID + 3) }));

}

void WotlkDungeonToCStrategy::InitMultipliers(std::vector<Multiplier*> &/*multipliers*/)
{
}
