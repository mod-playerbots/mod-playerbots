/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUESTVALUES_H
#define PLAYERBOTS_QUESTVALUES_H

#include <unordered_set>
#include <vector>

#include "NamedObjectContext.h"
#include "TravelMgr.h"
#include "Value.h"

class Player;
class PlayerbotAI;

struct CreatureData;
struct GameObjectData;

enum class QuestRelationFlag : uint32
{
    none = 0,
    objective1 = 1,
    objective2 = 2,
    objective3 = 4,
    objective4 = 8,
    questGiver = 16,
    questTaker = 32,
    maxQuestRelationFlag = 64
};

//                     questId, QuestRelationFlag
typedef std::unordered_map<uint32, uint32> questRelationMap;
//                     entry
typedef std::unordered_map<int32, questRelationMap> entryQuestRelationMap;

//                      entry
typedef std::unordered_map<int32, std::vector<GuidPosition>> questEntryGuidps;

//                      QuestRelationFlag
typedef std::unordered_map<uint32, questEntryGuidps> questRelationGuidps;

//                      questId
typedef std::unordered_map<uint32, questRelationGuidps> questGuidpMap;

//                      questId
typedef std::unordered_map<uint32, std::vector<GuidPosition>> questGiverMap;

// Returns the quest relation Flags for all entries and quests
class EntryQuestRelationMapValue : public SingleCalculatedValue<entryQuestRelationMap>
{
public:
    EntryQuestRelationMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "entry quest relation map") {}

    entryQuestRelationMap Calculate() override;
};

// Generic quest object finder
class FindQuestObjectData
{
public:
    FindQuestObjectData() { GetObjectiveEntries(); }

    void GetObjectiveEntries();
    void operator()(CreatureData const& creatureData);
    void operator()(GameObjectData const& gameobjectData);
    questGuidpMap GetResult() const { return data; };

private:
    std::unordered_map<int32, std::vector<std::pair<uint32, QuestRelationFlag>>> entryMap;
    std::unordered_map<uint32, std::vector<std::pair<uint32, QuestRelationFlag>>> itemMap;

    entryQuestRelationMap relationMap;

    questGuidpMap data;
};

// All objects to start, do or finish a quest.
class QuestGuidpMapValue : public SingleCalculatedValue<questGuidpMap>
{
public:
    QuestGuidpMapValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest guidp map") {}

    questGuidpMap Calculate() override;
};

// All questgivers and their quests that are Useful for a specific level
class QuestGiversValue : public SingleCalculatedValue<questGiverMap>, public Qualified
{
public:
    QuestGiversValue(PlayerbotAI* botAI) : SingleCalculatedValue(botAI, "quest givers") {}

    questGiverMap Calculate() override;
};

// Quest objective spawn index.
// Everything below is built by BuildQuestObjectiveSpawns and read-only afterwards.

// Single-threaded, before any bot reads it.
void BuildQuestObjectiveSpawns();

// Entries satisfying one objective of a quest, keyed by objective bit (objective1..4, then
// item objectives at QUEST_OBJECTIVES_COUNT + i). Negative = gameobject. Empty if the quest
// has none.
std::vector<int32> const& GetQuestObjectiveEntries(uint32 questId, uint32 objectiveFlag);

// Spawn positions of one entry on one map.
std::vector<GuidPosition> const& GetEntrySpawns(int32 entry, uint32 mapId);

// Creatures and gameobjects the quest turns in at. Negative = gameobject.
std::vector<int32> const& GetQuestEnderEntries(uint32 questId);

// Creature entries this quest's provided item transforms into the creature that carries a
// required drop (SmartAI SPELLHIT -> UPDATE_TEMPLATE). Empty for the vast majority of quests.
std::unordered_set<uint32> const& GetQuestTransformSources(uint32 questId);

// Where an objective mob with no spawn row has to be conjured with the quest's provided item:
// beside a spell-focus gameobject, which the core enforces, or beside a creature, which only the
// summoned trigger's own script enforces. Both zero for an ordinary objective.
struct QuestSummonAnchor
{
    uint32 focusId{0};
    uint32 creature{0};
    // The anchor is also the kill target, so the item has to land before the fight starts.
    bool killAnchor{false};
    // The item's spell summons the objective itself. When it does not, the objective is an
    // ordinary spawned mob and its presence must not suppress a re-use.
    bool summonsObjective{false};
};

QuestSummonAnchor GetQuestSummonAnchor(uint32 questId, uint32 objectiveIdx);

// Spawned creatures to kill for this objective when the entry the quest names is not killable
// itself: a credit marker some other mob credits on death, or one that does not exist until a
// mob summons it. Empty for an ordinary objective.
std::vector<uint32> const& GetQuestKillSources(uint32 questId, uint32 objectiveIdx);

// Reagents a craft objective's required item is made from, when that item drops nowhere. Empty
// for an ordinary looted objective. Indexed by item objective, not by objectiveIdx.
struct QuestCraftReagent
{
    uint32 item{0};
    uint32 count{0};
};

std::vector<QuestCraftReagent> const& GetQuestCraftReagents(uint32 questId, uint32 itemObjectiveIdx);

// The item whose on-use spell crafts this objective's required item. 0 if it is not a craft
// objective. The reagents do not name it: it is only the carrier when it consumes itself.
uint32 GetQuestCraftItem(uint32 questId, uint32 itemObjectiveIdx);

// Reagents the quest's provided item consumes per use. Empty if it consumes nothing.
std::vector<QuestCraftReagent> const& GetQuestItemReagents(uint32 questId);

// True while the bot still wants `itemId` for `quest` on grounds RequiredItemId does not cover:
// a quest-only ItemDrop, or a reagent the provided item spends.
bool IsQuestExtraItemWanted(Player* bot, Quest const* quest, uint32 itemId);

// All questgivers that have a quest for the bot.
class ActiveQuestGiversValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestGiversValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest givers", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// All quest takers that the bot has a quest for.
class ActiveQuestTakersValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestTakersValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest takers", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// All objectives that the bot still has to complete.
class ActiveQuestObjectivesValue : public CalculatedValue<std::vector<GuidPosition>>
{
public:
    ActiveQuestObjectivesValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "active quest objectives", 5) {}

    std::vector<GuidPosition> Calculate() override;
};

// Free quest log slots
class FreeQuestLogSlotValue : public Uint8CalculatedValue
{
public:
    FreeQuestLogSlotValue(PlayerbotAI* botAI) : Uint8CalculatedValue(botAI, "free quest log slots", 2) {}

    uint8 Calculate() override;
};

// Dialog status npc
class DialogStatusValue : public Uint32CalculatedValue, public Qualified
{
public:
    DialogStatusValue(PlayerbotAI* botAI, std::string const name = "dialog status")
        : Uint32CalculatedValue(botAI, name, 2)
    {
    }

    static uint32 getDialogStatus(Player* bot, int32 questgiver, uint32 questId = 0);

    uint32 Calculate() override;
};

// Dialog status npc quest
class DialogStatusQuestValue : public DialogStatusValue
{
public:
    DialogStatusQuestValue(PlayerbotAI* botAI) : DialogStatusValue(botAI, "dialog status quest") {}

    uint32 Calculate() override;
};

// Can accept quest from npc
class CanAcceptQuestValue : public BoolCalculatedValue, public Qualified
{
public:
    CanAcceptQuestValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can accept quest npc") {}

    bool Calculate() override;
};

// Can accept low level quest from npc
class CanAcceptQuestLowLevelValue : public BoolCalculatedValue, public Qualified
{
public:
    CanAcceptQuestLowLevelValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can accept quest low level npc") {}

    bool Calculate() override;
};

// Can hand in quest to npc
class CanTurnInQuestValue : public BoolCalculatedValue, public Qualified
{
public:
    CanTurnInQuestValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "can turn in quest npc") {}

    bool Calculate() override;
};

#endif
