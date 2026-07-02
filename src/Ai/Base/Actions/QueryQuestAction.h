/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUERYQUESTACTION_H
#define PLAYERBOTS_QUERYQUESTACTION_H

#include "Action.h"

class PlayerbotAI;

class QueryQuestAction : public Action
{
public:
    QueryQuestAction(PlayerbotAI* botAI) : Action(botAI, "query quest") {}

    bool Execute(Event event) override;

private:
    void TellObjectives(uint32 questId);
    void TellObjective(std::string const name, uint32 available, uint32 required);
};

#endif
