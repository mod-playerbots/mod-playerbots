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
    // Only the bot's own attacker may wake the combat engine: "wake on combat start" sets it as
    // the current target and the transition happens in PlayerbotAI::DoNextAction, never targetless.
    triggers.push_back(
        new TriggerNode(
            "combat start",
            {
                NextAction("wake on combat start", ACTION_PASSTHROUGH)
            }
        )
    );
}
