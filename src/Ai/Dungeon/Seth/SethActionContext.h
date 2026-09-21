/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SETHACTIONCONTEXT_H
#define PLAYERBOTS_SETHACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SethActions.h"

class TbcDungeonSethekkHallsActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonSethekkHallsActionContext()
    {
        creators["time-lost controller mark charming totem with skull"] =
            &TbcDungeonSethekkHallsActionContext::time_lost_controller_mark_charming_totem_with_skull;

        creators["sethekk prophet set tremor totem"] =
            &TbcDungeonSethekkHallsActionContext::sethekk_prophet_set_tremor_totem;

        creators["darkweaver syth mark elementals with skull"] =
            &TbcDungeonSethekkHallsActionContext::darkweaver_syth_mark_elementals_with_skull;

        creators["anzu alternate marks on boss"] =
            &TbcDungeonSethekkHallsActionContext::anzu_alternate_marks_on_boss;

        creators["anzu cast heal over time spell on bird spirit"] =
            &TbcDungeonSethekkHallsActionContext::anzu_cast_heal_over_time_spell_on_bird_spirit;

        creators["talon king ikiss tank move boss to pillar position"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_tank_move_boss_to_pillar_position;

        creators["talon king ikiss ranged stay near victim of boss"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_ranged_stay_near_victim_of_boss;

        creators["talon king ikiss los arcane explosion"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_los_arcane_explosion;

        creators["talon king ikiss move to within los"] =
            &TbcDungeonSethekkHallsActionContext::talon_king_ikiss_move_to_within_los;
    }
private:
    static Action* time_lost_controller_mark_charming_totem_with_skull(PlayerbotAI* botAI) {
        return new TimeLostControllerMarkCharmingTotemWithSkullAction(botAI);
    }
    static Action* sethekk_prophet_set_tremor_totem(PlayerbotAI* botAI) {
        return new SethekkProphetSetTremorTotemAction(botAI);
    }
    static Action* darkweaver_syth_mark_elementals_with_skull(PlayerbotAI* botAI) {
        return new DarkweaverSythMarkElementalsWithSkullAction(botAI);
    }
    static Action* anzu_alternate_marks_on_boss(PlayerbotAI* botAI) {
        return new AnzuAlternateMarksOnBossAction(botAI);
    }
    static Action* anzu_cast_heal_over_time_spell_on_bird_spirit(PlayerbotAI* botAI) {
        return new AnzuCastHealOverTimeSpellOnBirdSpiritAction(botAI);
    }
    static Action* talon_king_ikiss_tank_move_boss_to_pillar_position(PlayerbotAI* botAI) {
        return new TalonKingIkissTankMoveBossToPillarPositionAction(botAI);
    }
    static Action* talon_king_ikiss_ranged_stay_near_victim_of_boss(PlayerbotAI* botAI) {
        return new TalonKingIkissRangedStayNearVictimOfBossAction(botAI);
    }
    static Action* talon_king_ikiss_los_arcane_explosion(PlayerbotAI* botAI) {
        return new TalonKingIkissLosArcaneExplosionAction(botAI);
    }
    static Action* talon_king_ikiss_move_to_within_los(PlayerbotAI* botAI) {
        return new TalonKingIkissMoveToWithinLosAction(botAI);
    }
};

#endif
