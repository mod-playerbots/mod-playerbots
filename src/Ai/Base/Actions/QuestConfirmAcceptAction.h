/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTCONFIRMACCEPTACTION_H
#define PLAYERBOTS_QUESTCONFIRMACCEPTACTION_H

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "QuestAction.h"

class ObjectGuid;
class Quest;
class Player;
class PlayerbotAI;
class WorldObject;

class QuestConfirmAcceptAction : public Action
{
public:
    QuestConfirmAcceptAction(PlayerbotAI* botAI) : Action(botAI, "quest confirm accept") {}
    bool Execute(Event event) override;
};

#endif
