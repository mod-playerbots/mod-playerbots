/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBACTIONCONTEXT_H
#define PLAYERBOTS_UBACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "UBActions.h"

class TbcDungeonUnderbogActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonUnderbogActionContext()
    {
        creators["ub retreat from foul spores"] = &TbcDungeonUnderbogActionContext::ub_retreat_from_foul_spores;
        creators["ub vacate spore cloud"] = &TbcDungeonUnderbogActionContext::ub_vacate_spore_cloud;
    }

private:
    static Action* ub_retreat_from_foul_spores(PlayerbotAI* botAI) { return new UBRetreatFromFoulSporesAction(botAI); }

    static Action* ub_vacate_spore_cloud(PlayerbotAI* botAI) { return new UBVacateSporeCloudAction(botAI); }
};

#endif
