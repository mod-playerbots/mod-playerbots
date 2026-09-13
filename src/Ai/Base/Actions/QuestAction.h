/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTACTION_H
#define PLAYERBOTS_QUESTACTION_H

#include "Action.h"
#include "Object.h"
#include "QuestDef.h"

class ObjectGuid;
class Quest;
class Player;
class PlayerbotAI;
class WorldObject;
class Object;

class QuestAction : public Action
{
public:
    QuestAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) { }
    bool Execute(Event event) override;
    // Shared quest-accept path (guards, packet, sync fallback, broadcast); also used for quest-starting items
    bool AcceptQuest(Quest const* quest, ObjectGuid questGiver);
    // Whether the bot can and should accept this quest: core eligibility (level/race/class/rep/chain/...)
    // plus the module's "no grey quests" level band (same as QuestValues.cpp). Null-safe.
    // Used for quest-starting items and shared quests; the NPC/GO quest-giver path keeps the
    // core's dialog-status / RPG low-level fallback behaviour.
    static bool CanAcceptQuest(Player* bot, Quest const* quest);

protected:
    bool CompleteQuest(Player* player, uint32 entry);
    virtual bool ProcessQuest(Quest const* quest, Object* questGiver) = 0;
    bool ProcessQuests(ObjectGuid questGiver);
    bool ProcessQuests(WorldObject* questGiver);
};

class QuestUpdateCompleteAction : public Action
{
public:
    QuestUpdateCompleteAction(PlayerbotAI* ai) : Action(ai, "quest update complete") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddKillAction : public Action
{
public:
    QuestUpdateAddKillAction(PlayerbotAI* ai) : Action(ai, "quest update add kill") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddItemAction : public Action
{
public:
    QuestUpdateAddItemAction(PlayerbotAI* ai) : Action(ai, "quest update add item") {}
    bool Execute(Event event) override;
};

class QuestUpdateFailedAction : public Action
{
public:
    QuestUpdateFailedAction(PlayerbotAI* ai) : Action(ai, "quest update failed") {}
    bool Execute(Event event) override;
};

class QuestUpdateFailedTimerAction : public Action
{
public:
    QuestUpdateFailedTimerAction(PlayerbotAI* ai) : Action(ai, "quest update failed timer") {}
    bool Execute(Event event) override;
};

class QuestItemPushResultAction : public Action
{
public:
    QuestItemPushResultAction(PlayerbotAI* ai) : Action(ai, "quest item push result") {}
    bool Execute(Event event) override;
};

#endif
