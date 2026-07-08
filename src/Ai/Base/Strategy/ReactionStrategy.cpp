/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ReactionStrategy.h"

#include "Playerbots.h"

void ReactionStrategy::InitReactionTriggers(std::vector<TriggerNode*>& triggers)
{
    // Intentionally empty. In upstream cmangos this strategy wires bot state-transition
    // triggers (combat start/end, death, resurrect) so they fire through the reaction engine.
    // This port keeps all state transitions inline in PlayerbotAI::UpdateAIInternal, so there
    // is nothing to wire here. ReactionStrategy's purpose is to contribute STRATEGY_TYPE_REACTION
    // to the engine's type mask and give server operators a "react" toggle handle for the
    // reaction engine as a whole (e.g. ChangeStrategy("-react", BOT_STATE_REACTION)).
}
