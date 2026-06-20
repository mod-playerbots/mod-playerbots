#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSACTIONCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSACTIONCONTEXT_H

#include "AiObjectContext.h"
#include "Action.h"
#include "HRActions.h"

class TbcDungeonHellfireRampartsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonHellfireRampartsActionContext() : NamedObjectContext<Action>(false, true)
    {
        // Watchkeeper Gargolmar
        creators["gargolmar tank position boss"] =
            &TbcDungeonHellfireRampartsActionContext::gargolmar_tank_position_boss;

        creators["gargolmar mark hellfire watchers"] =
            &TbcDungeonHellfireRampartsActionContext::gargolmar_mark_hellfire_watchers;

        // Omor the Unscarred
        creators["omor treacherous aura flee from players"] =
            &TbcDungeonHellfireRampartsActionContext::omor_treacherous_aura_flee_from_players;

        creators["omor bane of treachery aura flee from players"] =
            &TbcDungeonHellfireRampartsActionContext::omor_bane_of_treachery_aura_flee_from_players;
    }
private:
    // Watchkeeper Gargolmar
    static Action* gargolmar_tank_position_boss(
        PlayerbotAI* botAI) { return new GargolmarTankPositionBossAction(botAI); }

    static Action* gargolmar_mark_hellfire_watchers(
        PlayerbotAI* botAI) { return new GargolmarMarkHellfireWatchersAction(botAI); }

    // Omor the Unscarred
    static Action* omor_treacherous_aura_flee_from_players(
        PlayerbotAI* botAI) { return new OmorTreacherousAuraFleeFromPlayersAction(botAI); }

    static Action* omor_bane_of_treachery_aura_flee_from_players(
        PlayerbotAI* botAI) { return new OmorBaneOfTreacheryAuraFleeFromPlayersAction(botAI); }

};

#endif
