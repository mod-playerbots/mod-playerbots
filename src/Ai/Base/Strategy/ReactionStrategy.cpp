/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from cmangos/playerbots (ReactionStrategy) with modifications.
 */

#include "ReactionStrategy.h"

#include "Playerbots.h"

void ReactionStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    // Upstream cmangos switches engines here; this port switches only for the bot's own attacker,
    // target-first ("wake on combat start"), since a targetless entry would bounce straight back.
    triggers.push_back(
        new TriggerNode(
            "combat start",
            {
                NextAction("wake on combat start", ACTION_PASSTHROUGH)
            }
        )
    );
}
