/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_ResistAura.h"

#include "ToCEncounterGate.h"
#include "ToCHelpers_Anubarak.h"
#include "ToCHelpers_NorthrendBeasts.h"
#include "ToCHelpers_Staging.h"

namespace TrialOfTheCrusaderHelpers
{
namespace
{
uint8 SchoolsOfArrival(ToCStagingBoss arrival)
{
    switch (arrival)
    {
        case ToCStagingBoss::Gormok:
            return RAID_RESIST_FIRE;
        case ToCStagingBoss::Worms:
            return RAID_RESIST_FIRE | RAID_RESIST_NATURE;
        case ToCStagingBoss::Icehowl:
            return RAID_RESIST_FROST;
        case ToCStagingBoss::Jaraxxus:
            // Fel Lightning is fire + nature and resists by the lower one
            return RAID_RESIST_FIRE | RAID_RESIST_NATURE;
        case ToCStagingBoss::Twins:
            return RAID_RESIST_FIRE | RAID_RESIST_SHADOW;
        case ToCStagingBoss::Anubarak:
            return RAID_RESIST_FROST;  // nature joins from phase 2: scarab Mandibles, then Leeching Swarm
        case ToCStagingBoss::Champions:
            return RAID_RESIST_FIRE;  // mostly physical, but the hunter, warlock and enhancement shaman burn
        default:
            return 0;
    }
}

// A heroic overlap has two beasts up at once, so their sets add up
uint8 SchoolsOfBeastsStage(uint32 mask)
{
    uint8 schools = 0;
    if (mask & BEASTS_STAGE_GORMOK)
        schools |= SchoolsOfArrival(ToCStagingBoss::Gormok);
    if (mask & BEASTS_STAGE_WORMS)
        schools |= SchoolsOfArrival(ToCStagingBoss::Worms);
    if (mask & BEASTS_STAGE_ICEHOWL)
        schools |= SchoolsOfArrival(ToCStagingBoss::Icehowl);

    return schools;
}
}  // namespace

bool ToCResistFireWantsMastery(PlayerbotAI* botAI)
{
    ToCEncounter const live = ToCLiveEncounter(botAI);
    if (live != ToCEncounter::None)
        return live == ToCEncounter::Jaraxxus || live == ToCEncounter::TwinValkyr;

    ToCStagingBoss const staged = GetToCStagedArrival(botAI);
    return staged == ToCStagingBoss::Jaraxxus || staged == ToCStagingBoss::Twins;
}

bool ToCResistFrostWantsMastery(PlayerbotAI* botAI)
{
    return ToCLiveEncounter(botAI) == ToCEncounter::Anubarak && AnubarakSwarmHealing(botAI);
}

uint8 ToCResistWantedSchools(PlayerbotAI* botAI, uint8 previous)
{
    switch (ToCLiveEncounter(botAI))
    {
        case ToCEncounter::NorthrendBeasts:
        {
            // The gate finds Icehowl by its own search, so for a moment the stage mask can trail it.
            // An empty set here would drop every holder for that tick.
            uint8 const schools = SchoolsOfBeastsStage(GetBeastsStageMask(botAI));
            return schools ? schools : previous;
        }
        case ToCEncounter::Jaraxxus:
            return SchoolsOfArrival(ToCStagingBoss::Jaraxxus);
        case ToCEncounter::FactionChampions:
            return SchoolsOfArrival(ToCStagingBoss::Champions);
        case ToCEncounter::TwinValkyr:
            return SchoolsOfArrival(ToCStagingBoss::Twins);
        case ToCEncounter::Anubarak:
        {
            AnubarakPhase const phase = GetAnubarakPhase(botAI);
            bool const nature = phase == AnubarakPhase::Submerged || phase == AnubarakPhase::Swarm;
            return static_cast<uint8>(SchoolsOfArrival(ToCStagingBoss::Anubarak) | (nature ? RAID_RESIST_NATURE : 0));
        }
        default:
            return SchoolsOfArrival(GetToCStagedArrival(botAI));
    }
}

bool IsToCResistPaladinHolding(PlayerbotAI* botAI) { return IsRaidResistPaladinHolding(botAI, TOC_RESIST_AURA); }

bool IsToCResistHunterHolding(PlayerbotAI* botAI) { return IsRaidResistHunterHolding(botAI, TOC_RESIST_AURA); }

}
