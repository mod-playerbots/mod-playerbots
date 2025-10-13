#ifndef _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H

#include "RaidMagtheridonTriggers.h"
#include "AiObjectContext.h"

class RaidMagtheridonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMagtheridonTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["magtheridon hellfire channeler main tank"] = &RaidMagtheridonTriggerContext::magtheridon_hellfire_channeler_main_tank;
        creators["magtheridon hellfire channeler nw channeler tank"] = &RaidMagtheridonTriggerContext::magtheridon_hellfire_channeler_nw_channeler_tank;
        creators["magtheridon hellfire channeler ne channeler tank"] = &RaidMagtheridonTriggerContext::magtheridon_hellfire_channeler_ne_channeler_tank;
        creators["magtheridon hellfire channeler misdirection"] = &RaidMagtheridonTriggerContext::magtheridon_hellfire_channeler_misdirection;
        creators["magtheridon hellfire channeler dps priority"] = &RaidMagtheridonTriggerContext::magtheridon_hellfire_channeler_dps_priority;
        creators["magtheridon burning abyssal warlock cc"] = &RaidMagtheridonTriggerContext::magtheridon_burning_abyssal_warlock_cc;
        creators["magtheridon position boss"] = &RaidMagtheridonTriggerContext::magtheridon_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagtheridonTriggerContext::magtheridon_spread_ranged;
        creators["magtheridon use manticron cube"] = &RaidMagtheridonTriggerContext::magtheridon_use_manticron_cube;
        creators["magtheridon reset timers and assignments"] = &RaidMagtheridonTriggerContext::magtheridon_reset_timers_and_assignments;
    }

private:
    static Trigger* magtheridon_hellfire_channeler_main_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMainTankTrigger(botAI); }
    static Trigger* magtheridon_hellfire_channeler_nw_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNWChannelerTankTrigger(botAI); }
    static Trigger* magtheridon_hellfire_channeler_ne_channeler_tank(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerNEChannelerTankTrigger(botAI); }
    static Trigger* magtheridon_hellfire_channeler_misdirection(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerMisdirectionTrigger(botAI); }
    static Trigger* magtheridon_hellfire_channeler_dps_priority(PlayerbotAI* botAI) { return new MagtheridonHellfireChannelerDPSPriorityTrigger(botAI); }
    static Trigger* magtheridon_burning_abyssal_warlock_cc(PlayerbotAI* botAI) { return new MagtheridonBurningAbyssalWarlockCCTrigger(botAI); }
    static Trigger* magtheridon_position_boss(PlayerbotAI* botAI) { return new MagtheridonPositionBossTrigger(botAI); }
    static Trigger* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedTrigger(botAI); }
    static Trigger* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeTrigger(botAI); }
    static Trigger* magtheridon_reset_timers_and_assignments(PlayerbotAI* botAI) { return new MagtheridonResetTimersAndAssignmentsTrigger(botAI); }
};

#endif
