/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCMultipliers_Shared.h"
#include "ToCEncounterGate.h"
#include "ToCMultipliers_Anubarak.h"
#include "ToCMultipliers_FactionChampions.h"
#include "ToCMultipliers_Jaraxxus.h"
#include "ToCMultipliers_NorthrendBeasts.h"
#include "ToCMultipliers_TwinValkyr.h"

RaidBurstWindow ToCLiveBurstWindow(PlayerbotAI* botAI)
{
    switch (ToCLiveEncounter(botAI))
    {
        case ToCEncounter::NorthrendBeasts:
            return ToCNorthrendBeastsBurstWindow(botAI);
        case ToCEncounter::Jaraxxus:
            return ToCJaraxxusBurstWindow(botAI);
        case ToCEncounter::FactionChampions:
            return ToCFactionChampionsBurstWindow(botAI);
        case ToCEncounter::TwinValkyr:
            return ToCTwinValkyrBurstWindow(botAI);
        case ToCEncounter::Anubarak:
            return ToCAnubarakBurstWindow(botAI);
        case ToCEncounter::None:
            break;
    }

    return {};
}
