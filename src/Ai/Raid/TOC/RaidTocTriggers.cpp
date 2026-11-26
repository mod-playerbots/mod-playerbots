/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidTocTriggers.h"
#include "Playerbots.h"
#include "SpellMgr.h"

bool TocChampionsChasedTrigger::IsActive()
{
    if (bot->GetMapId() != 649)  // Trial of the Crusader only
        return false;

    // The tank must never kite: it has to hold the boss and its facing.
    if (PlayerbotAI::IsTank(bot))
        return false;

    // Heroic champions hit much harder, so the escape has to start earlier
    // (65% health instead of 50%). The difficulty flag is per instance, so
    // this works for both 10 and 25 man.
    uint8 healthThreshold = 50;
    if (Map* map = bot->GetMap())
        if (map->IsHeroic())
            healthThreshold = 65;

    if (AI_VALUE2(uint8, "health", "self target") > healthThreshold)
        return false;

    for (ObjectGuid const guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || !attacker->IsAlive() || attacker->IsPet())
            continue;

        // Being actively chased: it targets us and is already in melee range.
        if (attacker->GetVictim() != bot || attacker->GetExactDist2d(bot) >= 12.0f)
            continue;

        // Only the Faction Champions qualify: they are player-class NPCs
        // (unit_class set), while the regular TOC trash and the adds of the
        // other four encounters have CLASS_NONE. Without this filter the
        // kiting would also fire during the beasts or Anub'arak adds.
        Creature* creature = attacker->ToCreature();
        if (!creature || creature->getClass() == CLASS_NONE)
            continue;

        return true;
    }

    return false;
}

bool TocTwinValkyrFightTrigger::IsActive()
{
    if (bot->GetMapId() != 649)  // Trial of the Crusader only
        return false;

    // The Touch mechanic (and therefore the portal dance) only exists in the
    // heroic versions - 10 and 25 man heroic.
    if (Map* map = bot->GetMap())
        if (!map->IsHeroic())
            return false;

    // Look the sisters up by name, not by entry: TOC uses different entries
    // per difficulty (34496/34497 for 10-man normal, 36065/36066 for the
    // others), and a hardcoded entry list silently breaks the other modes.
    for (char const* name : {"fjola lightbane", "eydis darkbane"})
    {
        Unit* sister = AI_VALUE2(Unit*, "find target", name);
        if (sister && sister->IsAlive() && sister->IsInCombat())
            return true;
    }

    return false;
}

bool TocIcehowlChargeTrigger::IsActive()
{
    if (bot->GetMapId() != 649)
        return false;

    Unit* icehowl = AI_VALUE2(Unit*, "find target", "icehowl");
    if (!icehowl || !icehowl->IsAlive() || !icehowl->IsInCombat())
        return false;

    // Whoever carries the Ferocious Butt marker is the charge target. Check
    // the bot itself and then every group member.
    constexpr uint32 SPELL_FEROCIOUS_BUTT = 66770;

    if (bot->HasAura(SPELL_FEROCIOUS_BUTT))
        return true;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_FEROCIOUS_BUTT))
                return true;
        }
    }

    return false;
}

bool TocAnubSubmergeTrigger::IsActive()
{
    if (bot->GetMapId() != 649)
        return false;

    Unit* anub = AI_VALUE2(Unit*, "find target", "anub'arak");
    if (!anub || !anub->IsAlive() || !anub->IsInCombat())
        return false;

    // Submerge (phase 2): the boss burrows and the pursuing spikes appear.
    constexpr uint32 SPELL_SUBMERGE = 65981;
    return anub->HasAura(SPELL_SUBMERGE) || anub->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
}

bool TocJaraxxusAddsTrigger::IsActive()
{
    if (bot->GetMapId() != 649)
        return false;

    Unit* jaraxxus = AI_VALUE2(Unit*, "find target", "lord jaraxxus");
    if (!jaraxxus || !jaraxxus->IsAlive() || !jaraxxus->IsInCombat())
        return false;

    // Nether Portal spawns the Mistress of Pain (healer), the Infernal Volcano
    // spawns infernals. Portal first: a free-healing mistress outvalues a few
    // infernals.
    for (uint32 entry : {34825 /* Nether Portal */, 34813 /* Infernal Volcano */})
    {
        if (Unit* add = bot->FindNearestCreature(entry, 120.0f))
            if (add->IsAlive())
                return true;
    }

    return false;
}

bool TocJaraxxusNetherPowerTrigger::IsActive()
{
    if (bot->GetMapId() != 649)
        return false;

    // Only the tank burns a defensive cooldown here.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* jaraxxus = AI_VALUE2(Unit*, "find target", "lord jaraxxus");
    if (!jaraxxus || !jaraxxus->IsAlive() || !jaraxxus->IsInCombat())
        return false;

    // Nether Power stacks one per uninterrupted cast. Two or more stacks means
    // the kick was missed and nobody stole/dispelled it in time. The spell id
    // differs per difficulty, so resolve it through the spell manager instead
    // of hardcoding the base id.
    constexpr uint32 SPELL_NETHER_POWER = 66228;
    uint32 const spellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_NETHER_POWER, jaraxxus);
    if (Aura* aura = jaraxxus->GetAura(spellId ? spellId : SPELL_NETHER_POWER))
        return aura->GetStackAmount() >= 2;

    return false;
}

bool TocBurningBileTrigger::IsActive()
{
    if (bot->GetMapId() != 649)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* dreadscale = AI_VALUE2(Unit*, "find target", "dreadscale");
    if (!dreadscale || !dreadscale->IsAlive() || !dreadscale->IsInCombat())
        return false;

    // Only the tank that got bitten carries the burning debuff.
    static constexpr uint32 SPELL_BURNING_BITE = 66879;
    return bot->HasAura(SPELL_BURNING_BITE);
}
