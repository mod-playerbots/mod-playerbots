/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SWPINSTANCESTATE_H
#define PLAYERBOTS_SWPINSTANCESTATE_H

#include "EncounterHelpers.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include <optional>

namespace SwpHelpers
{

struct SwpInstanceState
{
    std::optional<KalecgosEncounterState> kalecgos;
    std::optional<BrutallusEncounterState> brutallus;
    std::optional<FelmystEncounterState> felmyst;
    std::optional<EredarTwinsEncounterState> eredarTwins;
    std::optional<MuruEncounterState> muru;
    std::optional<KiljaedenEncounterState> kiljaeden;
    // Class roles survive SwpResetInstance (see ClassRoleAssignment).
    EncounterHelpers::ClassRoleAssignment alythessPaladinTank;
    // Not a class role (any class can fly), but chosen, kept, and announced the same way.
    EncounterHelpers::ClassRoleAssignment kiljaedenDragonOrbUser;
};

SwpInstanceState& SwpState(uint32 instanceId);
SwpInstanceState* FindSwpState(uint32 instanceId);
// Clears every boss's state but keeps the class roles and the orb user. Returns true if any boss
// state was set.
bool SwpResetInstance(uint32 instanceId);
// Picks every class role player, and Kil'jaeden's orb user, again out of encounters; during one,
// only fills an empty role or replaces a player who died or left.
bool UpdateSwpClassRoles(PlayerbotAI* botAI);

}

#endif
