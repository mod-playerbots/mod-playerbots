/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LISTQUESTSACTIONS_H
#define PLAYERBOTS_LISTQUESTSACTIONS_H

#include "Action.h"

class PlayerbotAI;

enum QuestListFilter
{
    QUEST_LIST_FILTER_SUMMARY = 0,
    QUEST_LIST_FILTER_COMPLETED = 1,
    QUEST_LIST_FILTER_INCOMPLETED = 2,
    QUEST_LIST_FILTER_ALL = QUEST_LIST_FILTER_COMPLETED | QUEST_LIST_FILTER_INCOMPLETED
};

enum QuestTravelDetail
{
    QUEST_TRAVEL_DETAIL_NONE = 0,
    QUEST_TRAVEL_DETAIL_SUMMARY = 1,
    QUEST_TRAVEL_DETAIL_FULL = 2
};

class ListQuestsAction : public Action
{
public:
    ListQuestsAction(PlayerbotAI* botAI) : Action(botAI, "quests") {}

    bool Execute(Event event) override;

private:
    void ListQuests(QuestListFilter filter, QuestTravelDetail travelDetail = QUEST_TRAVEL_DETAIL_NONE);
    uint32 ListQuests(bool completed, bool silent, QuestTravelDetail travelDetail);
};

#endif
