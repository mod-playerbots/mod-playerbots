/*
* This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
* information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
* or (at your option) any later version.
*/

#ifndef _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSTRIGGERCONTEXT_H
#define _PLAYERBOT_TBCDUNGEONHELLFIRERAMPARTSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "TriggerContext.h"
#include "HRTriggers.h"

class TbcDungeonHellfireRampartsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonHellfireRampartsTriggerContext()
    {
        // Watchkeeper Gargolmar
        creators["gargolmar hellfire watchers are active"] =
            &TbcDungeonHellfireRampartsTriggerContext::gargolmar_hellfire_watchers_are_active;

        // Omor the Unscarred
        creators["omor treachery aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_treachery_aura;

        creators["omor ranged spread"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_ranged_spread;

        creators["omor fiendish hound is active"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_fiendish_hound_is_active;

        creators["omor tank has treachery aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_tank_has_treachery_aura;

        // Vazruden
        creators["vazruden tank position boss"] =
            &TbcDungeonHellfireRampartsTriggerContext::vazruden_tank_position_boss;
    }
private:
    // Watchkeeper Gargolmar
    static Trigger* gargolmar_hellfire_watchers_are_active(
        PlayerbotAI* botAI) {return new GargolmarHellfireWatchersAreActiveTrigger(botAI); }

    // Omor the Unscarred
    static Trigger* omor_treachery_aura(
        PlayerbotAI* botAI) {return new OmorTreacheryAuraTrigger(botAI); }

    static Trigger* omor_ranged_spread(
        PlayerbotAI* botAI) {return new OmorRangedSpreadTrigger(botAI); }

    static Trigger* omor_fiendish_hound_is_active(
        PlayerbotAI* botAI) {return new OmorFiendishHoundIsActiveTrigger(botAI); }

    static Trigger* omor_tank_has_treachery_aura(
        PlayerbotAI* botAI) {return new OmorTankHasTreacheryAuraTrigger(botAI); }

    // Vazruden
    static Trigger* vazruden_tank_position_boss(
        PlayerbotAI* botAI) {return new VazrudenTankPositionBossTrigger(botAI); }
};

#endif
