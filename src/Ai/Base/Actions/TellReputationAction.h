/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TELLREPUTATIONACTION_H
#define PLAYERBOTS_TELLREPUTATIONACTION_H

#include <string>

#include "Action.h"

struct FactionEntry;
class PlayerbotAI;

class TellReputationAction : public Action
{
public:
    TellReputationAction(PlayerbotAI* botAI) : Action(botAI, "reputation") {}

    bool Execute(Event event) override;

private:
    std::string BuildReputationLine(FactionEntry const* entry);
};

#endif
