/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCMULTIPLIERS_H
#define PLAYERBOTS_RAIDTOCMULTIPLIERS_H

#include "Multiplier.h"

class PlayerbotAI;

// Trial of the Crusader, Anub'arak: Starfall must not be used before phase 3.
// Its ground AoE would shred the frost spheres and the swarm adds the tactic
// depends on, so it is blocked until the boss drops below 30% health.
class TocStarfallMultiplier : public Multiplier
{
public:
    TocStarfallMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "toc starfall multiplier") {}
    float GetValue(Action* action) override;
};

#endif
