/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHEATACTION_H
#define PLAYERBOTS_CHEATACTION_H

#include "Action.h"

class PlayerbotAI;

enum class BotCheatMask : uint32;

class CheatAction : public Action
{
public:
    CheatAction(PlayerbotAI* botAI) : Action(botAI, "cheat") {}

    bool Execute(Event event) override;

private:
    static BotCheatMask GetCheatMask(std::string const cheat);
    static std::string const GetCheatName(BotCheatMask cheatMask);
    void ListCheats();
};

#endif
