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

        creators["bwl razorgore fire resistance"] = &RaidBwlActionContext::bwl_razorgore_fire_resistance_action;
        creators["bwl razorgore avoid aoe"] = &RaidBwlActionContext::bwl_razorgore_avoid_aoe;
        creators["bwl razorgore mark boss"] = &RaidBwlActionContext::bwl_razorgore_mark_boss;

        creators["bwl vaelastrasz fire resistance"] = &RaidBwlActionContext::bwl_vaelastrasz_fire_resistance_action;
        creators["bwl vaelastrasz move away"] = &RaidBwlActionContext::bwl_vaelastrasz_move_away;

        creators["bwl use hourglass sand"] = &RaidBwlActionContext::bwl_use_hourglass_sand;
        creators["bwl nefarian fear ward"] = &RaidBwlActionContext::bwl_nefarian_fear_ward;
        creators["bwl death talon wyrmguard tank move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_tank_move_away;
        creators["bwl death talon wyrmguard ranged move away"] = &RaidBwlActionContext::bwl_death_talon_wyrmguard_ranged_move_away;
    }

private:
    static Action* bwl_check_onyxia_scale_cloak(PlayerbotAI* ai) { return new BwlOnyxiaScaleCloakAuraCheckAction(ai); }
    static Action* bwl_turn_off_suppression_device(PlayerbotAI* ai) { return new BwlTurnOffSuppressionDeviceAction(ai); }
    static Action* bwl_razorgore_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "razorgore the untamed"); }
    static Action* bwl_razorgore_avoid_aoe(PlayerbotAI* ai) { return new BwlRazorgoreAvoidAoeAction(ai); }
    static Action* bwl_razorgore_mark_boss(PlayerbotAI* ai) { return new BwlRazorgoreMarkBossAction(ai); }
    static Action* bwl_vaelastrasz_fire_resistance_action(PlayerbotAI* ai) { return new BossFireResistanceAction(ai, "vaelastrasz the corrupt"); }
    static Action* bwl_vaelastrasz_move_away(PlayerbotAI* ai) { return new BwlVaelastraszMoveAwayAction(ai); }
    static Action* bwl_use_hourglass_sand(PlayerbotAI* ai) { return new BwlUseHourglassSandAction(ai); }
    static Action* bwl_nefarian_fear_ward(PlayerbotAI* ai) { return new BwlNefarianFearWardAction(ai); }
    static Action* bwl_death_talon_wyrmguard_tank_move_away(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardTankMoveAwayAction(ai); }
    static Action* bwl_death_talon_wyrmguard_ranged_move_away(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardRangedMoveAwayAction(ai); }
};

#endif
