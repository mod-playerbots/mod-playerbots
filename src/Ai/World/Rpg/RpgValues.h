/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RPGVALUES_H
#define PLAYERBOTS_RPGVALUES_H

#include "Value.h"

class PlayerbotAI;

class NextRpgActionValue : public ManualSetValue<std::string>
{
public:
    NextRpgActionValue(PlayerbotAI* botAI, std::string const defaultValue = "",
                       std::string const name = "next rpg action")
        : ManualSetValue(botAI, defaultValue, name){};
};

#endif
