/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RTIACTION_H
#define PLAYERBOTS_RTIACTION_H

#include "Action.h"

class PlayerbotAI;

class RtiAction : public Action
{
public:
    RtiAction(PlayerbotAI* botAI) : Action(botAI, "rti") {}

    bool Execute(Event event) override;

private:
    void AppendRti(std::ostringstream& out, std::string const type);
};

class MarkRtiAction : public Action
{
public:
    MarkRtiAction(PlayerbotAI* botAI) : Action(botAI, "mark rti") {}

    bool Execute(Event event) override;
};

#endif
