/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

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
        creators["gargolmar mark hellfire watchers"] =
            &TbcDungeonHellfireRampartsActionContext::gargolmar_mark_hellfire_watchers;

        // Omor the Unscarred
        creators["omor treachery aura flee from players"] =
            &TbcDungeonHellfireRampartsActionContext::omor_treachery_aura_flee_from_players;

        creators["omor ranged spread"] =
            &TbcDungeonHellfireRampartsActionContext::omor_ranged_spread;

        creators["omor mark fiendish hound"] =
            &TbcDungeonHellfireRampartsActionContext::omor_mark_fiendish_hound;

        creators["omor treachery aura flee from tank"] =
            &TbcDungeonHellfireRampartsActionContext::omor_treachery_aura_flee_from_tank;

        // Vazruden
        creators["vazruden tank position boss"] =
            &TbcDungeonHellfireRampartsActionContext::vazruden_tank_position_boss;
    }
private:
    // Watchkeeper Gargolmar
    static Action* gargolmar_mark_hellfire_watchers(
        PlayerbotAI* botAI) { return new GargolmarMarkHellfireWatchersAction(botAI); }

    // Omor the Unscarred
    static Action* omor_treachery_aura_flee_from_players(
        PlayerbotAI* botAI) { return new OmorTreacheryAuraFleeFromPlayersAction(botAI); }

    static Action* omor_ranged_spread(
        PlayerbotAI* botAI) { return new OmorRangedSpreadAction(botAI); }

    static Action* omor_mark_fiendish_hound(
        PlayerbotAI* botAI) { return new OmorMarkFiendishHoundAction(botAI); }

    static Action* omor_treachery_aura_flee_from_tank(
        PlayerbotAI* botAI) { return new OmorTreacheryAuraFleeFromTankAction(botAI); }

    // Vazruden
    static Action* vazruden_tank_position_boss(
        PlayerbotAI* botAI) { return new VazrudenTankPositionBossAction(botAI); }

};

#endif
