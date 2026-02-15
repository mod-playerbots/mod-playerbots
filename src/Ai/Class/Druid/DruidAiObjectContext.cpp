/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DruidAiObjectContext.h"

#include "BearTankDruidStrategy.h"
#include "CasterDruidStrategy.h"
#include "CatDpsDruidStrategy.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "DruidShapeshiftActions.h"
#include "DruidTriggers.h"
#include "GenericDruidNonCombatStrategy.h"
#include "GenericDruidStrategy.h"
#include "HealDruidStrategy.h"
#include "MeleeDruidStrategy.h"
#include "OffhealDruidCatStrategy.h"

class DruidStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DruidStrategyFactoryInternal()
    {
        creators["nc"] = &DruidStrategyFactoryInternal::nc;
        creators["cat aoe"] = &DruidStrategyFactoryInternal::cat_aoe;
        creators["caster aoe"] = &DruidStrategyFactoryInternal::caster_aoe;
        creators["caster debuff"] = &DruidStrategyFactoryInternal::caster_debuff;
        creators["dps debuff"] = &DruidStrategyFactoryInternal::caster_debuff;
        creators["cure"] = &DruidStrategyFactoryInternal::cure;
        creators["melee"] = &DruidStrategyFactoryInternal::melee;
        creators["buff"] = &DruidStrategyFactoryInternal::buff;
        creators["boost"] = &DruidStrategyFactoryInternal::boost;
        creators["cc"] = &DruidStrategyFactoryInternal::cc;
        creators["healer dps"] = &DruidStrategyFactoryInternal::healer_dps;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericDruidNonCombatStrategy(botAI); }
    static Strategy* cat_aoe(PlayerbotAI* botAI) { return new CatAoeDruidStrategy(botAI); }
    static Strategy* caster_aoe(PlayerbotAI* botAI) { return new CasterDruidAoeStrategy(botAI); }
    static Strategy* caster_debuff(PlayerbotAI* botAI) { return new CasterDruidDebuffStrategy(botAI); }
    static Strategy* cure(PlayerbotAI* botAI) { return new DruidCureStrategy(botAI); }
    static Strategy* melee(PlayerbotAI* botAI) { return new MeleeDruidStrategy(botAI); }
    static Strategy* buff(PlayerbotAI* botAI) { return new GenericDruidBuffStrategy(botAI); }
    static Strategy* boost(PlayerbotAI* botAI) { return new DruidBoostStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new DruidCcStrategy(botAI); }
    static Strategy* healer_dps(PlayerbotAI* botAI) { return new DruidHealerDpsStrategy(botAI); }
};

class DruidDruidStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DruidDruidStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bear"] = &DruidDruidStrategyFactoryInternal::bear;
        creators["tank"] = &DruidDruidStrategyFactoryInternal::bear;
        creators["cat"] = &DruidDruidStrategyFactoryInternal::cat;
        creators["caster"] = &DruidDruidStrategyFactoryInternal::caster;
        creators["dps"] = &DruidDruidStrategyFactoryInternal::cat;
        creators["heal"] = &DruidDruidStrategyFactoryInternal::heal;
        creators["offheal"] = &DruidDruidStrategyFactoryInternal::offheal;
    }

private:
    static Strategy* bear(PlayerbotAI* botAI) { return new BearTankDruidStrategy(botAI); }
    static Strategy* cat(PlayerbotAI* botAI) { return new CatDpsDruidStrategy(botAI); }
    static Strategy* caster(PlayerbotAI* botAI) { return new CasterDruidStrategy(botAI); }
    static Strategy* heal(PlayerbotAI* botAI) { return new HealDruidStrategy(botAI); }
    static Strategy* offheal(PlayerbotAI* botAI) { return new OffhealDruidCatStrategy(botAI); }
};

class DruidTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    DruidTriggerFactoryInternal()
    {
        creators["omen of clarity"] = &DruidTriggerFactoryInternal::omen_of_clarity;
        creators["thorns"] = &DruidTriggerFactoryInternal::thorns;
        creators["thorns on party"] = &DruidTriggerFactoryInternal::thorns_on_party;
        creators["thorns on main tank"] = &DruidTriggerFactoryInternal::thorns_on_main_tank;
        creators["bash"] = &DruidTriggerFactoryInternal::bash;
        creators["faerie fire (feral)"] = &DruidTriggerFactoryInternal::faerie_fire_feral;
        creators["faerie fire"] = &DruidTriggerFactoryInternal::faerie_fire;
        creators["insect swarm"] = &DruidTriggerFactoryInternal::insect_swarm;
        creators["moonfire"] = &DruidTriggerFactoryInternal::moonfire;
        creators["nature's grasp"] = &DruidTriggerFactoryInternal::natures_grasp;
        creators["tiger's fury"] = &DruidTriggerFactoryInternal::tigers_fury;
        creators["berserk"] = &DruidTriggerFactoryInternal::berserk;
        creators["savage roar"] = &DruidTriggerFactoryInternal::savage_roar;
        creators["rake"] = &DruidTriggerFactoryInternal::rake;
        creators["mark of the wild"] = &DruidTriggerFactoryInternal::mark_of_the_wild;
        creators["mark of the wild on party"] = &DruidTriggerFactoryInternal::mark_of_the_wild_on_party;
        creators["cure poison"] = &DruidTriggerFactoryInternal::cure_poison;
        creators["party member cure poison"] = &DruidTriggerFactoryInternal::party_member_cure_poison;
        creators["entangling roots"] = &DruidTriggerFactoryInternal::entangling_roots;
        creators["entangling roots kite"] = &DruidTriggerFactoryInternal::entangling_roots_kite;
        creators["hibernate"] = &DruidTriggerFactoryInternal::hibernate;
        creators["bear form"] = &DruidTriggerFactoryInternal::bear_form;
        creators["cat form"] = &DruidTriggerFactoryInternal::cat_form;
        creators["tree form"] = &DruidTriggerFactoryInternal::tree_form;
        creators["eclipse (solar)"] = &DruidTriggerFactoryInternal::eclipse_solar;
        creators["eclipse (lunar)"] = &DruidTriggerFactoryInternal::eclipse_lunar;
        creators["bash on enemy healer"] = &DruidTriggerFactoryInternal::bash_on_enemy_healer;
        creators["nature's swiftness"] = &DruidTriggerFactoryInternal::natures_swiftness;
        creators["party member remove curse"] = &DruidTriggerFactoryInternal::party_member_remove_curse;
        creators["eclipse (solar) cooldown"] = &DruidTriggerFactoryInternal::eclipse_solar_cooldown;
        creators["eclipse (lunar) cooldown"] = &DruidTriggerFactoryInternal::eclipse_lunar_cooldown;
        creators["mangle (cat)"] = &DruidTriggerFactoryInternal::mangle_cat;
        creators["ferocious bite time"] = &DruidTriggerFactoryInternal::ferocious_bite_time;
        creators["hurricane channel check"] = &DruidTriggerFactoryInternal::hurricane_channel_check;
    }

private:
    static Trigger* natures_swiftness(PlayerbotAI* botAI) { return new NaturesSwiftnessTrigger(botAI); }
    static Trigger* eclipse_solar(PlayerbotAI* botAI) { return new EclipseSolarTrigger(botAI); }
    static Trigger* eclipse_lunar(PlayerbotAI* botAI) { return new EclipseLunarTrigger(botAI); }
    static Trigger* thorns(PlayerbotAI* botAI) { return new ThornsTrigger(botAI); }
    static Trigger* thorns_on_party(PlayerbotAI* botAI) { return new ThornsOnPartyTrigger(botAI); }
    static Trigger* thorns_on_main_tank(PlayerbotAI* botAI) { return new ThornsOnMainTankTrigger(botAI); }
    static Trigger* bash(PlayerbotAI* botAI) { return new BashInterruptSpellTrigger(botAI); }
    static Trigger* faerie_fire_feral(PlayerbotAI* botAI) { return new FaerieFireFeralTrigger(botAI); }
    static Trigger* insect_swarm(PlayerbotAI* botAI) { return new InsectSwarmTrigger(botAI); }
    static Trigger* moonfire(PlayerbotAI* botAI) { return new MoonfireTrigger(botAI); }
    static Trigger* faerie_fire(PlayerbotAI* botAI) { return new FaerieFireTrigger(botAI); }
    static Trigger* natures_grasp(PlayerbotAI* botAI) { return new NaturesGraspTrigger(botAI); }
    static Trigger* tigers_fury(PlayerbotAI* botAI) { return new TigersFuryTrigger(botAI); }
    static Trigger* berserk(PlayerbotAI* botAI) { return new BerserkTrigger(botAI); }
    static Trigger* savage_roar(PlayerbotAI* botAI) { return new SavageRoarTrigger(botAI); }
    static Trigger* rake(PlayerbotAI* botAI) { return new RakeTrigger(botAI); }
    static Trigger* mark_of_the_wild(PlayerbotAI* botAI) { return new MarkOfTheWildTrigger(botAI); }
    static Trigger* mark_of_the_wild_on_party(PlayerbotAI* botAI) { return new MarkOfTheWildOnPartyTrigger(botAI); }
    static Trigger* cure_poison(PlayerbotAI* botAI) { return new CurePoisonTrigger(botAI); }
    static Trigger* party_member_cure_poison(PlayerbotAI* botAI) { return new PartyMemberCurePoisonTrigger(botAI); }
    static Trigger* entangling_roots(PlayerbotAI* botAI) { return new EntanglingRootsTrigger(botAI); }
    static Trigger* entangling_roots_kite(PlayerbotAI* botAI) { return new EntanglingRootsKiteTrigger(botAI); }
    static Trigger* hibernate(PlayerbotAI* botAI) { return new HibernateTrigger(botAI); }
    static Trigger* bear_form(PlayerbotAI* botAI) { return new BearFormTrigger(botAI); }
    static Trigger* cat_form(PlayerbotAI* botAI) { return new CatFormTrigger(botAI); }
    static Trigger* tree_form(PlayerbotAI* botAI) { return new TreeFormTrigger(botAI); }
    static Trigger* bash_on_enemy_healer(PlayerbotAI* botAI) { return new BashInterruptEnemyHealerSpellTrigger(botAI); }
    static Trigger* omen_of_clarity(PlayerbotAI* botAI) { return new OmenOfClarityTrigger(botAI); }
    static Trigger* party_member_remove_curse(PlayerbotAI* ai) { return new DruidPartyMemberRemoveCurseTrigger(ai); }
    static Trigger* eclipse_solar_cooldown(PlayerbotAI* ai) { return new EclipseSolarCooldownTrigger(ai); }
    static Trigger* eclipse_lunar_cooldown(PlayerbotAI* ai) { return new EclipseLunarCooldownTrigger(ai); }
    static Trigger* mangle_cat(PlayerbotAI* ai) { return new MangleCatTrigger(ai); }
    static Trigger* ferocious_bite_time(PlayerbotAI* ai) { return new FerociousBiteTimeTrigger(ai); }
    static Trigger* hurricane_channel_check(PlayerbotAI* ai) { return new HurricaneChannelCheckTrigger(ai); }
};

SharedNamedObjectContextList<Strategy> DruidAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> DruidAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> DruidAiObjectContext::sharedValueContexts;

DruidAiObjectContext::DruidAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts, sharedTriggerContexts, sharedValueContexts)
{
}

void DruidAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void DruidAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new DruidStrategyFactoryInternal());
    strategyContexts.Add(new DruidDruidStrategyFactoryInternal());
}

void DruidAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new DruidTriggerFactoryInternal());
}

void DruidAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}
