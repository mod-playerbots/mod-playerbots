/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ACCEPTQUESTACTION_H
#define PLAYERBOTS_ACCEPTQUESTACTION_H

#include "QuestAction.h"

class Quest;
class PlayerbotAI;
class WorldObject;

class AcceptAllQuestsAction : public QuestAction
{
public:
    AcceptAllQuestsAction(PlayerbotAI* botAI, std::string const name = "accept all quests") : QuestAction(botAI, name)
    {
    }

protected:
    bool ProcessQuest(Quest const* quest, Object* questGiver) override;
};

class AcceptQuestAction : public AcceptAllQuestsAction
{
public:
    AcceptQuestAction(PlayerbotAI* botAI) : AcceptAllQuestsAction(botAI, "accept quest") {}
    bool Execute(Event event) override;
};

// An auto-accept quest is taken by the core inside the handler for
// CMSG_QUESTGIVER_QUERY_QUEST, so no CMSG_QUESTGIVER_ACCEPT_QUEST is ever sent
// and the bots have nothing to mirror. This listens to the query instead, and
// only acts when the quest really carries QUEST_FLAGS_AUTO_ACCEPT, so merely
// opening the dialog on an ordinary quest still decides nothing.
class AcceptAutoQuestAction : public AcceptAllQuestsAction
{
public:
    AcceptAutoQuestAction(PlayerbotAI* botAI) : AcceptAllQuestsAction(botAI, "accept auto quest") {}
    bool Execute(Event event) override;
};

class AcceptQuestShareAction : public Action
{
public:
    AcceptQuestShareAction(PlayerbotAI* botAI) : Action(botAI, "accept quest share") {}
    bool Execute(Event event) override;
};

class ConfirmQuestAction : public Action {
public:
    ConfirmQuestAction(PlayerbotAI* ai) : Action(ai, "confirm quest") {}
    bool Execute(Event event);
};

#endif
