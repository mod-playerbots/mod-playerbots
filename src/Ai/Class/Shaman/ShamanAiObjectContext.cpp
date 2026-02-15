/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ShamanAiObjectContext.h"

#include "ElementalShamanStrategy.h"
#include "GenericShamanStrategy.h"
#include "RestoShamanStrategy.h"
#include "EnhancementShamanStrategy.h"
#include "NamedObjectContext.h"
#include "ShamanActions.h"
#include "ShamanNonCombatStrategy.h"
#include "ShamanTriggers.h"
#include "TotemsShamanStrategy.h"

class ShamanStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanStrategyFactoryInternal()
    {
        creators["nc"] = &ShamanStrategyFactoryInternal::nc;
        creators["aoe"] = &ShamanStrategyFactoryInternal::aoe;
        creators["cure"] = &ShamanStrategyFactoryInternal::cure;
        creators["healer dps"] = &ShamanStrategyFactoryInternal::healer_dps;
        creators["boost"] = &ShamanStrategyFactoryInternal::boost;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new ShamanNonCombatStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new ShamanAoeStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new ShamanCureStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new ShamanHealerDpsStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new ShamanBoostStrategy(botAI); }
};

class ShamanCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["heal"] = &ShamanCombatStrategyFactoryInternal::resto;
        creators["melee"] = &ShamanCombatStrategyFactoryInternal::enh;
        creators["dps"] = &ShamanCombatStrategyFactoryInternal::enh;
        creators["caster"] = &ShamanCombatStrategyFactoryInternal::ele;
        //creators["offheal"] = &ShamanCombatStrategyFactoryInternal::offheal;
        creators["resto"] = &ShamanCombatStrategyFactoryInternal::resto;
        creators["enh"] = &ShamanCombatStrategyFactoryInternal::enh;
        creators["ele"] = &ShamanCombatStrategyFactoryInternal::ele;
    }

private:
    static Strategy* resto(PlayerbotAI* botAI) { return new RestoShamanStrategy(botAI); }
    static Strategy* enh(PlayerbotAI* botAI) { return new EnhancementShamanStrategy(botAI); }
    static Strategy* ele(PlayerbotAI* botAI) { return new ElementalShamanStrategy(botAI); }
};

class ShamanEarthTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanEarthTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["strength of earth"] = &ShamanEarthTotemStrategyFactoryInternal::strength_of_earth_totem;
        creators["stoneskin"] = &ShamanEarthTotemStrategyFactoryInternal::stoneclaw_totem;
        creators["tremor"] = &ShamanEarthTotemStrategyFactoryInternal::earth_totem;
        creators["earthbind"] = &ShamanEarthTotemStrategyFactoryInternal::earthbind_totem;
    }

private:
    static Strategy* strength_of_earth_totem(PlayerbotAI* botAI) { return new StrengthOfEarthTotemStrategy(botAI); }
    static Strategy* stoneclaw_totem(PlayerbotAI* botAI) { return new StoneclawTotemStrategy(botAI); }
    static Strategy* earth_totem(PlayerbotAI* botAI) { return new EarthTotemStrategy(botAI); }
    static Strategy* earthbind_totem(PlayerbotAI* botAI) { return new EarthbindTotemStrategy(botAI); }
};

class ShamanFireTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanFireTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["searing"] = &ShamanFireTotemStrategyFactoryInternal::searing_totem;
        creators["magma"] = &ShamanFireTotemStrategyFactoryInternal::magma_totem;
        creators["flametongue"] = &ShamanFireTotemStrategyFactoryInternal::flametongue_totem;
        creators["wrath"] = &ShamanFireTotemStrategyFactoryInternal::totem_of_wrath;
        creators["frost resistance"] = &ShamanFireTotemStrategyFactoryInternal::frost_resistance_totem;
    }

private:
    static Strategy* searing_totem(PlayerbotAI* botAI) { return new SearingTotemStrategy(botAI); }
    static Strategy* magma_totem(PlayerbotAI* botAI) { return new MagmaTotemStrategy(botAI); }
    static Strategy* flametongue_totem(PlayerbotAI* botAI) { return new FlametongueTotemStrategy(botAI); }
    static Strategy* totem_of_wrath(PlayerbotAI* botAI) { return new TotemOfWrathStrategy(botAI); }
    static Strategy* frost_resistance_totem(PlayerbotAI* botAI) { return new FrostResistanceTotemStrategy(botAI); }
};

class ShamanWaterTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanWaterTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["healing stream"] = &ShamanWaterTotemStrategyFactoryInternal::healing_stream_totem;
        creators["mana spring"] = &ShamanWaterTotemStrategyFactoryInternal::mana_spring_totem;
        creators["cleansing"] = &ShamanWaterTotemStrategyFactoryInternal::cleansing_totem;
        creators["fire resistance"] = &ShamanWaterTotemStrategyFactoryInternal::fire_resistance_totem;
    }

private:
    static Strategy* healing_stream_totem(PlayerbotAI* botAI) { return new HealingStreamTotemStrategy(botAI); }
    static Strategy* mana_spring_totem(PlayerbotAI* botAI) { return new ManaSpringTotemStrategy(botAI); }
    static Strategy* cleansing_totem(PlayerbotAI* botAI) { return new CleansingTotemStrategy(botAI); }
    static Strategy* fire_resistance_totem(PlayerbotAI* botAI) { return new FireResistanceTotemStrategy(botAI); }
};

class ShamanAirTotemStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    ShamanAirTotemStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["wrath of air"] = &ShamanAirTotemStrategyFactoryInternal::wrath_of_air_totem;
        creators["windfury"] = &ShamanAirTotemStrategyFactoryInternal::windfury_totem;
        creators["nature resistance"] = &ShamanAirTotemStrategyFactoryInternal::nature_resistance_totem;
        creators["grounding"] = &ShamanAirTotemStrategyFactoryInternal::grounding_totem;
    }

private:
    static Strategy* wrath_of_air_totem(PlayerbotAI* botAI) { return new WrathOfAirTotemStrategy(botAI); }
    static Strategy* windfury_totem(PlayerbotAI* botAI) { return new WindfuryTotemStrategy(botAI); }
    static Strategy* nature_resistance_totem(PlayerbotAI* botAI) { return new NatureResistanceTotemStrategy(botAI); }
    static Strategy* grounding_totem(PlayerbotAI* botAI) { return new GroundingTotemStrategy(botAI); }
};

class ShamanATriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    ShamanATriggerFactoryInternal()
    {
        creators["wind shear"] = &ShamanATriggerFactoryInternal::wind_shear;
        creators["purge"] = &ShamanATriggerFactoryInternal::purge;
        creators["main hand weapon no imbue"] = &ShamanATriggerFactoryInternal::main_hand_weapon_no_imbue;
        creators["off hand weapon no imbue"] = &ShamanATriggerFactoryInternal::off_hand_weapon_no_imbue;
        creators["water shield"] = &ShamanATriggerFactoryInternal::water_shield;
        creators["lightning shield"] = &ShamanATriggerFactoryInternal::lightning_shield;
        creators["water breathing"] = &ShamanATriggerFactoryInternal::water_breathing;
        creators["water walking"] = &ShamanATriggerFactoryInternal::water_walking;
        creators["water breathing on party"] = &ShamanATriggerFactoryInternal::water_breathing_on_party;
        creators["water walking on party"] = &ShamanATriggerFactoryInternal::water_walking_on_party;
        creators["cleanse spirit poison"] = &ShamanATriggerFactoryInternal::cleanse_poison;
        creators["cleanse spirit curse"] = &ShamanATriggerFactoryInternal::cleanse_curse;
        creators["cleanse spirit disease"] = &ShamanATriggerFactoryInternal::cleanse_disease;
        creators["party member cleanse spirit poison"] = &ShamanATriggerFactoryInternal::party_member_cleanse_poison;
        creators["party member cleanse spirit curse"] = &ShamanATriggerFactoryInternal::party_member_cleanse_curse;
        creators["party member cleanse spirit disease"] = &ShamanATriggerFactoryInternal::party_member_cleanse_disease;
        creators["shock"] = &ShamanATriggerFactoryInternal::shock;
        creators["frost shock snare"] = &ShamanATriggerFactoryInternal::frost_shock_snare;
        creators["heroism"] = &ShamanATriggerFactoryInternal::heroism;
        creators["bloodlust"] = &ShamanATriggerFactoryInternal::bloodlust;
        creators["elemental mastery"] = &ShamanATriggerFactoryInternal::elemental_mastery;
        creators["wind shear on enemy healer"] = &ShamanATriggerFactoryInternal::wind_shear_on_enemy_healer;
        creators["cure poison"] = &ShamanATriggerFactoryInternal::cure_poison;
        creators["party member cure poison"] = &ShamanATriggerFactoryInternal::party_member_cure_poison;
        creators["cure disease"] = &ShamanATriggerFactoryInternal::cure_disease;
        creators["party member cure disease"] = &ShamanATriggerFactoryInternal::party_member_cure_disease;
        creators["earth shield on main tank"] = &ShamanATriggerFactoryInternal::earth_shield_on_main_tank;
        creators["maelstrom weapon 3"] = &ShamanATriggerFactoryInternal::maelstrom_weapon_3;
        creators["maelstrom weapon 4"] = &ShamanATriggerFactoryInternal::maelstrom_weapon_4;
        creators["maelstrom weapon 5"] = &ShamanATriggerFactoryInternal::maelstrom_weapon_5;
        creators["flame shock"] = &ShamanATriggerFactoryInternal::flame_shock;
        creators["fire elemental totem"] = &ShamanATriggerFactoryInternal::fire_elemental_totem;
        creators["earth shock execute"] = &ShamanATriggerFactoryInternal::earth_shock_execute;
        creators["spirit walk ready"] = &ShamanATriggerFactoryInternal::spirit_walk_ready;
        creators["chain lightning no cd"] = &ShamanATriggerFactoryInternal::chain_lightning_no_cd;
        creators["call of the elements and enemy within melee"] = &ShamanATriggerFactoryInternal::call_of_the_elements_and_enemy_within_melee;
        creators["maelstrom weapon 5 and medium aoe"] = &ShamanATriggerFactoryInternal::maelstrom_weapon_5_and_medium_aoe;
        creators["maelstrom weapon 4 and medium aoe"] = &ShamanATriggerFactoryInternal::maelstrom_weapon_4_and_medium_aoe;
        creators["call of the elements"] = &ShamanATriggerFactoryInternal::call_of_the_elements;
        creators["totemic recall"] = &ShamanATriggerFactoryInternal::totemic_recall;
        creators["no earth totem"] = &ShamanATriggerFactoryInternal::no_earth_totem;
        creators["no fire totem"] = &ShamanATriggerFactoryInternal::no_fire_totem;
        creators["no water totem"] = &ShamanATriggerFactoryInternal::no_water_totem;
        creators["no air totem"] = &ShamanATriggerFactoryInternal::no_air_totem;
        creators["set strength of earth totem"] = &ShamanATriggerFactoryInternal::set_strength_of_earth_totem;
        creators["set stoneskin totem"] = &ShamanATriggerFactoryInternal::set_stoneskin_totem;
        creators["set tremor totem"] = &ShamanATriggerFactoryInternal::set_tremor_totem;
        creators["set earthbind totem"] = &ShamanATriggerFactoryInternal::set_earthbind_totem;
        creators["set searing totem"] = &ShamanATriggerFactoryInternal::set_searing_totem;
        creators["set magma totem"] = &ShamanATriggerFactoryInternal::set_magma_totem;
        creators["set flametongue totem"] = &ShamanATriggerFactoryInternal::set_flametongue_totem;
        creators["set totem of wrath"] = &ShamanATriggerFactoryInternal::set_totem_of_wrath;
        creators["set frost resistance totem"] = &ShamanATriggerFactoryInternal::set_frost_resistance_totem;
        creators["set healing stream totem"] = &ShamanATriggerFactoryInternal::set_healing_stream_totem;
        creators["set mana spring totem"] = &ShamanATriggerFactoryInternal::set_mana_spring_totem;
        creators["set cleansing totem"] = &ShamanATriggerFactoryInternal::set_cleansing_totem;
        creators["set fire resistance totem"] = &ShamanATriggerFactoryInternal::set_fire_resistance_totem;
        creators["set wrath of air totem"] = &ShamanATriggerFactoryInternal::set_wrath_of_air_totem;
        creators["set windfury totem"] = &ShamanATriggerFactoryInternal::set_windfury_totem;
        creators["set nature resistance totem"] = &ShamanATriggerFactoryInternal::set_nature_resistance_totem;
        creators["set grounding totem"] = &ShamanATriggerFactoryInternal::set_grounding_totem;
    }

private:
    static Trigger* maelstrom_weapon_3(PlayerbotAI* botAI) { return new MaelstromWeaponTrigger(botAI, 3); }
    static Trigger* maelstrom_weapon_4(PlayerbotAI* botAI) { return new MaelstromWeaponTrigger(botAI, 4); }
    static Trigger* maelstrom_weapon_5(PlayerbotAI* botAI) { return new MaelstromWeaponTrigger(botAI, 5); }
    static Trigger* heroism(PlayerbotAI* botAI) { return new HeroismTrigger(botAI); }
    static Trigger* bloodlust(PlayerbotAI* botAI) { return new BloodlustTrigger(botAI); }
    static Trigger* elemental_mastery(PlayerbotAI* botAI) { return new ElementalMasteryTrigger(botAI); }
    static Trigger* party_member_cleanse_disease(PlayerbotAI* botAI) { return new PartyMemberCleanseSpiritDiseaseTrigger(botAI); }
    static Trigger* party_member_cleanse_curse(PlayerbotAI* botAI) { return new PartyMemberCleanseSpiritCurseTrigger(botAI); }
    static Trigger* party_member_cleanse_poison(PlayerbotAI* botAI) { return new PartyMemberCleanseSpiritPoisonTrigger(botAI); }
    static Trigger* cleanse_disease(PlayerbotAI* botAI) { return new CleanseSpiritDiseaseTrigger(botAI); }
    static Trigger* cleanse_curse(PlayerbotAI* botAI) { return new CleanseSpiritCurseTrigger(botAI); }
    static Trigger* cleanse_poison(PlayerbotAI* botAI) { return new CleanseSpiritPoisonTrigger(botAI); }
    static Trigger* water_breathing(PlayerbotAI* botAI) { return new WaterBreathingTrigger(botAI); }
    static Trigger* water_walking(PlayerbotAI* botAI) { return new WaterWalkingTrigger(botAI); }
    static Trigger* water_breathing_on_party(PlayerbotAI* botAI) { return new WaterBreathingOnPartyTrigger(botAI); }
    static Trigger* water_walking_on_party(PlayerbotAI* botAI) { return new WaterWalkingOnPartyTrigger(botAI); }
    static Trigger* wind_shear(PlayerbotAI* botAI) { return new WindShearInterruptSpellTrigger(botAI); }
    static Trigger* purge(PlayerbotAI* botAI) { return new PurgeTrigger(botAI); }
    static Trigger* main_hand_weapon_no_imbue(PlayerbotAI* botAI) { return new MainHandWeaponNoImbueTrigger(botAI); }
    static Trigger* off_hand_weapon_no_imbue(PlayerbotAI* botAI) { return new OffHandWeaponNoImbueTrigger(botAI); }
    static Trigger* water_shield(PlayerbotAI* botAI) { return new WaterShieldTrigger(botAI); }
    static Trigger* lightning_shield(PlayerbotAI* botAI) { return new LightningShieldTrigger(botAI); }
    static Trigger* shock(PlayerbotAI* botAI) { return new ShockTrigger(botAI); }
    static Trigger* frost_shock_snare(PlayerbotAI* botAI) { return new FrostShockSnareTrigger(botAI); }
    static Trigger* wind_shear_on_enemy_healer(PlayerbotAI* botAI) { return new WindShearInterruptEnemyHealerSpellTrigger(botAI); }
    static Trigger* cure_poison(PlayerbotAI* botAI) { return new CurePoisonTrigger(botAI); }
    static Trigger* party_member_cure_poison(PlayerbotAI* botAI) { return new PartyMemberCurePoisonTrigger(botAI); }
    static Trigger* cure_disease(PlayerbotAI* botAI) { return new CureDiseaseTrigger(botAI); }
    static Trigger* party_member_cure_disease(PlayerbotAI* botAI) { return new PartyMemberCureDiseaseTrigger(botAI); }
    static Trigger* earth_shield_on_main_tank(PlayerbotAI* ai) { return new EarthShieldOnMainTankTrigger(ai); }
    static Trigger* flame_shock(PlayerbotAI* ai) { return new FlameShockTrigger(ai); }
    static Trigger* fire_elemental_totem(PlayerbotAI* botAI) { return new FireElementalTotemTrigger(botAI); }
    static Trigger* earth_shock_execute(PlayerbotAI* botAI) { return new EarthShockExecuteTrigger(botAI); }
    static Trigger* spirit_walk_ready(PlayerbotAI* ai) { return new SpiritWalkTrigger(ai); }
    static Trigger* chain_lightning_no_cd(PlayerbotAI* ai) { return new ChainLightningNoCdTrigger(ai); }
    static Trigger* call_of_the_elements_and_enemy_within_melee(PlayerbotAI* ai) { return new CallOfTheElementsAndEnemyWithinMeleeTrigger(ai); }
    static Trigger* maelstrom_weapon_5_and_medium_aoe(PlayerbotAI* ai) { return new MaelstromWeapon5AndMediumAoeTrigger(ai); }
    static Trigger* maelstrom_weapon_4_and_medium_aoe(PlayerbotAI* ai) { return new MaelstromWeapon4AndMediumAoeTrigger(ai); }
    static Trigger* call_of_the_elements(PlayerbotAI* ai) { return new CallOfTheElementsTrigger(ai); }
    static Trigger* totemic_recall(PlayerbotAI* ai) { return new TotemicRecallTrigger(ai); }
    static Trigger* no_earth_totem(PlayerbotAI* ai) { return new NoEarthTotemTrigger(ai); }
    static Trigger* no_fire_totem(PlayerbotAI* ai) { return new NoFireTotemTrigger(ai); }
    static Trigger* no_water_totem(PlayerbotAI* ai) { return new NoWaterTotemTrigger(ai); }
    static Trigger* no_air_totem(PlayerbotAI* ai) { return new NoAirTotemTrigger(ai); }
    static Trigger* set_strength_of_earth_totem(PlayerbotAI* ai) { return new SetStrengthOfEarthTotemTrigger(ai); }
    static Trigger* set_stoneskin_totem(PlayerbotAI* ai) { return new SetStoneskinTotemTrigger(ai); }
    static Trigger* set_tremor_totem(PlayerbotAI* ai) { return new SetTremorTotemTrigger(ai); }
    static Trigger* set_earthbind_totem(PlayerbotAI* ai) { return new SetEarthbindTotemTrigger(ai); }
    static Trigger* set_searing_totem(PlayerbotAI* ai) { return new SetSearingTotemTrigger(ai); }
    static Trigger* set_magma_totem(PlayerbotAI* ai) { return new SetMagmaTotemTrigger(ai); }
    static Trigger* set_flametongue_totem(PlayerbotAI* ai) { return new SetFlametongueTotemTrigger(ai); }
    static Trigger* set_totem_of_wrath(PlayerbotAI* ai) { return new SetTotemOfWrathTrigger(ai); }
    static Trigger* set_frost_resistance_totem(PlayerbotAI* ai) { return new SetFrostResistanceTotemTrigger(ai); }
    static Trigger* set_healing_stream_totem(PlayerbotAI* ai) { return new SetHealingStreamTotemTrigger(ai); }
    static Trigger* set_mana_spring_totem(PlayerbotAI* ai) { return new SetManaSpringTotemTrigger(ai); }
    static Trigger* set_cleansing_totem(PlayerbotAI* ai) { return new SetCleansingTotemTrigger(ai); }
    static Trigger* set_fire_resistance_totem(PlayerbotAI* ai) { return new SetFireResistanceTotemTrigger(ai); }
    static Trigger* set_wrath_of_air_totem(PlayerbotAI* ai) { return new SetWrathOfAirTotemTrigger(ai); }
    static Trigger* set_windfury_totem(PlayerbotAI* ai) { return new SetWindfuryTotemTrigger(ai); }
    static Trigger* set_nature_resistance_totem(PlayerbotAI* ai) { return new SetNatureResistanceTotemTrigger(ai); }
    static Trigger* set_grounding_totem(PlayerbotAI* ai) { return new SetGroundingTotemTrigger(ai); }
};

SharedNamedObjectContextList<Strategy> ShamanAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> ShamanAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> ShamanAiObjectContext::sharedValueContexts;

ShamanAiObjectContext::ShamanAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void ShamanAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void ShamanAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new ShamanStrategyFactoryInternal());
    strategyContexts.Add(new ShamanCombatStrategyFactoryInternal());
    strategyContexts.Add(new ShamanEarthTotemStrategyFactoryInternal());
    strategyContexts.Add(new ShamanFireTotemStrategyFactoryInternal());
    strategyContexts.Add(new ShamanWaterTotemStrategyFactoryInternal());
    strategyContexts.Add(new ShamanAirTotemStrategyFactoryInternal());
}

void ShamanAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new ShamanATriggerFactoryInternal());
}

void ShamanAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
