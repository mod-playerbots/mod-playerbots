/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarriorAiObjectContext.h"

#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"
#include "GenericWarriorNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"
#include "TankWarriorStrategy.h"
#include "WarriorTriggers.h"

class WarriorStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarriorStrategyFactoryInternal()
    {
        creators["nc"] = &WarriorStrategyFactoryInternal::nc;
        creators["pull"] = &WarriorStrategyFactoryInternal::pull;
        creators["aoe"] = &WarriorStrategyFactoryInternal::warrior_aoe;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarriorNonCombatStrategy(botAI); }
    static Strategy* warrior_aoe(PlayerbotAI* botAI) { return new WarrirorAoeStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
};

class WarriorCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarriorCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &WarriorCombatStrategyFactoryInternal::tank;
        creators["arms"] = &WarriorCombatStrategyFactoryInternal::arms;
        creators["fury"] = &WarriorCombatStrategyFactoryInternal::fury;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarriorStrategy(botAI); }
    static Strategy* arms(PlayerbotAI* botAI) { return new ArmsWarriorStrategy(botAI); }
    static Strategy* fury(PlayerbotAI* botAI) { return new FuryWarriorStrategy(botAI); }
};

class WarriorTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarriorTriggerFactoryInternal()
    {
        creators["hamstring"] = &WarriorTriggerFactoryInternal::hamstring;
        creators["victory rush"] = &WarriorTriggerFactoryInternal::victory_rush;
        creators["death wish"] = &WarriorTriggerFactoryInternal::death_wish;
        creators["recklessness"] = &WarriorTriggerFactoryInternal::recklessness;
        creators["battle shout"] = &WarriorTriggerFactoryInternal::battle_shout;
        creators["rend"] = &WarriorTriggerFactoryInternal::rend;
        creators["rend on attacker"] = &WarriorTriggerFactoryInternal::rend_on_attacker;
        creators["bloodrage"] = &WarriorTriggerFactoryInternal::bloodrage;
        creators["shield bash"] = &WarriorTriggerFactoryInternal::shield_bash;
        creators["disarm"] = &WarriorTriggerFactoryInternal::disarm;
        creators["concussion blow"] = &WarriorTriggerFactoryInternal::concussion_blow;
        creators["sword and board"] = &WarriorTriggerFactoryInternal::SwordAndBoard;
        creators["shield bash on enemy healer"] = &WarriorTriggerFactoryInternal::shield_bash_on_enemy_healer;
        creators["battle stance"] = &WarriorTriggerFactoryInternal::battle_stance;
        creators["defensive stance"] = &WarriorTriggerFactoryInternal::defensive_stance;
        creators["berserker stance"] = &WarriorTriggerFactoryInternal::berserker_stance;
        creators["shield block"] = &WarriorTriggerFactoryInternal::shield_block;
        creators["sunder armor"] = &WarriorTriggerFactoryInternal::sunder_armor;
        creators["revenge"] = &WarriorTriggerFactoryInternal::revenge;
        creators["overpower"] = &WarriorTriggerFactoryInternal::overpower;
        creators["mocking blow"] = &WarriorTriggerFactoryInternal::mocking_blow;
        creators["rampage"] = &WarriorTriggerFactoryInternal::rampage;
        creators["mortal strike"] = &WarriorTriggerFactoryInternal::mortal_strike;
        creators["thunder clap on snare target"] = &WarriorTriggerFactoryInternal::thunder_clap_on_snare_target;
        creators["thunder clap"] = &WarriorTriggerFactoryInternal::thunder_clap;
        creators["bloodthirst"] = &WarriorTriggerFactoryInternal::bloodthirst;
        creators["whirlwind"] = &WarriorTriggerFactoryInternal::whirlwind;
        creators["berserker rage"] = &WarriorTriggerFactoryInternal::berserker_rage;
        creators["pummel on enemy healer"] = &WarriorTriggerFactoryInternal::pummel_on_enemy_healer;
        creators["pummel"] = &WarriorTriggerFactoryInternal::pummel;
        creators["intercept on enemy healer"] = &WarriorTriggerFactoryInternal::intercept_on_enemy_healer;
        creators["intercept"] = &WarriorTriggerFactoryInternal::intercept;
        creators["taunt on snare target"] = &WarriorTriggerFactoryInternal::taunt_on_snare_target;
        creators["commanding shout"] = &WarriorTriggerFactoryInternal::commanding_shout;
        creators["intercept on snare target"] = &WarriorTriggerFactoryInternal::intercept_on_snare_target;
        creators["spell reflection"] = &WarriorTriggerFactoryInternal::spell_reflection;
        creators["sudden death"] = &WarriorTriggerFactoryInternal::sudden_death;
        creators["instant slam"] = &WarriorTriggerFactoryInternal::instant_slam;
        creators["shockwave"] = &WarriorTriggerFactoryInternal::shockwave;
        creators["shockwave on snare target"] = &WarriorTriggerFactoryInternal::shockwave_on_snare_target;
        creators["taste for blood"] = &WarriorTriggerFactoryInternal::taste_for_blood;

        creators["thunder clap and rage"] = &WarriorTriggerFactoryInternal::thunderclap_and_rage;
        creators["intercept can cast"] = &WarriorTriggerFactoryInternal::intercept_can_cast;
        creators["intercept and far enemy"] = &WarriorTriggerFactoryInternal::intercept_and_far_enemy;
        creators["intercept and rage"] = &WarriorTriggerFactoryInternal::intercept_and_rage;
        // creators["slam"] = &WarriorTriggerFactoryInternal::slam;

        creators["vigilance"] = &WarriorTriggerFactoryInternal::vigilance;
        creators["shattering throw trigger"] = &WarriorTriggerFactoryInternal::shattering_throw_trigger;
    }

private:
    static Trigger* shield_block(PlayerbotAI* botAI) { return new ShieldBlockTrigger(botAI); }
    static Trigger* defensive_stance(PlayerbotAI* botAI) { return new DefensiveStanceTrigger(botAI); }
    static Trigger* berserker_stance(PlayerbotAI* botAI) { return new BerserkerStanceTrigger(botAI); }
    static Trigger* battle_stance(PlayerbotAI* botAI) { return new BattleStanceTrigger(botAI); }
    static Trigger* hamstring(PlayerbotAI* botAI) { return new HamstringTrigger(botAI); }
    static Trigger* victory_rush(PlayerbotAI* botAI) { return new VictoryRushTrigger(botAI); }
    static Trigger* death_wish(PlayerbotAI* botAI) { return new DeathWishTrigger(botAI); }
    static Trigger* recklessness(PlayerbotAI* botAI) { return new RecklessnessTrigger(botAI); }
    static Trigger* battle_shout(PlayerbotAI* botAI) { return new BattleShoutTrigger(botAI); }
    static Trigger* rend(PlayerbotAI* botAI) { return new RendDebuffTrigger(botAI); }
    static Trigger* rend_on_attacker(PlayerbotAI* botAI) { return new RendDebuffOnAttackerTrigger(botAI); }
    static Trigger* bloodrage(PlayerbotAI* botAI) { return new BloodrageBuffTrigger(botAI); }
    static Trigger* shield_bash(PlayerbotAI* botAI) { return new ShieldBashInterruptSpellTrigger(botAI); }
    static Trigger* disarm(PlayerbotAI* botAI) { return new DisarmDebuffTrigger(botAI); }
    static Trigger* concussion_blow(PlayerbotAI* botAI) { return new ConcussionBlowTrigger(botAI); }
    static Trigger* SwordAndBoard(PlayerbotAI* botAI) { return new SwordAndBoardTrigger(botAI); }
    static Trigger* shield_bash_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new ShieldBashInterruptEnemyHealerSpellTrigger(botAI);
    }

    static Trigger* thunderclap_and_rage(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "thunder clap", "light rage available");
    }
    static Trigger* intercept_can_cast(PlayerbotAI* botAI) { return new InterceptCanCastTrigger(botAI); }
    static Trigger* intercept_and_far_enemy(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "enemy is out of melee", "intercept can cast");
    }
    static Trigger* intercept_and_rage(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "intercept and far enemy", "light rage available");
    }

    static Trigger* intercept_on_snare_target(PlayerbotAI* botAI) { return new InterceptSnareTrigger(botAI); }
    static Trigger* spell_reflection(PlayerbotAI* botAI) { return new SpellReflectionTrigger(botAI); }
    static Trigger* taste_for_blood(PlayerbotAI* botAI) { return new TasteForBloodTrigger(botAI); }
    static Trigger* shockwave_on_snare_target(PlayerbotAI* botAI) { return new ShockwaveSnareTrigger(botAI); }
    static Trigger* shockwave(PlayerbotAI* botAI) { return new ShockwaveTrigger(botAI); }
    static Trigger* instant_slam(PlayerbotAI* botAI) { return new SlamInstantTrigger(botAI); }
    static Trigger* sudden_death(PlayerbotAI* botAI) { return new SuddenDeathTrigger(botAI); }
    static Trigger* commanding_shout(PlayerbotAI* botAI) { return new CommandingShoutTrigger(botAI); }
    static Trigger* taunt_on_snare_target(PlayerbotAI* botAI) { return new TauntSnareTrigger(botAI); }
    static Trigger* intercept(PlayerbotAI* botAI) { return new InterceptInterruptSpellTrigger(botAI); }
    static Trigger* intercept_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new InterceptInterruptEnemyHealerSpellTrigger(botAI);
    }
    static Trigger* pummel(PlayerbotAI* botAI) { return new PummelInterruptSpellTrigger(botAI); }
    static Trigger* pummel_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new PummelInterruptEnemyHealerSpellTrigger(botAI);
    }
    static Trigger* berserker_rage(PlayerbotAI* botAI) { return new BerserkerRageBuffTrigger(botAI); }
    static Trigger* bloodthirst(PlayerbotAI* botAI) { return new BloodthirstBuffTrigger(botAI); }
    static Trigger* whirlwind(PlayerbotAI* botAI) { return new WhirlwindTrigger(botAI); }
    static Trigger* thunder_clap_on_snare_target(PlayerbotAI* botAI) { return new ThunderClapSnareTrigger(botAI); }
    static Trigger* thunder_clap(PlayerbotAI* botAI) { return new ThunderClapTrigger(botAI); }
    static Trigger* mortal_strike(PlayerbotAI* botAI) { return new MortalStrikeDebuffTrigger(botAI); }
    static Trigger* rampage(PlayerbotAI* botAI) { return new RampageAvailableTrigger(botAI); }
    static Trigger* mocking_blow(PlayerbotAI* botAI) { return new MockingBlowTrigger(botAI); }
    static Trigger* overpower(PlayerbotAI* botAI) { return new OverpowerAvailableTrigger(botAI); }
    static Trigger* revenge(PlayerbotAI* botAI) { return new RevengeAvailableTrigger(botAI); }
    static Trigger* sunder_armor(PlayerbotAI* botAI) { return new SunderArmorDebuffTrigger(botAI); }
    // static Trigger* slam(PlayerbotAI* ai) { return new SlamTrigger(ai); }

    static Trigger* vigilance(PlayerbotAI* botAI) { return new VigilanceTrigger(botAI); }
    static Trigger* shattering_throw_trigger(PlayerbotAI* botAI) { return new ShatteringThrowTrigger(botAI); }
};

SharedNamedObjectContextList<Strategy> WarriorAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> WarriorAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> WarriorAiObjectContext::sharedValueContexts;

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void WarriorAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void WarriorAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new WarriorStrategyFactoryInternal());
    strategyContexts.Add(new WarriorCombatStrategyFactoryInternal());
}

void WarriorAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new WarriorTriggerFactoryInternal());
}

void WarriorAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}