#ifndef PLAYERBOTS_GRUULACTIONCONTEXT_H
#define PLAYERBOTS_GRUULACTIONCONTEXT_H

#include "GruulActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        // High King Maulgar
        creators["high king maulgar main tank attack maulgar"] =
            &RaidGruulsLairActionContext::high_king_maulgar_main_tank_attack_maulgar;

        creators["high king maulgar first assist tank attack olm"] =
            &RaidGruulsLairActionContext::high_king_maulgar_first_assist_tank_attack_olm;

        creators["high king maulgar second assist tank attack blindeye"] =
            &RaidGruulsLairActionContext::high_king_maulgar_second_assist_tank_attack_blindeye;

        creators["high king maulgar mage tank attack krosh"] =
            &RaidGruulsLairActionContext::high_king_maulgar_mage_tank_attack_krosh;

        creators["high king maulgar moonkin tank attack kiggler"] =
            &RaidGruulsLairActionContext::high_king_maulgar_moonkin_tank_attack_kiggler;

        creators["high king maulgar assign dps priority"] =
            &RaidGruulsLairActionContext::high_king_maulgar_assign_dps_priority;

        creators["high king maulgar run away from whirlwind"] =
            &RaidGruulsLairActionContext::high_king_maulgar_run_away_from_whirlwind;

        creators["high king maulgar move away from blast nova danger"] =
            &RaidGruulsLairActionContext::high_king_maulgar_move_away_from_blast_nova_danger;

        creators["high king maulgar banish fel stalker"] =
            &RaidGruulsLairActionContext::high_king_maulgar_banish_fel_stalker;

        creators["high king maulgar misdirect ogres to tanks"] =
            &RaidGruulsLairActionContext::high_king_maulgar_misdirect_ogres_to_tanks;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller tanks position boss"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_tanks_position_boss;

        creators["gruul the dragonkiller spread ranged"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_spread_ranged;

        creators["gruul the dragonkiller shatter spread"] =
            &RaidGruulsLairActionContext::gruul_the_dragonkiller_shatter_spread;
    }

private:
    // High King Maulgar
    static Action* high_king_maulgar_main_tank_attack_maulgar(PlayerbotAI* botAI) {
        return new HighKingMaulgarMainTankAttackMaulgarAction(botAI);
    }

    static Action* high_king_maulgar_first_assist_tank_attack_olm(PlayerbotAI* botAI) {
        return new HighKingMaulgarFirstAssistTankAttackOlmAction(botAI);
    }

    static Action* high_king_maulgar_second_assist_tank_attack_blindeye(PlayerbotAI* botAI) {
        return new HighKingMaulgarSecondAssistTankAttackBlindeyeAction(botAI);
    }

    static Action* high_king_maulgar_mage_tank_attack_krosh(PlayerbotAI* botAI) {
        return new HighKingMaulgarMageTankAttackKroshAction(botAI);
    }

    static Action* high_king_maulgar_moonkin_tank_attack_kiggler(PlayerbotAI* botAI) {
        return new HighKingMaulgarMoonkinTankAttackKigglerAction(botAI);
    }

    static Action* high_king_maulgar_assign_dps_priority(PlayerbotAI* botAI) {
        return new HighKingMaulgarAssignDPSPriorityAction(botAI);
    }

    static Action* high_king_maulgar_run_away_from_whirlwind(PlayerbotAI* botAI) {
        return new HighKingMaulgarRunAwayFromWhirlwindAction(botAI);
    }

    static Action* high_king_maulgar_move_away_from_blast_nova_danger(PlayerbotAI* botAI) {
        return new HighKingMaulgarMoveAwayFromBlastNovaDangerAction(botAI);
    }

    static Action* high_king_maulgar_banish_fel_stalker(PlayerbotAI* botAI) {
        return new HighKingMaulgarBanishFelStalkerAction(botAI);
    }

    static Action* high_king_maulgar_misdirect_ogres_to_tanks(PlayerbotAI* botAI) {
        return new HighKingMaulgarMisdirectOgresToTanksAction(botAI);
    }

    // Gruul the Dragonkiller
    static Action* gruul_the_dragonkiller_tanks_position_boss(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerTanksPositionBossAction(botAI);
    }

    static Action* gruul_the_dragonkiller_spread_ranged(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerSpreadRangedAction(botAI);
    }

    static Action* gruul_the_dragonkiller_shatter_spread(PlayerbotAI* botAI) {
        return new GruulTheDragonkillerShatterSpreadAction(botAI);
    }
};

#endif
