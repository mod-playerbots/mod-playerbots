/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ResetInstancesAction.h"

#include "PlayerbotAI.h"

#include "InstancePackets.h"

bool ResetInstancesAction::Execute(Event /*event*/)
{
    WorldPacket packet(CMSG_RESET_INSTANCES, 0);
    WorldPackets::Instance::ResetInstances resetInstance(std::move(packet));
    bot->GetSession()->HandleResetInstancesOpcode(resetInstance);

    return true;
}

bool ResetInstancesAction::isUseful() { return botAI->GetGroupLeader() == bot; };
