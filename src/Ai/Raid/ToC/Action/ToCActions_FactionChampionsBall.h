/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSBALL_H
#define PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSBALL_H

#include "Action.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"

// An ice duty DK's attack on its own champion
class FactionChampionsIceTargetAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "faction champions ice target";

    FactionChampionsIceTargetAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsChainsOfIceAction : public Action
{
public:
    static constexpr char const* Name = "faction champions chains of ice";

    FactionChampionsChainsOfIceAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// An ice duty DK's walk into Chains of Ice range of its champion
class FactionChampionsIceReachAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions ice reach";

    FactionChampionsIceReachAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsCycloneStandAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions cyclone stand";

    FactionChampionsCycloneStandAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// A healer's walk off the melee champions next to it
class FactionChampionsStepOffAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions step off";

    FactionChampionsStepOffAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Boosts away from the champions focusing it
class FactionChampionsChasedAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions chased";

    FactionChampionsChasedAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// A bot's hop away from the melee champions on it, till they re-pick someone else
class FactionChampionsKiteAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions kite";

    FactionChampionsKiteAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Sprint or Dash: no cast time and no GCD, so the kite's walk goes on
class FactionChampionsSpeedBoostAction : public Action
{
public:
    static constexpr char const* Name = "faction champions speed boost";

    FactionChampionsSpeedBoostAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// The AoE fear or Frost Nova that makes the champions burn their trinket
class FactionChampionsOpenerAction : public Action
{
public:
    static constexpr char const* Name = "faction champions opener";

    FactionChampionsOpenerAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// A caster's Nova, Blink, circle teleport or fear off the melee champions on it
class FactionChampionsEscapeAction : public Action
{
public:
    static constexpr char const* Name = "faction champions escape";

    FactionChampionsEscapeAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// The warlock's circle on its staging stand, for the teleport back once a melee champion reaches it
class FactionChampionsDemonicCircleAction : public Action
{
public:
    static constexpr char const* Name = "faction champions demonic circle";

    FactionChampionsDemonicCircleAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
