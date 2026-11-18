/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_Opener.h"

#include "Playerbots.h"
#include "Strategy.h"

void AddToCOpenerTriggerNodes(std::vector<TriggerNode*>& triggers)
{
    // Above the resistance aura: the swing lands on the next attack timer, not the next GCD
    triggers.push_back(new TriggerNode("toc opener swing", {
        NextAction("toc opener stop swing", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("toc opener redirect", {
        NextAction("toc opener redirect", ACTION_RAID + 1) }));
}
