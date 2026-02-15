/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinAiObjectContext.h"

#include "DpsPaladinStrategy.h"
#include "GenericPaladinNonCombatStrategy.h"
#include "HealPaladinStrategy.h"
#include "NamedObjectContext.h"
#include "OffhealRetPaladinStrategy.h"
#include "PaladinActions.h"
#include "PaladinBuffStrategies.h"
#include "PaladinTriggers.h"
#include "TankPaladinStrategy.h"

class PaladinStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinStrategyFactoryInternal()
    {
        creators["nc"] = &PaladinStrategyFactoryInternal::nc;
        creators["cure"] = &PaladinStrategyFactoryInternal::cure;
        creators["boost"] = &PaladinStrategyFactoryInternal::boost;
        creators["cc"] = &PaladinStrategyFactoryInternal::cc;
        creators["bthreat"] = &PaladinStrategyFactoryInternal::bthreat;
        creators["healer dps"] = &PaladinStrategyFactoryInternal::healer_dps;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericPaladinNonCombatStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new PaladinCureStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new PaladinBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new PaladinCcStrategy(botAI); }
    static Strategy* bthreat(PlayerbotAI* botAI) { return new PaladinBuffThreatStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new PaladinHealerDpsStrategy(botAI); }
};

class PaladinResistanceStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinResistanceStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["rshadow"] = &PaladinResistanceStrategyFactoryInternal::rshadow;
        creators["rfrost"] = &PaladinResistanceStrategyFactoryInternal::rfrost;
        creators["rfire"] = &PaladinResistanceStrategyFactoryInternal::rfire;
        creators["baoe"] = &PaladinResistanceStrategyFactoryInternal::baoe;
        creators["barmor"] = &PaladinResistanceStrategyFactoryInternal::barmor;
        creators["bcast"] = &PaladinResistanceStrategyFactoryInternal::bcast;
        creators["bspeed"] = &PaladinResistanceStrategyFactoryInternal::bspeed;
    }

private:
    static Strategy* rshadow(PlayerbotAI* botAI) { return new PaladinShadowResistanceStrategy(botAI); }
    static Strategy* rfrost(PlayerbotAI* botAI) { return new PaladinFrostResistanceStrategy(botAI); }
    static Strategy* rfire(PlayerbotAI* botAI) { return new PaladinFireResistanceStrategy(botAI); }
    static Strategy* baoe(PlayerbotAI* botAI) { return new PaladinBuffAoeStrategy(botAI); }
    static Strategy* barmor(PlayerbotAI* botAI) { return new PaladinBuffArmorStrategy(botAI); }
    static Strategy* bcast(PlayerbotAI* botAI) { return new PaladinBuffCastStrategy(botAI); }
    static Strategy* bspeed(PlayerbotAI* botAI) { return new PaladinBuffSpeedStrategy(botAI); }
};

class PaladinBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bhealth"] = &PaladinBuffStrategyFactoryInternal::bhealth;
        creators["bmana"] = &PaladinBuffStrategyFactoryInternal::bmana;
        creators["bdps"] = &PaladinBuffStrategyFactoryInternal::bdps;
        creators["bstats"] = &PaladinBuffStrategyFactoryInternal::bstats;
    }

private:
    static Strategy* bhealth(PlayerbotAI* botAI) { return new PaladinBuffHealthStrategy(botAI); }
    static Strategy* bmana(PlayerbotAI* botAI) { return new PaladinBuffManaStrategy(botAI); }
    static Strategy* bdps(PlayerbotAI* botAI) { return new PaladinBuffDpsStrategy(botAI); }
    static Strategy* bstats(PlayerbotAI* botAI) { return new PaladinBuffStatsStrategy(botAI); }
};

class PaladinCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    PaladinCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &PaladinCombatStrategyFactoryInternal::tank;
        creators["dps"] = &PaladinCombatStrategyFactoryInternal::dps;
        creators["heal"] = &PaladinCombatStrategyFactoryInternal::heal;
        creators["offheal"] = &PaladinCombatStrategyFactoryInternal::offheal;
    }

private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankPaladinStrategy(botAI); }
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsPaladinStrategy(botAI); }
    static Strategy* heal(PlayerbotAI* botAI) { return new HealPaladinStrategy(botAI); }
    static Strategy* offheal(PlayerbotAI* botAI) { return new OffhealRetPaladinStrategy(botAI); }
};

class PaladinTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    PaladinTriggerFactoryInternal()
    {
        creators["judgement"] = &PaladinTriggerFactoryInternal::judgement;
        creators["judgement of wisdom"] = &PaladinTriggerFactoryInternal::judgement_of_wisdom;
        creators["judgement of light"] = &PaladinTriggerFactoryInternal::judgement_of_light;
        creators["blessing"] = &PaladinTriggerFactoryInternal::blessing;
        creators["seal"] = &PaladinTriggerFactoryInternal::seal;
        creators["art of war"] = &PaladinTriggerFactoryInternal::art_of_war;
        creators["blessing on party"] = &PaladinTriggerFactoryInternal::blessing_on_party;
        creators["crusader aura"] = &PaladinTriggerFactoryInternal::crusader_aura;
        creators["retribution aura"] = &PaladinTriggerFactoryInternal::retribution_aura;
        creators["devotion aura"] = &PaladinTriggerFactoryInternal::devotion_aura;
        creators["sanctity aura"] = &PaladinTriggerFactoryInternal::sanctity_aura;
        creators["concentration aura"] = &PaladinTriggerFactoryInternal::concentration_aura;
        creators["shadow resistance aura"] = &PaladinTriggerFactoryInternal::shadow_resistance_aura;
        creators["frost resistance aura"] = &PaladinTriggerFactoryInternal::frost_resistance_aura;
        creators["fire resistance aura"] = &PaladinTriggerFactoryInternal::fire_resistance_aura;
        creators["hammer of justice snare"] = &PaladinTriggerFactoryInternal::hammer_of_justice_snare;
        creators["hammer of justice interrupt"] = &PaladinTriggerFactoryInternal::hammer_of_justice_interrupt;
        creators["cleanse cure disease"] = &PaladinTriggerFactoryInternal::CleanseCureDisease;
        creators["cleanse party member cure disease"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberDisease;
        creators["cleanse cure poison"] = &PaladinTriggerFactoryInternal::CleanseCurePoison;
        creators["cleanse party member cure poison"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberPoison;
        creators["cleanse cure magic"] = &PaladinTriggerFactoryInternal::CleanseCureMagic;
        creators["cleanse party member cure magic"] = &PaladinTriggerFactoryInternal::CleanseCurePartyMemberMagic;
        creators["righteous fury"] = &PaladinTriggerFactoryInternal::righteous_fury;
        creators["holy shield"] = &PaladinTriggerFactoryInternal::holy_shield;
        creators["hammer of justice on enemy healer"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_enemy_target;
        creators["hammer of justice on snare target"] =
            &PaladinTriggerFactoryInternal::hammer_of_justice_on_snare_target;
        creators["divine favor"] = &PaladinTriggerFactoryInternal::divine_favor;
        creators["turn undead"] = &PaladinTriggerFactoryInternal::turn_undead;
        creators["avenger's shield"] = &PaladinTriggerFactoryInternal::avenger_shield;
        creators["consecration"] = &PaladinTriggerFactoryInternal::consecration;
        creators["repentance on enemy healer"] = &PaladinTriggerFactoryInternal::repentance_on_enemy_healer;
        creators["repentance on snare target"] = &PaladinTriggerFactoryInternal::repentance_on_snare_target;
        creators["repentance interrupt"] = &PaladinTriggerFactoryInternal::repentance_interrupt;
        creators["beacon of light on main tank"] = &PaladinTriggerFactoryInternal::beacon_of_light_on_main_tank;
        creators["sacred shield on main tank"] = &PaladinTriggerFactoryInternal::sacred_shield_on_main_tank;

        creators["blessing of kings on party"] = &PaladinTriggerFactoryInternal::blessing_of_kings_on_party;
        creators["blessing of wisdom on party"] = &PaladinTriggerFactoryInternal::blessing_of_wisdom_on_party;
        creators["blessing of might on party"] = &PaladinTriggerFactoryInternal::blessing_of_might_on_party;
        creators["blessing of sanctuary on party"] = &PaladinTriggerFactoryInternal::blessing_of_sanctuary_on_party;

        creators["avenging wrath"] = &PaladinTriggerFactoryInternal::avenging_wrath;
    }

private:
    static Trigger* turn_undead(PlayerbotAI* botAI) { return new TurnUndeadTrigger(botAI); }
    static Trigger* divine_favor(PlayerbotAI* botAI) { return new DivineFavorTrigger(botAI); }
    static Trigger* holy_shield(PlayerbotAI* botAI) { return new HolyShieldTrigger(botAI); }
    static Trigger* righteous_fury(PlayerbotAI* botAI) { return new RighteousFuryTrigger(botAI); }
    static Trigger* judgement(PlayerbotAI* botAI) { return new JudgementTrigger(botAI); }
    static Trigger* judgement_of_wisdom(PlayerbotAI* botAI) { return new JudgementOfWisdomTrigger(botAI); }
    static Trigger* judgement_of_light(PlayerbotAI* botAI) { return new JudgementOfLightTrigger(botAI); }
    static Trigger* blessing(PlayerbotAI* botAI) { return new BlessingTrigger(botAI); }
    static Trigger* seal(PlayerbotAI* botAI) { return new SealTrigger(botAI); }
    static Trigger* art_of_war(PlayerbotAI* botAI) { return new ArtOfWarTrigger(botAI); }
    static Trigger* blessing_on_party(PlayerbotAI* botAI) { return new BlessingOnPartyTrigger(botAI); }
    static Trigger* crusader_aura(PlayerbotAI* botAI) { return new CrusaderAuraTrigger(botAI); }
    static Trigger* retribution_aura(PlayerbotAI* botAI) { return new RetributionAuraTrigger(botAI); }
    static Trigger* devotion_aura(PlayerbotAI* botAI) { return new DevotionAuraTrigger(botAI); }
    static Trigger* sanctity_aura(PlayerbotAI* botAI) { return new SanctityAuraTrigger(botAI); }
    static Trigger* concentration_aura(PlayerbotAI* botAI) { return new ConcentrationAuraTrigger(botAI); }
    static Trigger* shadow_resistance_aura(PlayerbotAI* botAI) { return new ShadowResistanceAuraTrigger(botAI); }
    static Trigger* frost_resistance_aura(PlayerbotAI* botAI) { return new FrostResistanceAuraTrigger(botAI); }
    static Trigger* fire_resistance_aura(PlayerbotAI* botAI) { return new FireResistanceAuraTrigger(botAI); }
    static Trigger* hammer_of_justice_snare(PlayerbotAI* botAI) { return new HammerOfJusticeSnareTrigger(botAI); }
    static Trigger* hammer_of_justice_interrupt(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeInterruptSpellTrigger(botAI);
    }
    static Trigger* CleanseCureDisease(PlayerbotAI* botAI) { return new CleanseCureDiseaseTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberDisease(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberDiseaseTrigger(botAI);
    }
    static Trigger* CleanseCurePoison(PlayerbotAI* botAI) { return new CleanseCurePoisonTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberPoison(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberPoisonTrigger(botAI);
    }
    static Trigger* CleanseCureMagic(PlayerbotAI* botAI) { return new CleanseCureMagicTrigger(botAI); }
    static Trigger* CleanseCurePartyMemberMagic(PlayerbotAI* botAI)
    {
        return new CleanseCurePartyMemberMagicTrigger(botAI);
    }
    static Trigger* hammer_of_justice_on_enemy_target(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeEnemyHealerTrigger(botAI);
    }
    static Trigger* hammer_of_justice_on_snare_target(PlayerbotAI* botAI)
    {
        return new HammerOfJusticeSnareTrigger(botAI);
    }
    static Trigger* avenger_shield(PlayerbotAI* botAI) { return new AvengerShieldTrigger(botAI); }
    static Trigger* consecration(PlayerbotAI* botAI) { return new ConsecrationTrigger(botAI); }
    static Trigger* repentance_on_enemy_healer(PlayerbotAI* botAI) { return new RepentanceOnHealerTrigger(botAI); }
    static Trigger* repentance_on_snare_target(PlayerbotAI* botAI) { return new RepentanceSnareTrigger(botAI); }
    static Trigger* repentance_interrupt(PlayerbotAI* botAI) { return new RepentanceInterruptTrigger(botAI); }
    static Trigger* beacon_of_light_on_main_tank(PlayerbotAI* ai) { return new BeaconOfLightOnMainTankTrigger(ai); }
    static Trigger* sacred_shield_on_main_tank(PlayerbotAI* ai) { return new SacredShieldOnMainTankTrigger(ai); }

    static Trigger* blessing_of_kings_on_party(PlayerbotAI* botAI) { return new BlessingOfKingsOnPartyTrigger(botAI); }
    static Trigger* blessing_of_wisdom_on_party(PlayerbotAI* botAI)
    {
        return new BlessingOfWisdomOnPartyTrigger(botAI);
    }
    static Trigger* blessing_of_might_on_party(PlayerbotAI* botAI) { return new BlessingOfMightOnPartyTrigger(botAI); }
    static Trigger* blessing_of_sanctuary_on_party(PlayerbotAI* botAI)
    {
        return new BlessingOfSanctuaryOnPartyTrigger(botAI);
    }

    static Trigger* avenging_wrath(PlayerbotAI* botAI) { return new AvengingWrathTrigger(botAI); }
};

SharedNamedObjectContextList<Strategy> PaladinAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> PaladinAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> PaladinAiObjectContext::sharedValueContexts;

PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void PaladinAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void PaladinAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new PaladinStrategyFactoryInternal());
    strategyContexts.Add(new PaladinCombatStrategyFactoryInternal());
    strategyContexts.Add(new PaladinBuffStrategyFactoryInternal());
    strategyContexts.Add(new PaladinResistanceStrategyFactoryInternal());
}

void PaladinAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new PaladinTriggerFactoryInternal());
}

void PaladinAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}