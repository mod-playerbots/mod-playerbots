/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSBALL_H
#define PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSBALL_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"

// An ice duty DK's attack on its own champion
class FactionChampionsIceTargetAction : public RaidDirectAttackAction
{
public:
    FactionChampionsIceTargetAction(PlayerbotAI* botAI)
        : RaidDirectAttackAction(botAI, "faction champions ice target") {}
    bool Execute(Event event) override;
};

class FactionChampionsChainsOfIceAction : public Action
{
public:
    FactionChampionsChainsOfIceAction(PlayerbotAI* botAI) : Action(botAI, "faction champions chains of ice") {}
    bool Execute(Event event) override;
};

// An ice duty DK's walk into Chains of Ice range of its champion
class FactionChampionsIceReachAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsIceReachAction(PlayerbotAI* botAI)
        : NorthrendBeastsWalkAction(botAI, "faction champions ice reach") {}
    bool Execute(Event event) override;
};

class FactionChampionsCycloneStandAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsCycloneStandAction(PlayerbotAI* botAI)
        : NorthrendBeastsWalkAction(botAI, "faction champions cyclone stand") {}
    bool Execute(Event event) override;
};

// A healer's walk off the melee champions next to it
class FactionChampionsStepOffAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsStepOffAction(PlayerbotAI* botAI)
        : NorthrendBeastsWalkAction(botAI, "faction champions step off") {}
    bool Execute(Event event) override;
};

// Boosts away from the champions focusing it
class FactionChampionsChasedAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsChasedAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, "faction champions chased") {}
    bool Execute(Event event) override;
};

// A bot's hop away from the melee champions on it, till they re-pick someone else
class FactionChampionsKiteAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsKiteAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, "faction champions kite") {}
    bool Execute(Event event) override;
};

// Sprint or Dash: no cast time and no GCD, so the kite's walk goes on
class FactionChampionsSpeedBoostAction : public Action
{
public:
    FactionChampionsSpeedBoostAction(PlayerbotAI* botAI) : Action(botAI, "faction champions speed boost") {}
    bool Execute(Event event) override;
};

// The AoE fear or Frost Nova that makes the champions burn their trinket
class FactionChampionsOpenerAction : public Action
{
public:
    FactionChampionsOpenerAction(PlayerbotAI* botAI) : Action(botAI, "faction champions opener") {}
    bool Execute(Event event) override;
};

// A caster's Nova, Blink, circle teleport or fear off the melee champions on it
class FactionChampionsEscapeAction : public Action
{
public:
    FactionChampionsEscapeAction(PlayerbotAI* botAI) : Action(botAI, "faction champions escape") {}
    bool Execute(Event event) override;
};

// The warlock's circle on its staging stand, for the teleport back once a melee champion reaches it
class FactionChampionsDemonicCircleAction : public Action
{
public:
    FactionChampionsDemonicCircleAction(PlayerbotAI* botAI) : Action(botAI, "faction champions demonic circle") {}
    bool Execute(Event event) override;
};

class ToCFactionChampionsBallActionContext : public NamedObjectContext<Action>
{
public:
    ToCFactionChampionsBallActionContext()
    {
        creators["faction champions escape"] = &ToCFactionChampionsBallActionContext::faction_champions_escape;
        creators["faction champions demonic circle"] =
            &ToCFactionChampionsBallActionContext::faction_champions_demonic_circle;
        creators["faction champions ice target"] = &ToCFactionChampionsBallActionContext::faction_champions_ice_target;
        creators["faction champions chains of ice"] =
            &ToCFactionChampionsBallActionContext::faction_champions_chains_of_ice;
        creators["faction champions ice reach"] = &ToCFactionChampionsBallActionContext::faction_champions_ice_reach;
        creators["faction champions cyclone stand"] =
            &ToCFactionChampionsBallActionContext::faction_champions_cyclone_stand;
        creators["faction champions step off"] = &ToCFactionChampionsBallActionContext::faction_champions_step_off;
        creators["faction champions opener"] = &ToCFactionChampionsBallActionContext::faction_champions_opener;
        creators["faction champions chased"] = &ToCFactionChampionsBallActionContext::faction_champions_chased;
        creators["faction champions kite"] = &ToCFactionChampionsBallActionContext::faction_champions_kite;
        creators["faction champions speed boost"] =
            &ToCFactionChampionsBallActionContext::faction_champions_speed_boost;
    }

private:
    static Action* faction_champions_ice_target(PlayerbotAI* botAI)
    {
        return new FactionChampionsIceTargetAction(botAI);
    }

    static Action* faction_champions_chains_of_ice(PlayerbotAI* botAI)
    {
        return new FactionChampionsChainsOfIceAction(botAI);
    }

    static Action* faction_champions_ice_reach(PlayerbotAI* botAI) { return new FactionChampionsIceReachAction(botAI); }

    static Action* faction_champions_cyclone_stand(PlayerbotAI* botAI)
    {
        return new FactionChampionsCycloneStandAction(botAI);
    }

    static Action* faction_champions_step_off(PlayerbotAI* botAI) { return new FactionChampionsStepOffAction(botAI); }

    static Action* faction_champions_opener(PlayerbotAI* botAI) { return new FactionChampionsOpenerAction(botAI); }

    static Action* faction_champions_chased(PlayerbotAI* botAI) { return new FactionChampionsChasedAction(botAI); }

    static Action* faction_champions_kite(PlayerbotAI* botAI) { return new FactionChampionsKiteAction(botAI); }

    static Action* faction_champions_speed_boost(PlayerbotAI* botAI)
    {
        return new FactionChampionsSpeedBoostAction(botAI);
    }

    static Action* faction_champions_escape(PlayerbotAI* botAI) { return new FactionChampionsEscapeAction(botAI); }

    static Action* faction_champions_demonic_circle(PlayerbotAI* botAI)
    {
        return new FactionChampionsDemonicCircleAction(botAI);
    }
};

#endif
