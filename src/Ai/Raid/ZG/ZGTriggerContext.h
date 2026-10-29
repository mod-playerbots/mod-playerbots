/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ZGTRIGGERCONTEXT_H
#define PLAYERBOTS_ZGTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "ZGTriggers.h"

class RaidZulGurubTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidZulGurubTriggerContext()
    {
        // Bloodlord Mandokir
        creators["bloodlord mandokir threatening gaze"] =
            &RaidZulGurubTriggerContext::bloodlord_mandokir_threatening_gaze;
    }

private:
    static Trigger* bloodlord_mandokir_threatening_gaze(PlayerbotAI* botAI)
    {
        return new BloodlordMandokirThreateningGazeTrigger(botAI);
    }
};

#endif
