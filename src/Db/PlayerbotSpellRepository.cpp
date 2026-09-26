/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotSpellRepository.h"

#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Log.h"
#include "QueryResult.h"  // Required due to a poor implementation by AC
#include "SpellInfo.h"
#include "SpellMgr.h"

//  caches the result set
void PlayerbotSpellRepository::Initialize()
{
    LOG_INFO("playerbots", "Playerbots: ListSpellsAction caches initialized");

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        if (SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j))
            skillSpells[skillLine->Spell] = skillLine;
    }

    for (uint32 spellId = 0; spellId < sSpellMgr->GetSpellInfoStoreSize(); ++spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        for (SpellEffectInfo const& spellEffectInfo : spellInfo->GetEffects())
        {
            if ((spellEffectInfo.Effect == SPELL_EFFECT_OPEN_LOCK || spellEffectInfo.Effect == SPELL_EFFECT_SKINNING) &&
                spellEffectInfo.MiscValue)
                _openingSpells[uint32(spellEffectInfo.MiscValue)].push_back(spellId);
        }
    }

    // Fill the vendorItems cache once from the world database.
    QueryResult results = WorldDatabase.Query("SELECT item FROM npc_vendor WHERE maxcount = 0");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            int32 entry = fields[0].Get<int32>();
            if (entry <= 0)
                continue;

            vendorItems.insert(static_cast<uint32>(entry));
        } while (results->NextRow());
    }

    LOG_DEBUG("playerbots", "ListSpellsAction: initialized caches (skillSpells={}, vendorItems={}).",
              skillSpells.size(), vendorItems.size());
}

SkillLineAbilityEntry const* PlayerbotSpellRepository::GetSkillLine(uint32 spellId) const
{
    auto itr = skillSpells.find(spellId);
    if (itr != skillSpells.end())
        return itr->second;
    return nullptr;
}

bool PlayerbotSpellRepository::IsItemBuyable(uint32 itemId) const
{
    return vendorItems.find(itemId) != vendorItems.end();
}

std::vector<uint32> const& PlayerbotSpellRepository::GetOpeningSpells(uint32 lockType) const
{
    static std::vector<uint32> const empty;
    auto const itr = _openingSpells.find(lockType);
    return itr != _openingSpells.end() ? itr->second : empty;
}
