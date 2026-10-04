/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootObjectStack.h"
#include "Log.h"
#include "LootAction.h"
#include "LootMgr.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "ReputationMgr.h"
#include "Unit.h"
#include <set>

#define MAX_LOOT_OBJECT_COUNT 200

namespace
{
bool IsGatheringSkill(uint32 skillId)
{
    return skillId == SKILL_SKINNING || skillId == SKILL_HERBALISM || skillId == SKILL_MINING ||
           skillId == SKILL_ENGINEERING;
}
}

LootTarget::LootTarget(ObjectGuid guid) : guid(guid), asOfTime(time(nullptr)) {}

LootTarget::LootTarget(LootTarget const& other)
{
    guid = other.guid;
    asOfTime = other.asOfTime;
}

LootTarget& LootTarget::operator=(LootTarget const& other)
{
    if ((void*)this == (void*)&other)
        return *this;

    guid = other.guid;
    asOfTime = other.asOfTime;

    return *this;
}

bool LootTarget::operator<(LootTarget const& other) const { return guid < other.guid; }

void LootTargetList::shrink(time_t fromTime)
{
    for (std::set<LootTarget>::iterator i = begin(); i != end();)
    {
        if (i->asOfTime <= fromTime)
            erase(i++);
        else
            ++i;
    }
}

bool IsGameObjectHostileTo(GameObject const* go, Player const* player)
{
    if (!go || !player)
        return false;

    FactionTemplateEntry const* goFaction = sFactionTemplateStore.LookupEntry(go->GetUInt32Value(GAMEOBJECT_FACTION));
    if (!goFaction)
        return false;

    FactionTemplateEntry const* playerFaction = player->GetFactionTemplateEntry();
    if (!playerFaction)
        return false;

    // A forced reaction (e.g. a disguise) overrides the faction template in both directions,
    // exactly like the client's ActivateToQuest check.
    if (ReputationRank const* forcedRank = player->GetReputationMgr().GetForcedRankIfAny(goFaction))
        return *forcedRank <= REP_HOSTILE;

    return goFaction->IsHostileTo(*playerFaction);
}

LootObject::LootObject(Player* bot, ObjectGuid guid) : guid(), skillId(SKILL_NONE), reqSkillValue(0), reqItem(0)
{
    Refresh(bot, guid);
}

void LootObject::Refresh(Player* bot, ObjectGuid lootGUID)
{
    skillId = SKILL_NONE;
    reqSkillValue = 0;
    reqItem = 0;
    guid.Clear();

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return;
    }
    Creature* creature = botAI->GetCreature(lootGUID);
    if (creature && creature->getDeathState() == DeathState::Corpse)
    {
        if (creature->HasFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE))
            guid = lootGUID;

        if (creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE))
        {
            skillId = creature->GetCreatureTemplate()->GetRequiredLootSkill();
            uint32 targetLevel = creature->GetLevel();
            reqSkillValue = targetLevel < 10 ? 1 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;
            if (botAI->HasSkill((SkillType)skillId) && bot->GetSkillValue(skillId) >= reqSkillValue)
                guid = lootGUID;
        }

        return;
    }

    GameObject* go = botAI->GetGameObject(lootGUID);
    if (go && go->isSpawned() && go->GetGoState() == GO_STATE_READY)
    {
        // The client refuses to interact with hostile game objects; check live so forced
        // reactions (e.g. disguises) that drop hostility are honoured.
        if (IsGameObjectHostileTo(go, bot))
            return;

        bool onlyHasQuestItems = true;
        bool hasAnyQuestItems = false;
        bool neededQuestItem = false;

        GameObjectQuestItemList const* items = sObjectMgr->GetGameObjectQuestItemList(go->GetEntry());
        for (size_t i = 0; i < MAX_GAMEOBJECT_QUEST_ITEMS; i++)
        {
            if (!items || i >= items->size())
                break;

            uint32 itemId = uint32((*items)[i]);
            if (!itemId)
                continue;

            // The client only marks the game object as holding a quest item when the player
            // actually has a quest requiring it.
            if (bot->HasQuestForItem(itemId))
                hasAnyQuestItems = true;

            if (IsNeededForQuest(bot, itemId))
            {
                // A gathering node can also drop a needed quest item (e.g.
                // Root Sample off Barrens herbs); gathering yields both, so
                // keep reading the lock below to set skillId rather than
                // bailing here.
                this->guid = lootGUID;
                neededQuestItem = true;
            }

            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_QUEST)
            {
                onlyHasQuestItems = false;
            }
        }

        // Retrieve the correct loot table entry
        uint32 lootEntry = go->GetGOInfo()->GetLootId();
        if (lootEntry == 0)
            return;

        // If the gameobject carries only quest items the bot has a quest for but does not need,
        // skip it. The template walk only happens in this rare case.
        if (!neededQuestItem && hasAnyQuestItems)
        {
            if (LootTemplate const* lootTemplate = LootTemplates_Gameobject.GetLootFor(lootEntry))
            {
                std::set<uint32> itemIds;
                lootTemplate->CollectItemIds(itemIds);

                for (uint32 itemId : itemIds)
                {
                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                    if (proto && proto->Class != ITEM_CLASS_QUEST)
                    {
                        onlyHasQuestItems = false;
                        break;
                    }
                }
            }
            else
            {
                onlyHasQuestItems = false;
            }

            if (onlyHasQuestItems)
                return;
        }

        // The bot walks to and opens only what it would actually store. A quest item it still
        // needs bypasses the filter; the always-loot list and the all/* strategy are handled by
        // the value itself.
        if (!neededQuestItem)
        {
            Value<bool>* useful = botAI->GetAiObjectContext()->GetValue<bool>("loot entry useful", int32(lootEntry));
            if (!useful || !useful->Get())
                return;
        }

        // Otherwise, loot it.
        guid = lootGUID;

        uint32 goId = go->GetEntry();
        uint32 lockId = go->GetGOInfo()->GetLockId();
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
        if (!lockInfo)
            return;

        // A lock opens when ANY case is satisfiable, and the client serves the non-skill cases
        // (LOCKTYPE_OPEN/QUICK_OPEN/OPEN_TINKERING/OPEN_KNEELING/...) with its hidden "Opening"
        // abilities (3365/6247/6477/6478). Track one so a gathering case on the same lock cannot
        // make the object unusable for a bot without the profession.
        bool skillFreeOpener = false;

        for (uint8 i = 0; i < 8; ++i)
        {
            switch (lockInfo->Type[i])
            {
                case LOCK_KEY_ITEM:
                    if (lockInfo->Index[i] > 0)
                    {
                        reqItem = lockInfo->Index[i];
                        guid = lootGUID;
                    }
                    break;

                case LOCK_KEY_SKILL:
                    if (goId == 13891 || goId == 19535)  // Serpentbloom
                    {
                        this->guid = lootGUID;
                    }
                    else if (SkillByLockType(LockType(lockInfo->Index[i])) > 0)
                    {
                        skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                        reqSkillValue = std::max((uint32)1, lockInfo->Skill[i]);
                        guid = lootGUID;
                    }
                    else
                    {
                        skillFreeOpener = true;
                        guid = lootGUID;
                    }
                    break;

                case LOCK_KEY_NONE:
                    guid = lootGUID;
                    break;
            }
        }

        // The bot cannot satisfy the gathering case, but the client would open this with a plain
        // opener instead (e.g. the Corrupted Flower's kneel case -> 6478), so drop the
        // profession/key requirement and let GetOpeningSpell pick the matching spell.
        // Locked objects (chests, strongboxes, coffers, ...) are excluded: their Open case is a
        // convenience next to a Lockpicking/key case, and dropping the requirement would let the
        // hidden opener bypass it (the cast path does not check GO_FLAG_LOCKED). It would also
        // stop key holders from using their key.
        bool const gatheringSatisfied = skillId != SKILL_NONE && botAI->HasSkill((SkillType)skillId) &&
                                        bot->GetSkillValue(skillId) >= reqSkillValue;
        if (skillFreeOpener && !gatheringSatisfied && !go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_LOCKED))
        {
            skillId = SKILL_NONE;
            reqSkillValue = 0;
            reqItem = 0;
        }
    }
}

bool LootObject::IsNeededForQuest(Player* bot, uint32 itemId)
{
    for (int qs = 0; qs < MAX_QUEST_LOG_SIZE; ++qs)
    {
        uint32 questId = bot->GetQuestSlotQuestId(qs);
        if (questId == 0)
            continue;

        QuestStatusData& qData = bot->getQuestStatusMap()[questId];
        if (qData.Status != QUEST_STATUS_INCOMPLETE)
            continue;

        Quest const* qInfo = sObjectMgr->GetQuestTemplate(questId);
        if (!qInfo)
            continue;

        for (int i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            if (!qInfo->RequiredItemCount[i] || (qInfo->RequiredItemCount[i] - qData.ItemCount[i]) <= 0)
                continue;

            if (qInfo->RequiredItemId[i] != itemId)
                continue;

            return true;
        }
    }

    return false;
}

WorldObject* LootObject::GetWorldObject(Player* bot)
{
    Refresh(bot, guid);

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        return nullptr;
    }
    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == DeathState::Corpse && creature->IsInWorld())
        return creature;

    GameObject* go = botAI->GetGameObject(guid);
    if (go && go->isSpawned() && go->IsInWorld())
        return go;

    return nullptr;
}

LootObject::LootObject(LootObject const& other)
{
    guid = other.guid;
    skillId = other.skillId;
    reqSkillValue = other.reqSkillValue;
    reqItem = other.reqItem;
}

bool LootObject::IsLootPossible(Player* bot)
{
    if (IsEmpty() || !bot)
        return false;

    WorldObject* worldObj = GetWorldObject(bot);  // Store result to avoid multiple calls
    if (!worldObj)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    if (reqItem && !bot->HasItemCount(reqItem, 1))
        return false;

    if (abs(worldObj->GetPositionZ() - bot->GetPositionZ()) > INTERACTION_DISTANCE - 2.0f)
        return false;

    Creature* creature = botAI->GetCreature(guid);
    if (creature && creature->getDeathState() == DeathState::Corpse)
    {
        // Gathering is independent of loot rights in the core: a corpse only needs to be fully
        // looted and the bot to have the matching skill. Only normal corpse loot is gated here.
        if (!bot->isAllowedToLoot(creature) && !IsGatheringSkill(skillId))
            return false;
    }

    // Prevent bot from running to chests that are unlootable (e.g. Gunship Armory before completing the event) or on
    // respawn time
    GameObject* go = botAI->GetGameObject(guid);
    if (go && (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE) || !go->isSpawned()))
        return false;

    // Client parity: the client cannot interact with hostile game objects.
    if (go && IsGameObjectHostileTo(go, bot))
        return false;

    // Conditional objects (quest chests, goobers, ...) are gated client-side on quest state.
    // A bot has no client, so make the same call the server makes for one.
    if (go && go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND) && !go->ActivateToQuest(bot))
        return false;

    //Prevents bot from getting stuck in an infinite loop of
    //gathering herb/ore/skin -> bag too full, don't pick up -> gather again
    bool gatheringObject = IsGatheringSkill(skillId);

    Player* master = botAI->GetMaster();
    bool hasActivePlayerMaster = master && !GET_PLAYERBOT_AI(master);
    if (gatheringObject && !hasActivePlayerMaster)
    {
        uint8 bagUsage = botAI->GetAiObjectContext() ->GetValue<uint8>("bag space")->Get();

        if (bagUsage > 80)
            return false;
    }

    if (skillId == SKILL_NONE)
        return true;

    if (skillId == SKILL_FISHING)
        return false;

    if (!botAI->HasSkill((SkillType)skillId))
        return false;

    if (!reqSkillValue)
        return true;

    uint32 skillValue = uint32(bot->GetSkillValue(skillId));
    if (reqSkillValue > skillValue)
        return false;

    if (skillId == SKILL_MINING && !bot->HasItemCount(756, 1) && !bot->HasItemCount(778, 1) &&
        !bot->HasItemCount(1819, 1) && !bot->HasItemCount(1893, 1) && !bot->HasItemCount(1959, 1) &&
        !bot->HasItemCount(2901, 1) && !bot->HasItemCount(9465, 1) && !bot->HasItemCount(20723, 1) &&
        !bot->HasItemCount(40772, 1) && !bot->HasItemCount(40892, 1) && !bot->HasItemCount(40893, 1))
    {
        return false;  // Bot is missing a mining pick
    }

    if (skillId == SKILL_SKINNING && !bot->HasItemCount(7005, 1) && !bot->HasItemCount(40772, 1) &&
        !bot->HasItemCount(40893, 1) && !bot->HasItemCount(12709, 1) && !bot->HasItemCount(19901, 1))
    {
        return false;  // Bot is missing a skinning knife
    }

    return true;
}

bool LootObjectStack::Add(ObjectGuid guid)
{
    skippedLoot.shrink(time(nullptr) - SkipDuration);

    if (skippedLoot.count(guid))
        return false;

    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT)
    {
        availableLoot.shrink(time(nullptr) - SkipDuration);
    }

    if (availableLoot.size() >= MAX_LOOT_OBJECT_COUNT)
    {
        availableLoot.clear();
    }

    if (!availableLoot.insert(guid).second)
        return false;

    return true;
}

void LootObjectStack::Remove(ObjectGuid guid)
{
    LootTargetList::iterator i = availableLoot.find(guid);
    if (i != availableLoot.end())
        availableLoot.erase(i);
}

void LootObjectStack::Skip(ObjectGuid guid)
{
    if (skippedLoot.insert(guid).second)
        LOG_DEBUG("playerbots", "LootObjectStack::Skip: bot={} guid={} entry={} for {}s", bot->GetName(),
                  guid.ToString(), guid.GetEntry(), SkipDuration);
}

void LootObjectStack::Unskip(ObjectGuid guid)
{
    LootTargetList::iterator i = skippedLoot.find(guid);
    if (i != skippedLoot.end())
        skippedLoot.erase(i);
}

void LootObjectStack::Clear()
{
    availableLoot.clear();
    skippedLoot.clear();
}

bool LootObjectStack::IsItemStoreable(uint32 itemId)
{
    uint32 const cacheTime = sPlayerbotAIConfig.lootItemStoreableCacheTime;
    uint32 const cacheMaxSize = sPlayerbotAIConfig.lootItemStoreableCacheMaxSize;
    bool const cacheEnabled = cacheTime > 0 && cacheMaxSize > 0;
    time_t const now = time(nullptr);

    if (cacheEnabled)
    {
        auto itr = itemStoreableCache.find(itemId);
        if (itr != itemStoreableCache.end() && now - itr->second.second < cacheTime)
            return itr->second.first;
    }

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    bool const storeable = botAI && StoreLootAction::IsLootAllowed(itemId, botAI);

    if (cacheEnabled)
    {
        if (itemStoreableCache.size() >= cacheMaxSize)
        {
            for (auto i = itemStoreableCache.begin(); i != itemStoreableCache.end();)
            {
                if (now - i->second.second >= cacheTime)
                    i = itemStoreableCache.erase(i);
                else
                    ++i;
            }

            if (itemStoreableCache.size() >= cacheMaxSize)
                itemStoreableCache.clear();
        }

        itemStoreableCache[itemId] = std::make_pair(storeable, now);
    }

    return storeable;
}

bool LootObjectStack::CanLoot(float maxDistance)
{
    LootObject nearest = GetNearest(maxDistance);
    return !nearest.IsEmpty();
}

LootObject LootObjectStack::GetLoot(float maxDistance)
{
    LootObject nearest = GetNearest(maxDistance);
    return nearest.IsEmpty() ? LootObject() : nearest;
}

LootObject LootObjectStack::GetNearest(float maxDistance)
{
    availableLoot.shrink(time(nullptr) - SkipDuration);
    skippedLoot.shrink(time(nullptr) - SkipDuration);

    LootObject nearest;
    float nearestDistance = std::numeric_limits<float>::max();

    LootTargetList safeCopy(availableLoot);
    for (LootTargetList::iterator i = safeCopy.begin(); i != safeCopy.end(); i++)
    {
        ObjectGuid guid = i->guid;

        if (skippedLoot.count(guid))
            continue;

        WorldObject* worldObj = ObjectAccessor::GetWorldObject(*bot, guid);
        if (!worldObj)
            continue;

        float distance = bot->GetDistance(worldObj);

        if (distance >= nearestDistance || (maxDistance && distance > maxDistance))
            continue;

        LootObject lootObject(bot, guid);

        if (!lootObject.IsLootPossible(bot))
            continue;

        nearestDistance = distance;
        nearest = lootObject;
    }

    return nearest;
}
