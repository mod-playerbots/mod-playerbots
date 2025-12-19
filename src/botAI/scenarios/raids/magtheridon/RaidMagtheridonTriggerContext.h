#ifndef _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDMAGTHERIDONTRIGGERCONTEXT_H

#include "RaidMagtheridonTriggers.h"
#include "AiObjectContext.h"

class RaidMagtheridonTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidMagtheridonTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["magtheridon first three channelers engaged by main tank"] = &RaidMagtheridonTriggerContext::magtheridon_first_three_channelers_engaged_by_main_tank;
        creators["magtheridon nw channeler engaged by first assist tank"] = &RaidMagtheridonTriggerContext::magtheridon_nw_channeler_engaged_by_first_assist_tank;
        creators["magtheridon ne channeler engaged by second assist tank"] = &RaidMagtheridonTriggerContext::magtheridon_ne_channeler_engaged_by_second_assist_tank;
        creators["magtheridon pulling west and east channelers"] = &RaidMagtheridonTriggerContext::magtheridon_pull_west_and_east_channelers;
        creators["magtheridon determining kill order"] = &RaidMagtheridonTriggerContext::magtheridon_determining_kill_order;
        creators["magtheridon burning abyssal spawned"] = &RaidMagtheridonTriggerContext::magtheridon_burning_abyssal_spawned;
        creators["magtheridon boss engaged by main tank"] = &RaidMagtheridonTriggerContext::magtheridon_boss_engaged_by_main_tank;
        creators["magtheridon boss engaged by ranged"] = &RaidMagtheridonTriggerContext::magtheridon_boss_engaged_by_ranged;
        creators["magtheridon incoming blast nova"] = &RaidMagtheridonTriggerContext::magtheridon_incoming_blast_nova;
        creators["magtheridon need to manage timers and assignments"] = &RaidMagtheridonTriggerContext::magtheridon_need_to_manage_timers_and_assignments;
    }

private:
    static Trigger* magtheridon_first_three_channelers_engaged_by_main_tank(PlayerbotAI* botAI) { return new MagtheridonFirstThreeChannelersEngagedByMainTankTrigger(botAI); }
    static Trigger* magtheridon_nw_channeler_engaged_by_first_assist_tank(PlayerbotAI* botAI) { return new MagtheridonNWChannelerEngagedByFirstAssistTankTrigger(botAI); }
    static Trigger* magtheridon_ne_channeler_engaged_by_second_assist_tank(PlayerbotAI* botAI) { return new MagtheridonNEChannelerEngagedBySecondAssistTankTrigger(botAI); }
    static Trigger* magtheridon_pull_west_and_east_channelers(PlayerbotAI* botAI) { return new MagtheridonPullingWestAndEastChannelersTrigger(botAI); }
    static Trigger* magtheridon_determining_kill_order(PlayerbotAI* botAI) { return new MagtheridonDeterminingKillOrderTrigger(botAI); }
    static Trigger* magtheridon_burning_abyssal_spawned(PlayerbotAI* botAI) { return new MagtheridonBurningAbyssalSpawnedTrigger(botAI); }
    static Trigger* magtheridon_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new MagtheridonBossEngagedByMainTankTrigger(botAI); }
    static Trigger* magtheridon_boss_engaged_by_ranged(PlayerbotAI* botAI) { return new MagtheridonBossEngagedByRangedTrigger(botAI); }
    static Trigger* magtheridon_incoming_blast_nova(PlayerbotAI* botAI) { return new MagtheridonIncomingBlastNovaTrigger(botAI); }
    static Trigger* magtheridon_need_to_manage_timers_and_assignments(PlayerbotAI* botAI) { return new MagtheridonNeedToManageTimersAndAssignmentsTrigger(botAI); }
};

#endif
