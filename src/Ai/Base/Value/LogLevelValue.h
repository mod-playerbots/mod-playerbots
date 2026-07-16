/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LOGLEVELVALUE_H
#define PLAYERBOTS_LOGLEVELVALUE_H

#include "LogCommon.h"
#include "Value.h"

class PlayerbotAI;

class LogLevelValue : public ManualSetValue<LogLevel>
{
public:
    LogLevelValue(PlayerbotAI* botAI, std::string const name = "log level")
        : ManualSetValue<LogLevel>(botAI, LOG_LEVEL_DEBUG, name)
    {
    }
};

#endif
