/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SKIPSPELLSLISTACTION_H
#define PLAYERBOTS_SKIPSPELLSLISTACTION_H

#include "Action.h"
#include "ChatHelper.h"

class PlayerbotAI;

class SkipSpellsListAction : public Action
{
public:
    SkipSpellsListAction(PlayerbotAI* botAI) : Action(botAI, "ss") {}

    bool Execute(Event event) override;

private:
    SpellIds parseIds(std::string const text);
};

#endif
