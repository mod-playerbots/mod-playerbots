/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RogueAiObjectContext.h"

#include "AiObjectContext.h"
#include "AssassinationRogueStrategy.h"
#include "DpsRogueStrategy.h"
#include "GenericRogueNonCombatStrategy.h"
#include "NamedObjectContext.h"
#include "PullStrategy.h"
#include "RogueActions.h"
#include "RogueComboActions.h"
#include "RogueFinishingActions.h"
#include "RogueOpeningActions.h"
#include "RogueTriggers.h"

class RogueStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    RogueStrategyFactoryInternal()
    {
        creators["nc"] = &RogueStrategyFactoryInternal::nc;
        creators["pull"] = &RogueStrategyFactoryInternal::pull;
        creators["aoe"] = &RogueStrategyFactoryInternal::aoe;
        creators["boost"] = &RogueStrategyFactoryInternal::boost;
        creators["stealthed"] = &RogueStrategyFactoryInternal::stealthed;
        creators["stealth"] = &RogueStrategyFactoryInternal::stealth;
        creators["cc"] = &RogueStrategyFactoryInternal::cc;
    }

private:
    static Strategy* boost(PlayerbotAI* botAI) { return new RogueBoostStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new RogueAoeStrategy(botAI); }
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericRogueNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new PullStrategy(botAI, "shoot"); }
    static Strategy* stealthed(PlayerbotAI* botAI) { return new StealthedRogueStrategy(botAI); }
    static Strategy* stealth(PlayerbotAI* botAI) { return new StealthStrategy(botAI); }
    static Strategy* cc(PlayerbotAI* botAI) { return new RogueCcStrategy(botAI); }
};

class RogueCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    RogueCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["dps"] = &RogueCombatStrategyFactoryInternal::dps;
        creators["melee"] = &RogueCombatStrategyFactoryInternal::melee;
    }

private:
    static Strategy* dps(PlayerbotAI* botAI) { return new DpsRogueStrategy(botAI); }
    static Strategy* melee(PlayerbotAI* botAI) { return new AssassinationRogueStrategy(botAI); }
};

class RogueTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    RogueTriggerFactoryInternal()
    {
        creators["kick"] = &RogueTriggerFactoryInternal::kick;
        creators["rupture"] = &RogueTriggerFactoryInternal::rupture;
        creators["slice and dice"] = &RogueTriggerFactoryInternal::slice_and_dice;
        creators["hunger for blood"] = &RogueTriggerFactoryInternal::hunger_for_blood;
        creators["expose armor"] = &RogueTriggerFactoryInternal::expose_armor;
        creators["kick on enemy healer"] = &RogueTriggerFactoryInternal::kick_on_enemy_healer;
        creators["unstealth"] = &RogueTriggerFactoryInternal::unstealth;
        creators["sap"] = &RogueTriggerFactoryInternal::sap;
        creators["in stealth"] = &RogueTriggerFactoryInternal::in_stealth;
        creators["no stealth"] = &RogueTriggerFactoryInternal::no_stealth;
        creators["stealth"] = &RogueTriggerFactoryInternal::stealth;
        creators["sprint"] = &RogueTriggerFactoryInternal::sprint;
        creators["main hand weapon no enchant"] = &RogueTriggerFactoryInternal::main_hand_weapon_no_enchant;
        creators["off hand weapon no enchant"] = &RogueTriggerFactoryInternal::off_hand_weapon_no_enchant;
        creators["tricks of the trade on main tank"] = &RogueTriggerFactoryInternal::tricks_of_the_trade_on_main_tank;
        creators["adrenaline rush"] = &RogueTriggerFactoryInternal::adrenaline_rush;
        creators["blade fury"] = &RogueTriggerFactoryInternal::blade_fury;
    }

private:
    static Trigger* adrenaline_rush(PlayerbotAI* botAI) { return new AdrenalineRushTrigger(botAI); }
    static Trigger* blade_fury(PlayerbotAI* botAI) { return new BladeFuryTrigger(botAI); }
    static Trigger* kick(PlayerbotAI* botAI) { return new KickInterruptSpellTrigger(botAI); }
    static Trigger* rupture(PlayerbotAI* botAI) { return new RuptureTrigger(botAI); }
    static Trigger* slice_and_dice(PlayerbotAI* botAI) { return new SliceAndDiceTrigger(botAI); }
    static Trigger* hunger_for_blood(PlayerbotAI* botAI) { return new HungerForBloodTrigger(botAI); }
    static Trigger* expose_armor(PlayerbotAI* botAI) { return new ExposeArmorTrigger(botAI); }
    static Trigger* kick_on_enemy_healer(PlayerbotAI* botAI) { return new KickInterruptEnemyHealerSpellTrigger(botAI); }
    static Trigger* unstealth(PlayerbotAI* botAI) { return new UnstealthTrigger(botAI); }
    static Trigger* sap(PlayerbotAI* botAI) { return new SapTrigger(botAI); }
    static Trigger* in_stealth(PlayerbotAI* botAI) { return new InStealthTrigger(botAI); }
    static Trigger* no_stealth(PlayerbotAI* botAI) { return new NoStealthTrigger(botAI); }
    static Trigger* stealth(PlayerbotAI* botAI) { return new StealthTrigger(botAI); }
    static Trigger* sprint(PlayerbotAI* botAI) { return new SprintTrigger(botAI); }
    static Trigger* main_hand_weapon_no_enchant(PlayerbotAI* ai) { return new MainHandWeaponNoEnchantTrigger(ai); }
    static Trigger* off_hand_weapon_no_enchant(PlayerbotAI* ai) { return new OffHandWeaponNoEnchantTrigger(ai); }
    static Trigger* tricks_of_the_trade_on_main_tank(PlayerbotAI* ai)
    {
        return new TricksOfTheTradeOnMainTankTrigger(ai);
    }
};

SharedNamedObjectContextList<Strategy> RogueAiObjectContext::sharedStrategyContexts;
SharedNamedObjectContextList<Trigger> RogueAiObjectContext::sharedTriggerContexts;
SharedNamedObjectContextList<UntypedValue> RogueAiObjectContext::sharedValueContexts;

RogueAiObjectContext::RogueAiObjectContext(PlayerbotAI* botAI)
    : AiObjectContext(botAI, sharedStrategyContexts,
                      sharedTriggerContexts, sharedValueContexts)
{
}

void RogueAiObjectContext::BuildSharedContexts()
{
    BuildSharedStrategyContexts(sharedStrategyContexts);
    BuildSharedTriggerContexts(sharedTriggerContexts);
    BuildSharedValueContexts(sharedValueContexts);
}

void RogueAiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    AiObjectContext::BuildSharedStrategyContexts(strategyContexts);
    strategyContexts.Add(new RogueStrategyFactoryInternal());
    strategyContexts.Add(new RogueCombatStrategyFactoryInternal());
}

void RogueAiObjectContext::BuildSharedTriggerContexts(SharedNamedObjectContextList<Trigger>& triggerContexts)
{
    AiObjectContext::BuildSharedTriggerContexts(triggerContexts);
    triggerContexts.Add(new RogueTriggerFactoryInternal());
}

void RogueAiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    AiObjectContext::BuildSharedValueContexts(valueContexts);
}