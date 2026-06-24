#ifndef PLAYERBOTS_BWLACTIONCONTEXT_H
#define PLAYERBOTS_BWLACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "BWLActions.h"

class RaidBwlActionContext : public NamedObjectContext<Action>
{
public:
    RaidBwlActionContext()
    {
        creators["bwl check onyxia scale cloak"] = &RaidBwlActionContext::bwl_check_onyxia_scale_cloak;
        creators["bwl turn off suppression device"] = &RaidBwlActionContext::bwl_turn_off_suppression_device;
        creators["bwl use hourglass sand"] = &RaidBwlActionContext::bwl_use_hourglass_sand;
        creators["bwl nefarian fear ward"] = &RaidBwlActionContext::bwl_nefarian_fear_ward;
        creators["bwl death talon wyrmguard tank move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_tank_move_away;
        creators["bwl death talon wyrmguard ranged move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_ranged_move_away;
    }

private:
    static Action* bwl_check_onyxia_scale_cloak(PlayerbotAI* botAI) { return new BwlOnyxiaScaleCloakAuraCheckAction(botAI); }
    static Action* bwl_turn_off_suppression_device(PlayerbotAI* botAI) { return new BwlTurnOffSuppressionDeviceAction(botAI); }
    static Action* bwl_use_hourglass_sand(PlayerbotAI* botAI) { return new BwlUseHourglassSandAction(botAI); }
    static Action* bwl_nefarian_fear_ward(PlayerbotAI* botAI) { return new BwlNefarianFearWardAction(botAI); }
    static Action* bwl_death_talon_wyrmguard_tank_move_away(PlayerbotAI* botAI) { return new BwlDeathTalonWyrmguardTankMoveAwayAction(botAI); }
    static Action* bwl_death_talon_wyrmguard_ranged_move_away(PlayerbotAI* botAI) { return new BwlDeathTalonWyrmguardRangedMoveAwayAction(botAI); }
};

#endif
