/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GATHERINGLEVELINGTRIGGERS_H
#define PLAYERBOTS_GATHERINGLEVELINGTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// Fires when the bot's collecting profession is below max level AND it can see
// a gathering node whose skill requirement is above its current skill. This is
// the signal that prof meaningfully needs levelling (i.e. reach a trainer and
// start gathering), driving the bot towards the "gather leveling" behaviour.
class NeedGatheringLevelingTrigger : public Trigger
{
public:
    NeedGatheringLevelingTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "need gather leveling", 5000)
    {
    }

    bool IsActive() override;

private:
    bool HasUnmaxedGatheringProfession();
    bool SeesBoostNode();
};

#endif
