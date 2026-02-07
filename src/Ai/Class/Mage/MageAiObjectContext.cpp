/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MageAiObjectContext.h"
#include "ArcaneMageStrategy.h"
#include "FireMageStrategy.h"
#include "FrostFireMageStrategy.h"
#include "FrostMageStrategy.h"
#include "GenericMageNonCombatStrategy.h"
#include "MageActions.h"
#include "MageTriggers.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"

class MageStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageStrategyFactoryInternal()
    {
        creators["nc"] = &MageStrategyFactoryInternal::nc;
        creators["pull"] = &MageStrategyFactoryInternal::pull;
        creators["aoe"] = &MageStrategyFactoryInternal::aoe;
        creators["cure"] = &MageStrategyFactoryInternal::cure;
        creators["buff"] = &MageStrategyFactoryInternal::buff;
        creators["boost"] = &MageStrategyFactoryInternal::boost;
        creators["cc"] = &MageStrategyFactoryInternal::cc;
        creators["firestarter"] = &MageStrategyFactoryInternal::firestarter;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericMageNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new MageAoeStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new MageCureStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new MageBuffStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new MageBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new MageCcStrategy(botAI); }
    static Strategy* firestarter(PlayerbotAI* botAI) { return new FirestarterStrategy(botAI); }
};

class MageCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["frost"] = &MageCombatStrategyFactoryInternal::frost;
        creators["fire"] = &MageCombatStrategyFactoryInternal::fire;
        creators["frostfire"] = &MageCombatStrategyFactoryInternal::frostfire;
        creators["arcane"] = &MageCombatStrategyFactoryInternal::arcane;
    }

private:
    static Strategy* frost(PlayerbotAI* botAI) { return new FrostMageStrategy(botAI); }
    static Strategy* fire(PlayerbotAI* botAI) { return new FireMageStrategy(botAI); }
    static Strategy* frostfire(PlayerbotAI* botAI) { return new FrostFireMageStrategy(botAI); }
    static Strategy* arcane(PlayerbotAI* botAI) { return new ArcaneMageStrategy(botAI); }
};

class MageBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    MageBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bmana"] = &MageBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &MageBuffStrategyFactoryInternal::bdps;
    }

private:
    static Strategy* bmana(PlayerbotAI* botAI) { return new MageBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new MageBuffDpsStrategy(botAI); }
};

class MageTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    MageTriggerFactoryInternal()
    {
        creators["fireball"] = &MageTriggerFactoryInternal::fireball;
        creators["pyroblast"] = &MageTriggerFactoryInternal::pyroblast;
        creators["combustion"] = &MageTriggerFactoryInternal::combustion;
        creators["fingers of frost"] = &MageTriggerFactoryInternal::fingers_of_frost;
        creators["brain freeze"] = &MageTriggerFactoryInternal::brain_freeze;
        creators["icy veins"] = &MageTriggerFactoryInternal::icy_veins;
        creators["cold snap"] = &MageTriggerFactoryInternal::cold_snap;
        creators["ice barrier"] = &MageTriggerFactoryInternal::ice_barrier;
        creators["arcane intellect"] = &MageTriggerFactoryInternal::arcane_intellect;
        creators["arcane intellect on party"] = &MageTriggerFactoryInternal::arcane_intellect_on_party;
        creators["mage armor"] = &MageTriggerFactoryInternal::mage_armor;
        creators["remove curse"] = &MageTriggerFactoryInternal::remove_curse;
        creators["remove curse on party"] = &MageTriggerFactoryInternal::remove_curse_on_party;
        creators["counterspell"] = &MageTriggerFactoryInternal::counterspell;
        creators["polymorph"] = &MageTriggerFactoryInternal::polymorph;
        creators["spellsteal"] = &MageTriggerFactoryInternal::spellsteal;
        creators["hot streak"] = &MageTriggerFactoryInternal::hot_streak;
        creators["living bomb"] = &MageTriggerFactoryInternal::living_bomb;
        creators["living bomb on attackers"] = &MageTriggerFactoryInternal::living_bomb_on_attackers;
        creators["missile barrage"] = &MageTriggerFactoryInternal::missile_barrage;
        creators["arcane blast"] = &MageTriggerFactoryInternal::arcane_blast;
        creators["counterspell on enemy healer"] = &MageTriggerFactoryInternal::counterspell_enemy_healer;
        creators["arcane power"] = &MageTriggerFactoryInternal::arcane_power;
        creators["presence of mind"] = &MageTriggerFactoryInternal::presence_of_mind;
        creators["fire ward"] = &MageTriggerFactoryInternal::fire_ward;
        creators["frost ward"] = &MageTriggerFactoryInternal::frost_ward;
        creators["arcane blast stack"] = &MageTriggerFactoryInternal::arcane_blast_stack;
        creators["mirror image"] = &MageTriggerFactoryInternal::mirror_image;
        creators["frost nova on target"] = &MageTriggerFactoryInternal::frost_nova_on_target;
        creators["frostbite on target"] = &MageTriggerFactoryInternal::frostbite_on_target;
        creators["no focus magic"] = &MageTriggerFactoryInternal::no_focus_magic;
        creators["frostfire bolt"] = &MageTriggerFactoryInternal::frostfire_bolt;
        creators["firestarter"] = &MageTriggerFactoryInternal::firestarter;
        creators["improved scorch"] = &MageTriggerFactoryInternal::improved_scorch;
        creators["flamestrike nearby"] = &MageTriggerFactoryInternal::flamestrike_nearby;
        creators["flamestrike active and medium aoe"] = &MageTriggerFactoryInternal::flamestrike_blizzard;
        creators["arcane blast 4 stacks and missile barrage"] = &MageTriggerFactoryInternal::arcane_blast_4_stacks_and_missile_barrage;
        creators["icy veins on cd"] = &MageTriggerFactoryInternal::icy_veins_on_cd;
        creators["deep freeze on cd"] = &MageTriggerFactoryInternal::deep_freeze_on_cd;
        creators["no mana gem"] = &MageTriggerFactoryInternal::NoManaGem;
        creators["blizzard channel check"] = &MageTriggerFactoryInternal::blizzard_channel_check;
        creators["blast wave off cd"] = &MageTriggerFactoryInternal::blast_wave_off_cd;
        creators["blast wave off cd and medium aoe"] = &MageTriggerFactoryInternal::blast_wave_off_cd_and_medium_aoe;
        creators["no firestarter strategy"] = &MageTriggerFactoryInternal::no_firestarter_strategy;
        creators["enemy is close and no firestarter strategy"] = &MageTriggerFactoryInternal::enemy_is_close_and_no_firestarter_strategy;
        creators["enemy too close for spell and no firestarter strategy"] = &MageTriggerFactoryInternal::enemy_too_close_for_spell_and_no_firestarter_strategy;
    }

private:
    static Trigger* presence_of_mind(PlayerbotAI* botAI) { return new PresenceOfMindTrigger(botAI); }
    static Trigger* frost_ward(PlayerbotAI* botAI) { return new FrostWardTrigger(botAI); }
    static Trigger* fire_ward(PlayerbotAI* botAI) { return new FireWardTrigger(botAI); }
    static Trigger* arcane_power(PlayerbotAI* botAI) { return new ArcanePowerTrigger(botAI); }
    static Trigger* hot_streak(PlayerbotAI* botAI) { return new HotStreakTrigger(botAI); }
    static Trigger* fireball(PlayerbotAI* botAI) { return new FireballTrigger(botAI); }
    static Trigger* pyroblast(PlayerbotAI* botAI) { return new PyroblastTrigger(botAI); }
    static Trigger* combustion(PlayerbotAI* botAI) { return new CombustionTrigger(botAI); }
    static Trigger* fingers_of_frost(PlayerbotAI* botAI) { return new FingersOfFrostTrigger(botAI); }
    static Trigger* brain_freeze(PlayerbotAI* botAI) { return new BrainFreezeTrigger(botAI); }
    static Trigger* icy_veins(PlayerbotAI* botAI) { return new IcyVeinsTrigger(botAI); }
    static Trigger* cold_snap(PlayerbotAI* botAI) { return new ColdSnapTrigger(botAI); }
    static Trigger* ice_barrier(PlayerbotAI* botAI) { return new IceBarrierTrigger(botAI); }
    static Trigger* arcane_intellect(PlayerbotAI* botAI) { return new ArcaneIntellectTrigger(botAI); }
    static Trigger* arcane_intellect_on_party(PlayerbotAI* botAI) { return new ArcaneIntellectOnPartyTrigger(botAI); }
    static Trigger* mage_armor(PlayerbotAI* botAI) { return new MageArmorTrigger(botAI); }
    static Trigger* remove_curse(PlayerbotAI* botAI) { return new RemoveCurseTrigger(botAI); }
    static Trigger* remove_curse_on_party(PlayerbotAI* botAI) { return new PartyMemberRemoveCurseTrigger(botAI); }
    static Trigger* counterspell(PlayerbotAI* botAI) { return new CounterspellInterruptSpellTrigger(botAI); }
    static Trigger* polymorph(PlayerbotAI* botAI) { return new PolymorphTrigger(botAI); }
    static Trigger* spellsteal(PlayerbotAI* botAI) { return new SpellstealTrigger(botAI); }
    static Trigger* living_bomb(PlayerbotAI* botAI) { return new LivingBombTrigger(botAI); }
    static Trigger* living_bomb_on_attackers(PlayerbotAI* botAI) { return new LivingBombOnAttackersTrigger(botAI); }
    static Trigger* missile_barrage(PlayerbotAI* botAI) { return new MissileBarrageTrigger(botAI); }
    static Trigger* arcane_blast(PlayerbotAI* botAI) { return new ArcaneBlastTrigger(botAI); }
    static Trigger* counterspell_enemy_healer(PlayerbotAI* botAI) { return new CounterspellEnemyHealerTrigger(botAI); }
    static Trigger* arcane_blast_stack(PlayerbotAI* botAI) { return new ArcaneBlastStackTrigger(botAI); }
    static Trigger* mirror_image(PlayerbotAI* botAI) { return new MirrorImageTrigger(botAI); }
    static Trigger* frost_nova_on_target(PlayerbotAI* botAI) { return new FrostNovaOnTargetTrigger(botAI); }
    static Trigger* frostbite_on_target(PlayerbotAI* botAI) { return new FrostbiteOnTargetTrigger(botAI); }
    static Trigger* no_focus_magic(PlayerbotAI* botAI) { return new NoFocusMagicTrigger(botAI); }
    static Trigger* frostfire_bolt(PlayerbotAI* botAI) { return new FrostfireBoltTrigger(botAI); }
    static Trigger* improved_scorch(PlayerbotAI* botAI) { return new ImprovedScorchTrigger(botAI); }
    static Trigger* firestarter(PlayerbotAI* botAI) { return new FirestarterTrigger(botAI); }
    static Trigger* flamestrike_nearby(PlayerbotAI* botAI) { return new FlamestrikeNearbyTrigger(botAI); }
    static Trigger* flamestrike_blizzard(PlayerbotAI* botAI) { return new FlamestrikeBlizzardTrigger(botAI); }
    static Trigger* arcane_blast_4_stacks_and_missile_barrage(PlayerbotAI* botAI) { return new ArcaneBlast4StacksAndMissileBarrageTrigger(botAI); }
    static Trigger* icy_veins_on_cd(PlayerbotAI* botAI) { return new IcyVeinsCooldownTrigger(botAI); }
    static Trigger* deep_freeze_on_cd(PlayerbotAI* botAI) { return new DeepFreezeCooldownTrigger(botAI); }
    static Trigger* NoManaGem(PlayerbotAI* botAI) { return new NoManaGemTrigger(botAI); }
    static Trigger* blizzard_channel_check(PlayerbotAI* botAI) { return new BlizzardChannelCheckTrigger(botAI); }
    static Trigger* blast_wave_off_cd(PlayerbotAI* botAI) { return new BlastWaveOffCdTrigger(botAI); }
    static Trigger* blast_wave_off_cd_and_medium_aoe(PlayerbotAI* botAI) { return new BlastWaveOffCdTriggerAndMediumAoeTrigger(botAI); }
    static Trigger* no_firestarter_strategy(PlayerbotAI* botAI) { return new NoFirestarterStrategyTrigger(botAI); }
    static Trigger* enemy_is_close_and_no_firestarter_strategy(PlayerbotAI* botAI) { return new EnemyIsCloseAndNoFirestarterStrategyTrigger(botAI); }
    static Trigger* enemy_too_close_for_spell_and_no_firestarter_strategy(PlayerbotAI* botAI) { return new EnemyTooCloseForSpellAndNoFirestarterStrategyTrigger(botAI); }
};

SharedNamedObjectContextList<Strategy> MageAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> MageAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> MageAiObjectContext::sharedValueContexts;

MageAiObjectContext::MageAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void MageAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void MageAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new MageStrategyFactoryInternal());
    strategyContexts.Add(new MageCombatStrategyFactoryInternal());
    strategyContexts.Add(new MageBuffStrategyFactoryInternal());
}

void MageAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new MageTriggerFactoryInternal());
}

void MageAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
