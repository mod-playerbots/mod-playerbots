/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCEncounterGate.h"

#include <cstdlib>

#include "CellImpl.h"
#include "Creature.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "RaidInstanceState.h"
#include "Timer.h"
#include "ToCData.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_Shared.h"

namespace
{
using TrialOfTheCrusaderHelpers::ToCEntrySet;
using TrialOfTheCrusaderHelpers::ToCFactionChampions;
using TrialOfTheCrusaderHelpers::ToCNpcs;

struct EncounterPrefix
{
    char const* prefix;
    ToCEncounter encounter;
};

// No prefix here is a prefix of another, so first match wins.
constexpr EncounterPrefix ENCOUNTER_PREFIXES[] = {
    {"gormok", ToCEncounter::NorthrendBeasts},
    {"northrend worms", ToCEncounter::NorthrendBeasts},
    {"icehowl", ToCEncounter::NorthrendBeasts},
    {"jaraxxus", ToCEncounter::Jaraxxus},
    {"faction champions", ToCEncounter::FactionChampions},
    {"twin valkyr", ToCEncounter::TwinValkyr},
    {"anubarak", ToCEncounter::Anubarak},
};

// The arena reaches about 80 yd from ARENA_CENTER (gate 77, stands 64), so from anywhere on the
// floor this sees all of it.
constexpr float ENTRY_SEARCH_RADIUS = 200.0f;

std::vector<uint32> const CHAMPION_ENTRIES = {
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_DRUID_RESTORATION),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_SHAMAN_RESTORATION),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_PALADIN_HOLY),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_PRIEST_DISCIPLINE),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_DRUID_RESTORATION),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_SHAMAN_RESTORATION),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_PALADIN_HOLY),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_PRIEST_DISCIPLINE),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_DEATH_KNIGHT),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_DRUID_BALANCE),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_HUNTER),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_MAGE),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_PALADIN_RETRIBUTION),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_PRIEST_SHADOW),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_ROGUE),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_SHAMAN_ENHANCEMENT),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_WARLOCK),
    static_cast<uint32>(ToCFactionChampions::NPC_ALLIANCE_WARRIOR),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_DEATH_KNIGHT),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_DRUID_BALANCE),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_HUNTER),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_MAGE),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_PALADIN_RETRIBUTION),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_PRIEST_SHADOW),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_ROGUE),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_SHAMAN_ENHANCEMENT),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_WARLOCK),
    static_cast<uint32>(ToCFactionChampions::NPC_HORDE_WARRIOR),
};
ToCEntrySet const CHAMPION_ENTRY_SET(CHAMPION_ENTRIES);

// One read per instance per ms: every gated trigger and multiplier asks, and a read can walk a
// 200 yd grid.
struct ToCGateState
{
    uint32 memoMs = 0;
    bool memoValid = false;  // 0 is a real getMSTime value
    uint32 stage = 0;
    ToCEncounter live = ToCEncounter::None;
    ObjectGuid champion;  // the FC sweep's last hit, tried before the next sweep
};

RaidInstanceState<ToCGateState> gateStates;

// The kill moves the stage on before the adds are gone: snobolds can outlive Gormok, and Jaraxxus's
// Mistresses and Infernals outlive him
ToCEncounter EncounterBeforeStage(uint32 stage)
{
    switch (stage)
    {
        case TOC_PROGRESS_BEASTS_DEAD:
            return ToCEncounter::NorthrendBeasts;
        case TOC_PROGRESS_JARAXXUS_DEAD:
            return ToCEncounter::Jaraxxus;
        default:
            return ToCEncounter::None;
    }
}

bool IsEngaged(Creature const* creature) { return creature && creature->IsAlive() && creature->IsInCombat(); }

// A guid resolves map-wide, hence the combat test on top.
bool GuidEngaged(Map* map, InstanceScript* instance, uint32 type)
{
    ObjectGuid const guid = instance->GetGuidData(type);
    return !guid.IsEmpty() && IsEngaged(map->GetCreature(guid));
}

template <typename EntryTest>
Creature* FirstEngaged(Player* bot, EntryTest const& isEntry)
{
    auto check = [bot, &isEntry](Creature* creature)
    {
        return isEntry(creature->GetEntry()) && bot->IsWithinDist(creature, ENTRY_SEARCH_RADIUS, false) &&
               IsEngaged(creature);
    };
    Creature* found = nullptr;
    Acore::CreatureSearcher<decltype(check)> searcher(bot, found, check);
    Cell::VisitObjects(bot, searcher, ENTRY_SEARCH_RADIUS);
    return found;
}

// True only where FirstEngaged would find it: passes the sweep's tests, its cell within one of the bot's,
// and in that cell's grid container, which lags its position until the map's move list runs
template <typename EntryTest>
bool SweepFinds(Player* bot, Map* map, ObjectGuid const& guid, EntryTest const& isEntry)
{
    if (guid.IsEmpty())
        return false;

    Creature* candidate = map->GetCreature(guid);
    if (!candidate || !candidate->InSamePhase(bot->GetPhaseMask()) || !isEntry(candidate->GetEntry()) ||
        !bot->IsWithinDist(candidate, ENTRY_SEARCH_RADIUS, false) || !IsEngaged(candidate))
        return false;

    if (!Acore::IsValidMapCoord(bot->GetPositionX(), bot->GetPositionY()))
        return false;

    CellCoord const centre = Acore::ComputeCellCoord(bot->GetPositionX(), bot->GetPositionY());
    CellCoord const at = Acore::ComputeCellCoord(candidate->GetPositionX(), candidate->GetPositionY());
    if (!at.IsCoordValid() || std::abs(int32(at.x_coord) - int32(centre.x_coord)) > 1 ||
        std::abs(int32(at.y_coord) - int32(centre.y_coord)) > 1)
        return false;

    Creature* seen = nullptr;
    auto isCandidate = [candidate](Creature* creature) { return creature == candidate; };
    Acore::CreatureSearcher<decltype(isCandidate)> searcher(bot, seen, isCandidate);
    TypeContainerVisitor<decltype(searcher), GridTypeMapContainer> visitor(searcher);
    map->Visit(Cell(at), visitor);
    return seen != nullptr;
}

bool EntryEngaged(Player* bot, Map* map, ObjectGuid const& slot, uint32 entry)
{
    auto const isEntry = [entry](uint32 candidate) { return candidate == entry; };
    return SweepFinds(bot, map, slot, isEntry) || FirstEngaged(bot, isEntry) != nullptr;
}

// Icehowl, Jaraxxus and the champions count only inside the 200 yd sweep, so their guids (the
// script's slot, the last champion found) only spare the sweep, never stand in for it.
bool EncounterEngaged(Player* bot, Map* map, InstanceScript* instance, ToCEncounter encounter,
                      ToCGateState& state)
{
    switch (encounter)
    {
        case ToCEncounter::NorthrendBeasts:
            return GuidEngaged(map, instance, TOC_DATA_GORMOK) || GuidEngaged(map, instance, TOC_DATA_DREADSCALE) ||
                   GuidEngaged(map, instance, TOC_DATA_ACIDMAW) ||
                   EntryEngaged(bot, map, instance->GetGuidData(TOC_DATA_ICEHOWL),
                                static_cast<uint32>(ToCNpcs::NPC_ICEHOWL));
        case ToCEncounter::Jaraxxus:
            return EntryEngaged(bot, map, instance->GetGuidData(TOC_DATA_JARAXXUS),
                                static_cast<uint32>(ToCNpcs::NPC_JARAXXUS));
        case ToCEncounter::FactionChampions:
        {
            auto const isChampion = [](uint32 entry) { return CHAMPION_ENTRY_SET.Contains(entry); };
            if (SweepFinds(bot, map, state.champion, isChampion))
                return true;

            Creature* found = FirstEngaged(bot, isChampion);
            state.champion = found ? found->GetGUID() : ObjectGuid::Empty;
            return found != nullptr;
        }
        case ToCEncounter::TwinValkyr:
            return GuidEngaged(map, instance, TOC_DATA_FJOLA) || GuidEngaged(map, instance, TOC_DATA_EYDIS);
        case ToCEncounter::Anubarak:
            return GuidEngaged(map, instance, TOC_DATA_ANUBARAK);
        default:
            return false;
    }
}

// Null off a ToC instance.
ToCGateState const* ReadGate(PlayerbotAI* botAI)
{
    Player* bot = botAI ? botAI->GetBot() : nullptr;
    Map* map = bot ? bot->FindMap() : nullptr;
    if (!map || map->GetId() != TrialOfTheCrusaderHelpers::TRIAL_OF_THE_CRUSADER_MAP_ID || !bot->GetInstanceId())
        return nullptr;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return nullptr;

    ToCGateState& state = gateStates.For(bot->GetInstanceId());
    uint32 const now = getMSTime();
    if (state.memoValid && state.memoMs == now)
        return &state;

    state.memoMs = now;
    state.memoValid = true;
    state.stage = instance->GetData(TOC_DATA_INSTANCE_PROGRESS);

    ToCEncounter const encounter = ToCEncounterOfStage(state.stage);
    bool const live = encounter != ToCEncounter::None && instance->IsEncounterInProgress() &&
                      EncounterEngaged(bot, map, instance, encounter, state);
    state.live = live ? encounter : ToCEncounter::None;

    return &state;
}
}  // namespace

ToCEncounter ToCEncounterOfStage(uint32 stage)
{
    switch (stage)
    {
        case TOC_PROGRESS_INITIAL:
        case TOC_PROGRESS_INTRO_DONE:
            return ToCEncounter::NorthrendBeasts;
        case TOC_PROGRESS_BEASTS_DEAD:
        case TOC_PROGRESS_JARAXXUS_INTRO_DONE:
            return ToCEncounter::Jaraxxus;
        case TOC_PROGRESS_JARAXXUS_DEAD:
            return ToCEncounter::FactionChampions;
        case TOC_PROGRESS_FACTION_CHAMPIONS_DEAD:
            return ToCEncounter::TwinValkyr;
        case TOC_PROGRESS_ANUB_ARAK:
            return ToCEncounter::Anubarak;
        default:
            return ToCEncounter::None;
    }
}

bool ToCEncounterGateOpen(PlayerbotAI* botAI, ToCEncounter encounter)
{
    ToCGateState const* state = ReadGate(botAI);
    if (!state || ToCEncounterOfStage(state->stage) == encounter)
        return true;

    return state->live == ToCEncounter::None && EncounterBeforeStage(state->stage) == encounter;
}

bool ToCEncounterIsLive(PlayerbotAI* botAI, ToCEncounter encounter)
{
    return encounter != ToCEncounter::None && ToCLiveEncounter(botAI) == encounter;
}

ToCEncounter ToCLiveEncounter(PlayerbotAI* botAI)
{
    ToCGateState const* state = ReadGate(botAI);
    return state ? state->live : ToCEncounter::None;
}

bool ToCGateOpen(PlayerbotAI* botAI, uint32 encounterId)
{
    return ToCEncounterGateOpen(botAI, static_cast<ToCEncounter>(encounterId));
}

bool ToCRaidWideGateOpen(PlayerbotAI* /*botAI*/, uint32 /*encounterId*/) { return true; }

bool ToCEncounterOfTrigger(std::string const& triggerName, ToCEncounter& encounter)
{
    for (EncounterPrefix const& entry : ENCOUNTER_PREFIXES)
    {
        if (triggerName.rfind(entry.prefix, 0) == 0)
        {
            encounter = entry.encounter;
            return true;
        }
    }

    return false;
}

// Interval copied off the inner trigger: the engine calls needCheck on the wrapper, and the default
// of 1 would run a throttled trigger every tick.
ToCGatedTrigger::ToCGatedTrigger(PlayerbotAI* botAI, Trigger* inner, ToCEncounter encounter)
    : Trigger(botAI, inner ? inner->getName() : "trigger", inner ? inner->getCheckInterval() : 1),
      _inner(inner),
      _encounter(encounter)
{
}

ToCGatedTrigger::~ToCGatedTrigger() { delete _inner; }

Event ToCGatedTrigger::Check()
{
    if (!_inner || !ToCEncounterGateOpen(botAI, _encounter))
        return Event();

    return _inner->Check();
}

bool ToCGatedTrigger::IsActive() { return _inner && ToCEncounterGateOpen(botAI, _encounter) && _inner->IsActive(); }

bool ToCGatedTrigger::IsBuffTrigger() { return _inner && _inner->IsBuffTrigger(); }

bool ToCGatedTrigger::IsDebuffTrigger() { return _inner && _inner->IsDebuffTrigger(); }

std::vector<NextAction> ToCGatedTrigger::getHandlers()
{
    return _inner ? _inner->getHandlers() : std::vector<NextAction>();
}

void ToCGatedTrigger::Reset()
{
    if (_inner)
        _inner->Reset();
}

Unit* ToCGatedTrigger::GetTarget() { return _inner ? _inner->GetTarget() : Trigger::GetTarget(); }

Value<Unit*>* ToCGatedTrigger::GetTargetValue()
{
    return _inner ? _inner->GetTargetValue() : Trigger::GetTargetValue();
}

std::string const ToCGatedTrigger::GetTargetName()
{
    return _inner ? _inner->GetTargetName() : Trigger::GetTargetName();
}

void ToCGatedTrigger::ExternalEvent(std::string const param, Player* owner)
{
    if (_inner)
        _inner->ExternalEvent(param, owner);
}

void ToCGatedTrigger::ExternalEvent(WorldPacket& packet, Player* owner)
{
    if (_inner)
        _inner->ExternalEvent(packet, owner);
}
