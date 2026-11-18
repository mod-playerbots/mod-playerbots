/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidOpener.h"

#include <list>
#include <vector>

#include "AttackAction.h"
#include "Creature.h"
#include "Group.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidCombatHelpers.h"
#include "RaidInstanceState.h"
#include "Timer.h"

namespace
{
constexpr uint32 SPELL_MISDIRECTION = 34477;
constexpr uint32 SPELL_TRICKS_OF_THE_TRADE = 57934;
// the redirect after the rogue's first attack
constexpr uint32 SPELL_TRICKS_OF_THE_TRADE_PROC = 59628;

// ToC's whole arena from anywhere on its floor. A boss further out than this isn't held.
constexpr float BOSS_SEARCH_RADIUS = 200.0f;

// Area damage from this close counts as hitting the boss
constexpr float OPENER_AREA_THREAT_REACH = 30.0f;

struct OpenerBoss
{
    ObjectGuid guid;
    uint32 engagedMs = 0;
    uint32 tankSinceMs = 0;
    bool tankHeld = false;
    bool released = false;
};

// One read per instance per ms: every DPS bot's multiplier asks per action
struct OpenerState
{
    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value
    std::vector<OpenerBoss> bosses;
};

RaidInstanceState<OpenerState> openerStates;

Player* GroupTankHolding(Unit* boss, Group* group)
{
    Unit* victim = boss ? boss->GetVictim() : nullptr;
    Player* holder = victim ? victim->ToPlayer() : nullptr;
    if (!holder || !group || holder->GetGroup() != group)
        return nullptr;

    return PlayerbotAI::IsTank(holder) || PlayerbotAI::IsTank(holder, true) ? holder : nullptr;
}

OpenerState const* ReadOpener(PlayerbotAI* botAI, RaidOpenerConfig const& config)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Map* map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != config.mapId || !bot->GetInstanceId())
        return nullptr;

    OpenerState& state = openerStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;

    // A boss out of combat drops out, so a wipe or a reset starts its opener over
    std::vector<OpenerBoss> next;
    Group* group = bot->GetGroup();
    std::vector<uint32> const& entries = config.liveBosses(botAI);

    // One sweep for every entry, read back entry by entry in config order
    std::list<Creature*> creatures;
    if (!entries.empty())
        bot->GetCreatureListWithEntryInGrid(creatures, entries, BOSS_SEARCH_RADIUS);

    for (uint32 entry : entries)
    {
        for (Creature* creature : creatures)
        {
            if (creature->GetEntry() != entry || !creature->IsAlive() || !creature->IsInCombat())
                continue;

            OpenerBoss boss;
            boss.guid = creature->GetGUID();
            boss.engagedMs = now;
            for (OpenerBoss const& known : state.bosses)
            {
                if (known.guid == boss.guid)
                {
                    boss = known;
                    break;
                }
            }

            if (!boss.released)
            {
                if (!GroupTankHolding(creature, group))
                    boss.tankHeld = false;
                else if (!boss.tankHeld)
                {
                    boss.tankHeld = true;
                    boss.tankSinceMs = now;
                }

                boss.released = (boss.tankHeld && getMSTimeDiff(boss.tankSinceMs, now) >= RAID_OPENER_DWELL_MS) ||
                                getMSTimeDiff(boss.engagedMs, now) >= RAID_OPENER_CAP_MS;
            }

            next.push_back(boss);
        }
    }

    state.bosses = std::move(next);
    return &state;
}

bool RedirectRunning(Player* bot)
{
    switch (bot->getClass())
    {
        case CLASS_HUNTER:
            return bot->HasAura(SPELL_MISDIRECTION) || bot->HasAura(SPELL_MISDIRECTION_PROC);
        case CLASS_ROGUE:
            return bot->HasAura(SPELL_TRICKS_OF_THE_TRADE) || bot->HasAura(SPELL_TRICKS_OF_THE_TRADE_PROC);
        default:
            return false;
    }
}

// null when this bot holds on nothing. Not gated on the bot's own combat: its first hit is the one
// that matters.
OpenerState const* HoldingState(PlayerbotAI* botAI, RaidOpenerConfig const& config)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || !IsRaidDpsBot(bot) || RedirectRunning(bot))
        return nullptr;

    OpenerState const* state = ReadOpener(botAI, config);
    if (!state)
        return nullptr;

    for (OpenerBoss const& boss : state->bosses)
        if (!boss.released)
            return state;

    return nullptr;
}

bool Holds(OpenerState const& state, ObjectGuid guid)
{
    for (OpenerBoss const& boss : state.bosses)
        if (!boss.released && boss.guid == guid)
            return true;

    return false;
}
}  // namespace

bool IsRaidOpenerHeld(PlayerbotAI* botAI, RaidOpenerConfig const& config, Unit* unit)
{
    if (!unit)
        return false;

    OpenerState const* state = HoldingState(botAI, config);
    return state && Holds(*state, unit->GetGUID());
}

Unit* GetRaidOpenerHeldBossNear(PlayerbotAI* botAI, RaidOpenerConfig const& config, float reach)
{
    OpenerState const* state = HoldingState(botAI, config);
    if (!state)
        return nullptr;

    Player* bot = botAI->GetBot();
    for (OpenerBoss const& boss : state->bosses)
    {
        if (boss.released)
            continue;

        Unit* unit = botAI->GetUnit(boss.guid);
        if (unit && bot->GetExactDist2d(unit) <= reach)
            return unit;
    }

    return nullptr;
}

bool RaidOpenerHolds(PlayerbotAI* botAI, RaidOpenerConfig const& config)
{
    return HoldingState(botAI, config) != nullptr;
}

Player* GetRaidOpenerRedirectTank(PlayerbotAI* botAI, RaidOpenerConfig const& config)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    if (!bot || (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE))
        return nullptr;

    OpenerState const* state = HoldingState(botAI, config);
    if (!state)
        return nullptr;

    Group* group = bot->GetGroup();
    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (current && Holds(*state, current->GetGUID()))
        if (Player* tank = GroupTankHolding(current, group))
            return tank;

    for (OpenerBoss const& boss : state->bosses)
    {
        if (boss.released)
            continue;

        if (Player* tank = GroupTankHolding(botAI->GetUnit(boss.guid), group))
            return tank;
    }

    return nullptr;
}

bool RaidOpenerSwingTrigger::IsActive()
{
    return RaidOpenerHolds(botAI, _config) && bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING) &&
           IsRaidOpenerHeld(botAI, _config, bot->GetVictim());
}

bool RaidOpenerRedirectTrigger::IsActive()
{
    Player* tank = GetRaidOpenerRedirectTank(botAI, _config);
    char const* spell = bot->getClass() == CLASS_ROGUE ? "tricks of the trade" : "misdirection";
    return tank && botAI->CanCastSpell(spell, tank);
}

bool RaidOpenerStopSwingAction::Execute(Event /*event*/) { return bot->AttackStop(); }

bool RaidOpenerStopSwingAction::isUseful()
{
    return bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING) && IsRaidOpenerHeld(botAI, _config, bot->GetVictim());
}

Player* RaidOpenerRedirectAction::GetRedirectTank() { return GetRaidOpenerRedirectTank(botAI, _config); }

float RaidOpenerHoldMultiplier::GetValue(Action* action)
{
    if (!action || dynamic_cast<RaidRedirectThreatAction*>(action) || !RaidOpenerHolds(botAI, _config))
        return 1.0f;

    Unit* const current = AI_VALUE(Unit*, "current target");
    Unit* target = action->GetTarget();
    if (!target)
        target = current;

    if (dynamic_cast<AttackAction*>(action) && !dynamic_cast<MeleeAction*>(action))
        return target == current && IsRaidOpenerHeld(botAI, _config, target) ? 0.0f : 1.0f;

    if (IsRaidOpenerHeld(botAI, _config, target))
        return 0.0f;

    return IsRaidAreaDamage(botAI, action, _splash) &&
                   GetRaidOpenerHeldBossNear(botAI, _config, OPENER_AREA_THREAT_REACH)
               ? 0.0f
               : 1.0f;
}
