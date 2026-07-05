/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef PLAYERBOTS_PASSLEADERSHIPTOMASTERACTION_H
#define PLAYERBOTS_PASSLEADERSHIPTOMASTERACTION_H

#include "Action.h"

class PlayerbotAI;

class PassLeadershipToMasterAction : public Action
{
public:
    PassLeadershipToMasterAction(PlayerbotAI* botAI, std::string const name = "leader",
                                 std::string const message = "Passing leader to you!")
        : Action(botAI, name), message(message)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    std::string const message;
};

class GiveLeaderAction : public PassLeadershipToMasterAction
{
public:
    GiveLeaderAction(PlayerbotAI* botAI, std::string const message = "Lead the way!")
        : PassLeadershipToMasterAction(botAI, "give leader", message)
    {
    }

    bool isUseful() override;
};

#endif
