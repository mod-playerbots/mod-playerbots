/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_RESISTAURA_H
#define PLAYERBOTS_RAID_TOCACTIONS_RESISTAURA_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidResistAura.h"
#include "ToCHelpers_ResistAura.h"

class ToCResistAuraActionContext : public NamedObjectContext<Action>
{
public:
    ToCResistAuraActionContext()
    {
        creators["toc raise resistance aura"] = &ToCResistAuraActionContext::toc_raise_resistance_aura;
    }

private:
    static Action* toc_raise_resistance_aura(PlayerbotAI* botAI)
    {
        return new RaidRaiseResistAuraAction(botAI, "toc raise resistance aura",
                                             TrialOfTheCrusaderHelpers::TOC_RESIST_AURA);
    }
};

#endif
