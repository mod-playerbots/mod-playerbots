/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidTocStrategy.h"
#include "Playerbots.h"
#include "RaidTocMultipliers.h"
#include "RaidTocTriggers.h"

void RaidTocStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Twin Val'kyr: a toucher has to cleanse on the opposite portal, retake the
    // original essence and return to its spot. The tank never gets a Touch and
    // must not move (the twins follow their victim), the action guards both.
    triggers.push_back(new TriggerNode(
        "toc twin valkyr fight",
        { NextAction("toc twin valkyr touch", ACTION_EMERGENCY) }));

    // While a healer is dragged off to the portals, retribution paladins and
    // cat druids switch to off-healing so the group keeps its sustain.
    triggers.push_back(new TriggerNode(
        "toc twin valkyr fight",
        { NextAction("toc offheal", ACTION_NORMAL) }));

    // Faction Champions chase squishies regardless of tank threat. A bot that
    // is being meleed by a champion and is low on health must kite, otherwise
    // it just dies in place. Non-tanks only: the tank has to hold position.
    triggers.push_back(new TriggerNode(
        "toc champions chased",
        { NextAction("runaway", ACTION_EMERGENCY + 1) }));

    // Icehowl charges a random target: sidestep out of the lane, everyone else
    // clears it. Highest movement priority so the dodge beats any attack.
    triggers.push_back(new TriggerNode(
        "toc icehowl charge",
        { NextAction("toc icehowl dodge", ACTION_EMERGENCY + 2) }));

    // Gormok: Rising Anger stacks on the tank, swap at 3.
    triggers.push_back(new TriggerNode(
        "toc gormok tank swap",
        { NextAction("taunt spell", ACTION_EMERGENCY) }));

    // Jormungar pair: the burning tank cleanses paralysed players with its
    // Burning Bite, then drags the boss away from the raid.
    triggers.push_back(new TriggerNode(
        "toc burning bile",
        { NextAction("toc burning bile", ACTION_EMERGENCY) }));

    // Anub'arak submerge: tank north, raid south, holy paladin sticks to the
    // tank. Moved in before anything else.
    triggers.push_back(new TriggerNode(
        "toc anub submerge",
        { NextAction("toc anub submerge move", ACTION_EMERGENCY + 3) }));

    // Jaraxxus portals and volcanoes never attack, so the dps has to be told
    // to kill them. Melee interrupt classes deliberately stay on the boss to
    // keep Fel Fireball kicked (handled inside the action).
    triggers.push_back(new TriggerNode(
        "toc jaraxxus adds",
        { NextAction("toc jaraxxus adds", ACTION_RAID + 5) }));

    // Nether Power stacked twice means the kick was missed and no mage/shaman
    // handled it: the tank eats a defensive before the boosted cast lands.
    // Mages steal and shamans purge by themselves (base magic-buff triggers).
    triggers.push_back(new TriggerNode(
        "toc jaraxxus nether power",
        {
            NextAction("shield wall", ACTION_EMERGENCY),
            NextAction("icebound fortitude", ACTION_EMERGENCY),
            NextAction("divine protection", ACTION_EMERGENCY),
            NextAction("barkskin", ACTION_EMERGENCY),
            NextAction("survival instincts", ACTION_EMERGENCY),
        }));
}

void RaidTocStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Anub'arak: no Starfall before phase 3.
    multipliers.push_back(new TocStarfallMultiplier(botAI));
}
