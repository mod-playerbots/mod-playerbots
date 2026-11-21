/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RITUALOFSOULSTRIGGERS_H
#define PLAYERBOTS_RITUALOFSOULSTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class RitualPortalNearbyTrigger : public Trigger
{
public:
    RitualPortalNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ritual of souls portal nearby", 2) {}
    bool IsActive() override;
};

class SoulwellNearbyTrigger : public Trigger
{
public:
    SoulwellNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "soulwell nearby", 2) {}
    bool IsActive() override;
};

#endif
