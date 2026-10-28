/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidDefensive.h"

#include <algorithm>
#include <array>
#include <vector>

#include "EncounterHelpers.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidTankDefensive.h"
#include "Spell.h"
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"

namespace
{
constexpr uint32 SPELL_ANTI_MAGIC_SHELL = 48707;
constexpr uint32 SPELL_CLOAK_OF_SHADOWS = 31224;
constexpr uint32 SPELL_DISPERSION = 47585;
constexpr uint32 SPELL_ICE_BLOCK = 45438;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
// Physical cooldowns of dps classes, which the tank rows leave out: Evasion (both ranks), Deterrence,
// Pain Suppression on itself
constexpr std::array<uint32, 4> DPS_PHYSICAL_DEFENSIVES = {26669, 5277, 19263, 33206};
// Raid cooldowns a bot channels, which a personal button would break
constexpr std::array<char const*, 2> RAID_CHANNELS = {"divine hymn", "tranquility"};

// a stone heals about this much of a bot's health, so over it most of the heal is lost
constexpr float HEALTHSTONE_MAX_PCT = 70.0f;

// getMSTime wraps, so a time either side of now is read signed
int32 MsFrom(uint32 from, uint32 to) { return static_cast<int32>(to - from); }

// What the hit takes off the member: its average resistance and the damage taken modifiers up now, a
// running Divine Guardian or Barkskin included
float ExpectedDamage(Player* member, RaidPredictedHit const& hit)
{
    if (member->IsImmunedToDamage(hit.school))
        return 0.0f;

    if (hit.measured)
        return static_cast<float>(hit.damage);

    float const resist = Unit::GetEffectiveResistChance(nullptr, hit.school, member, nullptr, hit.casterLevel);
    float const taken = member->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, hit.school);
    return static_cast<float>(hit.damage) * std::max(0.0f, 1.0f - resist) * taken;
}

bool EndsUnderFloor(Player* member, float expected)
{
    float const floor = member->GetMaxHealth() * RAID_DEFENSIVE_FLOOR_PCT / 100.0f;
    return static_cast<float>(member->GetHealth()) - expected < floor;
}

bool ChannelsRaidCooldown(PlayerbotAI* botAI)
{
    Spell* channel = botAI->GetBot()->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!channel || !channel->GetSpellInfo())
        return false;

    for (char const* name : RAID_CHANNELS)
        if (botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get() == channel->GetSpellInfo()->Id)
            return true;

    return false;
}

bool ItemCoolingDown(Player* bot, Item* item)
{
    ItemTemplate const* proto = item->GetTemplate();
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE && proto->Spells[i].SpellId > 0 &&
            bot->HasSpellCooldown(static_cast<uint32>(proto->Spells[i].SpellId)))
            return true;

    return false;
}

// Not on the move: using an item stops the bot, mid dodge too
bool HealthstoneReady(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->isMoving())
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Action* action = context->GetAction("healthstone");
    if (!action || !action->isPossible() || !action->isUseful())
        return false;

    std::vector<Item*> const items = context->GetValue<std::vector<Item*>>("inventory items", "healthstone")->Get();
    return !items.empty() && !ItemCoolingDown(bot, items.front());
}

bool PersonalRunning(Player* bot)
{
    for (uint32 const spellId :
         {SPELL_ANTI_MAGIC_SHELL, SPELL_CLOAK_OF_SHADOWS, SPELL_DISPERSION, SPELL_ICE_BLOCK, SPELL_DIVINE_SHIELD})
        if (bot->HasAura(spellId))
            return true;

    return false;
}

uint32 Castable(PlayerbotAI* botAI, uint32 spellId)
{
    Player* bot = botAI->GetBot();
    return spellId && bot->HasSpell(spellId) && botAI->CanCastSpell(spellId, bot) ? spellId : 0;
}

// Not Castable: that fails on the GCD, and Divine Protection is off it, so it would take the pick and
// its Forbearance would lock the shield out. The action stops a cast, and on the GCD the trigger retries.
uint32 ImmunityReady(Player* bot, uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info || !bot->HasSpell(spellId) || bot->HasSpellCooldown(spellId) ||
        (info->ExcludeCasterAuraSpell && bot->HasAura(info->ExcludeCasterAuraSpell)))
        return 0;

    if (info->PreventionType == SPELL_PREVENTION_TYPE_SILENCE && bot->HasUnitFlag(UNIT_FLAG_SILENCED))
        return 0;

    bool const held =
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) && !info->HasAttribute(SPELL_ATTR5_ALLOW_WHILE_STUNNED);
    return held ? 0 : spellId;
}

RaidPersonalPick PickPersonal(PlayerbotAI* botAI, RaidPredictedHit const& hit)
{
    RaidPersonalPick pick;
    Player* bot = botAI->GetBot();
    uint32 const now = getMSTime();
    int32 const fromStart = MsFrom(hit.startMs, now);
    if (fromStart < -static_cast<int32>(RAID_PERSONAL_LEAD_MS) || fromStart > static_cast<int32>(hit.durationMs))
        return pick;

    if (ChannelsRaidCooldown(botAI) || PersonalRunning(bot))
        return pick;

    float const expected = ExpectedDamage(bot, hit);
    if (!EndsUnderFloor(bot, expected))
        return pick;

    // Divine Protection's Forbearance would lock the shield out for the rest of the fight
    if (!hit.keepVictim && hit.measured && bot->getClass() == CLASS_PALADIN && !PlayerbotAI::IsTank(bot))
        pick.spellId = ImmunityReady(bot, SPELL_DIVINE_SHIELD);
    else if (!hit.keepVictim && hit.focus && bot->getClass() == CLASS_MAGE)
        pick.spellId = ImmunityReady(bot, SPELL_ICE_BLOCK);

    uint32 const needMs = static_cast<uint32>(std::max<int32>(static_cast<int32>(hit.durationMs) - fromStart, 0));
    if (!pick)
        pick.spellId = hit.school == SPELL_SCHOOL_MASK_NORMAL ? NextPhysicalDefensive(botAI, bot, needMs)
                                                              : NextMagicDefensive(botAI, bot, needMs);

    if (!pick && hit.school == SPELL_SCHOOL_MASK_NORMAL)
        for (uint32 const spellId : DPS_PHYSICAL_DEFENSIVES)
            if ((pick.spellId = Castable(botAI, spellId)))
                break;

    if (pick || expected < static_cast<float>(bot->GetHealth()))
        return pick;

    if (bot->getClass() == CLASS_PRIEST)
        pick.spellId = Castable(botAI, SPELL_DISPERSION);

    if (!pick)
        pick.spellId = Castable(botAI, EncounterHelpers::GetSelfImmunitySpell(bot));

    if (!pick && bot->GetHealthPct() <= HEALTHSTONE_MAX_PCT && HealthstoneReady(botAI))
        pick.consumable = "healthstone";

    return pick;
}
}  // namespace

RaidPersonalPick GetRaidPersonalDefensive(PlayerbotAI* botAI, RaidDefensiveScope const& scope,
                                          RaidPredictedHit const* hit)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!hit || !bot || !bot->IsInWorld() || bot->GetMapId() != scope.mapId || !bot->GetInstanceId())
        return {};

    return PickPersonal(botAI, *hit);
}
