/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSDEFENCE_H
#define PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONSDEFENCE_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"

// Walks out of a Bladestorm or Hellfire, the two champion AoEs a bot can see coming
class FactionChampionsAvoidAoeAction : public MovementAction
{
public:
    FactionChampionsAvoidAoeAction(PlayerbotAI* botAI) : MovementAction(botAI, "faction champions avoid aoe") {}
    bool Execute(Event event) override;

private:
    bool IssueLeg(Position const& spot, uint32 now);

    Position _spot;
    Position _lastIssued;
    bool _hasSpot = false;
    uint32 _issuedMs = 0;
};

class FactionChampionsMassDispelAction : public Action
{
public:
    FactionChampionsMassDispelAction(PlayerbotAI* botAI) : Action(botAI, "faction champions mass dispel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsDispelCcAction : public Action
{
public:
    FactionChampionsDispelCcAction(PlayerbotAI* botAI) : Action(botAI, "faction champions dispel cc") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsKillTotemAction : public RaidDirectAttackAction
{
public:
    FactionChampionsKillTotemAction(
        PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, "faction champions kill totem") {}
    bool Execute(Event event) override;
};

class FactionChampionsPhysicalSwitchAction : public AttackAction
{
public:
    FactionChampionsPhysicalSwitchAction(
        PlayerbotAI* botAI) : AttackAction(botAI, "faction champions physical switch") {}
    bool Execute(Event event) override;
};

class FactionChampionsPurgeKillTargetAction : public Action
{
public:
    FactionChampionsPurgeKillTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "faction champions purge kill target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Walks the DK to its latched stand, then grips the kill target to it
class FactionChampionsDeathGripAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsDeathGripAction(PlayerbotAI* botAI, std::string const name = "faction champions death grip")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;

protected:
    // false once at the stand, with the walk dropped
    bool StillWalking(Position const& stand, bool& walked);
};

// A grip chain link's walk to its post, then its grip on the champion the player called
class FactionChampionsChainGripAction : public FactionChampionsDeathGripAction
{
public:
    FactionChampionsChainGripAction(PlayerbotAI* botAI)
        : FactionChampionsDeathGripAction(botAI, "faction champions chain grip") {}
    bool Execute(Event event) override;
};

// A bot DK opens the grip chain on the player's target when the player types pull
class FactionChampionsChainRequestAction : public Action
{
public:
    FactionChampionsChainRequestAction(PlayerbotAI* botAI) : Action(botAI, "faction champions chain request") {}
    bool Execute(Event event) override;
};

// Walks the DK to its wall stand, grips the melee champion there, then slows it with Chains of Ice
class FactionChampionsWallGripAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsWallGripAction(PlayerbotAI* botAI)
        : NorthrendBeastsWalkAction(botAI, "faction champions wall grip") {}
    bool Execute(Event event) override;
};

class FactionChampionsFreedomAction : public Action
{
public:
    FactionChampionsFreedomAction(PlayerbotAI* botAI) : Action(botAI, "faction champions freedom") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsChannelInterruptAction : public Action
{
public:
    FactionChampionsChannelInterruptAction(
        PlayerbotAI* botAI) : Action(botAI, "faction champions channel interrupt") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsSnareAction : public Action
{
public:
    FactionChampionsSnareAction(PlayerbotAI* botAI) : Action(botAI, "faction champions snare") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Walks into spell range first when the duty was dealt from out of it
class FactionChampionsCrowdControlAction : public NorthrendBeastsWalkAction
{
public:
    FactionChampionsCrowdControlAction(
        PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, "faction champions crowd control") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsPeelAction : public Action
{
public:
    FactionChampionsPeelAction(PlayerbotAI* botAI) : Action(botAI, "faction champions peel") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsProtectAction : public Action
{
public:
    FactionChampionsProtectAction(PlayerbotAI* botAI) : Action(botAI, "faction champions protect") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsBattleResAction : public Action
{
public:
    FactionChampionsBattleResAction(PlayerbotAI* botAI) : Action(botAI, "faction champions battle res") {}
    bool Execute(Event event) override;
    bool isUseful() override;
    std::vector<NextAction> getPrerequisites() override;
};

class FactionChampionsPersonalDefensiveAction : public Action
{
public:
    FactionChampionsPersonalDefensiveAction(PlayerbotAI* botAI)
        : Action(botAI, "faction champions personal defensive") {}
    bool Execute(Event event) override;
};

class ToCFactionChampionsDefenceActionContext : public NamedObjectContext<Action>
{
public:
    ToCFactionChampionsDefenceActionContext()
    {
        creators["faction champions personal defensive"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_personal_defensive;
        creators["faction champions avoid aoe"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_avoid_aoe;
        creators["faction champions mass dispel"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_mass_dispel;
        creators["faction champions dispel cc"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_dispel_cc;
        creators["faction champions kill totem"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_kill_totem;
        creators["faction champions physical switch"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_physical_switch;
        creators["faction champions purge kill target"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_purge_kill_target;
        creators["faction champions death grip"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_death_grip;
        creators["faction champions chain grip"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_chain_grip;
        creators["faction champions chain request"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_chain_request;
        creators["faction champions wall grip"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_wall_grip;
        creators["faction champions freedom"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_freedom;
        creators["faction champions channel interrupt"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_channel_interrupt;
        creators["faction champions snare"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_snare;
        creators["faction champions crowd control"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_crowd_control;
        creators["faction champions peel"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_peel;
        creators["faction champions protect"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_protect;
        creators["faction champions battle res"] =
            &ToCFactionChampionsDefenceActionContext::faction_champions_battle_res;
    }

private:
    static Action* faction_champions_avoid_aoe(PlayerbotAI* botAI) {
        return new FactionChampionsAvoidAoeAction(botAI);
    }

    static Action* faction_champions_mass_dispel(PlayerbotAI* botAI) {
        return new FactionChampionsMassDispelAction(botAI);
    }

    static Action* faction_champions_dispel_cc(PlayerbotAI* botAI) {
        return new FactionChampionsDispelCcAction(botAI);
    }

    static Action* faction_champions_kill_totem(PlayerbotAI* botAI) {
        return new FactionChampionsKillTotemAction(botAI);
    }

    static Action* faction_champions_physical_switch(PlayerbotAI* botAI) {
        return new FactionChampionsPhysicalSwitchAction(botAI);
    }

    static Action* faction_champions_purge_kill_target(PlayerbotAI* botAI) {
        return new FactionChampionsPurgeKillTargetAction(botAI);
    }

    static Action* faction_champions_death_grip(PlayerbotAI* botAI) {
        return new FactionChampionsDeathGripAction(botAI);
    }

    static Action* faction_champions_chain_grip(PlayerbotAI* botAI) {
        return new FactionChampionsChainGripAction(botAI);
    }

    static Action* faction_champions_chain_request(PlayerbotAI* botAI) {
        return new FactionChampionsChainRequestAction(botAI);
    }

    static Action* faction_champions_wall_grip(PlayerbotAI* botAI) {
        return new FactionChampionsWallGripAction(botAI);
    }

    static Action* faction_champions_freedom(PlayerbotAI* botAI) {
        return new FactionChampionsFreedomAction(botAI);
    }

    static Action* faction_champions_channel_interrupt(PlayerbotAI* botAI) {
        return new FactionChampionsChannelInterruptAction(botAI);
    }

    static Action* faction_champions_snare(PlayerbotAI* botAI) {
        return new FactionChampionsSnareAction(botAI);
    }

    static Action* faction_champions_crowd_control(PlayerbotAI* botAI) {
        return new FactionChampionsCrowdControlAction(botAI);
    }

    static Action* faction_champions_peel(PlayerbotAI* botAI) {
        return new FactionChampionsPeelAction(botAI);
    }

    static Action* faction_champions_protect(PlayerbotAI* botAI) {
        return new FactionChampionsProtectAction(botAI);
    }

    static Action* faction_champions_battle_res(PlayerbotAI* botAI) {
        return new FactionChampionsBattleResAction(botAI);
    }

    static Action* faction_champions_personal_defensive(PlayerbotAI* botAI) {
        return new FactionChampionsPersonalDefensiveAction(botAI);
    }
};

#endif
