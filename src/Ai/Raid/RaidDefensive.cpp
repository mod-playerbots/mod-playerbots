/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidDefensive.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>
#include <vector>

#include "BossAuraTriggers.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidInstanceState.h"
#include "RaidTankDefensive.h"
#include "Spell.h"
#include "SpellAuraDefines.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"

namespace
{
constexpr uint32 SPELL_DIVINE_SACRIFICE = 64205;
constexpr uint32 SPELL_DIVINE_GUARDIAN = 70940;
constexpr uint32 SPELL_DIVINE_GUARDIAN_RANK_1 = 53527;
constexpr uint32 SPELL_DIVINE_GUARDIAN_RANK_2 = 53530;
constexpr uint32 SPELL_ANTI_MAGIC_ZONE = 51052;
// The zone the Anti-Magic Zone totem keeps up, whose radius it covers
constexpr uint32 SPELL_ANTI_MAGIC_ZONE_AURA = 50461;
constexpr uint32 SPELL_ANTI_MAGIC_SHELL = 48707;
constexpr uint32 SPELL_CLOAK_OF_SHADOWS = 31224;
constexpr uint32 SPELL_DISPERSION = 47585;
constexpr uint32 SPELL_ICE_BLOCK = 45438;
constexpr uint32 SPELL_DIVINE_SHIELD = 642;
// Physical cooldowns of dps classes, which the tank rows leave out: Evasion (both ranks), Deterrence,
// Pain Suppression on itself
constexpr std::array<uint32, 4> DPS_PHYSICAL_DEFENSIVES = {26669, 5277, 19263, 33206};

// a stone heals about this much of a bot's health, so over it most of the heal is lost
constexpr float HEALTHSTONE_MAX_PCT = 70.0f;

// Divine Sacrifice drops under 20% of the paladin's health, so it starts well above that
constexpr float SACRIFICE_MIN_PCT = 50.0f;
// Two reads of one hit, a bot's tick apart, put its start this close
constexpr int32 SAME_HIT_MS = 2000;
// A hit nobody would end under the floor for yet is counted again this often, not every tick
constexpr uint32 DEAL_RETRY_MS = 250;

struct CoverSpec
{
    RaidCover cover;
    uint8 playerClass;
    char const* name;
    bool channel;
    // by id where the name lookup misses: it compares the DBC name, and "anti magic zone" isn't
    // "Anti-Magic Zone"
    uint32 spellId = 0;
};

// Deal order, and a bot takes one at most: the aura holder gets Aura Mastery before Divine Sacrifice
// could claim it.
constexpr CoverSpec COVERS[] = {
    {RaidCover::AuraMastery, CLASS_PALADIN, "aura mastery", false},
    {RaidCover::DivineSacrifice, CLASS_PALADIN, "divine sacrifice", false},
    {RaidCover::AntiMagicZone, CLASS_DEATH_KNIGHT, "anti magic zone", false, SPELL_ANTI_MAGIC_ZONE},
    {RaidCover::DivineHymn, CLASS_PRIEST, "divine hymn", true},
    {RaidCover::Tranquility, CLASS_DRUID, "tranquility", true},
};
constexpr size_t COVER_COUNT = std::size(COVERS);

struct DefensiveState
{
    bool active = false;
    bool dealt = false;
    uint32 startMs = 0;
    uint32 retryMs = 0;
    std::array<ObjectGuid, COVER_COUNT> holder{};
};

RaidInstanceState<DefensiveState> defensiveStates;

DefensiveState* StateFor(Player* bot, RaidDefensiveScope const& scope)
{
    if (!bot || !bot->IsInWorld() || bot->GetMapId() != scope.mapId || !bot->GetInstanceId())
        return nullptr;

    return &defensiveStates.For(bot->GetInstanceId());
}

// getMSTime wraps, so a time either side of now is read signed
int32 MsFrom(uint32 from, uint32 to) { return static_cast<int32>(to - from); }

CoverSpec const* SpecOf(RaidCover cover)
{
    for (CoverSpec const& spec : COVERS)
        if (spec.cover == cover)
            return &spec;

    return nullptr;
}

std::vector<Player*> MembersInInstance(Player* bot)
{
    std::vector<Player*> members;
    Group* group = bot->GetGroup();
    if (!group)
    {
        if (bot->IsAlive())
            members.push_back(bot);

        return members;
    }

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsInWorld() && member->IsAlive() && member->IsInMap(bot))
            members.push_back(member);
    }

    return members;
}

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

bool CanPay(Player* caster, SpellInfo const* info)
{
    if (info->PowerType >= MAX_POWERS || info->PowerType == POWER_RUNE)
        return true;

    return static_cast<int32>(caster->GetPower(Powers(info->PowerType))) >=
           info->CalcPowerCost(caster, info->GetSchoolMask());
}

uint32 ResistAuraOf(SpellSchoolMask school)
{
    if (school & SPELL_SCHOOL_MASK_FIRE)
        return SPELL_FIRE_RESISTANCE_AURA_RANK_1;

    if (school & SPELL_SCHOOL_MASK_FROST)
        return SPELL_FROST_RESISTANCE_AURA_RANK_1;

    if (school & SPELL_SCHOOL_MASK_SHADOW)
        return SPELL_SHADOW_RESISTANCE_AURA_RANK_1;

    return 0;
}

uint32 CoverSpellId(Player* member, CoverSpec const& spec)
{
    if (spec.spellId)
        return member->HasSpell(spec.spellId) ? spec.spellId : 0;

    PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
    return memberAI ? memberAI->GetAiObjectContext()->GetValue<uint32>("spell id", spec.name)->Get() : 0;
}

// Could cast the cover from where it stands, its own cast check aside: a healer mid heal fails
// CanCastSpell, and the pinned-cast break gets it out of that inside the window
bool CoverReady(Player* member, CoverSpec const& spec, RaidPredictedHit const& hit,
                std::vector<Player*> const& members)
{
    if (member->getClass() != spec.playerClass || !GET_PLAYERBOT_AI(member))
        return false;

    uint32 const spellId = CoverSpellId(member, spec);
    SpellInfo const* info = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!info || member->HasSpellCooldown(spellId) || !CanPay(member, info) ||
        info->CheckShapeshift(member->GetShapeshiftForm()) != SPELL_CAST_OK)
        return false;

    if (spec.channel && hit.noChannel && hit.noChannel(member))
        return false;

    switch (spec.cover)
    {
        case RaidCover::AuraMastery:
        {
            uint32 const aura = ResistAuraOf(hit.school);
            return aura && member->GetAuraOfRankedSpell(aura, member->GetGUID());
        }
        case RaidCover::DivineSacrifice:
            return RaidHasDivineGuardian(member) && member->GetHealthPct() >= SACRIFICE_MIN_PCT;
        case RaidCover::AntiMagicZone:
        {
            SpellInfo const* zone = sSpellMgr->GetSpellInfo(SPELL_ANTI_MAGIC_ZONE_AURA);
            float const radius = zone ? zone->Effects[EFFECT_0].CalcRadius() : 0.0f;
            uint32 const inside = static_cast<uint32>(std::count_if(
                members.begin(), members.end(),
                [member, radius](Player* other) { return member->GetExactDist(other) <= radius; }));
            return inside >= RAID_COVER_MIN_UNDER;
        }
        default:
            return true;
    }
}

void ClearDeal(DefensiveState& state)
{
    state.holder.fill(ObjectGuid::Empty);
    state.active = false;
    state.dealt = false;
}

void Deal(Player* bot, DefensiveState& state, RaidPredictedHit const& hit, uint32 now)
{
    if (state.retryMs && MsFrom(state.retryMs, now) < 0)
        return;

    state.retryMs = now + DEAL_RETRY_MS;
    std::vector<Player*> const members = MembersInInstance(bot);
    auto const hurt = [&hit](Player* member) { return EndsUnderFloor(member, ExpectedDamage(member, hit)); };
    uint32 const under = static_cast<uint32>(std::count_if(members.begin(), members.end(), hurt));
    if (under < RAID_COVER_MIN_UNDER)
        return;

    state.dealt = true;
    std::vector<ObjectGuid> taken;
    for (size_t i = 0; i < COVER_COUNT; ++i)
    {
        CoverSpec const& spec = COVERS[i];
        Player* best = nullptr;
        for (Player* member : members)
        {
            if (std::find(taken.begin(), taken.end(), member->GetGUID()) != taken.end() ||
                !CoverReady(member, spec, hit, members))
                continue;

            // a healer's channel heals for most, the others go lowest guid first
            auto const rank = [&spec](Player* player)
            { return std::make_pair(spec.channel && !PlayerbotAI::IsHeal(player), player->GetGUID()); };
            if (!best || rank(member) < rank(best))
                best = member;
        }

        if (!best)
            continue;

        state.holder[i] = best->GetGUID();
        taken.push_back(best->GetGUID());
    }
}

bool InWindow(RaidPredictedHit const& hit, uint32 now)
{
    int32 const fromStart = MsFrom(hit.startMs, now);
    return fromStart >= -static_cast<int32>(RAID_COVER_WINDOW_LEAD_MS) &&
           fromStart <= static_cast<int32>(RAID_COVER_WINDOW_LATE_MS);
}

RaidCover HeldBy(DefensiveState const& state, Player* bot)
{
    for (size_t i = 0; i < COVER_COUNT; ++i)
        if (state.holder[i] == bot->GetGUID())
            return COVERS[i].cover;

    return RaidCover::None;
}

bool ChannelsRaidCover(Player* bot)
{
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!channel || !channel->GetSpellInfo())
        return false;

    for (CoverSpec const& spec : COVERS)
        if (spec.channel && CoverSpellId(bot, spec) == channel->GetSpellInfo()->Id)
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

    if (ChannelsRaidCover(bot) || PersonalRunning(bot))
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

// A passive talent is only in the talent map, never the spell map HasSpell reads
bool RaidHasDivineGuardian(Player* paladin)
{
    uint8 const spec = paladin->GetActiveSpec();
    return paladin->HasTalent(SPELL_DIVINE_GUARDIAN_RANK_1, spec) ||
           paladin->HasTalent(SPELL_DIVINE_GUARDIAN_RANK_2, spec);
}

RaidCover GetRaidCover(PlayerbotAI* botAI, RaidDefensiveScope const& scope, RaidPredictedHit const* hit)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    DefensiveState* state = StateFor(bot, scope);
    if (!state)
        return RaidCover::None;

    if (!hit)
    {
        if (state->active)
            ClearDeal(*state);

        return RaidCover::None;
    }

    uint32 const now = getMSTime();
    if (!state->active || std::abs(MsFrom(state->startMs, hit->startMs)) > SAME_HIT_MS)
    {
        ClearDeal(*state);
        state->active = true;
        state->startMs = hit->startMs;
        state->retryMs = 0;
    }

    if (!state->dealt)
        Deal(bot, *state, *hit, now);

    RaidCover const cover = HeldBy(*state, bot);
    CoverSpec const* spec = SpecOf(cover);
    if (!spec || !InWindow(*hit, now) || (spec->channel && hit->noChannel && hit->noChannel(bot)))
        return RaidCover::None;

    // on cooldown is cast: it was dealt only while ready
    uint32 const spellId = CoverSpellId(bot, *spec);
    return spellId && !bot->HasSpellCooldown(spellId) ? cover : RaidCover::None;
}

uint32 RaidCoverSpellId(PlayerbotAI* botAI, RaidCover cover)
{
    CoverSpec const* spec = SpecOf(cover);
    return spec && botAI ? CoverSpellId(botAI->GetBot(), *spec) : 0;
}

bool IsRaidCoverChannel(RaidCover cover)
{
    CoverSpec const* spec = SpecOf(cover);
    return spec && spec->channel;
}

void FinishRaidCover(PlayerbotAI* botAI, RaidCover cover)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (bot && cover == RaidCover::DivineSacrifice && bot->HasAura(SPELL_DIVINE_GUARDIAN))
        bot->RemoveAurasDueToSpell(SPELL_DIVINE_SACRIFICE);
}

bool RaidHitLeadIn(RaidPredictedHit const& hit) { return MsFrom(hit.startMs, getMSTime()) < 0; }

std::vector<std::string> RaidCoverClassNodes()
{
    std::vector<std::string> names;
    // no class strategy has an Aura Mastery node to hold
    for (CoverSpec const& spec : COVERS)
        if (spec.cover != RaidCover::AuraMastery)
            names.push_back(spec.name);

    return names;
}

RaidPersonalPick GetRaidPersonalDefensive(PlayerbotAI* botAI, RaidDefensiveScope const& scope,
                                          RaidPredictedHit const* hit)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!hit || !bot || !bot->IsInWorld() || bot->GetMapId() != scope.mapId || !bot->GetInstanceId())
        return {};

    return PickPersonal(botAI, *hit);
}
