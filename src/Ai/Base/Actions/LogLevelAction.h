/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOGLEVELACTION_H
#define PLAYERBOTS_LOGLEVELACTION_H

#include "Action.h"
#include "LogCommon.h"

class PlayerbotAI;

class LogLevelAction : public Action
{
public:
    LogLevelAction(PlayerbotAI* botAI) : Action(botAI, "log") {}

    bool Execute(Event event) override;

public:
    static std::string const logLevel2string(LogLevel level);
    static LogLevel string2logLevel(std::string const level);
};

#endif
