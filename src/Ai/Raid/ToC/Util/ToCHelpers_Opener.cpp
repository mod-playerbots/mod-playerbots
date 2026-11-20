/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCHelpers_Opener.h"

#include "ToCEncounterGate.h"

namespace TrialOfTheCrusaderHelpers
{

std::vector<uint32> const& ToCOpenerBosses(PlayerbotAI* botAI)
{
    static std::vector<uint32> const beasts = {
        static_cast<uint32>(ToCNpcs::NPC_GORMOK), static_cast<uint32>(ToCNpcs::NPC_ACIDMAW),
        static_cast<uint32>(ToCNpcs::NPC_DREADSCALE), static_cast<uint32>(ToCNpcs::NPC_ICEHOWL)};
    static std::vector<uint32> const jaraxxus = {static_cast<uint32>(ToCNpcs::NPC_JARAXXUS)};
    static std::vector<uint32> const twins = {static_cast<uint32>(ToCNpcs::NPC_FJOLA_LIGHTBANE),
                                              static_cast<uint32>(ToCNpcs::NPC_EYDIS_DARKBANE)};
    static std::vector<uint32> const anubarak = {static_cast<uint32>(ToCNpcs::NPC_ANUBARAK)};
    static std::vector<uint32> const none;

    switch (ToCLiveEncounter(botAI))
    {
        case ToCEncounter::NorthrendBeasts:
            return beasts;
        case ToCEncounter::Jaraxxus:
            return jaraxxus;
        case ToCEncounter::TwinValkyr:
            return twins;
        case ToCEncounter::Anubarak:
            return anubarak;
        default:
            return none;
    }
}

}
