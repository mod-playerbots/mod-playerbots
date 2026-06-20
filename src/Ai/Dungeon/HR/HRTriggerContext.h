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
};

#endif
