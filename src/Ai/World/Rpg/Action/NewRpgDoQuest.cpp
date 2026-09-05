/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NewRpgDoQuest.h"
#include "BroadcastHelper.h"
#include "CellImpl.h"
#include "ChatHelper.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "G3D/Vector2.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "LastMovementValue.h"
#include "LootMgr.h"
#include "LootObjectStack.h"
#include "NearestGameObjects.h"
#include "NewRpgInfo.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "Random.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "TravelMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>
#include <cfloat>
#include <unordered_map>

bool StartRpgDoQuestAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(owner);
    uint32 questId = ch.extractQuestId(text);
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (quest)
    {
        botAI->rpgInfo.ChangeToDoQuest(questId, quest);
        bot->Whisper("Start to do quest " + std::to_string(questId), LANG_UNIVERSAL, owner);
        return true;
    }
    bot->Whisper("Invalid quest " + text, LANG_UNIVERSAL, owner);
    return false;
}

Unit* NewRpgAttackQuestTargetAction::GetTarget()
{
    auto* data = std::get_if<NewRpgInfo::DoQuest>(&botAI->rpgInfo.data);
    if (!data || !data->targetGuid.IsAnyTypeCreature())
        return nullptr;

    return botAI->GetUnit(data->targetGuid);
}

bool NewRpgAttackQuestTargetAction::Execute(Event /*event*/)
{
    verbose = false;

    Unit* target = GetTarget();
    if (!target || !target->IsInWorld() || !target->IsAlive())
        return false;

    if (bot->IsFriendlyTo(target) || !bot->IsValidAttackTarget(target))
        return false;

    bot->SetSelection(target->GetGUID());
    context->GetValue<Unit*>("current target")->Set(target);

    // Halt at commit range even when a later rule defers the attack this
    // tick - the in-flight spline ends at the mob's exact position.
    LastMovement& lastMovement = AI_VALUE(LastMovement&, "last movement");
    bool moveControlled = bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
    if (bot->isMoving() && lastMovement.priority < MovementPriority::MOVEMENT_COMBAT && !moveControlled)
    {
        lastMovement.clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }

    bool result = Attack(target);
    LOG_DEBUG("playerbots", "[New RPG] {} attack quest target {} ({}): {}", bot->GetName(), target->GetName(),
              target->GetGUID().ToString(), result ? "engaged" : "refused");
    return result;
}

bool NewRpgDoQuestAction::Execute(Event /*event*/)
{
    if (SearchQuestGiverAndAcceptOrReward())
        return true;

    NewRpgInfo& info = botAI->rpgInfo;
    auto* dataPtr = std::get_if<NewRpgInfo::DoQuest>(&info.data);
    if (!dataPtr)
        return false;
    auto& data = *dataPtr;
    uint32 questId = data.questId;
    uint8 questStatus = bot->GetQuestStatus(questId);
    switch (questStatus)
    {
        case QUEST_STATUS_INCOMPLETE:
            return DoIncompleteQuest(data);
        case QUEST_STATUS_COMPLETE:
            return DoCompletedQuest(data);
        default:
            break;
    }
    info.ChangeToIdle();
    return true;
}

QuestStatusData const* NewRpgDoQuestAction::GetQuestStatus(uint32 questId) const
{
    auto const& statusMap = bot->getQuestStatusMap();
    auto it = statusMap.find(questId);
    return it == statusMap.end() ? nullptr : &it->second;
}

bool NewRpgDoQuestAction::IsObjectiveCompleted(NewRpgInfo::DoQuest const& data) const
{
    // objectiveIdx is -1 while heading to turn-in. If the quest reverted to incomplete,
    // report done so a fresh objective is picked instead of indexing arrays with -1.
    if (data.objectiveIdx < 0)
        return true;

    Quest const* quest = sObjectMgr->GetQuestTemplate(data.questId);
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!quest || !q_status)
        return true;

    int32 idx = data.objectiveIdx;
    if (idx < QUEST_OBJECTIVES_COUNT)
        return q_status->CreatureOrGOCount[idx] >= quest->RequiredNpcOrGoCount[idx];

    if (idx < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        return q_status->ItemCount[idx - QUEST_OBJECTIVES_COUNT] >=
               quest->RequiredItemCount[idx - QUEST_OBJECTIVES_COUNT];

    return true;
}

bool NewRpgDoQuestAction::HasObjectiveProgress(NewRpgInfo::DoQuest const& data) const
{
    if (data.objectiveIdx < 0)
        return false;

    Quest const* quest = sObjectMgr->GetQuestTemplate(data.questId);
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!quest || !q_status)
        return false;

    int32 idx = data.objectiveIdx;
    if (idx < QUEST_OBJECTIVES_COUNT)
        return q_status->CreatureOrGOCount[idx] != 0 && quest->RequiredNpcOrGoCount[idx];

    if (idx < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT)
        return q_status->ItemCount[idx - QUEST_OBJECTIVES_COUNT] != 0 &&
               quest->RequiredItemCount[idx - QUEST_OBJECTIVES_COUNT];

    return false;
}

void NewRpgDoQuestAction::ClearTarget(NewRpgInfo::DoQuest& data)
{
    if (data.targetGuid.IsGameObject())
    {
        LootObject lootTarget = AI_VALUE(LootObject, "loot target");
        if (lootTarget.guid == data.targetGuid)
            context->GetValue<LootObject>("loot target")->Set(LootObject());
    }
    data.targetGuid = ObjectGuid::Empty;
    data.targetPos = WorldPosition();
    data.targetSince = 0;
    data.lastEngage = 0;
}

void NewRpgDoQuestAction::ResetObjectiveSpawn(NewRpgInfo::DoQuest& data)
{
    data.spawnGuid = 0;
    data.pos = WorldPosition();
    data.objectiveIdx = 0;
    data.lastReachPOI = 0;
    data.spawnSince = 0;
    data.lastScan = 0;
    data.poiFallback = false;
    ClearTarget(data);
}

bool NewRpgDoQuestAction::SelectObjectiveSpawn(NewRpgInfo::DoQuest& data)
{
    Quest const* quest = data.quest;
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!quest || !q_status)
        return false;

    GuidPosition const* best = nullptr;
    int32 bestObjective = 0;
    float bestDist = FLT_MAX;

    // Item objectives follow the creature/GO ones at QUEST_OBJECTIVES_COUNT + i, indexed
    // against whoever drops them, so a quest needing a drop walks to its source mobs.
    for (int32 idx = 0; idx < QUEST_OBJECTIVES_COUNT + QUEST_ITEM_OBJECTIVES_COUNT; ++idx)
    {
        if (idx < QUEST_OBJECTIVES_COUNT)
        {
            if (!quest->RequiredNpcOrGo[idx] ||
                q_status->CreatureOrGOCount[idx] >= quest->RequiredNpcOrGoCount[idx])
                continue;
        }
        else
        {
            int32 const item = idx - QUEST_OBJECTIVES_COUNT;
            if (!quest->RequiredItemId[item] || !quest->RequiredItemCount[item] ||
                q_status->ItemCount[item] >= quest->RequiredItemCount[item])
                continue;
        }

        for (int32 entry : GetQuestObjectiveEntries(data.questId, static_cast<uint32>(1 << idx)))
        {
            for (GuidPosition const& pos : GetEntrySpawns(entry, bot->GetMapId()))
            {
                if (data.triedSpawns.count(pos.GetRawValue()))
                    continue;

                float const dist = bot->GetDistance(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                if (dist < bestDist)
                {
                    bestDist = dist;
                    best = &pos;
                    bestObjective = idx;
                }
            }
        }
    }

    if (!best)
    {
        if (data.triedSpawns.empty())
            return false;  // nothing spawned for any incomplete objective on this map

        // every spawn has had its window without progress - let them be retried
        data.triedSpawns.clear();
        return true;
    }

    data.spawnGuid = best->GetRawValue();
    data.objectiveIdx = bestObjective;
    data.pos = *best;
    data.lastReachPOI = 0;
    data.spawnSince = getMSTime();
    data.lastScan = 0;
    return true;
}

// Objectives whose entries have no spawn row anywhere. Fall back to what the
// quest's own POI says and let the grind strategy work the area,
// which is all this action did before it targeted spawns.
bool NewRpgDoQuestAction::SelectObjectivePoi(NewRpgInfo::DoQuest& data)
{
    std::vector<POIInfo> poiInfo;
    if (!GetQuestPOIPosAndObjectiveIdx(data.questId, poiInfo))
        return false;

    POIInfo const& poi = poiInfo[urand(0, poiInfo.size() - 1)];
    data.spawnGuid = 0;
    data.objectiveIdx = poi.objectiveIdx;
    data.pos = WorldPosition(bot->GetMapId(), poi.pos.x, poi.pos.y, poi.z);
    data.poiFallback = true;
    data.lastReachPOI = 0;
    data.spawnSince = getMSTime();
    data.lastScan = 0;
    return true;
}

// The turn-in's own spawn point, which the quest POI cannot give: its rows carry x/y only.
// Same relation the core's `.go quest ender` walks.
bool NewRpgDoQuestAction::SelectQuestEnder(NewRpgInfo::DoQuest& data)
{
    GuidPosition const* best = nullptr;
    float bestDist = FLT_MAX;
    for (int32 entry : GetQuestEnderEntries(data.questId))
    {
        for (GuidPosition const& pos : GetEntrySpawns(entry, bot->GetMapId()))
        {
            if (data.triedSpawns.count(pos.GetRawValue()))
                continue;

            float const dist = bot->GetDistance(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
            if (dist < bestDist)
            {
                bestDist = dist;
                best = &pos;
            }
        }
    }

    if (!best)
        return false;

    data.spawnGuid = best->GetRawValue();
    data.pos = *best;
    return true;
}

bool NewRpgDoQuestAction::IsIncompleteObjectiveEntry(NewRpgInfo::DoQuest const& data, uint32 entry) const
{
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!data.quest || !q_status)
        return false;

    for (int32 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
        if (data.quest->RequiredNpcOrGo[idx] == int32(entry) &&
            q_status->CreatureOrGOCount[idx] < data.quest->RequiredNpcOrGoCount[idx])
            return true;

    return false;
}

bool NewRpgDoQuestAction::IsWrittenOff(NewRpgInfo::DoQuest const& data, ObjectGuid guid) const
{
    auto it = data.visited.find(guid);
    return it != data.visited.end() && GetMSTimeDiffToNow(it->second) < writeOffTime;
}

bool NewRpgDoQuestAction::IsValidCreatureTarget(NewRpgInfo::DoQuest const& data, Creature* creature) const
{
    if (!creature || !creature->IsInWorld() || !creature->IsAlive())
        return false;

    if (IsWrittenOff(data, creature->GetGUID()))
        return false;

    // tapped by someone else - no credit/loot for us
    if (creature->hasLootRecipient() && !creature->isTappedBy(bot))
        return false;

    return true;
}

bool NewRpgDoQuestAction::IsValidGoTarget(NewRpgInfo::DoQuest const& data, GameObject* go) const
{
    if (!go || !go->IsInWorld() || !go->isSpawned())
        return false;

    if (go->GetGoState() != GO_STATE_READY)
        return false;

    if (IsWrittenOff(data, go->GetGUID()))
        return false;

    return true;
}

bool NewRpgDoQuestAction::HasNeededQuestItem(GameObject* go) const
{
    GameObjectQuestItemList const* items = sObjectMgr->GetGameObjectQuestItemList(go->GetEntry());
    if (!items)
        return false;

    for (uint32 itemId : *items)
        if (LootObject::IsNeededForQuest(bot, itemId))
            return true;

    return false;
}

// Yield the tick while loot is pending, bounded: nothing guarantees the pipeline finishes,
// and a corpse that despawned leaves "loot target" set with nothing able to clear it.
bool NewRpgDoQuestAction::HoldForLoot(NewRpgInfo::DoQuest& data)
{
    // The loot pipeline runs at higher relevance, so "loot target" is already committed.
    if (AI_VALUE(LootObject, "loot target").IsEmpty())
    {
        data.lootHoldSince = 0;
        return false;
    }

    if (!data.lootHoldSince)
        data.lootHoldSince = getMSTime();

    return GetMSTimeDiffToNow(data.lootHoldSince) < lootHoldTime;
}

void NewRpgDoQuestAction::WriteOffTarget(NewRpgInfo::DoQuest& data)
{
    if (data.targetGuid)
        data.visited[data.targetGuid] = getMSTime();
    ClearTarget(data);
}

WorldObject* NewRpgDoQuestAction::ScanEntryObjectives(NewRpgInfo::DoQuest const& data, float scanRange) const
{
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!q_status)
        return nullptr;

    WorldObject* nearest = nullptr;
    float nearestDist = scanRange;
    auto keepNearest = [&](WorldObject* obj)
    {
        float const dist = bot->GetDistance(obj);
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearest = obj;
        }
    };

    // Every incomplete objective, not just the one being walked to - a mob of another is
    // free progress. Collected first so all of them cost one sweep.
    std::vector<uint32> creatureEntries;
    std::vector<uint32> goEntries;
    for (int32 idx = 0; idx < QUEST_OBJECTIVES_COUNT; ++idx)
    {
        int32 const entry = data.quest->RequiredNpcOrGo[idx];
        if (!entry || q_status->CreatureOrGOCount[idx] >= data.quest->RequiredNpcOrGoCount[idx])
            continue;

        if (entry > 0)
        {
            // The objective entry plus anything that has to die in its place: a marker the
            // quest names is never spawned itself, so hunting its entry alone finds nothing.
            creatureEntries.push_back(uint32(entry));
            std::vector<uint32> const& sources = GetQuestKillSources(data.questId, uint32(idx));
            creatureEntries.insert(creatureEntries.end(), sources.begin(), sources.end());
        }
        else
        {
            goEntries.push_back(static_cast<uint32>(-entry));
        }
    }

    if (!creatureEntries.empty())
    {
        std::list<Creature*> creatures;
        bot->GetCreatureListWithEntryInGrid(creatures, creatureEntries, scanRange);
        for (Creature* creature : creatures)
            if (IsValidCreatureTarget(data, creature))
                keepNearest(creature);
    }

    if (!goEntries.empty())
    {
        std::list<GameObject*> gos;
        bot->GetGameObjectListWithEntryInGrid(gos, goEntries, scanRange);
        for (GameObject* go : gos)
            if (IsValidGoTarget(data, go))
                keepNearest(go);
    }

    return nearest;
}

WorldObject* NewRpgDoQuestAction::ScanItemObjective(NewRpgInfo::DoQuest const& data, float scanRange) const
{
    std::vector<int32> const* craftSources = nullptr;
    if (!GetQuestCraftReagents(data.questId, uint32(data.objectiveIdx) - QUEST_OBJECTIVES_COUNT).empty())
        craftSources = &GetQuestObjectiveEntries(data.questId, 1u << data.objectiveIdx);

    WorldObject* nearest = nullptr;
    float nearestDist = scanRange;
    auto keepNearest = [&](WorldObject* obj)
    {
        float const dist = bot->GetDistance(obj);
        if (dist < nearestDist)
        {
            nearestDist = dist;
            nearest = obj;
        }
    };

    std::unordered_set<uint32> const& transformSources = GetQuestTransformSources(data.questId);
    std::unordered_map<uint32, bool> questLoot;
    std::list<Unit*> units;
    Acore::AnyUnitInObjectRangeCheck check(bot, scanRange);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, scanRange);
    for (Unit* unit : units)
    {
        Creature* creature = unit->ToCreature();
        if (!creature || !IsValidCreatureTarget(data, creature))
            continue;

        // Either it drops what we need, or the quest item turns it into something
        // that does (GetQuestTransformSources), or it carries a craft reagent.
        uint32 lootId = creature->GetCreatureTemplate()->lootid;
        bool const dropsReagent =
            craftSources && std::find(craftSources->begin(), craftSources->end(),
                                      int32(creature->GetEntry())) != craftSources->end();
        bool dropsWanted = false;
        if (!dropsReagent && lootId)
        {
            auto cached = questLoot.find(lootId);
            if (cached == questLoot.end())
                cached = questLoot.emplace(lootId, LootTemplates_Creature.HaveQuestLootForPlayer(lootId, bot)).first;

            dropsWanted = cached->second;
        }

        if (!dropsReagent && !dropsWanted && !transformSources.count(creature->GetEntry()))
            continue;

        keepNearest(creature);
    }

    std::list<GameObject*> gos;
    AnyGameObjectInObjectRangeCheck goCheck(bot, scanRange);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> goSearcher(bot, gos, goCheck);
    Cell::VisitObjects(bot, goSearcher, scanRange);
    for (GameObject* go : gos)
        if (IsValidGoTarget(data, go) && HasNeededQuestItem(go))
            keepNearest(go);

    return nearest;
}

bool NewRpgDoQuestAction::ScanForObjectiveTarget(NewRpgInfo::DoQuest& data, float maxDist)
{
    if (!data.quest)
        return false;

    // Item objectives name no source entry, so they scan by loot predicate, not by entry.
    // Clamped to sightDistance: the bot cannot act on anything further out.
    float const scanRange = std::min({maxDist, objectiveScanRange, sPlayerbotAIConfig.sightDistance});
    WorldObject* target = data.objectiveIdx < QUEST_OBJECTIVES_COUNT ? ScanEntryObjectives(data, scanRange)
                                                                     : ScanItemObjective(data, scanRange);
    if (!target)
        return false;

    data.targetGuid = target->GetGUID();
    data.targetPos = WorldPosition(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(),
                                   target->GetPositionZ());
    data.targetSince = 0;
    LOG_DEBUG("playerbots", "[New RPG] {} found quest target {} (dist {}) for quest {} objective {}",
              bot->GetName(), data.targetGuid.ToString(), bot->GetDistance(target), data.questId,
              data.objectiveIdx);
    return true;
}

bool NewRpgDoQuestAction::EngageTarget(NewRpgInfo::DoQuest& data)
{
    if (bot->IsInCombat())
    {
        data.targetSince = getMSTime();
        return false;
    }

    if (data.lastEngage && GetMSTimeDiffToNow(data.lastEngage) > 5000)
        data.targetSince = getMSTime();
    data.lastEngage = getMSTime();

    if (data.targetGuid.IsAnyTypeCreature())
    {
        Unit* unit = botAI->GetUnit(data.targetGuid);
        Creature* creature = unit ? unit->ToCreature() : nullptr;
        if (!creature || !creature->IsAlive())
        {
            // Killed or despawned - rescan. Queue the corpse first.
            if (creature)
                AI_VALUE(LootObjectStack*, "available loot")->Add(data.targetGuid);
            ClearTarget(data);
            return true;
        }

        if (creature->hasLootRecipient() && !creature->isTappedBy(bot))
        {
            WriteOffTarget(data);
            return true;
        }

        if (!data.targetSince)
            data.targetSince = getMSTime();
        else if (GetMSTimeDiffToNow(data.targetSince) >= targetEngageTimeout)
        {
            // unreachable, or the attack action keeps refusing it - don't chase forever
            LOG_DEBUG("playerbots", "[New RPG] {} wrote off quest target {} for quest {}",
                      bot->GetName(), data.targetGuid.ToString(), data.questId);
            WriteOffTarget(data);
            return true;
        }

        return EngageCreature(data, creature);
    }

    if (data.targetGuid.IsGameObject())
    {
        GameObject* go = botAI->GetGameObject(data.targetGuid);
        if (!go || !go->isSpawned())
        {
            // opened/used (by us or others) or despawned - rescan
            ClearTarget(data);
            return true;
        }

        if (!data.targetSince)
            data.targetSince = getMSTime();
        else if (GetMSTimeDiffToNow(data.targetSince) >= targetEngageTimeout)
        {
            LOG_DEBUG("playerbots", "[New RPG] {} wrote off quest target {} for quest {}",
                      bot->GetName(), data.targetGuid.ToString(), data.questId);
            WriteOffTarget(data);
            return true;
        }

        return EngageGameObject(data, go);
    }

    ClearTarget(data);
    return true;
}

bool NewRpgDoQuestAction::EngageCreature(NewRpgInfo::DoQuest& data, Creature* creature)
{
    // Cast objective: takes precedence over talk/attack.
    if (IsIncompleteObjectiveEntry(data, creature->GetEntry()))
        if (Item* castItem = GetCastQuestItem(data))
            return EngageCastTarget(data, creature, castItem);

    // Transform objective: the item has to be used before the kill. Afterwards the
    // transformed mob is an ordinary target and falls through below.
    if (data.quest && GetQuestTransformSources(data.questId).count(creature->GetEntry()))
        if (Item* item = bot->GetItemByEntry(data.quest->GetSrcItemId()))
            return EngageTransformTarget(data, creature, item);

    // Friendly objective NPC -> talk-to credit, not a kill.
    if (!bot->IsValidAttackTarget(creature))
        return EngageTalkTarget(data, creature);

    // Re-anchor only when the mob really moved
    if (creature->GetExactDist(data.targetPos) > 10.0f)
        data.targetPos = WorldPosition(bot->GetMapId(), creature->GetPositionX(), creature->GetPositionY(),
                                       creature->GetPositionZ());

    float const dist = bot->GetDistance(creature);

    // En-route re-target. Each switch needs a 2x improvement in distance, so two
    // near-equidistant mobs can't ping-pong. The displaced mob is not written off.
    if (dist > engageRange && (!data.lastScan || GetMSTimeDiffToNow(data.lastScan) >= scanInterval))
    {
        data.lastScan = getMSTime();
        ObjectGuid const held = data.targetGuid;
        if (ScanForObjectiveTarget(data, dist * 0.5f) && data.targetGuid != held)
        {
            data.lastEngage = 0;
            LOG_DEBUG("playerbots", "[New RPG] {} switched quest target {} -> {} for quest {}",
                      bot->GetName(), held.ToString(), data.targetGuid.ToString(), data.questId);
            return true;
        }
    }

    // LOS is part of the commit condition: while it is blocked, keep closing in rather
    // than retrying an attack that cannot land.
    if (dist > engageRange || !bot->IsWithinLOSInMap(creature))
    {
        if (MoveFarTo(data.targetPos))
            return true;
        // Same fallback the talk/cast/transform approaches take: a target the sampler cannot
        // path to must not pin the bot in place. Nudging it re-samples from somewhere else
        // next tick, and targetEngageTimeout retires the target if that never helps.
        return MoveRandomNear(10.0f);
    }

    return botAI->DoSpecificAction("new rpg attack quest target", Event(), true);
}

bool NewRpgDoQuestAction::EngageGameObject(NewRpgInfo::DoQuest& data, GameObject* go)
{
    if (bot->GetDistance(go) > INTERACTION_DISTANCE - 2.0f)
    {
        if (bot->isMoving())
            return false;
        return MoveTo(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ(),
                      false, false, false, false);
    }

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }
    if (bot->GetShapeshiftForm() != FORM_NONE)
    {
        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        return true;
    }

    // Lootable chest: hand it to the loot pipeline. Progress comes from the quest items in
    // the loot - the core grants a gameobject credit only for goobers.
    LootObject lootObj(bot, go->GetGUID());
    if (go->GetGoType() == GAMEOBJECT_TYPE_CHEST && !lootObj.IsEmpty())
    {
        LootObject currentTarget = AI_VALUE(LootObject, "loot target");
        bool keepCurrent = !currentTarget.IsEmpty() && currentTarget.IsLootPossible(bot) &&
                           AI_VALUE2(float, "distance", "loot target") <= sPlayerbotAIConfig.lootDistance;
        if (currentTarget.guid != go->GetGUID() && !keepCurrent)
            context->GetValue<LootObject>("loot target")->Set(LootObject(bot, go->GetGUID()));
        return AI_VALUE(LootObjectStack*, "available loot")->Add(go->GetGUID());
    }

    // One attempt then write off: Use() credits goobers directly (KillCreditGO), while
    // buttons and quest chests credit only through a linked trap or their loot.
    WorldPacket packet(CMSG_GAMEOBJ_USE, 8);
    packet << go->GetGUID();
    bot->GetSession()->HandleGameObjectUseOpcode(packet);
    LOG_DEBUG("playerbots", "[New RPG] {} used quest gameobject {} for quest {}",
              bot->GetName(), go->GetEntry(), data.questId);
    WriteOffTarget(data);
    return true;
}

bool NewRpgDoQuestAction::EngageTalkTarget(NewRpgInfo::DoQuest& data, Creature* creature)
{
    // Claim the tick rather than yielding on isMoving like the GO path: yielding lets a
    // competing strategy pull the bot off a talk target it cannot attack.
    if (bot->GetDistance(creature) > INTERACTION_DISTANCE - 1.0f)
    {
        if (MoveFarTo(data.targetPos))
            return true;
        return MoveRandomNear(5.0f);
    }

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }

    // TalkedToCreature grants the SPEAKTO credit directly. The gossip handler's own call is
    // commented out (NPCHandler.cpp), so opening a gossip menu would credit nothing.
    bot->SetFacingToObject(creature);
    bot->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
    LOG_DEBUG("playerbots", "[New RPG] {} talked to quest npc {} for quest {}",
              bot->GetName(), creature->GetEntry(), data.questId);
    WriteOffTarget(data);
    return true;
}

void NewRpgDoQuestAction::BuildCaches()
{
    IsCastQuest(0);
    GetQuestAreaTrigger(0);
}

bool NewRpgDoQuestAction::IsCastQuest(uint32 questId)
{
    // One query, cached for the process: the raw CAST bit is gone from the Quest object
    // at runtime, so it has to come from the DB column.
    static std::unordered_set<uint32> const castQuests = []
    {
        std::unordered_set<uint32> s;
        if (QueryResult result =
                WorldDatabase.Query("SELECT ID FROM quest_template_addon WHERE (SpecialFlags & 0x20) <> 0"))
            do
            {
                s.insert((*result)[0].Get<uint32>());
            } while (result->NextRow());
        return s;
    }();

    return castQuests.count(questId) != 0;
}

Item* NewRpgDoQuestAction::GetCastQuestItem(NewRpgInfo::DoQuest const& data)
{
    if (!IsCastQuest(data.questId) || !data.quest)
        return nullptr;

    uint32 itemId = data.quest->GetSrcItemId();
    if (!itemId)
        return nullptr;

    return bot->GetItemByEntry(itemId);
}

uint32 NewRpgDoQuestAction::GetItemUseSpell(Item* item)
{
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        if (item->GetTemplate()->Spells[i].SpellId > 0)
            return item->GetTemplate()->Spells[i].SpellId;

    return 0;
}

bool NewRpgDoQuestAction::EngageTransformTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item)
{
    // From range, unlike EngageCastTarget: combat pauses this engine, so a bot that walked
    // into melee would kill the mob untransformed.
    if (bot->GetDistance(creature) > transformCastRange || !bot->IsWithinLOSInMap(creature))
    {
        if (MoveFarTo(data.targetPos))
            return true;
        return MoveRandomNear(10.0f);
    }

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }

    // Same cooldown hold as EngageCastTarget. Hold the target rather than write it off;
    // targetEngageTimeout is the backstop if the cast never lands.
    uint32 const spellId = GetItemUseSpell(item);
    if (spellId && bot->HasSpellItemCooldown(spellId, item->GetEntry()))
        return true;

    // Not written off: the creature keeps its guid, SmartAI turns it into the entry that
    // carries the drop, and the next tick engages it as an ordinary kill/loot target.
    bot->SetFacingToObject(creature);
    botAI->ImbueItem(item, creature);
    LOG_DEBUG("playerbots", "[New RPG] {} used quest item {} to transform npc {} for quest {}",
              bot->GetName(), item->GetEntry(), creature->GetEntry(), data.questId);
    return true;
}

bool NewRpgDoQuestAction::EngageCastTarget(NewRpgInfo::DoQuest& data, Creature* creature, Item* item)
{
    if (bot->GetDistance(creature) > INTERACTION_DISTANCE - 1.0f)
    {
        if (MoveFarTo(data.targetPos))
            return true;
        return MoveRandomNear(5.0f);
    }

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }

    uint32 const spellId = GetItemUseSpell(item);
    if (spellId && bot->HasSpellItemCooldown(spellId, item->GetEntry()))
        return true;

    // The item's SPELL_EFFECT_KILL_CREDIT (or scripted credit) is what advances the
    // objective. A mis-cast is retried when the write-off list clears on progress.
    bot->SetFacingToObject(creature);
    botAI->ImbueItem(item, creature);
    LOG_DEBUG("playerbots", "[New RPG] {} used quest item {} on npc {} for quest {}",
              bot->GetName(), item->GetEntry(), creature->GetEntry(), data.questId);
    WriteOffTarget(data);
    return true;
}

bool NewRpgDoQuestAction::DoCraftObjective(NewRpgInfo::DoQuest& data)
{
    if (data.objectiveIdx < QUEST_OBJECTIVES_COUNT || !data.quest)
        return false;

    uint32 const itemIdx = uint32(data.objectiveIdx) - QUEST_OBJECTIVES_COUNT;
    std::vector<QuestCraftReagent> const& craft = GetQuestCraftReagents(data.questId, itemIdx);
    if (craft.empty() || IsObjectiveCompleted(data))
        return false;

    for (QuestCraftReagent const& reagent : craft)
        if (bot->GetItemCount(reagent.item, false) < reagent.count)
            return false;

    Item* item = bot->GetItemByEntry(GetQuestCraftItem(data.questId, itemIdx));
    if (!item)
        return false;

    uint32 const spellId = GetItemUseSpell(item);
    if (!spellId || bot->HasSpellItemCooldown(spellId, item->GetEntry()))
        return false;

    if (data.lastItemUse && GetMSTimeDiffToNow(data.lastItemUse) < QuestItemUseGrace(spellId))
        return true;

    if (bot->IsNonMeleeSpellCast(false))
        return true;

    botAI->ImbueItem(item);
    data.lastItemUse = getMSTime();
    LOG_DEBUG("playerbots", "[New RPG] {} used quest item {} to craft for quest {}", bot->GetName(),
              item->GetEntry(), data.questId);
    return true;
}

WorldObject* NewRpgDoQuestAction::SelectSummonAnchor(NewRpgInfo::DoQuest const& data,
                                                     QuestSummonAnchor const& anchor) const
{
    if (anchor.focusId)
        return FindFocusAnchor(anchor.focusId);

    if (!anchor.killAnchor)
        return FindCreatureAnchor(anchor.creature, summonCreatureAnchorRange);

    // Kill sources are alternates of one another, so take whichever the bot reached,
    // skipping the one it already used the item on so the fight can follow.
    Creature* best = nullptr;
    float nearest = summonKillAnchorRange;
    for (uint32 entry : GetQuestKillSources(data.questId, uint32(data.objectiveIdx)))
    {
        Creature* candidate = FindCreatureAnchor(entry, summonKillAnchorRange);
        if (!candidate || candidate->GetGUID() == data.lastSummonAnchor)
            continue;

        float const dist = bot->GetDistance(candidate);
        if (dist < nearest)
        {
            nearest = dist;
            best = candidate;
        }
    }

    return best;
}

bool NewRpgDoQuestAction::DoSummonObjective(NewRpgInfo::DoQuest& data, QuestSummonAnchor const& anchor)
{
    if (data.objectiveIdx < 0 || data.objectiveIdx >= QUEST_OBJECTIVES_COUNT || !data.quest)
        return false;

    if ((!anchor.focusId && !anchor.creature) || IsObjectiveCompleted(data))
        return false;

    Item* item = bot->GetItemByEntry(data.quest->GetSrcItemId());
    if (!item)
        return false;

    uint32 const spellId = GetItemUseSpell(item);
    if (!spellId)
        return false;

    for (QuestCraftReagent const& reagent : GetQuestItemReagents(data.questId))
        if (bot->GetItemCount(reagent.item, false) < reagent.count)
            return false;

    // Re-summoning replaces a trigger before its timer fires, but only suppress while it is
    // still near the fight - engaging a kill-anchor walks away from where it was planted.
    int32 const summoned = data.quest->RequiredNpcOrGo[data.objectiveIdx];
    if (anchor.summonsObjective && summoned > 0 &&
        FindCreatureAnchor(uint32(summoned), summonTriggerNearRange))
        return false;

    // Hold for the whole cast
    if (data.lastItemUse && GetMSTimeDiffToNow(data.lastItemUse) < QuestItemUseGrace(spellId))
        return true;

    if (bot->IsNonMeleeSpellCast(false))
        return true;

    if (bot->HasSpellItemCooldown(spellId, item->GetEntry()))
        return false;

    WorldObject* target = SelectSummonAnchor(data, anchor);
    if (!target)
        return false;

    if (bot->IsMounted())
    {
        bot->Dismount();
        return true;
    }

    bot->SetFacingToObject(target);
    botAI->ImbueItem(item);
    data.lastItemUse = getMSTime();
    data.lastSummonAnchor = target->GetGUID();
    LOG_DEBUG("playerbots", "[New RPG] {} used quest item {} at anchor {} to summon for quest {}",
              bot->GetName(), item->GetEntry(), target->GetEntry(), data.questId);
    return true;
}

GameObject* NewRpgDoQuestAction::FindFocusAnchor(uint32 focusId) const
{
    GameObject* found = nullptr;
    float nearest = FLT_MAX;

    // Only objects already close enough to cast at, so this stays a small grid visit
    // rather than an objectiveScanRange sweep on every summon quest tick.
    std::list<GameObject*> gos;
    AnyGameObjectInObjectRangeCheck goCheck(bot, summonAnchorScanRange);
    Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> goSearcher(bot, gos, goCheck);
    Cell::VisitObjects(bot, goSearcher, summonAnchorScanRange);
    for (GameObject* go : gos)
    {
        if (!go->isSpawned() || go->GetGoType() != GAMEOBJECT_TYPE_SPELL_FOCUS ||
            go->GetGOInfo()->spellFocus.focusId != focusId)
            continue;

        // GameObjectFocusCheck - the rule the core casts by - accepts the caster at half the
        // template's dist. A yard of margin so a cast that starts in range also finishes there.
        float const useRange = std::max(float(go->GetGOInfo()->spellFocus.dist) / 2.0f - 1.0f, 2.0f);
        float const dist = bot->GetDistance(go);
        if (dist <= useRange && dist < nearest)
        {
            nearest = dist;
            found = go;
        }
    }

    return found;
}

Creature* NewRpgDoQuestAction::FindCreatureAnchor(uint32 entry, float range) const
{
    Creature* found = nullptr;
    float nearest = range;

    std::list<Creature*> creatures;
    bot->GetCreatureListWithEntryInGrid(creatures, entry, std::max(range, summonAnchorScanRange));
    for (Creature* creature : creatures)
    {
        if (!creature->IsInWorld() || !creature->IsAlive())
            continue;

        float const dist = bot->GetDistance(creature);
        if (dist < nearest)
        {
            nearest = dist;
            found = creature;
        }
    }

    return found;
}

uint32 NewRpgDoQuestAction::QuestItemUseGrace(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    return std::max<uint32>(2000, (info ? info->CalcCastTime() : 0) + 1000);
}

uint32 NewRpgDoQuestAction::GetQuestAreaTrigger(uint32 questId)
{
    // ObjectMgr indexes trigger -> quest only, so read the reverse straight off the table it
    // loads from. DoExploreObjective re-checks the trigger and the quest flag at use time.
    static std::unordered_map<uint32, uint32> const questToTrigger = []
    {
        std::unordered_map<uint32, uint32> m;
        if (QueryResult result = WorldDatabase.Query("SELECT id, quest FROM areatrigger_involvedrelation ORDER BY id"))
            do
            {
                m[(*result)[1].Get<uint32>()] = (*result)[0].Get<uint32>();
            } while (result->NextRow());
        return m;
    }();

    auto it = questToTrigger.find(questId);
    return it == questToTrigger.end() ? 0 : it->second;
}

bool NewRpgDoQuestAction::DoExploreObjective(NewRpgInfo::DoQuest& data)
{
    Quest const* quest = data.quest;
    if (!quest || !quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_EXPLORATION_OR_EVENT))
        return false;

    // Explored is a separate completion requirement from the objective counts. Once set,
    // whatever remains is a scripted event we can't drive - leave it to the quest budget.
    QuestStatusData const* q_status = GetQuestStatus(data.questId);
    if (!q_status || q_status->Explored)
        return false;

    uint32 triggerId = GetQuestAreaTrigger(data.questId);
    if (!triggerId)
        return false;  // event quest, not an area-trigger explore

    AreaTrigger const* at = sObjectMgr->GetAreaTrigger(triggerId);
    if (!at || at->map != bot->GetMapId())
        return false;

    // HandleAreaTriggerOpcode re-validates the radius and calls AreaExploredOrEventHappens.
    if (bot->IsInAreaTriggerRadius(at, 0.0f))
    {
        WorldPacket p(CMSG_AREATRIGGER);
        p << triggerId;
        p.rpos(0);
        bot->GetSession()->HandleAreaTriggerOpcode(p);
        LOG_DEBUG("playerbots", "[New RPG] {} fired area trigger {} for quest {}",
                  bot->GetName(), triggerId, data.questId);
        return true;
    }

    WorldPosition pos(at->map, at->x, at->y, at->z);
    if (bot->GetDistance(pos) > 10.0f)
    {
        if (MoveFarTo(pos))
            return true;
        return MoveRandomNear(10.0f);
    }
    // Near the center but not yet inside a small/box trigger - inch in.
    return MoveRandomNear(5.0f);
}

bool NewRpgDoQuestAction::DoIncompleteQuest(NewRpgInfo::DoQuest& data)
{
    uint32 questId = data.questId;

    if (data.pos != WorldPosition() && IsObjectiveCompleted(data))
    {
        // Sweep nearby corpses into the loot stack before departing.
        botAI->DoSpecificAction("add all loot", Event(), true);
        data.triedSpawns.clear();
        data.visited.clear();
        ResetObjectiveSpawn(data);
    }

    // Bounded: a chest the pipeline reports as lootable but refuses to open would
    // otherwise pin the bot here for the rest of the run.
    if (HoldForLoot(data))
        return false;

    // Whole-quest budget: the spawn rotation below never gives up on its own, so this is
    // what releases a bot from a quest it cannot finish.
    if (botAI->rpgInfo.HasStatusPersisted(questStayTime))
    {
        /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
        botAI->lowPriorityQuest.insert(questId);
        botAI->rpgStatistic.questAbandoned++;
        LOG_DEBUG("playerbots", "[New RPG] {} marked as abandoned quest {} (quest budget)", bot->GetName(), questId);
        botAI->rpgInfo.ChangeToIdle();
        return true;
    }

    if (data.pos == WorldPosition())
    {
        if (!SelectObjectiveSpawn(data))
        {
            // An unmet explore objective has no spawn, so try it before giving up.
            if (DoExploreObjective(data))
                return true;
            // No spawn point for any objective - walk the POI and grind it instead.
            if (SelectObjectivePoi(data))
            {
                LOG_DEBUG("playerbots", "[New RPG] {} quest {} objective {}: no spawn indexed, POI fallback",
                          bot->GetName(), questId, data.objectiveIdx);
                return true;
            }
            LOG_DEBUG("playerbots", "[New RPG] {} gave up quest {}: no spawn and no POI on map {}",
                      bot->GetName(), questId, bot->GetMapId());
            botAI->rpgInfo.ChangeToIdle();
            return true;
        }
        LOG_DEBUG("playerbots", "[New RPG] {} quest {} heading to objective {} spawn at {} yd",
                  bot->GetName(), questId, data.objectiveIdx, uint32(bot->GetDistance(data.pos)));
        return true;
    }

    // No-progress backstop, measured from when this spawn was picked, not from arrival:
    // a bot that cannot path to it never arrives.
    if (data.spawnSince && GetMSTimeDiffToNow(data.spawnSince) >= poiStayTime)
    {
        LOG_DEBUG("playerbots", "[New RPG] {} quest {} objective {}: {} after {} s at this spawn, {} tried",
                  bot->GetName(), questId, data.objectiveIdx,
                  HasObjectiveProgress(data) ? "progress" : "nothing", poiStayTime / 1000, data.triedSpawns.size());

        if (!HasObjectiveProgress(data))
        {
            // dry (or unreachable) spawn: move on to the next nearest one. The quest
            // budget above is what eventually gives up on the quest entirely.
            if (data.spawnGuid)
                data.triedSpawns.insert(data.spawnGuid);
        }
        else
        {
            // The area works, so let dry spawns be retried and drop the write-offs with
            // them: `visited` stops a bot re-chasing what it just failed on, it is not a
            // whole-quest blacklist.
            data.triedSpawns.clear();
            data.visited.clear();
        }
        ResetObjectiveSpawn(data);
        return true;
    }

    QuestSummonAnchor const anchor = data.objectiveIdx >= 0
                                         ? GetQuestSummonAnchor(data.questId, uint32(data.objectiveIdx))
                                         : QuestSummonAnchor{};

    // A kill-anchor must be used on before it is fought: the scan targets it because it
    // grants the credit, and engaging first means fighting it un-primed.
    if (anchor.killAnchor && DoSummonObjective(data, anchor))
        return true;

    if (data.targetGuid)
        return EngageTarget(data);

    // Throttled scan - runs during the approach too, so targets en route are engaged
    // instead of walked past.
    if (!data.lastScan || GetMSTimeDiffToNow(data.lastScan) >= scanInterval)
    {
        data.lastScan = getMSTime();
        if (ScanForObjectiveTarget(data, objectiveScanRange))
            return true;
    }

    if (DoCraftObjective(data))
        return true;

    // Nothing of the objective in range - if it is conjured at an anchor, use the item here.
    // After the scan so anything already summoned is killed first, and before the walk below
    // so the anchor drives the last stretch.
    if (DoSummonObjective(data, anchor))
        return true;

    if (bot->GetDistance(data.pos) > 10.0f)
    {
        if (MoveFarTo(data.pos))
            return true;
        // Long-range sampler found no candidate - nudge the bot so the next tick
        // retries from a different position instead of sitting idle.
        return MoveRandomNear(10.0f);
    }

    // Give the destination one scan window before writing it off. The throttled scan above
    // has not necessarily run since arrival, and the cell may only just have loaded.
    if (!data.lastReachPOI)
    {
        data.lastReachPOI = getMSTime();
        return true;
    }

    if (GetMSTimeDiffToNow(data.lastReachPOI) < scanInterval)
        return true;

    // A POI has no spawn point to retire and nothing better to move to - stay and let the
    // grind strategy work the area until the no-progress window above rotates to another one.
    if (data.poiFallback)
        return MoveRandomNear(8.0f);

    // Standing on the spawn point with nothing in range: it is dead or empty, so take the
    // next nearest rather than waiting out poiStayTime here.
    LOG_DEBUG("playerbots", "[New RPG] {} quest {} objective {}: arrived, nothing in range - next spawn",
              bot->GetName(), questId, data.objectiveIdx);
    data.triedSpawns.insert(data.spawnGuid);
    ResetObjectiveSpawn(data);
    return true;
}

bool NewRpgDoQuestAction::DoCompletedQuest(NewRpgInfo::DoQuest& data)
{
    uint32 questId = data.questId;
    Quest const* quest = data.quest;

    if (data.objectiveIdx != -1)
    {
        // Same sweep as the objective-complete transition: the walk to the turn-in
        // starts immediately and outruns the random "add all loot" trigger.
        botAI->DoSpecificAction("add all loot", Event(), true);
        ClearTarget(data);
        // Nothing left to grind for - the walk to the turn-in must not be interrupted by
        // "attack anything", which outranks this action.
        data.poiFallback = false;
        // if quest is completed, back to poi with -1 idx to reward
        BroadcastHelper::BroadcastQuestUpdateComplete(botAI, bot, quest);
        botAI->rpgStatistic.questCompleted++;
        data.lastReachPOI = 0;
        data.objectiveIdx = -1;
        // Clear the objective's spawn point and its write-offs: from here they track the
        // turn-in's spawn points instead.
        data.spawnGuid = 0;
        data.pos = WorldPosition();
        data.triedSpawns.clear();
    }

    // Re-entered whenever a turn-in destination is retired below, so an entry with several
    // spawn points works through them instead of waiting out the clock at the first.
    if (data.pos == WorldPosition())
    {
        if (SelectQuestEnder(data))
        {
            LOG_DEBUG("playerbots", "[New RPG] {} quest {} complete, turn-in at ender spawn {} yd away ({} tried)",
                      bot->GetName(), questId, uint32(bot->GetDistance(data.pos)), data.triedSpawns.size());
        }
        else
        {
            std::vector<POIInfo> poiInfo;
            if (!GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
            {
                // can't find a poi pos to reward, stop doing quest for now
                LOG_DEBUG("playerbots", "[New RPG] {} quest {} complete but no turn-in on map {}: "
                          "{} ender entries indexed ({} spawns tried), no POI either",
                          bot->GetName(), questId, bot->GetMapId(), GetQuestEnderEntries(questId).size(),
                          data.triedSpawns.size());
                botAI->rpgInfo.ChangeToIdle();
                return false;
            }
            assert(poiInfo.size() > 0);
            // now we get the place to get rewarded
            data.pos = WorldPosition(bot->GetMapId(), poiInfo[0].pos.x, poiInfo[0].pos.y, poiInfo[0].z);
            LOG_DEBUG("playerbots", "[New RPG] {} quest {} complete, turn-in POI {} yd away ({} ender entries, "
                      "{} spawns tried)",
                      bot->GetName(), questId, uint32(bot->GetDistance(data.pos)),
                      GetQuestEnderEntries(questId).size(), data.triedSpawns.size());
        }
    }

    // Only for a POI destination, whose z is a guess: quest_poi carries x/y only, and
    // GetQuestPOIPosAndObjectiveIdx falls back to the bot's own height when the POI's grid is
    // not resident. Arrival is a 3D check and MoveFarTo's stuck recovery teleports to this
    // exact point, so a stale height strands the bot. A spawn point already has the real z.
    if (!data.spawnGuid)
    {
        float const groundZ = std::max(bot->GetMap()->GetHeight(data.pos.GetPositionX(), data.pos.GetPositionY(),
                                                                MAX_HEIGHT),
                                       bot->GetMap()->GetWaterLevel(data.pos.GetPositionX(), data.pos.GetPositionY()));
        if (groundZ != INVALID_HEIGHT && groundZ != VMAP_INVALID_HEIGHT_VALUE && groundZ != data.pos.GetPositionZ())
            data.pos = WorldPosition(bot->GetMapId(), data.pos.GetPositionX(), data.pos.GetPositionY(), groundZ);
    }

    // Same gate as DoIncompleteQuest: don't start the walk to the turn-in while lootable
    // corpses are pending nearby.
    if (HoldForLoot(data))
        return false;

    if (bot->GetDistance(data.pos) > 10.0f && !data.lastReachPOI)
    {
        if (MoveFarTo(data.pos))
            return true;
        return MoveRandomNear(10.0f);
    }

    // Now we are near the qoi of reward
    // the quest should be rewarded by SearchQuestGiverAndAcceptOrReward
    if (!data.lastReachPOI)
    {
        data.lastReachPOI = getMSTime();
        LOG_DEBUG("playerbots", "[New RPG] {} reached turn-in for quest {}, waiting on the hand-in",
                  bot->GetName(), questId);
        return true;
    }
    // Standing at an ender spawn point with nobody handing the quest in: it is empty (dead,
    // despawned, or only put in the world by a script). Retire it and try the next one
    if (data.spawnGuid && GetMSTimeDiffToNow(data.lastReachPOI) >= enderWaitTime)
    {
        LOG_DEBUG("playerbots", "[New RPG] {} nobody took quest {} at this ender spawn, trying the next",
                  bot->GetName(), questId);
        data.triedSpawns.insert(data.spawnGuid);
        data.spawnGuid = 0;
        data.pos = WorldPosition();
        data.lastReachPOI = 0;
        return true;
    }

    // stayed at this POI for more than 5 minutes
    if (GetMSTimeDiffToNow(data.lastReachPOI) >= poiStayTime)
    {
        // e.g. Can not reward quest to gameobjects
        /// @TODO: It may be better to make lowPriorityQuest a global set shared by all bots (or saved in db)
        botAI->lowPriorityQuest.insert(questId);
        botAI->rpgStatistic.questAbandoned++;
        LOG_DEBUG("playerbots", "[New RPG] {} marked as abandoned quest {}", bot->GetName(), questId);
        botAI->rpgInfo.ChangeToIdle();
        return true;
    }
    return false;
}
