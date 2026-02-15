/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WarlockAiObjectContext.h"

#include "AfflictionWarlockStrategy.h"
#include "DemonologyWarlockStrategy.h"
#include "DestructionWarlockStrategy.h"
#include "GenericTriggers.h"
#include "GenericWarlockNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"
#include "Strategy.h"
#include "TankWarlockStrategy.h"
#include "UseItemAction.h"
#include "WarlockActions.h"
#include "WarlockTriggers.h"

class WarlockStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockStrategyFactoryInternal()
    {
        creators["nc"] = &WarlockStrategyFactoryInternal::nc;
        creators["pull"] = &WarlockStrategyFactoryInternal::pull;
        creators["boost"] = &WarlockStrategyFactoryInternal::boost;
        creators["cc"] = &WarlockStrategyFactoryInternal::cc;
        creators["pet"] = &WarlockStrategyFactoryInternal::pet;
        creators["meta melee"] = &WarlockStrategyFactoryInternal::meta_melee_aoe;
        creators["tank"] = &WarlockStrategyFactoryInternal::tank;
        creators["aoe"] = &WarlockStrategyFactoryInternal::aoe;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarlockNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* boost(PlayerbotAI* botAI) { return new WarlockBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new WarlockCcStrategy(botAI); }
    static Strategy* pet(PlayerbotAI* botAI) { return new WarlockPetStrategy(botAI); }
    static Strategy* meta_melee_aoe(PlayerbotAI* botAI) { return new MetaMeleeAoeStrategy(botAI); }
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarlockStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new AoEWarlockStrategy(botAI); }
};

class WarlockCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["affli"] = &WarlockCombatStrategyFactoryInternal::affliction;
        creators["demo"] = &WarlockCombatStrategyFactoryInternal::demonology;
        creators["destro"] = &WarlockCombatStrategyFactoryInternal::destruction;
    }

private:
    static Strategy* affliction(PlayerbotAI* botAI) { return new AfflictionWarlockStrategy(botAI); }
    static Strategy* demonology(PlayerbotAI* botAI) { return new DemonologyWarlockStrategy(botAI); }
    static Strategy* destruction(PlayerbotAI* botAI) { return new DestructionWarlockStrategy(botAI); }
};

class WarlockPetStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockPetStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["imp"] = &WarlockPetStrategyFactoryInternal::imp;
        creators["voidwalker"] = &WarlockPetStrategyFactoryInternal::voidwalker;
        creators["succubus"] = &WarlockPetStrategyFactoryInternal::succubus;
        creators["felhunter"] = &WarlockPetStrategyFactoryInternal::felhunter;
        creators["felguard"] = &WarlockPetStrategyFactoryInternal::felguard;
    }

private:
    static Strategy* imp(PlayerbotAI* ai) { return new SummonImpStrategy(ai); }
    static Strategy* voidwalker(PlayerbotAI* ai) { return new SummonVoidwalkerStrategy(ai); }
    static Strategy* succubus(PlayerbotAI* ai) { return new SummonSuccubusStrategy(ai); }
    static Strategy* felhunter(PlayerbotAI* ai) { return new SummonFelhunterStrategy(ai); }
    static Strategy* felguard(PlayerbotAI* ai) { return new SummonFelguardStrategy(ai); }
};

class WarlockSoulstoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockSoulstoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["ss self"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_self;
        creators["ss master"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_master;
        creators["ss tank"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_tank;
        creators["ss healer"] = &WarlockSoulstoneStrategyFactoryInternal::soulstone_healer;
    }

private:
    static Strategy* soulstone_self(PlayerbotAI* ai) { return new SoulstoneSelfStrategy(ai); }
    static Strategy* soulstone_master(PlayerbotAI* ai) { return new SoulstoneMasterStrategy(ai); }
    static Strategy* soulstone_tank(PlayerbotAI* ai) { return new SoulstoneTankStrategy(ai); }
    static Strategy* soulstone_healer(PlayerbotAI* ai) { return new SoulstoneHealerStrategy(ai); }
};

class WarlockCurseStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockCurseStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["curse of agony"] = &WarlockCurseStrategyFactoryInternal::curse_of_agony;
        creators["curse of elements"] = &WarlockCurseStrategyFactoryInternal::curse_of_elements;
        creators["curse of doom"] = &WarlockCurseStrategyFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockCurseStrategyFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockCurseStrategyFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockCurseStrategyFactoryInternal::curse_of_weakness;
    }

private:
    static Strategy* curse_of_agony(PlayerbotAI* botAI) { return new WarlockCurseOfAgonyStrategy(botAI); }
    static Strategy* curse_of_elements(PlayerbotAI* botAI) { return new WarlockCurseOfTheElementsStrategy(botAI); }
    static Strategy* curse_of_doom(PlayerbotAI* botAI) { return new WarlockCurseOfDoomStrategy(botAI); }
    static Strategy* curse_of_exhaustion(PlayerbotAI* botAI) { return new WarlockCurseOfExhaustionStrategy(botAI); }
    static Strategy* curse_of_tongues(PlayerbotAI* botAI) { return new WarlockCurseOfTonguesStrategy(botAI); }
    static Strategy* curse_of_weakness(PlayerbotAI* botAI) { return new WarlockCurseOfWeaknessStrategy(botAI); }
};

class WarlockWeaponStoneStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarlockWeaponStoneStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["firestone"] = &WarlockWeaponStoneStrategyFactoryInternal::firestone;
        creators["spellstone"] = &WarlockWeaponStoneStrategyFactoryInternal::spellstone;
    }

private:
    static Strategy* firestone(PlayerbotAI* ai) { return new UseFirestoneStrategy(ai); }
    static Strategy* spellstone(PlayerbotAI* ai) { return new UseSpellstoneStrategy(ai); }
};

class WarlockTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarlockTriggerFactoryInternal()
    {
        creators["shadow trance"] = &WarlockTriggerFactoryInternal::shadow_trance;
        creators["demon armor"] = &WarlockTriggerFactoryInternal::demon_armor;
        creators["soul link"] = &WarlockTriggerFactoryInternal::soul_link;
        creators["no soul shard"] = &WarlockTriggerFactoryInternal::no_soul_shard;
        creators["too many soul shards"] = &WarlockTriggerFactoryInternal::too_many_soul_shards;
        creators["no healthstone"] = &WarlockTriggerFactoryInternal::HasHealthstone;
        creators["no firestone"] = &WarlockTriggerFactoryInternal::HasFirestone;
        creators["no spellstone"] = &WarlockTriggerFactoryInternal::HasSpellstone;
        creators["no soulstone"] = &WarlockTriggerFactoryInternal::OutOfSoulstone;
        creators["firestone"] = &WarlockTriggerFactoryInternal::firestone;
        creators["spellstone"] = &WarlockTriggerFactoryInternal::spellstone;
        creators["soulstone"] = &WarlockTriggerFactoryInternal::soulstone;
        creators["banish"] = &WarlockTriggerFactoryInternal::banish;
        creators["fear"] = &WarlockTriggerFactoryInternal::fear;
        creators["spell lock"] = &WarlockTriggerFactoryInternal::spell_lock;
        creators["devour magic purge"] = &WarlockTriggerFactoryInternal::devour_magic_purge;
        creators["devour magic cleanse"] = &WarlockTriggerFactoryInternal::devour_magic_cleanse;
        creators["backlash"] = &WarlockTriggerFactoryInternal::backlash;
        creators["corruption"] = &WarlockTriggerFactoryInternal::corruption;
        creators["corruption on attacker"] = &WarlockTriggerFactoryInternal::corruption_on_attacker;
        creators["immolate"] = &WarlockTriggerFactoryInternal::immolate;
        creators["immolate on attacker"] = &WarlockTriggerFactoryInternal::immolate_on_attacker;
        creators["unstable affliction"] = &WarlockTriggerFactoryInternal::unstable_affliction;
        creators["unstable affliction on attacker"] = &WarlockTriggerFactoryInternal::unstable_affliction_on_attacker;
        creators["haunt"] = &WarlockTriggerFactoryInternal::haunt;
        creators["decimation"] = &WarlockTriggerFactoryInternal::decimation;
        creators["life tap"] = &WarlockTriggerFactoryInternal::life_tap;
        creators["life tap glyph buff"] = &WarlockTriggerFactoryInternal::life_tap_glyph_buff;
        creators["molten core"] = &WarlockTriggerFactoryInternal::molten_core;
        creators["metamorphosis"] = &WarlockTriggerFactoryInternal::metamorphosis;
        creators["demonic empowerment"] = &WarlockTriggerFactoryInternal::demonic_empowerment;
        creators["immolation aura active"] = &WarlockTriggerFactoryInternal::immolation_aura_active;
        creators["metamorphosis not active"] = &WarlockTriggerFactoryInternal::metamorphosis_not_active;
        creators["meta melee flee check"] = &WarlockTriggerFactoryInternal::meta_melee_flee_check;
        creators["curse of agony"] = &WarlockTriggerFactoryInternal::curse_of_agony;
        creators["curse of agony on attacker"] = &WarlockTriggerFactoryInternal::curse_of_agony_on_attacker;
        creators["curse of the elements"] = &WarlockTriggerFactoryInternal::curse_of_the_elements;
        creators["curse of doom"] = &WarlockTriggerFactoryInternal::curse_of_doom;
        creators["curse of exhaustion"] = &WarlockTriggerFactoryInternal::curse_of_exhaustion;
        creators["curse of tongues"] = &WarlockTriggerFactoryInternal::curse_of_tongues;
        creators["curse of weakness"] = &WarlockTriggerFactoryInternal::curse_of_weakness;
        creators["wrong pet"] = &WarlockTriggerFactoryInternal::wrong_pet;
        creators["rain of fire channel check"] = &WarlockTriggerFactoryInternal::rain_of_fire_channel_check;
    }

private:
    static Trigger* shadow_trance(PlayerbotAI* botAI) { return new ShadowTranceTrigger(botAI); }
    static Trigger* demon_armor(PlayerbotAI* botAI) { return new DemonArmorTrigger(botAI); }
    static Trigger* soul_link(PlayerbotAI* botAI) { return new SoulLinkTrigger(botAI); }
    static Trigger* no_soul_shard(PlayerbotAI* botAI) { return new OutOfSoulShardsTrigger(botAI); }
    static Trigger* too_many_soul_shards(PlayerbotAI* botAI) { return new TooManySoulShardsTrigger(botAI); }
    static Trigger* HasHealthstone(PlayerbotAI* botAI) { return new HasHealthstoneTrigger(botAI); }
    static Trigger* HasFirestone(PlayerbotAI* botAI) { return new HasFirestoneTrigger(botAI); }
    static Trigger* HasSpellstone(PlayerbotAI* botAI) { return new HasSpellstoneTrigger(botAI); }
    static Trigger* OutOfSoulstone(PlayerbotAI* botAI) { return new OutOfSoulstoneTrigger(botAI); }
    static Trigger* firestone(PlayerbotAI* botAI) { return new FirestoneTrigger(botAI); }
    static Trigger* spellstone(PlayerbotAI* botAI) { return new SpellstoneTrigger(botAI); }
    static Trigger* soulstone(PlayerbotAI* botAI) { return new SoulstoneTrigger(botAI); }
    static Trigger* corruption(PlayerbotAI* botAI) { return new CorruptionTrigger(botAI); }
    static Trigger* corruption_on_attacker(PlayerbotAI* botAI) { return new CorruptionOnAttackerTrigger(botAI); }
    static Trigger* banish(PlayerbotAI* botAI) { return new BanishTrigger(botAI); }
    static Trigger* fear(PlayerbotAI* botAI) { return new FearTrigger(botAI); }
    static Trigger* spell_lock(PlayerbotAI* botAI) { return new SpellLockInterruptSpellTrigger(botAI); }
    static Trigger* devour_magic_purge(PlayerbotAI* botAI) { return new DevourMagicPurgeTrigger(botAI); }
    static Trigger* devour_magic_cleanse(PlayerbotAI* botAI) { return new DevourMagicCleanseTrigger(botAI); }
    static Trigger* backlash(PlayerbotAI* botAI) { return new BacklashTrigger(botAI); }
    static Trigger* immolate(PlayerbotAI* botAI) { return new ImmolateTrigger(botAI); }
    static Trigger* immolate_on_attacker(PlayerbotAI* ai) { return new ImmolateOnAttackerTrigger(ai); }
    static Trigger* unstable_affliction(PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); }
    static Trigger* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new UnstableAfflictionOnAttackerTrigger(ai); }
    static Trigger* haunt(PlayerbotAI* ai) { return new HauntTrigger(ai); }
    static Trigger* decimation(PlayerbotAI* ai) { return new DecimationTrigger(ai); }
    static Trigger* life_tap(PlayerbotAI* ai) { return new LifeTapTrigger(ai); }
    static Trigger* life_tap_glyph_buff(PlayerbotAI* ai) { return new LifeTapGlyphBuffTrigger(ai); }
    static Trigger* molten_core(PlayerbotAI* ai) { return new MoltenCoreTrigger(ai); }
    static Trigger* metamorphosis(PlayerbotAI* ai) { return new MetamorphosisTrigger(ai); }
    static Trigger* demonic_empowerment(PlayerbotAI* ai) { return new DemonicEmpowermentTrigger(ai); }
    static Trigger* immolation_aura_active(PlayerbotAI* ai) { return new ImmolationAuraActiveTrigger(ai); }
    static Trigger* metamorphosis_not_active(PlayerbotAI* ai) { return new MetamorphosisNotActiveTrigger(ai); }
    static Trigger* meta_melee_flee_check(PlayerbotAI* ai) { return new MetaMeleeEnemyTooCloseForSpellTrigger(ai); }
    static Trigger* curse_of_agony(PlayerbotAI* botAI) { return new CurseOfAgonyTrigger(botAI); }
    static Trigger* curse_of_agony_on_attacker(PlayerbotAI* botAI) { return new CurseOfAgonyOnAttackerTrigger(botAI); }
    static Trigger* curse_of_the_elements(PlayerbotAI* ai) { return new CurseOfTheElementsTrigger(ai); }
    static Trigger* curse_of_doom(PlayerbotAI* ai) { return new CurseOfDoomTrigger(ai); }
    static Trigger* curse_of_exhaustion(PlayerbotAI* ai) { return new CurseOfExhaustionTrigger(ai); }
    static Trigger* curse_of_tongues(PlayerbotAI* ai) { return new CurseOfTonguesTrigger(ai); }
    static Trigger* curse_of_weakness(PlayerbotAI* ai) { return new CurseOfWeaknessTrigger(ai); }
    static Trigger* wrong_pet(PlayerbotAI* ai) { return new WrongPetTrigger(ai); }
    static Trigger* rain_of_fire_channel_check(PlayerbotAI* ai) { return new RainOfFireChannelCheckTrigger(ai); }
};

SharedNamedObjectContextList<Strategy> WarlockAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> WarlockAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> WarlockAiObjectContext::sharedValueContexts;

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void WarlockAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void WarlockAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new WarlockStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCombatStrategyFactoryInternal());
    strategyContexts.Add(new WarlockPetStrategyFactoryInternal());
    strategyContexts.Add(new WarlockSoulstoneStrategyFactoryInternal());
    strategyContexts.Add(new WarlockCurseStrategyFactoryInternal());
    strategyContexts.Add(new WarlockWeaponStoneStrategyFactoryInternal());
}

void WarlockAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new WarlockTriggerFactoryInternal());
}

void WarlockAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
