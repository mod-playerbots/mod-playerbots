#include "RaidKarazhanStrategy.h"
#include "RaidKarazhanMultipliers.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(new TriggerNode("mana warp is about to explode",
        NextAction::array(0, new NextAction("mana warp stun creature before warp breach", ACTION_EMERGENCY + 6), nullptr)
    ));

    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen the huntsman need target priority",
        NextAction::array(0, new NextAction("attumen the huntsman mark target", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman attumen spawned",
        NextAction::array(0, new NextAction("attumen the huntsman split bosses", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman attumen is mounted",
        NextAction::array(0, new NextAction("attumen the huntsman stack behind", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("attumen the huntsman boss wipes aggro when mounting",
        NextAction::array(0, new NextAction("attumen the huntsman manage dps timer", ACTION_RAID + 2), nullptr)
    ));

    // Moroes
    triggers.push_back(new TriggerNode("moroes boss engaged by main tank",
        NextAction::array(0, new NextAction("moroes main tank attack boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("moroes need target priority",
        NextAction::array(0, new NextAction("moroes mark target", ACTION_RAID + 1), nullptr)
    ));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden of virtue healers are stunned by repentance",
        NextAction::array(0, new NextAction("maiden of virtue move boss to healer", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("maiden of virtue holy wrath deals chain damage",
        NextAction::array(0, new NextAction("maiden of virtue position ranged", ACTION_RAID + 1), nullptr)
    ));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode("big bad wolf boss is chasing little red riding hood",
        NextAction::array(0, new NextAction("big bad wolf run away from boss", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("big bad wolf boss engaged by tank",
        NextAction::array(0, new NextAction("big bad wolf position boss", ACTION_RAID + 1), nullptr)
    ));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode("romulo and julianne both bosses revived",
        NextAction::array(0, new NextAction("romulo and julianne mark target", ACTION_RAID + 1), nullptr)
    ));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode("wizard of oz need target priority",
        NextAction::array(0, new NextAction("wizard of oz mark target", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("wizard of oz strawman is vulnerable to fire",
        NextAction::array(0, new NextAction("wizard of oz scorch strawman", ACTION_RAID + 2), nullptr)
    ));

    // The Curator
    triggers.push_back(new TriggerNode("the curator astral flare spawned",
        NextAction::array(0, new NextAction("the curator mark astral flare", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("the curator boss engaged by tanks",
        NextAction::array(0, new NextAction("the curator position boss", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("the curator astral flares cast arcing sear",
        NextAction::array(0, new NextAction("the curator spread ranged", ACTION_RAID + 2), nullptr)
    ));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode("terestian illhoof need target priority",
        NextAction::array(0, new NextAction("terestian illhoof mark target", ACTION_RAID + 1), nullptr)
    ));

    // Shade of Aran
    triggers.push_back(new TriggerNode("shade of aran arcane explosion is casting",
        NextAction::array(0, new NextAction("shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("shade of aran flame wreath is active",
        NextAction::array(0, new NextAction("shade of aran stop moving during flame wreath", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("shade of aran conjured elementals summoned",
        NextAction::array(0, new NextAction("shade of aran mark conjured elemental", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("shade of aran boss uses counterspell and blizzard",
        NextAction::array(0, new NextAction("shade of aran ranged maintain distance", ACTION_RAID + 2), nullptr)
    ));

    // Netherspite
    triggers.push_back(new TriggerNode("netherspite red beam is active",
        NextAction::array(0, new NextAction("netherspite block red beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode("netherspite blue beam is active",
        NextAction::array(0, new NextAction("netherspite block blue beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode("netherspite green beam is active",
        NextAction::array(0, new NextAction("netherspite block green beam", ACTION_EMERGENCY + 8), nullptr)
    ));
    triggers.push_back(new TriggerNode("netherspite bot is not beam blocker",
        NextAction::array(0, new NextAction("netherspite avoid beam and void zone", ACTION_EMERGENCY + 7), nullptr)
    ));
    triggers.push_back(new TriggerNode("netherspite boss is banished",
        NextAction::array(0, new NextAction("netherspite banish phase avoid void zone", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("netherspite need to manage timers and trackers",
        NextAction::array(0, new NextAction("netherspite manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode("prince malchezaar bot is enfeebled",
        NextAction::array(0, new NextAction("prince malchezaar enfeebled avoid hazard", ACTION_EMERGENCY + 6), nullptr)
    ));
    triggers.push_back(new TriggerNode("prince malchezaar infernals are spawned",
        NextAction::array(0, new NextAction("prince malchezaar non tank avoid infernal", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("prince malchezaar boss engaged by main tank",
        NextAction::array(0, new NextAction("prince malchezaar main tank movement", ACTION_EMERGENCY + 6), nullptr)
    ));

    // Nightbane
    triggers.push_back(new TriggerNode("nightbane boss engaged by main tank",
        NextAction::array(0, new NextAction("nightbane ground phase position boss", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("nightbane ranged bots are in charred earth",
        NextAction::array(0, new NextAction("nightbane ground phase rotate ranged positions", ACTION_EMERGENCY + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("nightbane main tank is susceptible to fear",
        NextAction::array(0, new NextAction("nightbane cast fear ward on main tank", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("nightbane pets ignore collision to chase flying boss",
        NextAction::array(0, new NextAction("nightbane control pet aggression", ACTION_RAID + 2), nullptr)
    ));
    triggers.push_back(new TriggerNode("nightbane boss is flying",
        NextAction::array(0, new NextAction("nightbane flight phase movement", ACTION_RAID + 1), nullptr)
    ));
    triggers.push_back(new TriggerNode("nightbane need to manage timers and trackers",
        NextAction::array(0, new NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10), nullptr)
    ));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AttumenTheHuntsmanDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDpsMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new TheCuratorDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionDisableChargeMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteKeepBlockingBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteWaitForDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleKeepDistanceMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneWaitForDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}
