/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SAVEMANAACTION_H
#define PLAYERBOTS_SAVEMANAACTION_H

#include "Action.h"

class PlayerbotAI;

class SaveManaAction : public Action
{
public:
    SaveManaAction(PlayerbotAI* botAI) : Action(botAI, "save mana") {}

    bool Execute(Event event) override;

private:
    std::string const Format(double value);
};

#endif
