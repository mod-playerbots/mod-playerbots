#ifndef _PLAYERBOT_RAIDGRUULSLAIRTRIGGERCONTEXT_H
#define _PLAYERBOT_RAIDGRUULSLAIRTRIGGERCONTEXT_H

#include "RaidGruulsLairTriggers.h"
#include "AiObjectContext.h"

class RaidGruulsLairTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidGruulsLairTriggerContext() : NamedObjectContext<Trigger>()
    {
        // High King Maulgar
        creators["high king maulgar maulgar tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_maulgar_tank;
        creators["high king maulgar olm tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_olm_tank;
        creators["high king maulgar blindeye tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_blindeye_tank;
        creators["high king maulgar krosh mage tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_krosh_mage_tank;
        creators["high king maulgar kiggler moonkin tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_kiggler_moonkin_tank;
        creators["high king maulgar melee dps priority"] = &RaidGruulsLairTriggerContext::high_king_maulgar_melee_dps_priority;
        creators["high king maulgar ranged dps priority"] = &RaidGruulsLairTriggerContext::high_king_maulgar_ranged_dps_priority;
        creators["high king maulgar healer avoidance"] = &RaidGruulsLairTriggerContext::high_king_maulgar_healer_avoidance;
        creators["high king maulgar whirlwind run away"] = &RaidGruulsLairTriggerContext::high_king_maulgar_whirlwind_run_away;
        creators["high king maulgar banish felstalker"] = &RaidGruulsLairTriggerContext::high_king_maulgar_banish_felstalker;
        creators["high king maulgar hunter misdirection"] = &RaidGruulsLairTriggerContext::high_king_maulgar_hunter_misdirection;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller position boss"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_position_boss;
        creators["gruul the dragonkiller spread ranged"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_spread_ranged;
        creators["gruul the dragonkiller shatter spread"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_shatter_spread;
    }
   
private:
    // High King Maulgar
    static Trigger* high_king_maulgar_set_bot_sight(PlayerbotAI* botAI) { return new HighKingMaulgarSetBotSightTrigger(botAI); }
    static Trigger* high_king_maulgar_maulgar_tank(PlayerbotAI* botAI) { return new HighKingMaulgarMaulgarTankTrigger(botAI); }
    static Trigger* high_king_maulgar_olm_tank(PlayerbotAI* botAI) { return new HighKingMaulgarOlmTankTrigger(botAI); }
    static Trigger* high_king_maulgar_blindeye_tank(PlayerbotAI* botAI) { return new HighKingMaulgarBlindeyeTankTrigger(botAI); }
    static Trigger* high_king_maulgar_krosh_mage_tank(PlayerbotAI* botAI) { return new HighKingMaulgarKroshMageTankTrigger(botAI); }
    static Trigger* high_king_maulgar_kiggler_moonkin_tank(PlayerbotAI* botAI) { return new HighKingMaulgarKigglerMoonkinTankTrigger(botAI); }
    static Trigger* high_king_maulgar_melee_dps_priority(PlayerbotAI* botAI) { return new HighKingMaulgarMeleeDPSPriorityTrigger(botAI); }
    static Trigger* high_king_maulgar_ranged_dps_priority(PlayerbotAI* botAI) { return new HighKingMaulgarRangedDPSPriorityTrigger(botAI); }
    static Trigger* high_king_maulgar_healer_avoidance(PlayerbotAI* botAI) { return new HighKingMaulgarHealerAvoidanceTrigger(botAI); }
    static Trigger* high_king_maulgar_whirlwind_run_away(PlayerbotAI* botAI) { return new HighKingMaulgarWhirlwindRunAwayTrigger(botAI); }
    static Trigger* high_king_maulgar_banish_felstalker(PlayerbotAI* botAI) { return new HighKingMaulgarBanishFelstalkerTrigger(botAI); }
    static Trigger* high_king_maulgar_hunter_misdirection(PlayerbotAI* botAI) { return new HighKingMaulgarHunterMisdirectionTrigger(botAI); }

    // Gruul the Dragonkiller
    static Trigger* gruul_the_dragonkiller_position_boss(PlayerbotAI* botAI) { return new GruulTheDragonkillerPositionBossTrigger(botAI); }
    static Trigger* gruul_the_dragonkiller_spread_ranged(PlayerbotAI* botAI) { return new GruulTheDragonkillerSpreadRangedTrigger(botAI); }
    static Trigger* gruul_the_dragonkiller_shatter_spread(PlayerbotAI* botAI) { return new GruulTheDragonkillerShatterSpreadTrigger(botAI); }
};

#endif
