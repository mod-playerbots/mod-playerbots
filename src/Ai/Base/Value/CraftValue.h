/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CRAFTVALUE_H
#define PLAYERBOTS_CRAFTVALUE_H

#include <map>

#include "Value.h"

class PlayerbotAI;

class CraftData
{
public:
    CraftData() : itemId(0) {}

    uint32 itemId;
    std::map<uint32, uint32> required, obtained;

    bool IsEmpty() { return itemId == 0; }
    void Reset() { itemId = 0; }
    bool IsRequired(uint32 item) { return required.find(item) != required.end(); }
    bool IsFulfilled()
    {
        for (std::map<uint32, uint32>::iterator i = required.begin(); i != required.end(); ++i)
        {
            uint32 item = i->first;
            if (obtained[item] < i->second)
                return false;
        }

        return true;
    }
    void AddObtained(uint32 itemId, uint32 count)
    {
        if (IsRequired(itemId))
        {
            obtained[itemId] += count;
        }
    }

    void Crafted(uint32 count)
    {
        for (std::map<uint32, uint32>::iterator i = required.begin(); i != required.end(); ++i)
        {
            uint32 item = i->first;
            if (obtained[item] >= required[item] * count)
            {
                obtained[item] -= required[item] * count;
            }
        }
    }
};

class CraftValue : public ManualSetValue<CraftData&>
{
public:
    CraftValue(PlayerbotAI* botAI, std::string const name = "craft") : ManualSetValue<CraftData&>(botAI, data, name) {}

private:
    CraftData data;
};

#endif
