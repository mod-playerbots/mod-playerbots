/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "QueryGameObjectAction.h"
#include "ChatHelper.h"
#include "DBCStores.h"
#include "Event.h"
#include "GameObject.h"
#include "Log.h"
#include "LootMgr.h"
#include "LootObjectStack.h"
#include "LootValues.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "StringFormat.h"
#include "World.h"
#include <algorithm>
#include <cctype>

namespace
{
    std::string ToLower(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
        return text;
    }

    char const* BoolText(bool value) { return value ? "yes" : "no"; }
}

bool QueryGameObjectAction::Execute(Event event)
{
    GameObject* go = FindTarget(event.getParam());
    if (!go)
    {
        botAI->TellError("No game object found");
        return false;
    }

    DumpGameObject(go);
    return true;
}

GameObject* QueryGameObjectAction::FindTarget(std::string const& param)
{
    GuidVector nearest = AI_VALUE(GuidVector, "nearest game objects");

    if (!param.empty())
    {
        for (ObjectGuid const guid : ChatHelper::parseGameobjects(param))
            if (GameObject* go = botAI->GetGameObject(guid))
                return go;

        bool const numeric = param.find_first_not_of("0123456789") == std::string::npos;
        if (numeric)
        {
            uint32 const entry = uint32(atoi(param.c_str()));
            for (ObjectGuid const guid : nearest)
                if (GameObject* go = botAI->GetGameObject(guid))
                    if (go->GetEntry() == entry)
                        return go;
        }
        else
        {
            std::string const needle = ToLower(param);
            for (ObjectGuid const guid : nearest)
            {
                if (GameObject* go = botAI->GetGameObject(guid))
                {
                    std::string const name = ToLower(go->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()));
                    if (name.find(needle) != std::string::npos)
                        return go;
                }
            }
        }

        // An explicit target that did not resolve is an error; the loot-target and
        // nearest-object fallbacks are only for a bare call without arguments.
        return nullptr;
    }

    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.guid.IsGameObject())
        if (GameObject* go = botAI->GetGameObject(loot.guid))
            return go;

    for (ObjectGuid const guid : nearest)
        if (GameObject* go = botAI->GetGameObject(guid))
            return go;

    return nullptr;
}

void QueryGameObjectAction::DumpGameObject(GameObject* go)
{
    GameObjectTemplate const* info = go->GetGOInfo();
    GameObjectTemplateAddon const* addon = go->GetTemplateAddon();
    GameObjectData const* spawn = sObjectMgr->GetGameObjectData(go->GetSpawnId());

    auto tell = [&](std::string const& text)
    {
        botAI->TellMasterNoFacing(text);
        LOG_DEBUG("playerbots", "QGO {}", text);
    };

    tell(Acore::StringFormat("QGO {} '{}' guid={} spawnId={}", go->GetEntry(),
                             go->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale()), go->GetGUID().ToString(),
                             go->GetSpawnId()));
    tell(Acore::StringFormat("QGO world: map={} zone={} area={} x={:.3f} y={:.3f} z={:.3f} o={:.3f} phaseMask={}",
                             go->GetMapId(), go->GetZoneId(), go->GetAreaId(), go->GetPositionX(), go->GetPositionY(),
                             go->GetPositionZ(), go->GetOrientation(), go->GetPhaseMask()));
    tell(Acore::StringFormat(
        "QGO template: type={} displayId={} size={:.3f} lootId={} lockId={} scriptId={} AI='{}'", info->type,
        info->displayId, info->size, info->GetLootId(), info->GetLockId(), info->ScriptId, info->AIName));

    std::string data;
    for (uint32 i = 0; i < MAX_GAMEOBJECT_DATA; ++i)
        if (info->raw.data[i])
            data += Acore::StringFormat(" [{}]={}", i, info->raw.data[i]);
    tell(Acore::StringFormat("QGO data:{}", data.empty() ? " none" : data));

    if (addon)
        tell(Acore::StringFormat("QGO addon: faction={} flags=0x{:X} mingold={} maxgold={}", addon->faction, addon->flags,
                                 addon->mingold, addon->maxgold));

    if (spawn)
        tell(Acore::StringFormat(
            "QGO spawn: map={} x={:.3f} y={:.3f} z={:.3f} o={:.3f} respawnSecs={} phaseMask={} spawnMask=0x{:X} "
            "goState={} animprogress={} scriptId={} poolId={}",
            spawn->mapid, spawn->posX, spawn->posY, spawn->posZ, spawn->orientation, spawn->spawntimesecs,
            spawn->phaseMask, spawn->spawnMask, uint32(spawn->go_state), spawn->animprogress, spawn->ScriptId,
            spawn->poolId));

    tell(Acore::StringFormat(
        "QGO runtime: goState={} lootState={} spawned={} byDefault={} respawnTime={} useCount={} uniqueUsers={}",
        uint32(go->GetGoState()), uint32(go->getLootState()), BoolText(go->isSpawned()), BoolText(go->isSpawnedByDefault()),
        go->GetRespawnTime(), go->GetUseCount(), go->GetUniqueUseCount()));
    tell(Acore::StringFormat("QGO fields: flags=0x{:X} displayId={} faction={} level={} scale={:.3f}",
                             go->GetUInt32Value(GAMEOBJECT_FLAGS), go->GetUInt32Value(GAMEOBJECT_DISPLAYID),
                             go->GetUInt32Value(GAMEOBJECT_FACTION), go->GetUInt32Value(GAMEOBJECT_LEVEL),
                             go->GetFloatValue(OBJECT_FIELD_SCALE_X)));
    tell(Acore::StringFormat("QGO bytes1: state={} type={} artkit={} animprogress={}",
                             uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 0)),
                             uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 1)),
                             uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 2)),
                             uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 3))));
    tell(Acore::StringFormat("QGO loot state: items={} gold={} looted={} type={}", go->loot.items.size(), go->loot.gold,
                             BoolText(go->loot.isLooted()), uint32(go->loot.loot_type)));

    tell(Acore::StringFormat(
        "QGO bot: name={} distance={:.3f} dist2d={:.3f} withinInteract={} interactionDist={:.3f} los={} samePhase={} "
        "activatedToQuest={} hasQuestForGO={} usableMounted={}",
        bot->GetName(), bot->GetDistance(go), bot->GetDistance2d(go),
        BoolText(go->IsWithinDistInMap(bot, INTERACTION_DISTANCE)), go->GetInteractionDistance(),
        BoolText(go->IsWithinLOSInMap(bot)), BoolText(go->InSamePhase(bot)), BoolText(go->ActivateToQuest(bot)),
        BoolText(bot->HasQuestForGO(go->GetEntry())), BoolText(info->IsUsableMounted())));

    uint32 const lockId = info->GetLockId();
    LockEntry const* lock = sLockStore.LookupEntry(lockId);
    if (!lock)
        tell(Acore::StringFormat("QGO lock: {} (no Lock.dbc entry)", lockId));
    else
    {
        for (uint8 i = 0; i < 8; ++i)
        {
            if (!lock->Type[i] && !lock->Index[i] && !lock->Skill[i])
                continue;

            // Index[i] holds a lock type only for skill slots; item and spell slots use it
            // for their own id, so mapping it to a skill would report an unrelated profession.
            uint32 const mappedSkill =
                lock->Type[i] == LOCK_KEY_SKILL ? uint32(SkillByLockType(LockType(lock->Index[i]))) : 0;
            tell(Acore::StringFormat("QGO lock slot {}: type={} index={} skill={} skillByLockType={} botSkillValue={}",
                                     i, lock->Type[i], lock->Index[i], lock->Skill[i], mappedSkill,
                                     mappedSkill ? bot->GetSkillValue(mappedSkill) : 0));
        }
    }

    if (GameObjectDisplayInfoEntry const* display = sGameObjectDisplayInfoStore.LookupEntry(go->GetDisplayId()))
        tell(Acore::StringFormat("QGO display: file='{}' min=({:.3f},{:.3f},{:.3f}) max=({:.3f},{:.3f},{:.3f})",
                                 display->filename, display->minX, display->minY, display->minZ, display->maxX,
                                 display->maxY, display->maxZ));

    uint32 const lootId = info->GetLootId();
    if (lootId)
    {
        // Fishing holes load their loot into a separate store; reporting the gameobject
        // store for them either misses the table or shows an unrelated one with the same id.
        LootStore const& lootStore =
            info->type == GAMEOBJECT_TYPE_FISHINGHOLE ? LootTemplates_Fishing : LootTemplates_Gameobject;
        LootTemplate const* lTemplate = lootStore.GetLootFor(lootId);
        tell(Acore::StringFormat("QGO loot table {}: present={} questLootForBot={}", lootId,
                                 BoolText(lTemplate != nullptr),
                                 BoolText(lootStore.HaveQuestLootForPlayer(lootId, bot))));

        if (lTemplate)
        {
            auto const* access = reinterpret_cast<LootTemplateAccess const*>(lTemplate);
            for (LootStoreItem const* entry : access->Entries)
            {
                if (entry->reference)
                {
                    tell(Acore::StringFormat("QGO loot ref={} chance={:.2f} min={} max={} needsQuest={} mode={} group={}",
                                             entry->reference, entry->chance, entry->mincount, entry->maxcount,
                                             BoolText(entry->needs_quest), entry->lootmode, entry->groupid));
                    continue;
                }

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(entry->itemid);
                tell(Acore::StringFormat(
                    "QGO loot item={} '{}' class={} quality={} maxcount={} sell={} chance={:.2f} min={} max={} "
                    "needsQuest={}",
                    entry->itemid, proto ? proto->Name1 : "?", proto ? proto->Class : 0, proto ? proto->Quality : 0,
                    proto ? proto->MaxCount : 0, proto ? proto->SellPrice : 0, entry->chance, entry->mincount,
                    entry->maxcount, BoolText(entry->needs_quest)));
            }
        }
    }
}
