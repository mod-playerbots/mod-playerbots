/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RTITRIGGERS_H
#define PLAYERBOTS_RTITRIGGERS_H

#include "GenericTriggers.h"
#include "Trigger.h"

class PlayerbotAI;

class NoRtiTrigger : public Trigger
{
public:
    NoRtiTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no rti target") {}

    bool IsActive() override;
};

class RtiCcTrigger : public HasCcTargetTrigger
{
public:
    RtiCcTrigger(PlayerbotAI* botAI, std::string const name) : HasCcTargetTrigger(botAI, name) {}

    bool IsActive() override;
};

#endif
