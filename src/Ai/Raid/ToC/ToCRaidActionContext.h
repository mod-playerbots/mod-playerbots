/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCRAIDACTIONCONTEXT_H
#define PLAYERBOTS_RAID_TOCRAIDACTIONCONTEXT_H

#include "Log.h"
#include "NamedObjectContext.h"
#include "ToCRaidActions.h"

// Boss stems register their actions in their own contexts, this one only merges them.
class RaidTrialOfTheCrusaderActionContext : public NamedObjectContext<Action>
{
public:
    RaidTrialOfTheCrusaderActionContext()
    {
        Absorb(ToCGormokActionContext());
        Absorb(ToCJormungarsActionContext());
        Absorb(ToCIcehowlActionContext());
        Absorb(ToCNorthrendBeastsActionContext());
        Absorb(ToCJaraxxusActionContext());
        Absorb(ToCAnubarakActionContext());
        Absorb(ToCFactionChampionsActionContext());
        Absorb(ToCFactionChampionsDefenceActionContext());
        Absorb(ToCFactionChampionsBallActionContext());
        Absorb(ToCTwinValkyrActionContext());
        Absorb(ToCStagingActionContext());
        Absorb(ToCResistAuraActionContext());
        Absorb(ToCOpenerActionContext());
    }

private:
    void Absorb(NamedObjectContext<Action> const& stem)
    {
        for (auto const& [name, creator] : stem.creators)
            if (!creators.emplace(name, creator).second)
                LOG_ERROR("playerbots", "ToC action context: duplicate creator '{}'", name);
    }
};

#endif
