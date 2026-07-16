/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OutfitListValue.h"

#include "Playerbots.h"

std::string const OutfitListValue::Save()
{
    std::ostringstream out;
    bool first = true;
    for (Outfit::iterator i = value.begin(); i != value.end(); ++i)
    {
        if (!first)
            out << "^";
        else
            first = false;

        out << *i;
    }

    return out.str();
}

bool OutfitListValue::Load(std::string const text)
{
    value.clear();

    std::vector<std::string> ss = split(text, '^');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        value.push_back(*i);
    }

    return true;
}
