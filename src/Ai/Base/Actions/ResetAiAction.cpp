/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ResetAiAction.h"

#include "Event.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "PlayerbotRepository.h"
#include "Playerbots.h"
#include "WorldPacket.h"

bool ResetAiAction::Execute(Event event)
{
    if (!event.getPacket().empty())
    {
        WorldPacket packet = event.getPacket();
        if (packet.GetOpcode() == SMSG_GROUP_LIST)
        {
            uint8 groupType;
            Group::MemberSlot slot;
            packet >> groupType;
            packet >> slot.group;
            packet >> slot.flags;
            packet >> slot.roles;
            if (groupType & GROUPTYPE_LFG)
            {
                uint8 status;
                uint32 dungeon;
                packet >> status;
                packet >> dungeon;
            }
            ObjectGuid guid;
            uint32 counter;
            uint32 membersCount;
            packet >> guid;
            packet >> counter;
            packet >> membersCount;
            if (membersCount != 0)
            {
                return false;
            }
        }
    }
    if (Player* master = botAI->GetMaster())
    {
        Group* botGroup = bot->GetGroup();
        Group* masterGroup = master->GetGroup();
        if (botGroup && (!masterGroup || masterGroup != botGroup))
            botAI->SetMaster(nullptr);
    }
    if (sRandomPlayerbotMgr.IsRandomBot(bot) && !bot->InBattleground())
    {
        if (bot->GetGroup() && (!botAI->GetMaster() || GET_PLAYERBOT_AI(botAI->GetMaster())))
        {
            if (Player* newMaster = botAI->FindNewMaster())
                botAI->SetMaster(newMaster);
        }
    }
    PlayerbotRepository::instance().Reset(botAI);
    botAI->ResetStrategies(false);
    botAI->TellMaster("AI was reset to defaults");
    return true;
}
