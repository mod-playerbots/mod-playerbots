/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARAVALUECONTEXT_H
#define PLAYERBOTS_KARAVALUECONTEXT_H

#include "EncounterHelpers.h"
#include "KaraHelpers.h"
#include "NamedObjectContext.h"
#include "Value.h"
#include <vector>

class ShadeOfAranFlameWreathRingsValue : public CalculatedValue<std::vector<Position>>
{
public:
    ShadeOfAranFlameWreathRingsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "shade of aran flame wreath rings",
              KaraHelpers::ARAN_FLAME_WREATH_RING_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(
            bot, KaraHelpers::ARAN_ROOM_SEARCH_RADIUS,
            KaraHelpers::Id(KaraHelpers::KaraSpells::SPELL_FLAME_WREATH_AURA));
    }
};

class NightbaneCharredEarthsValue : public CalculatedValue<std::vector<Position>>
{
public:
    NightbaneCharredEarthsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<Position>>(
              botAI, "nightbane charred earths", KaraHelpers::CHARRED_EARTH_CACHE_INTERVAL_MS) {}

protected:
    std::vector<Position> Calculate() override
    {
        return EncounterHelpers::GetDynamicObjectPositions(
            bot, KaraHelpers::CHARRED_EARTH_SEARCH_RADIUS,
            KaraHelpers::Id(KaraHelpers::KaraSpells::SPELL_CHARRED_EARTH));
    }
};

class RaidKarazhanValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidKarazhanValueContext()
    {
        creators["shade of aran flame wreath rings"] =
            &RaidKarazhanValueContext::shade_of_aran_flame_wreath_rings;

        creators["nightbane charred earths"] = &RaidKarazhanValueContext::nightbane_charred_earths;
    }

private:
    static UntypedValue* shade_of_aran_flame_wreath_rings(PlayerbotAI* botAI)
    {
        return new ShadeOfAranFlameWreathRingsValue(botAI);
    }
    static UntypedValue* nightbane_charred_earths(PlayerbotAI* botAI)
    {
        return new NightbaneCharredEarthsValue(botAI);
    }
};

#endif
