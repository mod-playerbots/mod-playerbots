/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRAVELORDERACTIONS_H
#define PLAYERBOTS_TRAVELORDERACTIONS_H

#include "MovementActions.h"
#include "NonCombatStrategy.h"
#include "Trigger.h"

class PlayerbotAI;

// Chat command handler: "travel <map> <x> <y> <z>", "travel <x> <y> <z>"
// (current map), "travel stop", "travel ?". Validates the destination,
// stores a TravelOrder and enables the "travel order" strategy that drives
// it. Routing itself follows the funnel's normal rules (raw probe in sight,
// travel-node graph for long/cross-map when enabled).
class TravelCommandAction : public MovementAction
{
public:
    TravelCommandAction(PlayerbotAI* botAI) : MovementAction(botAI, "travel") {}

    bool Execute(Event event) override;
};

// Per-tick driver while a TravelOrder is active: whispers milestones
// (map change, transport board/disembark, taxi), announces the first
// resolved route, detects arrival and stuck orders, and otherwise feeds
// the destination into MoveTo2.
class DriveTravelOrderAction : public MovementAction
{
public:
    DriveTravelOrderAction(PlayerbotAI* botAI) : MovementAction(botAI, "drive travel order") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    void Finish(bool removeStrategy = true);
};

class TravelOrderActiveTrigger : public Trigger
{
public:
    TravelOrderActiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, "travel order active", 1) {}

    bool IsActive() override;
};

class TravelOrderStrategy : public NonCombatStrategy
{
public:
    TravelOrderStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}

    std::string const getName() override { return "travel order"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
