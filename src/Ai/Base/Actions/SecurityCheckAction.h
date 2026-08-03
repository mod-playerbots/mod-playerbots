/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SECURITYCHECKACTION_H
#define PLAYERBOTS_SECURITYCHECKACTION_H

#include "Action.h"

class PlayerbotAI;

class SecurityCheckAction : public Action
{
public:
    SecurityCheckAction(PlayerbotAI* botAI) : Action(botAI, "security check") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

#endif
