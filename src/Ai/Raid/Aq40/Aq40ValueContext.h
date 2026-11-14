/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40VALUECONTEXT_H
#define PLAYERBOTS_AQ40VALUECONTEXT_H

#include <vector>

#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "Value.h"

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
        creators["aq40 blizzard positions"] = &RaidAq40ValueContext::aq40_blizzard_positions;
    }

private:
    static UntypedValue* aq40_bot_cache(PlayerbotAI* botAI) { return new Aq40BotCacheValue(botAI); }
    static UntypedValue* aq40_assigned_target(PlayerbotAI* botAI) { return new Aq40AssignedTargetValue(botAI); }
    static UntypedValue* aq40_blizzard_positions(PlayerbotAI* botAI) { return new Aq40BlizzardPositionsValue(botAI); }
};

#endif
