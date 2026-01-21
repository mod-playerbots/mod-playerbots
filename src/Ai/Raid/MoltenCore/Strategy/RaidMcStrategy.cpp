#include "RaidMcStrategy.h"

#include "RaidMcMultipliers.h"
#include "Strategy.h"

void RaidMcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Lucifron
    triggers.push_back(
        new TriggerNode("mc lucifron shadow resistance",
                        { NextAction("mc lucifron shadow resistance", ACTION_RAID) }));

    // Magmadar
    // TODO: Fear ward / tremor totem, or general anti-fear strat development. Same as King Dred (Drak'Tharon) and faction commander (Nexus).
    triggers.push_back(
        new TriggerNode("mc magmadar fire resistance",
                        { NextAction("mc magmadar fire resistance", ACTION_RAID) }));

    // Gehennas
    triggers.push_back(
        new TriggerNode("mc gehennas shadow resistance",
                        { NextAction("mc gehennas shadow resistance", ACTION_RAID) }));

    // Garr
    triggers.push_back(
        new TriggerNode("mc garr fire resistance",
                        { NextAction("mc garr fire resistance", ACTION_RAID) }));

    // Baron Geddon
    triggers.push_back(
        new TriggerNode("mc baron geddon fire resistance",
                        { NextAction("mc baron geddon fire resistance", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc living bomb debuff",
                        { NextAction("mc move from group", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc baron geddon inferno",
                        { NextAction("mc move from baron geddon", ACTION_RAID) }));

    // Shazzrah
    triggers.push_back(
        new TriggerNode("mc shazzrah ranged",
                        { NextAction("mc shazzrah move away", ACTION_RAID) }));

    // Sulfuron Harbinger
    // Alternatively, shadow resistance is also possible.
    triggers.push_back(
        new TriggerNode("mc sulfuron harbinger fire resistance",
                        { NextAction("mc sulfuron harbinger fire resistance", ACTION_RAID) }));

    // Golemagg the Incinerator
    triggers.push_back(
        new TriggerNode("mc golemagg fire resistance",
                        { NextAction("mc golemagg fire resistance", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg mark boss",
                        { NextAction("mc golemagg mark boss", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg is main tank",
                        { NextAction("mc golemagg main tank attack golemagg", ACTION_RAID) }));
    triggers.push_back(
        new TriggerNode("mc golemagg is assist tank",
                        { NextAction("mc golemagg assist tank attack core rager", ACTION_RAID) }));

    // Majordomo Executus
    triggers.push_back(
        new TriggerNode("mc majordomo shadow resistance",
                        { NextAction("mc majordomo shadow resistance", ACTION_RAID) }));

    // Ragnaros
    triggers.push_back(
        new TriggerNode("mc ragnaros fire resistance",
                        { NextAction("mc ragnaros fire resistance", ACTION_RAID) }));
}

void RaidMcStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GarrDisableDpsAoeMultiplier(botAI));
    multipliers.push_back(new BaronGeddonAbilityMultiplier(botAI));
    multipliers.push_back(new GolemaggMultiplier(botAI));
}
