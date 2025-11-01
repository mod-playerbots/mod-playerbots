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
        creators["high king maulgar is main tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_is_main_tank;
        creators["high king maulgar is first assist tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_is_first_assist_tank;
        creators["high king maulgar is second assist tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_is_second_assist_tank;
        creators["high king maulgar is mage tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_is_mage_tank;
        creators["high king maulgar is moonkin tank"] = &RaidGruulsLairTriggerContext::high_king_maulgar_is_moonkin_tank;
        creators["high king maulgar determining kill order"] = &RaidGruulsLairTriggerContext::high_king_maulgar_determining_kill_order;
        creators["high king maulgar healer in danger"] = &RaidGruulsLairTriggerContext::high_king_maulgar_healer_in_danger;
        creators["high king maulgar boss channeling whirlwind"] = &RaidGruulsLairTriggerContext::high_king_maulgar_boss_channeling_whirlwind;
        creators["high king maulgar wild felstalker spawned"] = &RaidGruulsLairTriggerContext::high_king_maulgar_wild_felstalker_spawned;
        creators["high king maulgar pulling olm and blindeye"] = &RaidGruulsLairTriggerContext::high_king_maulgar_pulling_olm_and_blindeye;

        // Gruul the Dragonkiller
        creators["gruul the dragonkiller boss engaged by main tank"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_boss_engaged_by_main_tank;
        creators["gruul the dragonkiller boss engaged by range"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_boss_engaged_by_range;
        creators["gruul the dragonkiller incoming shatter"] = &RaidGruulsLairTriggerContext::gruul_the_dragonkiller_incoming_shatter;
    }

private:
    // High King Maulgar
    static Trigger* high_king_maulgar_is_main_tank(PlayerbotAI* botAI) { return new HighKingMaulgarIsMainTankTrigger(botAI); }
    static Trigger* high_king_maulgar_is_first_assist_tank(PlayerbotAI* botAI) { return new HighKingMaulgarIsFirstAssistTankTrigger(botAI); }
    static Trigger* high_king_maulgar_is_second_assist_tank(PlayerbotAI* botAI) { return new HighKingMaulgarIsSecondAssistTankTrigger(botAI); }
    static Trigger* high_king_maulgar_is_mage_tank(PlayerbotAI* botAI) { return new HighKingMaulgarIsMageTankTrigger(botAI); }
    static Trigger* high_king_maulgar_is_moonkin_tank(PlayerbotAI* botAI) { return new HighKingMaulgarIsMoonkinTankTrigger(botAI); }
    static Trigger* high_king_maulgar_determining_kill_order(PlayerbotAI* botAI) { return new HighKingMaulgarDeterminingKillOrderTrigger(botAI); }
    static Trigger* high_king_maulgar_healer_in_danger(PlayerbotAI* botAI) { return new HighKingMaulgarHealerInDangerTrigger(botAI); }
    static Trigger* high_king_maulgar_boss_channeling_whirlwind(PlayerbotAI* botAI) { return new HighKingMaulgarBossChannelingWhirlwindTrigger(botAI); }
    static Trigger* high_king_maulgar_wild_felstalker_spawned(PlayerbotAI* botAI) { return new HighKingMaulgarWildFelstalkerSpawnedTrigger(botAI); }
    static Trigger* high_king_maulgar_pulling_olm_and_blindeye(PlayerbotAI* botAI) { return new HighKingMaulgarPullingOlmAndBlindeyeTrigger(botAI); }

    // Gruul the Dragonkiller
    static Trigger* gruul_the_dragonkiller_boss_engaged_by_main_tank(PlayerbotAI* botAI) { return new GruulTheDragonkillerBossEngagedByMainTankTrigger(botAI); }
    static Trigger* gruul_the_dragonkiller_boss_engaged_by_range(PlayerbotAI* botAI) { return new GruulTheDragonkillerBossEngagedByRangeTrigger(botAI); }
    static Trigger* gruul_the_dragonkiller_incoming_shatter(PlayerbotAI* botAI) { return new GruulTheDragonkillerIncomingShatterTrigger(botAI); }
};

#endif
