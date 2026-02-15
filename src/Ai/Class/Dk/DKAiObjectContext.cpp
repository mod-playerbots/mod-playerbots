/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DKAiObjectContext.h"

#include "BloodDKStrategy.h"
#include "DKActions.h"
#include "DKTriggers.h"
#include "FrostDKStrategy.h"
#include "GenericDKNonCombatStrategy.h"
#include "GenericTriggers.h"
#include "PullStrategy.h"
#include "UnholyDKStrategy.h"

class DeathKnightStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightStrategyFactoryInternal()
    {
        creators["nc"] = &DeathKnightStrategyFactoryInternal::nc;
        creators["pull"] = &DeathKnightStrategyFactoryInternal::pull;
        creators["frost aoe"] = &DeathKnightStrategyFactoryInternal::frost_aoe;
        creators["unholy aoe"] = &DeathKnightStrategyFactoryInternal::unholy_aoe;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericDKNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "icy touch"); }
    static Strategy* frost_aoe(PlayerbotAI* botAI) { return new FrostDKAoeStrategy(botAI); }
    static Strategy* unholy_aoe(PlayerbotAI* botAI) { return new UnholyDKAoeStrategy(botAI); }
};

class DeathKnightCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &DeathKnightCombatStrategyFactoryInternal::blood;
        creators["blood"] = &DeathKnightCombatStrategyFactoryInternal::blood;
        creators["frost"] = &DeathKnightCombatStrategyFactoryInternal::frost_dps;
        creators["unholy"] = &DeathKnightCombatStrategyFactoryInternal::unholy_dps;
    }

private:
    static Strategy* frost_dps(PlayerbotAI* botAI) { return new FrostDKStrategy(botAI); }
    static Strategy* unholy_dps(PlayerbotAI* botAI) { return new UnholyDKStrategy(botAI); }
    static Strategy* tank(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
    static Strategy* blood(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
};

class DeathKnightDKBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightDKBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bdps"] = &DeathKnightDKBuffStrategyFactoryInternal::bdps;
    }

private:
    static Strategy* bdps(PlayerbotAI* botAI) { return new DKBuffDpsStrategy(botAI); }
};

class DeathKnightTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    DeathKnightTriggerFactoryInternal()
    {
        creators["bone shield"] = &DeathKnightTriggerFactoryInternal::bone_shield;
        creators["pestilence glyph"] = &DeathKnightTriggerFactoryInternal::pestilence_glyph;
        creators["blood strike"] = &DeathKnightTriggerFactoryInternal::blood_strike;
        creators["plague strike"] = &DeathKnightTriggerFactoryInternal::plague_strike;
        creators["plague strike on attacker"] = &DeathKnightTriggerFactoryInternal::plague_strike_on_attacker;
        creators["icy touch"] = &DeathKnightTriggerFactoryInternal::icy_touch;
        creators["icy touch 3s"] = &DeathKnightTriggerFactoryInternal::icy_touch_3s;
        creators["dd cd and icy touch 3s"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_icy_touch_3s;
        creators["death coil"] = &DeathKnightTriggerFactoryInternal::death_coil;
        creators["icy touch on attacker"] = &DeathKnightTriggerFactoryInternal::icy_touch_on_attacker;
        creators["improved icy talons"] = &DeathKnightTriggerFactoryInternal::improved_icy_talons;
        creators["plague strike"] = &DeathKnightTriggerFactoryInternal::plague_strike;
        creators["plague strike 3s"] = &DeathKnightTriggerFactoryInternal::plague_strike_3s;
        creators["dd cd and plague strike 3s"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_plague_strike_3s;
        creators["horn of winter"] = &DeathKnightTriggerFactoryInternal::horn_of_winter;
        creators["mind freeze"] = &DeathKnightTriggerFactoryInternal::mind_freeze;
        creators["mind freeze on enemy healer"] = &DeathKnightTriggerFactoryInternal::mind_freeze_on_enemy_healer;
        creators["strangulate"] = &DeathKnightTriggerFactoryInternal::strangulate;
        creators["strangulate on enemy healer"] = &DeathKnightTriggerFactoryInternal::strangulate_on_enemy_healer;
        creators["blood tap"] = &DeathKnightTriggerFactoryInternal::blood_tap;
        creators["raise dead"] = &DeathKnightTriggerFactoryInternal::raise_dead;
        creators["chains of ice"] = &DeathKnightTriggerFactoryInternal::chains_of_ice;
        creators["unbreakable armor"] = &DeathKnightTriggerFactoryInternal::unbreakable_armor;
        creators["high blood rune"] = &DeathKnightTriggerFactoryInternal::high_blood_rune;
        creators["high frost rune"] = &DeathKnightTriggerFactoryInternal::high_frost_rune;
        creators["high unholy rune"] = &DeathKnightTriggerFactoryInternal::high_unholy_rune;
        creators["no rune"] = &DeathKnightTriggerFactoryInternal::no_rune;
        creators["freezing fog"] = &DeathKnightTriggerFactoryInternal::freezing_fog;
        creators["no desolation"] = &DeathKnightTriggerFactoryInternal::no_desolation;
        creators["dd cd and no desolation"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_no_desolation;
        creators["death and decay cooldown"] = &DeathKnightTriggerFactoryInternal::death_and_decay_cooldown;
        creators["army of the dead"] = &DeathKnightTriggerFactoryInternal::army_of_the_dead;
    }

private:
    static Trigger* bone_shield(PlayerbotAI* botAI) { return new BoneShieldTrigger(botAI); }
    static Trigger* pestilence_glyph(PlayerbotAI* botAI) { return new PestilenceGlyphTrigger(botAI); }
    static Trigger* blood_strike(PlayerbotAI* botAI) { return new BloodStrikeTrigger(botAI); }
    static Trigger* plague_strike(PlayerbotAI* botAI) { return new PlagueStrikeDebuffTrigger(botAI); }
    static Trigger* plague_strike_3s(PlayerbotAI* botAI) { return new PlagueStrike3sDebuffTrigger(botAI); }
    static Trigger* dd_cd_and_plague_strike_3s(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "plague strike 3s");
    }
    static Trigger* plague_strike_on_attacker(PlayerbotAI* botAI)
    {
        return new PlagueStrikeDebuffOnAttackerTrigger(botAI);
    }
    static Trigger* icy_touch(PlayerbotAI* botAI) { return new IcyTouchDebuffTrigger(botAI); }
    static Trigger* icy_touch_3s(PlayerbotAI* botAI) { return new IcyTouch3sDebuffTrigger(botAI); }
    static Trigger* dd_cd_and_icy_touch_3s(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "icy touch 3s");
    }
    static Trigger* death_coil(PlayerbotAI* botAI) { return new DeathCoilTrigger(botAI); }
    static Trigger* icy_touch_on_attacker(PlayerbotAI* botAI) { return new IcyTouchDebuffOnAttackerTrigger(botAI); }
    static Trigger* improved_icy_talons(PlayerbotAI* botAI) { return new ImprovedIcyTalonsTrigger(botAI); }
    static Trigger* horn_of_winter(PlayerbotAI* botAI) { return new HornOfWinterTrigger(botAI); }
    static Trigger* mind_freeze(PlayerbotAI* botAI) { return new MindFreezeInterruptSpellTrigger(botAI); }
    static Trigger* mind_freeze_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new MindFreezeOnEnemyHealerTrigger(botAI);
    }
    static Trigger* strangulate(PlayerbotAI* botAI) { return new StrangulateInterruptSpellTrigger(botAI); }
    static Trigger* strangulate_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new StrangulateOnEnemyHealerTrigger(botAI);
    }
    static Trigger* blood_tap(PlayerbotAI* botAI) { return new BloodTapTrigger(botAI); }
    static Trigger* raise_dead(PlayerbotAI* botAI) { return new RaiseDeadTrigger(botAI); }
    static Trigger* chains_of_ice(PlayerbotAI* botAI) { return new ChainsOfIceSnareTrigger(botAI); }
    static Trigger* unbreakable_armor(PlayerbotAI* botAI) { return new UnbreakableArmorTrigger(botAI); }
    static Trigger* high_blood_rune(PlayerbotAI* botAI) { return new HighBloodRuneTrigger(botAI); }
    static Trigger* high_frost_rune(PlayerbotAI* botAI) { return new HighFrostRuneTrigger(botAI); }
    static Trigger* high_unholy_rune(PlayerbotAI* botAI) { return new HighUnholyRuneTrigger(botAI); }
    static Trigger* no_rune(PlayerbotAI* botAI) { return new NoRuneTrigger(botAI); }
    static Trigger* freezing_fog(PlayerbotAI* botAI) { return new FreezingFogTrigger(botAI); }
    static Trigger* no_desolation(PlayerbotAI* botAI) { return new DesolationTrigger(botAI); }
    static Trigger* dd_cd_and_no_desolation(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "no desolation");
    }
    static Trigger* death_and_decay_cooldown(PlayerbotAI* botAI) { return new DeathAndDecayCooldownTrigger(botAI); }
    static Trigger* army_of_the_dead(PlayerbotAI* botAI) { return new ArmyOfTheDeadTrigger(botAI); }
};

SharedNamedObjectContextList<Strategy> DKAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> DKAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> DKAiObjectContext::sharedValueContexts;

DKAiObjectContext::DKAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void DKAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    // BuildSharedActionContexts(sharedActionContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void DKAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new DeathKnightStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightCombatStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightDKBuffStrategyFactoryInternal());
}

void DKAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new DeathKnightTriggerFactoryInternal());
}

void DKAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
