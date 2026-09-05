/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGDOQUEST_H
#define PLAYERBOTS_NEWRPGDOQUEST_H

#include "AttackAction.h"
#include "NewRpgBaseAction.h"
#include "NewRpgInfo.h"
#include "PlayerbotAI.h"
#include "QuestValues.h"

class Creature;
class GameObject;
class Item;
class WorldObject;
struct QuestStatusData;

// Distance at which the do-quest engine stops closing in and opens combat.
constexpr float NEW_RPG_ENGAGE_RANGE = 30.0f;

class StartRpgDoQuestAction : public Action
{
public:
    StartRpgDoQuestAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do quest") {}

    bool Execute(Event event) override;
};

class NewRpgAttackQuestTargetAction : public AttackAction
{
public:
    NewRpgAttackQuestTargetAction(PlayerbotAI* botAI) : AttackAction(botAI, "new rpg attack quest target") {}

    bool Execute(Event event) override;
    Unit* GetTarget() override;
};

class NewRpgDoQuestAction : public NewRpgBaseAction
{
public:
    NewRpgDoQuestAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do quest") {}
    bool Execute(Event event) override;

    // Warm the static caches at load: IsCastQuest runs a blocking query.
    static void BuildCaches();

protected:
    bool DoIncompleteQuest(NewRpgInfo::DoQuest& data);
    bool DoCompletedQuest(NewRpgInfo::DoQuest& data);

    /* objective bookkeeping */
    // nullptr when the quest left the log mid-tick; .at() would throw.
    QuestStatusData const* GetQuestStatus(uint32 questId) const;
    bool IsObjectiveCompleted(NewRpgInfo::DoQuest const& data) const;
    bool HasObjectiveProgress(NewRpgInfo::DoQuest const& data) const;
    void ResetObjectiveSpawn(NewRpgInfo::DoQuest& data);
    void ClearTarget(NewRpgInfo::DoQuest& data);

    /* objective navigation - DB spawn points; POI is only the turn-in location */
    bool SelectObjectiveSpawn(NewRpgInfo::DoQuest& data);
    // Last resort when no objective of this quest has a spawn point to walk to.
    bool SelectObjectivePoi(NewRpgInfo::DoQuest& data);
    // Where the quest turns in, by spawn point rather than POI.
    bool SelectQuestEnder(NewRpgInfo::DoQuest& data);

    /* seek & engage */
    bool ScanForObjectiveTarget(NewRpgInfo::DoQuest& data, float maxDist);
    // Nearest valid creature/GO of any incomplete creature-or-GO objective, else nullptr.
    WorldObject* ScanEntryObjectives(NewRpgInfo::DoQuest const& data, float scanRange) const;
    // Nearest creature/GO holding what an item objective needs, else nullptr.
    WorldObject* ScanItemObjective(NewRpgInfo::DoQuest const& data, float scanRange) const;
    bool EngageTarget(NewRpgInfo::DoQuest& data);
    bool EngageCreature(NewRpgInfo::DoQuest& data, Creature* creature);
    bool EngageTalkTarget(NewRpgInfo::DoQuest& data, Creature* creature);
    bool EngageCastTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item);
    // Use the quest item on a creature that a SmartAI SPELLHIT transform turns into the one
    // carrying the required drop (see GetQuestTransformSources).
    bool EngageTransformTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item);
    bool EngageGameObject(NewRpgInfo::DoQuest& data, GameObject* go);
    // First on-use spell of an item, 0 if it has none.
    static uint32 GetItemUseSpell(Item* item);
    // Hold after using a quest item: its cast time plus a tick of slack.
    static uint32 QuestItemUseGrace(uint32 spellId);

    /* cast objective: credit needs the item's spell, which talk/kill credit never uses */
    // True when an incomplete creature objective of this quest names `entry` itself - not a
    // kill source or drop source standing in for it.
    bool IsIncompleteObjectiveEntry(NewRpgInfo::DoQuest const& data, uint32 entry) const;
    Item* GetCastQuestItem(NewRpgInfo::DoQuest const& data);
    // Raw quest_template_addon.SpecialFlags & 0x20: ObjectMgr ORs KILL|CAST|SPEAKTO onto
    // every creature-objective quest at load, so the runtime flag is unusable.
    static bool IsCastQuest(uint32 questId);

    /* craft objective: the required item is made from a looted reagent */
    bool DoCraftObjective(NewRpgInfo::DoQuest& data);

    /* summon objective: the mob has no spawn - the quest item conjures it beside an anchor */
    bool DoSummonObjective(NewRpgInfo::DoQuest& data, QuestSummonAnchor const& anchor);
    // The live anchor to use the item at - spell focus, plain creature, or kill source.
    WorldObject* SelectSummonAnchor(NewRpgInfo::DoQuest const& data, QuestSummonAnchor const& anchor) const;
    // Nearest spell-focus of `focusId` already within casting range, else nullptr.
    GameObject* FindFocusAnchor(uint32 focusId) const;
    Creature* FindCreatureAnchor(uint32 entry, float range) const;

    /* explore objective: area-trigger backed, so it navigates and fires rather than engages */
    bool DoExploreObjective(NewRpgInfo::DoQuest& data);
    // Reverse quest -> area trigger. ObjectMgr only indexes trigger -> quest. 0 = no trigger.
    static uint32 GetQuestAreaTrigger(uint32 questId);
    void WriteOffTarget(NewRpgInfo::DoQuest& data);
    // Written off recently enough to still skip. Expires after writeOffTime.
    bool IsWrittenOff(NewRpgInfo::DoQuest const& data, ObjectGuid guid) const;
    bool IsValidCreatureTarget(NewRpgInfo::DoQuest const& data, Creature* creature) const;
    bool IsValidGoTarget(NewRpgInfo::DoQuest const& data, GameObject* go) const;
    bool HasNeededQuestItem(GameObject* go) const;
    bool HoldForLoot(NewRpgInfo::DoQuest& data);

    const uint32 lootHoldTime = 15 * 1000;
    const uint32 poiStayTime = 5 * 60 * 1000;
    // Standing at an ender spawn: the questgiver search runs every tick at 80 yd, so if nothing
    // has taken the quest by now the spot is empty and the next spawn point is worth a try.
    const uint32 enderWaitTime = 30 * 1000;
    // A written-off target comes back after one destination window: by then the tap has
    // lapsed, the mob has respawned, or the bot stands somewhere else entirely.
    const uint32 writeOffTime = poiStayTime;
    // Whole-quest budget: the spawn rotation never gives up on its own.
    const uint32 questStayTime = 30 * 60 * 1000;
    const uint32 scanInterval = 2 * 1000;
    // Ceiling only; ScanForObjectiveTarget clamps to sightDistance.
    const float objectiveScanRange = 150.0f;
    const uint32 targetEngageTimeout = 45 * 1000;
    const float engageRange = NEW_RPG_ENGAGE_RANGE;
    // Inside the shortest transform spell's range, outside the usual aggro pull.
    const float transformCastRange = 30.0f;
    // Past half of any spell-focus template's dist, which is the core's own casting rule.
    const float summonAnchorScanRange = 25.0f;
    const float summonCreatureAnchorRange = 5.0f;
    // The item has to land before the mob aggroes; combat pauses this engine.
    const float summonKillAnchorRange = 30.0f;
    const float summonTriggerNearRange = 12.0f;
};

#endif
