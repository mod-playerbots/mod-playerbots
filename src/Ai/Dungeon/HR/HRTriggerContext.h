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
        creators["gargolmar tank position boss"] =
            &TbcDungeonHellfireRampartsTriggerContext::gargolmar_tank_position_boss;

        creators["gargolmar hellfire watchers are active"] =
            &TbcDungeonHellfireRampartsTriggerContext::gargolmar_hellfire_watchers_are_active;

        // Omor the Unscarred
        creators["omor treacherous aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_treacherous_aura;

        creators["omor bane of treachery aura"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_bane_of_treachery_aura;

        creators["omor ranged spread"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_ranged_spread;

        creators["omor fiendish hound is active"] =
            &TbcDungeonHellfireRampartsTriggerContext::omor_fiendish_hound_is_active;

        // Vazruden
        creators["vazruden tank position boss"] =
            &TbcDungeonHellfireRampartsTriggerContext::vazruden_tank_position_boss;
    }
private:
    // Watchkeeper Gargolmar
    static Trigger* gargolmar_tank_position_boss(
        PlayerbotAI* botAI) {return new GargolmarTankPositionBossTrigger(botAI); }

    static Trigger* gargolmar_hellfire_watchers_are_active(
        PlayerbotAI* botAI) {return new GargolmarHellfireWatchersAreActiveTrigger(botAI); }

    // Omor the Unscarred
    static Trigger* omor_treacherous_aura(
        PlayerbotAI* botAI) {return new OmorTreacherousAuraTrigger(botAI); }

    static Trigger* omor_bane_of_treachery_aura(
        PlayerbotAI* botAI) {return new OmorBaneOfTreacheryAuraTrigger(botAI); }

    static Trigger* omor_ranged_spread(
        PlayerbotAI* botAI) {return new OmorRangedSpreadTrigger(botAI); }

    static Trigger* omor_fiendish_hound_is_active(
        PlayerbotAI* botAI) {return new OmorFiendishHoundIsActiveTrigger(botAI); }

    // Vazruden
    static Trigger* vazruden_tank_position_boss(
        PlayerbotAI* botAI) {return new VazrudenTankPositionBossTrigger(botAI); }
};

#endif
