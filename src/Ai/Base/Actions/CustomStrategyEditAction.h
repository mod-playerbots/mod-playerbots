/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CUSTOMSTRATEGYEDITACTION_H
#define PLAYERBOTS_CUSTOMSTRATEGYEDITACTION_H

#include "Action.h"

class PlayerbotAI;

class CustomStrategyEditAction : public Action
{
public:
    CustomStrategyEditAction(PlayerbotAI* botAI) : Action(botAI, "cs") {}

    bool Execute(Event event) override;

private:
    bool PrintHelp();
    bool PrintActionLine(uint32 idx, std::string const command);
    bool Print(std::string const name);
    bool Edit(std::string const name, uint32 idx, std::string const command);
};

#endif
