/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

/*
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   David Parra Ausina (davidonete/Flekz) <davidparraausina@gmail.com> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   Cyberium <cyberium@users.noreply.github.com>
 */

#ifndef PLAYERBOTS_WAITFORATTACKACTION_H
#define PLAYERBOTS_WAITFORATTACKACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class WaitForAttackKeepSafeDistanceAction : public MovementAction
{
public:
    WaitForAttackKeepSafeDistanceAction(PlayerbotAI* botAI)
        : MovementAction(botAI, "wait for attack keep safe distance") {}

    bool Execute(Event event) override;
};

class SetWaitForAttackTimeAction : public Action
{
public:
    SetWaitForAttackTimeAction(PlayerbotAI* botAI)
        : Action(botAI, "wait for attack time") {}

    bool Execute(Event event) override;
};

#endif
