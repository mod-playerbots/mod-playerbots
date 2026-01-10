#include "PlayerbotSpellCache.h"

void PlayerbotSpellCache::Initialize()
{
    LOG_INFO("playerbots",
        "Playerbots: ListSpellsAction caches initialized");
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        if (SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j))
            skillSpells[skillLine->Spell] = skillLine;
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
            }
            while (results->NextRow());
        }

        LOG_DEBUG("playerbots",
            "ListSpellsAction: initialized caches (skillSpells={}, vendorItems={}).",
            skillSpells.size(), vendorItems.size());
}

SkillLineAbilityEntry const* PlayerbotSpellCache::GetSkillLine(uint32 spellId) const
{
    auto itr = skillSpells.find(spellId);
    if (itr != skillSpells.end())
        return itr->second;
    return nullptr;
}

bool PlayerbotSpellCache::IsItemBuyable(uint32 itemId) const
{
    return vendorItems.find(itemId) != vendorItems.end();
}
