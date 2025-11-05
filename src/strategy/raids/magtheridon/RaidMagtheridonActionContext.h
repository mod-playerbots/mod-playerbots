#ifndef _PLAYERBOT_RAIDMAGTHERIDONACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMAGTHERIDONACTIONCONTEXT_H

#include "RaidMagtheridonActions.h"
#include "NamedObjectContext.h"

class RaidMagtheridonActionContext : public NamedObjectContext<Action>
{
public:
    RaidMagtheridonActionContext()
    {
        creators["magtheridon main tank attack first three channelers"] = &RaidMagtheridonActionContext::magtheridon_main_tank_attack_first_three_channelers;
        creators["magtheridon first assist tank attack nw channeler"] = &RaidMagtheridonActionContext::magtheridon_first_assist_tank_attack_nw_channeler;
        creators["magtheridon second assist tank attack ne channeler"] = &RaidMagtheridonActionContext::magtheridon_second_assist_tank_attack_ne_channeler;
        creators["magtheridon misdirect hellfire channelers"] = &RaidMagtheridonActionContext::magtheridon_misdirect_hellfire_channelers;
        creators["magtheridon assign dps priority"] = &RaidMagtheridonActionContext::magtheridon_assign_dps_priority;
        creators["magtheridon warlock cc burning abyssal"] = &RaidMagtheridonActionContext::magtheridon_warlock_cc_burning_abyssal;
        creators["magtheridon main tank position boss"] = &RaidMagtheridonActionContext::magtheridon_main_tank_position_boss;
        creators["magtheridon spread ranged"] = &RaidMagtheridonActionContext::magtheridon_spread_ranged;
        creators["magtheridon use manticron cube"] = &RaidMagtheridonActionContext::magtheridon_use_manticron_cube;
        creators["magtheridon manage timers and assignments"] = &RaidMagtheridonActionContext::magtheridon_manage_timers_and_assignments;
    }

private:
    static Action* magtheridon_main_tank_attack_first_three_channelers(PlayerbotAI* botAI) { return new MagtheridonMainTankAttackFirstThreeChannelersAction(botAI); }
    static Action* magtheridon_first_assist_tank_attack_nw_channeler(PlayerbotAI* botAI) { return new MagtheridonFirstAssistTankAttackNWChannelerAction(botAI); }
    static Action* magtheridon_second_assist_tank_attack_ne_channeler(PlayerbotAI* botAI) { return new MagtheridonSecondAssistTankAttackNEChannelerAction(botAI); }
    static Action* magtheridon_misdirect_hellfire_channelers(PlayerbotAI* botAI) { return new MagtheridonMisdirectHellfireChannelers(botAI); }
    static Action* magtheridon_assign_dps_priority(PlayerbotAI* botAI) { return new MagtheridonAssignDPSPriorityAction(botAI); }
    static Action* magtheridon_warlock_cc_burning_abyssal(PlayerbotAI* botAI) { return new MagtheridonWarlockCCBurningAbyssalAction(botAI); }
    static Action* magtheridon_main_tank_position_boss(PlayerbotAI* botAI) { return new MagtheridonMainTankPositionBossAction(botAI); }
    static Action* magtheridon_spread_ranged(PlayerbotAI* botAI) { return new MagtheridonSpreadRangedAction(botAI); }
    static Action* magtheridon_use_manticron_cube(PlayerbotAI* botAI) { return new MagtheridonUseManticronCubeAction(botAI); }
    static Action* magtheridon_manage_timers_and_assignments(PlayerbotAI* botAI) { return new MagtheridonManageTimersAndAssignmentsAction(botAI); }
};

#endif
