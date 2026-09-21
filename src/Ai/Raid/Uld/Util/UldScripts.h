/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDSCRIPTS_H
#define PLAYERBOTS_ULDSCRIPTS_H

// NPC entries ported from ulduar.h in AC. ulduar.h is in ACs /scripts directory,
// so the entries are redefined here to avoid including ulduar.h as a relative path.
enum UlduarNPCs
{
    NPC_KOLOGARN                     = 32930,
    NPC_SALVAGED_SIEGE_ENGINE        = 33060,
    NPC_SALVAGED_SIEGE_ENGINE_TURRET = 33067,
    NPC_VEHICLE_CHOPPER              = 33062,
    NPC_SALVAGED_DEMOLISHER          = 33109,
    NPC_SALVAGED_DEMOLISHER_TURRET   = 33167
};

#endif
