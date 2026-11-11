/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40VALUECONTEXT_H
#define PLAYERBOTS_AQ40VALUECONTEXT_H

#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "Value.h"
#include <vector>

class Aq40BotCacheValue : public ManualSetValue<Aq40Helpers::Aq40BotCache&>
{
public:
    Aq40BotCacheValue(PlayerbotAI* botAI) : ManualSetValue<Aq40Helpers::Aq40BotCache&>(botAI, _data, "aq40 bot cache")
    {
    }

private:
    Aq40Helpers::Aq40BotCache _data;
};

class Aq40AssignedTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    Aq40AssignedTargetValue(PlayerbotAI* botAI)
        : CalculatedValue<ObjectGuid>(botAI, "aq40 assigned target", Aq40Helpers::BOT_CACHE_INTERVAL_MS)
    {
    }

protected:
    ObjectGuid Calculate() override
    {
        Unit* target = Aq40Helpers::ComputeAssignedTarget(botAI);
        return target ? target->GetGUID() : ObjectGuid::Empty;
    }
};

class Aq40HazardValue : public CalculatedValue<Aq40Helpers::Aq40Goal>
{
public:
    Aq40HazardValue(PlayerbotAI* botAI)
        : CalculatedValue<Aq40Helpers::Aq40Goal>(botAI, "aq40 hazard", Aq40Helpers::BOT_CACHE_INTERVAL_MS)
    {
    }

protected:
    Aq40Helpers::Aq40Goal Calculate() override
    {
        Aq40Helpers::Aq40Goal result;
        result.active = Aq40Helpers::ComputeHazardGoal(botAI, result.goal);
        return result;
    }
};

class Aq40FormationValue : public CalculatedValue<Aq40Helpers::Aq40Goal>
{
public:
    Aq40FormationValue(PlayerbotAI* botAI)
        : CalculatedValue<Aq40Helpers::Aq40Goal>(botAI, "aq40 formation", Aq40Helpers::BOT_CACHE_INTERVAL_MS)
    {
    }

protected:
    Aq40Helpers::Aq40Goal Calculate() override
    {
        Aq40Helpers::Aq40Goal result;
        result.active = Aq40Helpers::ComputeFormationGoal(botAI, result.goal);
        return result;
    }
};

class Aq40BlizzardPositionsValue : public CalculatedValue<std::vector<Position>>
{
public:
    Aq40BlizzardPositionsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(botAI, "aq40 blizzard positions", Aq40Helpers::TRACKER_INTERVAL_MS)
    {
    }

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(bot, Aq40Helpers::BLIZZARD_SEARCH_RADIUS,
                                                           Aq40Helpers::Id(Aq40Helpers::Aq40Spells::SPELL_BLIZZARD));
    }
};

class RaidAq40ValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidAq40ValueContext()
    {
        creators["aq40 bot cache"] = &RaidAq40ValueContext::aq40_bot_cache;
        creators["aq40 assigned target"] = &RaidAq40ValueContext::aq40_assigned_target;
        creators["aq40 hazard"] = &RaidAq40ValueContext::aq40_hazard;
        creators["aq40 formation"] = &RaidAq40ValueContext::aq40_formation;
        creators["aq40 blizzard positions"] = &RaidAq40ValueContext::aq40_blizzard_positions;
    }

private:
    static UntypedValue* aq40_bot_cache(PlayerbotAI* botAI) { return new Aq40BotCacheValue(botAI); }
    static UntypedValue* aq40_assigned_target(PlayerbotAI* botAI) { return new Aq40AssignedTargetValue(botAI); }
    static UntypedValue* aq40_hazard(PlayerbotAI* botAI) { return new Aq40HazardValue(botAI); }
    static UntypedValue* aq40_formation(PlayerbotAI* botAI) { return new Aq40FormationValue(botAI); }
    static UntypedValue* aq40_blizzard_positions(PlayerbotAI* botAI) { return new Aq40BlizzardPositionsValue(botAI); }
};

#endif
