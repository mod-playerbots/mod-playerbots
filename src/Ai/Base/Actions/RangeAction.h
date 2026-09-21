/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RANGEACTION_H
#define PLAYERBOTS_RANGEACTION_H

#include "Action.h"

class PlayerbotAI;

class RangeAction : public Action
{
public:
    RangeAction(PlayerbotAI* botAI) : Action(botAI, "range") {}

    bool Execute(Event event) override;

private:
    void PrintRange(std::string const type);
};

#endif
