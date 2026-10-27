/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RitualOfSoulsActions.h"

#include "GameObject.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "Timer.h"

#include <mutex>
#include <vector>

namespace
{
    constexpr uint32 RITUAL_SLOT_RESERVATION_MS = 5000;

    // One slot per helper, so a whole raid doesn't run at one portal. A slot runs out if
    // its bot never reaches the portal.
    struct RitualSlot
    {
        ObjectGuid go;
        ObjectGuid bot;
        uint32 expiry;
    };

    std::vector<RitualSlot> ritualReservations;
    std::mutex ritualReservationsMutex;

    bool TryReserveRitualSlot(ObjectGuid goGuid, ObjectGuid botGuid, uint32 reqHelpers)
    {
        uint32 now = getMSTime();
        std::lock_guard<std::mutex> lock(ritualReservationsMutex);
        for (auto it = ritualReservations.begin(); it != ritualReservations.end();)
        {
            if (it->expiry <= now)
                it = ritualReservations.erase(it);
            else
                ++it;
        }

        uint32 active = 0;
        for (RitualSlot const& slot : ritualReservations)
        {
            if (slot.go != goGuid)
                continue;
            if (slot.bot == botGuid)
                return true;
            ++active;
        }

        if (active >= reqHelpers)
            return false;

        ritualReservations.push_back({goGuid, botGuid, now + RITUAL_SLOT_RESERVATION_MS});
        return true;
    }
}  // namespace

bool UseRitualOfSoulsObjectAction::MoveToAndUse(GameObject* go)
{
    if (!go->IsAtInteractDistance(bot))
        return MoveTo(go, fmaxf(go->GetInteractionDistance() - 1.0f, 0.0f));

    WorldPacket data(CMSG_GAMEOBJ_USE);
    data << go->GetGUID();
    bot->GetSession()->HandleGameObjectUseOpcode(data);
    return true;
}

bool JoinRitualOfSoulsAction::Execute(Event /*event*/)
{
    GameObject* portal = botAI->GetGameObject(AI_VALUE(ObjectGuid, "ritual of souls portal"));
    if (!portal)
        return false;

    uint32 reqParticipants = 0;
    if (GameObjectTemplate const* info = portal->GetGOInfo())
        reqParticipants = info->summoningRitual.reqParticipants;

    // reqParticipants counts the channelling caster too
    uint32 reqHelpers = reqParticipants > 1 ? reqParticipants - 1 : 1;
    if (!TryReserveRitualSlot(portal->GetGUID(), bot->GetGUID(), reqHelpers))
        return false;

    return MoveToAndUse(portal);
}

bool UseSoulwellAction::isUseful()
{
    return AI_VALUE2(std::vector<Item*>, "inventory items", "healthstone").empty();
}

bool UseSoulwellAction::Execute(Event /*event*/)
{
    GameObject* soulwell = botAI->GetGameObject(AI_VALUE(ObjectGuid, "ritual of souls soulwell"));
    if (!soulwell)
        return false;

    return MoveToAndUse(soulwell);
}
