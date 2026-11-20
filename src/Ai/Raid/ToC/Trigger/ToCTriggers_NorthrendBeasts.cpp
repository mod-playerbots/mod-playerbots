/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCTriggers_NorthrendBeasts.h"

#include "Playerbots.h"
#include "RaidDirectTargeting.h"
#include "ToCData.h"
#include "ToCHelpers_NorthrendBeasts.h"

using namespace TrialOfTheCrusaderHelpers;

namespace
{
bool BelongsToStem(Unit* unit, NorthrendBeastsStem stem)
{
    ToCNpcs const entry = static_cast<ToCNpcs>(unit->GetEntry());
    switch (stem)
    {
        case NorthrendBeastsStem::Gormok:
            return entry == ToCNpcs::NPC_GORMOK || entry == ToCNpcs::NPC_SNOBOLD_VASSAL;
        case NorthrendBeastsStem::Worms:
            return entry == ToCNpcs::NPC_ACIDMAW || entry == ToCNpcs::NPC_DREADSCALE;
        case NorthrendBeastsStem::Icehowl:
            return entry == ToCNpcs::NPC_ICEHOWL;
    }

    return false;
}
}  // namespace

bool NorthrendBeastsDpsTargetTrigger::IsActive()
{
    Unit* pick = GetBeastsDpsTarget(botAI);
    return pick && BelongsToStem(pick, _stem) &&
           (AI_VALUE(Unit*, "current target") != pick || RaidPetOffPick(botAI, pick));
}
