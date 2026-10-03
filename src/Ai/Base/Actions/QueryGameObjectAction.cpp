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
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "StringFormat.h"
#include "World.h"
#include <algorithm>
#include <cctype>
#include <map>

namespace
{
    using QgoPlaceholders = std::map<std::string, std::string>;

    std::string ToLower(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
        return text;
    }

    std::string QgoText(char const* key, char const* fallback, QgoPlaceholders const& placeholders)
    {
        return PlayerbotTextMgr::instance().GetBotTextOrDefault(key, fallback, placeholders);
    }

    std::string QgoBool(bool value)
    {
        return PlayerbotTextMgr::instance().GetBotTextOrDefault(value ? "qgo_yes" : "qgo_no", value ? "yes" : "no", {});
    }

    std::string QgoFloat(float value) { return Acore::StringFormat("{:.3f}", value); }

    std::string QgoFloat2(float value) { return Acore::StringFormat("{:.2f}", value); }

    std::string QgoHex(uint32 value) { return Acore::StringFormat("0x{:X}", value); }
}

bool QueryGameObjectAction::Execute(Event event)
{
    GameObject* go = FindTarget(event.getParam());
    if (!go)
    {
        botAI->TellError(QgoText("qgo_no_game_object_found", "No game object found", {}));
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

    tell(QgoText("qgo_header", "QGO %entry '%name' guid=%guid spawnId=%spawnId",
                 {{"%entry", std::to_string(go->GetEntry())},
                  {"%name", go->GetNameForLocaleIdx(sWorld->GetDefaultDbcLocale())},
                  {"%guid", go->GetGUID().ToString()},
                  {"%spawnId", std::to_string(go->GetSpawnId())}}));
    tell(QgoText("qgo_world", "QGO world: map=%map zone=%zone area=%area x=%x y=%y z=%posZ o=%o phaseMask=%phaseMask",
                 {{"%map", std::to_string(go->GetMapId())},
                  {"%zone", std::to_string(go->GetZoneId())},
                  {"%area", std::to_string(go->GetAreaId())},
                  {"%x", QgoFloat(go->GetPositionX())},
                  {"%y", QgoFloat(go->GetPositionY())},
                  {"%posZ", QgoFloat(go->GetPositionZ())},
                  {"%o", QgoFloat(go->GetOrientation())},
                  {"%phaseMask", std::to_string(go->GetPhaseMask())}}));
    tell(QgoText("qgo_template",
                 "QGO template: type=%type displayId=%displayId size=%size lootId=%lootId lockId=%lockId "
                 "scriptId=%scriptId AI='%ai'",
                 {{"%type", std::to_string(info->type)},
                  {"%displayId", std::to_string(info->displayId)},
                  {"%size", QgoFloat(info->size)},
                  {"%lootId", std::to_string(info->GetLootId())},
                  {"%lockId", std::to_string(info->GetLockId())},
                  {"%scriptId", std::to_string(info->ScriptId)},
                  {"%ai", info->AIName}}));

    std::string data;
    for (uint32 i = 0; i < MAX_GAMEOBJECT_DATA; ++i)
        if (info->raw.data[i])
            data += Acore::StringFormat(" [{}]={}", i, info->raw.data[i]);
    tell(QgoText("qgo_data", "QGO data:%data", {{"%data", data.empty() ? " none" : data}}));

    if (addon)
        tell(QgoText("qgo_addon", "QGO addon: faction=%faction flags=%flags mingold=%mingold maxgold=%maxgold",
                     {{"%faction", std::to_string(addon->faction)},
                      {"%flags", QgoHex(addon->flags)},
                      {"%mingold", std::to_string(addon->mingold)},
                      {"%maxgold", std::to_string(addon->maxgold)}}));

    if (spawn)
        tell(QgoText("qgo_spawn",
                     "QGO spawn: map=%map x=%x y=%y z=%posZ o=%o respawnSecs=%respawn phaseMask=%phaseMask "
                     "spawnMask=%spawnMask goState=%goState animprogress=%animprogress scriptId=%scriptId "
                     "poolId=%poolId",
                     {{"%map", std::to_string(spawn->mapid)},
                      {"%x", QgoFloat(spawn->posX)},
                      {"%y", QgoFloat(spawn->posY)},
                      {"%posZ", QgoFloat(spawn->posZ)},
                      {"%o", QgoFloat(spawn->orientation)},
                      {"%respawn", std::to_string(spawn->spawntimesecs)},
                      {"%phaseMask", std::to_string(spawn->phaseMask)},
                      {"%spawnMask", QgoHex(spawn->spawnMask)},
                      {"%goState", std::to_string(uint32(spawn->go_state))},
                      {"%animprogress", std::to_string(spawn->animprogress)},
                      {"%scriptId", std::to_string(spawn->ScriptId)},
                      {"%poolId", std::to_string(spawn->poolId)}}));

    tell(QgoText("qgo_runtime",
                 "QGO runtime: goState=%goState lootState=%lootState spawned=%spawned byDefault=%byDefault "
                 "respawnTime=%respawnTime useCount=%useCount uniqueUsers=%uniqueUsers",
                 {{"%goState", std::to_string(uint32(go->GetGoState()))},
                  {"%lootState", std::to_string(uint32(go->getLootState()))},
                  {"%spawned", QgoBool(go->isSpawned())},
                  {"%byDefault", QgoBool(go->isSpawnedByDefault())},
                  {"%respawnTime", std::to_string(go->GetRespawnTime())},
                  {"%useCount", std::to_string(go->GetUseCount())},
                  {"%uniqueUsers", std::to_string(go->GetUniqueUseCount())}}));
    tell(QgoText("qgo_fields", "QGO fields: flags=%flags displayId=%displayId faction=%faction level=%level scale=%scale",
                 {{"%flags", QgoHex(go->GetUInt32Value(GAMEOBJECT_FLAGS))},
                  {"%displayId", std::to_string(go->GetUInt32Value(GAMEOBJECT_DISPLAYID))},
                  {"%faction", std::to_string(go->GetUInt32Value(GAMEOBJECT_FACTION))},
                  {"%level", std::to_string(go->GetUInt32Value(GAMEOBJECT_LEVEL))},
                  {"%scale", QgoFloat(go->GetFloatValue(OBJECT_FIELD_SCALE_X))}}));
    tell(QgoText("qgo_bytes1", "QGO bytes1: state=%state type=%type artkit=%artkit animprogress=%animprogress",
                 {{"%state", std::to_string(uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 0)))},
                  {"%type", std::to_string(uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 1)))},
                  {"%artkit", std::to_string(uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 2)))},
                  {"%animprogress", std::to_string(uint32(go->GetByteValue(GAMEOBJECT_BYTES_1, 3)))}}));
    tell(QgoText("qgo_loot_state", "QGO loot state: items=%items gold=%gold looted=%looted type=%type",
                 {{"%items", std::to_string(go->loot.items.size())},
                  {"%gold", std::to_string(go->loot.gold)},
                  {"%looted", QgoBool(go->loot.isLooted())},
                  {"%type", std::to_string(uint32(go->loot.loot_type))}}));

    tell(QgoText("qgo_bot",
                 "QGO bot: name=%name distance=%distance dist2d=%dist2d withinInteract=%withinInteract "
                 "interactionDist=%interactionDist los=%los samePhase=%samePhase activatedToQuest=%activatedToQuest "
                 "hasQuestForGO=%hasQuestForGO usableMounted=%usableMounted",
                 {{"%name", bot->GetName()},
                  {"%distance", QgoFloat(bot->GetDistance(go))},
                  {"%dist2d", QgoFloat(bot->GetDistance2d(go))},
                  {"%withinInteract", QgoBool(go->IsWithinDistInMap(bot, INTERACTION_DISTANCE))},
                  {"%interactionDist", QgoFloat(go->GetInteractionDistance())},
                  {"%los", QgoBool(go->IsWithinLOSInMap(bot))},
                  {"%samePhase", QgoBool(go->InSamePhase(bot))},
                  {"%activatedToQuest", QgoBool(go->ActivateToQuest(bot))},
                  {"%hasQuestForGO", QgoBool(bot->HasQuestForGO(go->GetEntry()))},
                  {"%usableMounted", QgoBool(info->IsUsableMounted())}}));

    uint32 const lockId = info->GetLockId();
    LockEntry const* lock = sLockStore.LookupEntry(lockId);
    if (!lock)
        tell(QgoText("qgo_lock_none", "QGO lock: %lockId (no Lock.dbc entry)", {{"%lockId", std::to_string(lockId)}}));
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
            tell(QgoText("qgo_lock_slot",
                         "QGO lock slot %slot: type=%type index=%index skill=%skill skillByLockType=%mappedSkill "
                         "botSkillValue=%botSkillValue",
                         {{"%slot", std::to_string(uint32(i))},
                          {"%type", std::to_string(lock->Type[i])},
                          {"%index", std::to_string(lock->Index[i])},
                          {"%skill", std::to_string(lock->Skill[i])},
                          {"%mappedSkill", std::to_string(mappedSkill)},
                          {"%botSkillValue", std::to_string(mappedSkill ? bot->GetSkillValue(mappedSkill) : 0)}}));
        }
    }

    if (GameObjectDisplayInfoEntry const* display = sGameObjectDisplayInfoStore.LookupEntry(go->GetDisplayId()))
        tell(QgoText("qgo_display", "QGO display: file='%file' min=(%minX,%minY,%minZ) max=(%maxX,%maxY,%maxZ)",
                     {{"%file", display->filename},
                      {"%minX", QgoFloat(display->minX)},
                      {"%minY", QgoFloat(display->minY)},
                      {"%minZ", QgoFloat(display->minZ)},
                      {"%maxX", QgoFloat(display->maxX)},
                      {"%maxY", QgoFloat(display->maxY)},
                      {"%maxZ", QgoFloat(display->maxZ)}}));

    uint32 const lootId = info->GetLootId();
    if (lootId)
    {
        // Fishing holes load their loot into a separate store; reporting the gameobject
        // store for them either misses the table or shows an unrelated one with the same id.
        LootStore const& lootStore =
            info->type == GAMEOBJECT_TYPE_FISHINGHOLE ? LootTemplates_Fishing : LootTemplates_Gameobject;
        LootTemplate const* lTemplate = lootStore.GetLootFor(lootId);
        tell(QgoText("qgo_loot_table", "QGO loot table %lootId: present=%present questLootForBot=%questLootForBot",
                     {{"%lootId", std::to_string(lootId)},
                      {"%present", QgoBool(lTemplate != nullptr)},
                      {"%questLootForBot", QgoBool(lootStore.HaveQuestLootForPlayer(lootId, bot))}}));

        if (lTemplate)
        {
            auto const* access = reinterpret_cast<LootTemplateAccess const*>(lTemplate);
            for (LootStoreItem const* entry : access->Entries)
            {
                if (entry->reference)
                {
                    tell(QgoText("qgo_loot_ref",
                                 "QGO loot ref=%reference chance=%chance min=%min max=%max needsQuest=%needsQuest "
                                 "mode=%mode group=%group",
                                 {{"%reference", std::to_string(entry->reference)},
                                  {"%chance", QgoFloat2(entry->chance)},
                                  {"%min", std::to_string(entry->mincount)},
                                  {"%max", std::to_string(entry->maxcount)},
                                  {"%needsQuest", QgoBool(entry->needs_quest)},
                                  {"%mode", std::to_string(entry->lootmode)},
                                  {"%group", std::to_string(entry->groupid)}}));
                    continue;
                }

                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(entry->itemid);
                tell(QgoText("qgo_loot_item",
                             "QGO loot item=%item '%name' class=%class quality=%quality maxcount=%stackMax sell=%sell "
                             "chance=%chance min=%min max=%max needsQuest=%needsQuest",
                             {{"%item", std::to_string(entry->itemid)},
                              {"%name", proto ? proto->Name1 : "?"},
                              {"%class", std::to_string(proto ? proto->Class : 0)},
                              {"%quality", std::to_string(proto ? proto->Quality : 0)},
                              {"%stackMax", std::to_string(proto ? proto->MaxCount : 0)},
                              {"%sell", std::to_string(proto ? proto->SellPrice : 0)},
                              {"%chance", QgoFloat2(entry->chance)},
                              {"%min", std::to_string(entry->mincount)},
                              {"%max", std::to_string(entry->maxcount)},
                              {"%needsQuest", QgoBool(entry->needs_quest)}}));
            }
        }
    }
}
