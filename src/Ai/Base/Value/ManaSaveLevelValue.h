/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MANASAVELEVELVALUE_H
#define PLAYERBOTS_MANASAVELEVELVALUE_H

#include "Value.h"

class PlayerbotAI;

class ManaSaveLevelValue : public ManualSetValue<double>
{
public:
    ManaSaveLevelValue(PlayerbotAI* botAI) : ManualSetValue<double>(botAI, 1.0, "mana save level") {}

    std::string const Save()
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    bool Load(std::string const text)
    {
        value = atof(text.c_str());
        return true;
    }
};

#endif
