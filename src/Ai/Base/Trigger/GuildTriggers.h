/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GUILDTRIGGERS_H
#define PLAYERBOTS_GUILDTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class PetitionTurnInTrigger : public Trigger
{
public:
    PetitionTurnInTrigger(PlayerbotAI* botAI) : Trigger(botAI) {}

    bool IsActive() override;
};

class BuyTabardTrigger : public Trigger
{
public:
    BuyTabardTrigger(PlayerbotAI* botAI) : Trigger(botAI) {}

    bool IsActive() override;
};

class LeaveLargeGuildTrigger : public Trigger
{
public:
    LeaveLargeGuildTrigger(PlayerbotAI* botAI) : Trigger(botAI) {}

    bool IsActive();
};

#endif
