/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TKVALUECONTEXT_H
#define PLAYERBOTS_TKVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "TKHelpers.h"
#include "Value.h"

class TKDeadLegendaryWeaponsValue : public CalculatedValue<GuidVector>
{
public:
    TKDeadLegendaryWeaponsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(botAI, "tk dead legendary weapons", 200) {}

protected:
    GuidVector Calculate() override { return TkHelpers::FindDeadLegendaryWeaponGuids(bot); }
};

class RaidTempestKeepValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidTempestKeepValueContext()
    {
        creators["tk dead legendary weapons"] =
            &RaidTempestKeepValueContext::tk_dead_legendary_weapons;
    }

private:
    static UntypedValue* tk_dead_legendary_weapons(PlayerbotAI* botAI) {
        return new TKDeadLegendaryWeaponsValue(botAI);
    }
};

#endif
