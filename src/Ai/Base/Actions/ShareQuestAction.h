/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHAREQUESTACTION_H
#define PLAYERBOTS_SHAREQUESTACTION_H

#include "Action.h"

class PlayerbotAI;

class ShareQuestAction : public Action
{
public:
    ShareQuestAction(PlayerbotAI* botAI, std::string name = "share quest") : Action(botAI, name) { }
    bool Execute(Event event) override;
};

class AutoShareQuestAction : public ShareQuestAction
{
public:
    AutoShareQuestAction(PlayerbotAI* botAI) : ShareQuestAction(botAI, "auto share quest") {}
    bool Execute(Event event) override;

    bool isUseful() override;
};

#endif
