/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HUMANPLAYERROSTER_H
#define PLAYERBOTS_HUMANPLAYERROSTER_H

#include <mutex>
#include <set>

#include "ObjectGuid.h"

// World-thread login/logout publishes GUIDs; map-thread AI never reads sessions or the bot manager.
class HumanPlayerRoster
{
public:
    static HumanPlayerRoster& Instance()
    {
        static HumanPlayerRoster roster;
        return roster;
    }

    void Add(ObjectGuid guid)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _guids.insert(guid);
    }

    void Remove(ObjectGuid guid)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _guids.erase(guid);
    }

    bool Contains(ObjectGuid guid) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _guids.contains(guid);
    }

private:
    HumanPlayerRoster() = default;

    mutable std::mutex _mutex;
    std::set<ObjectGuid> _guids;
};

#endif
