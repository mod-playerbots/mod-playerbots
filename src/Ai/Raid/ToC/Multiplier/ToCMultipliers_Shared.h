/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCMULTIPLIERS_SHARED_H
#define PLAYERBOTS_RAID_TOCMULTIPLIERS_SHARED_H

#include "RaidBurstWindow.h"

class PlayerbotAI;

// The live encounter's burst row, answered by its stem's ToC<Stem>BurstWindow. Default between pulls.
RaidBurstWindow ToCLiveBurstWindow(PlayerbotAI* botAI);

#endif
