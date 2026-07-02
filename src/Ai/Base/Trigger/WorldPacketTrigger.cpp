/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WorldPacketTrigger.h"

#include "Playerbots.h"

void WorldPacketTrigger::ExternalEvent(WorldPacket& revData, Player* eventOwner)
{
    packet = revData;
    owner = eventOwner;
    triggered = true;
}

Event WorldPacketTrigger::Check()
{
    if (!triggered)
        return Event();

    return Event(getName(), packet, owner);
}

void WorldPacketTrigger::Reset() { triggered = false; }
