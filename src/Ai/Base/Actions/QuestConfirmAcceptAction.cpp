/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QuestConfirmAcceptAction.h"

#include "QuestPackets.h"
#include "WorldPacket.h"

bool QuestConfirmAcceptAction::Execute(Event event)
{
    WorldPacket packet(event.getPacket());
    uint32 questId;
    packet >> questId;

    WorldPacket sendPacket(CMSG_QUEST_CONFIRM_ACCEPT);
    sendPacket << questId;
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest || !bot->CanAddQuest(quest, true))
    {
        return false;
    }
    std::ostringstream out;
    out << "Quest: " << chat->FormatQuest(quest) << " confirm accept";
    botAI->TellMaster(out);
    WorldPackets::Quest::QuestConfirmAcceptClient confirmAccept(std::move(sendPacket));
    confirmAccept.Read();
    bot->GetSession()->HandleQuestConfirmAccept(confirmAccept);
    return true;
}
