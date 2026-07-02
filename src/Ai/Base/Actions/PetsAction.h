/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PETSACTION_H
#define PLAYERBOTS_PETSACTION_H

#include <string>

#include "Action.h"

class PlayerbotAI;

class PetsAction : public Action
{
public:
    PetsAction(PlayerbotAI* botAI, const std::string& defaultCmd = "") : Action(botAI, "pet"), defaultCmd(defaultCmd) {}

    bool Execute(Event event) override;

private:
    std::string defaultCmd;
};

#endif
