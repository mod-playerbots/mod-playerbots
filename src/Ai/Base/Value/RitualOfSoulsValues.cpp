/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RitualOfSoulsValues.h"

#include "GameObject.h"
#include "Group.h"
#include "Playerbots.h"
#include "Timer.h"

#include <mutex>
#include <unordered_map>

namespace
{
    // The core keeps the portal on its caster's GO list but not the soulwell, so a soulwell
    // takes a grid search. Only done for a while after the group's last portal sighting.
    constexpr uint32 SOULWELL_SEARCH_WINDOW_MS = 190000;  // 3 min soulwell + slack

    std::unordered_map<ObjectGuid, uint32> lastPortalSeen;
    std::mutex lastPortalSeenMutex;

    void StampPortalSeen(ObjectGuid groupGuid)
    {
        uint32 now = getMSTime();
        std::lock_guard<std::mutex> lock(lastPortalSeenMutex);
        for (auto it = lastPortalSeen.begin(); it != lastPortalSeen.end();)
        {
            if (getMSTimeDiff(it->second, now) >= SOULWELL_SEARCH_WINDOW_MS)
                it = lastPortalSeen.erase(it);
            else
                ++it;
        }
        lastPortalSeen[groupGuid] = now;
    }

    bool PortalSeenRecently(ObjectGuid groupGuid)
    {
        std::lock_guard<std::mutex> lock(lastPortalSeenMutex);
        auto it = lastPortalSeen.find(groupGuid);
        return it != lastPortalSeen.end() && getMSTimeDiff(it->second, getMSTime()) < SOULWELL_SEARCH_WINDOW_MS;
    }
}  // namespace

ObjectGuid RitualOfSoulsPortalValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return ObjectGuid::Empty;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        // other maps update on other threads, so their GO lists are off limits
        if (!member || member->getClass() != CLASS_WARLOCK || !member->IsInMap(bot))
            continue;

        GameObject* portal = member->GetGameObject(SPELL_RITUAL_OF_SOULS_R2);
        if (!portal)
            portal = member->GetGameObject(SPELL_RITUAL_OF_SOULS_R1);
        if (!portal)
            continue;

        StampPortalSeen(group->GetGUID());

        if (member != bot && bot->IsWithinDistInMap(portal, RITUAL_OF_SOULS_GO_RANGE))
            return portal->GetGUID();
    }

    return ObjectGuid::Empty;
}

ObjectGuid RitualOfSoulsSoulwellValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group || !PortalSeenRecently(group->GetGUID()))
        return ObjectGuid::Empty;

    for (uint32 entry : {GO_SOULWELL_R1, GO_SOULWELL_R2})
    {
        GameObject* soulwell = bot->FindNearestGameObject(entry, RITUAL_OF_SOULS_GO_RANGE);
        if (soulwell && group->IsMember(soulwell->GetOwnerGUID()))
            return soulwell->GetGUID();
    }

    return ObjectGuid::Empty;
}
