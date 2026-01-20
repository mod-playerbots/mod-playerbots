#ifndef _PLAYERBOT_RAIDMCACTIONCONTEXT_H
#define _PLAYERBOT_RAIDMCACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidMcActions.h"

class RaidMcActionContext : public NamedObjectContext<Action>
{
public:
    RaidMcActionContext()
    {
        creators["mc lucifron shadow resistance"] = &RaidMcActionContext::lucifron_shadow_resistance;
        creators["mc magmadar fire resistance"] = &RaidMcActionContext::magmadar_fire_resistance;
        creators["mc gehennas shadow resistance"] = &RaidMcActionContext::gehennas_shadow_resistance;
        creators["mc garr fire resistance"] = &RaidMcActionContext::garr_fire_resistance;
        creators["mc baron geddon fire resistance"] = &RaidMcActionContext::baron_geddon_fire_resistance;
        creators["mc move from group"] = &RaidMcActionContext::check_should_move_from_group;
        creators["mc move from baron geddon"] = &RaidMcActionContext::move_from_baron_geddon;
        creators["mc shazzrah move away"] = &RaidMcActionContext::shazzrah_move_away;
        creators["mc sulfuron harbinger fire resistance"] = &RaidMcActionContext::sulfuron_harbinger_fire_resistance;
        creators["mc golemagg fire resistance"] = &RaidMcActionContext::golemagg_fire_resistance;
        creators["mc golemagg mark boss"] = &RaidMcActionContext::golemagg_mark_boss;
        creators["mc golemagg main tank attack golemagg"] = &RaidMcActionContext::golemagg_main_tank_attack_golemagg;
        creators["mc golemagg assist tank attack core rager"] = &RaidMcActionContext::golemagg_assist_tank_attack_core_rager;
        creators["mc majordomo shadow resistance"] = &RaidMcActionContext::majordomo_shadow_resistance;
        creators["mc ragnaros fire resistance"] = &RaidMcActionContext::ragnaros_fire_resistance;
    }

private:
    static Action* lucifron_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "lucifron"); }
    static Action* magmadar_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "magmadar"); }
    static Action* gehennas_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "gehennas"); }
    static Action* garr_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "garr"); }
    static Action* baron_geddon_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "baron geddon"); }
    static Action* check_should_move_from_group(PlayerbotAI* botAI) { return new McMoveFromGroupAction(botAI); }
    static Action* move_from_baron_geddon(PlayerbotAI* botAI) { return new McMoveFromBaronGeddonAction(botAI); }
    static Action* shazzrah_move_away(PlayerbotAI* botAI) { return new McShazzrahMoveAwayAction(botAI); }
    static Action* sulfuron_harbinger_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "sulfuron harbinger"); }
    static Action* golemagg_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "golemagg the incinerator"); }
    static Action* golemagg_mark_boss(PlayerbotAI* botAI) { return new McGolemaggMarkBossAction(botAI); }
    static Action* golemagg_main_tank_attack_golemagg(PlayerbotAI* botAI) { return new McGolemaggMainTankAttackGolemaggAction(botAI); }
    static Action* golemagg_assist_tank_attack_core_rager(PlayerbotAI* botAI) { return new McGolemaggAssistTankAttackCoreRagerAction(botAI); }
    static Action* majordomo_shadow_resistance(PlayerbotAI* botAI) { return new BossShadowResistanceAction(botAI, "majordomo executus"); }
    static Action* ragnaros_fire_resistance(PlayerbotAI* botAI) { return new BossFireResistanceAction(botAI, "ragnaros"); }
};

#endif
