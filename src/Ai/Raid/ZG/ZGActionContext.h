/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZGACTIONCONTEXT_H
#define PLAYERBOTS_ZGACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "ZGActions.h"

class RaidZulGurubActionContext : public NamedObjectContext<Action>
{
public:
    RaidZulGurubActionContext()
    {
        // Bloodlord Mandokir
        creators["bloodlord mandokir stop causing threat"] =
            &RaidZulGurubActionContext::bloodlord_mandokir_stop_causing_threat;
    }

private:
    static Action* bloodlord_mandokir_stop_causing_threat(PlayerbotAI* botAI)
    {
        return new BloodlordMandokirStopCausingThreatAction(botAI);
    }
};

#endif
