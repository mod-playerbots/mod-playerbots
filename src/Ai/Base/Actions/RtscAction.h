/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RTSCACTION_H
#define PLAYERBOTS_RTSCACTION_H

#include "SeeSpellAction.h"

class PlayerbotAI;

#define RTSC_MOVE_SPELL 30758  // Aedm (Awesome Energetic do move)

class RTSCAction : public SeeSpellAction
{
public:
    RTSCAction(PlayerbotAI* botAI) : SeeSpellAction(botAI, "rtsc") {}

    bool Execute(Event event) override;
};

#endif
