/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHECKVALUESACTION_H
#define PLAYERBOTS_CHECKVALUESACTION_H

#include "Action.h"

class PlayerbotAI;

class CheckValuesAction : public Action
{
public:
    CheckValuesAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
