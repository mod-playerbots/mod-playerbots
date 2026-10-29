/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZGActions.h"
#include "Playerbots.h"

bool BloodlordMandokirStopCausingThreatAction::Execute(Event /*event*/)
{
    // Auto-attack, Auto Shot and a cast under way; the multiplier blocks new ones.
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(false);
    return true;
}
