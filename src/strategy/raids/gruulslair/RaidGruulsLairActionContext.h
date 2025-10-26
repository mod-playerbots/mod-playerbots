#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONCONTEXT_H

#include "RaidGruulsLairActions.h"
#include "NamedObjectContext.h"

class RaidGruulsLairActionContext : public NamedObjectContext<Action>
{
public:
    RaidGruulsLairActionContext()
    {
        // High King Maulgar
        creators["high king maulgar main tank attack maulgar"] = &RaidGruulsLairActionContext::high_king_maulgar_main_tank_attack_maulgar;
        creators["high king maulgar first assist tank attack olm"] = &RaidGruulsLairActionContext::high_king_maulgar_first_assist_tank_attack_olm;
        creators["high king maulgar second assist tank attack blindeye"] = &RaidGruulsLairActionContext::high_king_maulgar_second_assist_tank_attack_blindeye;
        creators["high king maulgar mage tank attack krosh"] = &RaidGruulsLairActionContext::high_king_maulgar_mage_tank_attack_krosh;
        creators["high king maulgar moonkin tank attack kiggler"] = &RaidGruulsLairActionContext::high_king_maulgar_moonkin_tank_attack_kiggler;
        creators["high king maulgar assign dps priority"] = &RaidGruulsLairActionContext::high_king_maulgar_assign_dps_priority;
        creators["high king maulgar healer find safe position"] = &RaidGruulsLairActionContext::high_king_maulgar_healer_find_safe_position;
        creators["high king maulgar run away from whirlwind"] = &RaidGruulsLairActionContext::high_king_maulgar_run_away_from_whirlwind;
        creators["high king maulgar banish felstalker"] = &RaidGruulsLairActionContext::high_king_maulgar_banish_felstalker;
        creators["high king maulgar misdirect olm and blindeye"] = &RaidGruulsLairActionContext::high_king_maulgar_misdirect_olm_and_blindeye;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller main tank position boss"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_main_tank_position_boss;
        creators["gruul the dragonkiller spread ranged"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_spread_ranged;
        creators["gruul the dragonkiller shatter spread"] = &RaidGruulsLairActionContext::gruul_the_dragonkiller_shatter_spread;
    }

private:
    // High King Maulgar
    static Action* high_king_maulgar_main_tank_attack_maulgar(PlayerbotAI* botAI) { return new HighKingMaulgarMainTankAttackMaulgarAction(botAI); }
    static Action* high_king_maulgar_first_assist_tank_attack_olm(PlayerbotAI* botAI) { return new HighKingMaulgarFirstAssistTankAttackOlmAction(botAI); }
    static Action* high_king_maulgar_second_assist_tank_attack_blindeye(PlayerbotAI* botAI) { return new HighKingMaulgarSecondAssistTankAttackBlindeyeAction(botAI); }
    static Action* high_king_maulgar_mage_tank_attack_krosh(PlayerbotAI* botAI) { return new HighKingMaulgarMageTankAttackKroshAction(botAI); }
    static Action* high_king_maulgar_moonkin_tank_attack_kiggler(PlayerbotAI* botAI) { return new HighKingMaulgarMoonkinTankAttackKigglerAction(botAI); }
    static Action* high_king_maulgar_assign_dps_priority(PlayerbotAI* botAI) { return new HighKingMaulgarAssignDPSPriorityAction(botAI); }
    static Action* high_king_maulgar_healer_find_safe_position(PlayerbotAI* botAI) { return new HighKingMaulgarHealerFindSafePositionAction(botAI); }
    static Action* high_king_maulgar_run_away_from_whirlwind(PlayerbotAI* botAI) { return new HighKingMaulgarRunAwayFromWhirlwindAction(botAI); }
    static Action* high_king_maulgar_banish_felstalker(PlayerbotAI* botAI) { return new HighKingMaulgarBanishFelstalkerAction(botAI); }
    static Action* high_king_maulgar_misdirect_olm_and_blindeye(PlayerbotAI* botAI) { return new HighKingMaulgarMisdirectOlmAndBlindeyeAction(botAI); }

    // Gruul the Dragonkiller
    static Action* gruul_the_dragonkiller_main_tank_position_boss(PlayerbotAI* botAI) { return new GruulTheDragonkillerMainTankPositionBossAction(botAI); }
    static Action* gruul_the_dragonkiller_spread_ranged(PlayerbotAI* botAI) { return new GruulTheDragonkillerSpreadRangedAction(botAI); }
    static Action* gruul_the_dragonkiller_shatter_spread(PlayerbotAI* botAI) { return new GruulTheDragonkillerShatterSpreadAction(botAI); }
};

#endif
