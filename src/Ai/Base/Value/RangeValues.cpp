/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RangeValues.h"

#include "Playerbots.h"

RangeValue::RangeValue(PlayerbotAI* botAI) : ManualSetValue<float>(botAI, 0.f, "range"), Qualified() {}

std::string const RangeValue::Save()
{
    std::ostringstream out;
    out << value;
    return out.str();
}

bool RangeValue::Load(std::string const text)
{
    value = atof(text.c_str());
    return true;
}
