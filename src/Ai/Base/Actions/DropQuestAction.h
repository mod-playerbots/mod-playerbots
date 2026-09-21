/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DROPQUESTACTION_H
#define PLAYERBOTS_DROPQUESTACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;
class Quest;

class DropQuestAction : public Action
{
public:
    DropQuestAction(PlayerbotAI* botAI) : Action(botAI, "drop quest") {}

    bool Execute(Event event) override;
};

class CleanQuestLogAction : public Action
{
public:
    CleanQuestLogAction(PlayerbotAI* botAI) : Action(botAI, "clean quest log") {}

    bool Execute(Event event) override;
    void DropQuestType(uint8& numQuest, uint8 wantNum = 100, bool isGreen = false, bool hasProgress = false,
                       bool isComplete = false);

    static bool HasProgress(Player* bot, Quest const* quest);
};

#endif
