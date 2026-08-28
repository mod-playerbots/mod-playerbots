/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AcceptSummonAction.h"
#include "Event.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

bool AcceptSummonAction::Execute(Event event)
{
    if (!sPlayerbotAIConfig.autoAcceptSummons)
        return false;

    WorldPacket p(event.getPacket());
    p.rpos(0);
    ObjectGuid summonerGuid;
    p >> summonerGuid;

    Player* summoner = ObjectAccessor::FindPlayer(summonerGuid);
    if (!summoner)
        return false;

    // Only accept summons from our master or a group/raid member.
    if (summoner != GetMaster() && !summoner->IsInSameRaidWith(bot))
        return false;

    WorldPacket data(CMSG_SUMMON_RESPONSE, 8 + 1);
    data << summonerGuid << uint8(1);
    bot->GetSession()->HandleSummonResponseOpcode(data);
    return true;
}
