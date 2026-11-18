/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_GORMOK_H
#define PLAYERBOTS_RAID_TOCACTIONS_GORMOK_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "NamedObjectContext.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCActions_Shared.h"

class GormokTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    GormokTankHoldBossAction(PlayerbotAI* botAI, std::string const name = "gormok tank hold boss")
        : ToCMainTankHoldAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Never taunts, that would pull him off the holder between swaps. The swap itself is
// "gormok tank swap taunt".
class GormokSwapTankHoldAction : public RaidDirectAttackAction
{
public:
    GormokSwapTankHoldAction(PlayerbotAI* botAI, std::string const name = "gormok swap tank hold")
        : RaidDirectAttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class GormokTankSwapTauntAction : public AttackAction
{
public:
    GormokTankSwapTauntAction(
        PlayerbotAI* botAI, std::string const name = "gormok tank swap taunt") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class GormokTankDefensiveAction : public Action
{
public:
    GormokTankDefensiveAction(
        PlayerbotAI* botAI, std::string const name = "gormok tank defensive") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Hand of Protection on the tank dealt to this paladin bot: the immunity purges Impale
class GormokHandOfProtectionAction : public Action
{
public:
    GormokHandOfProtectionAction(
        PlayerbotAI* botAI, std::string const name = "gormok hand of protection") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class GormokCancelHandOfProtectionAction : public Action
{
public:
    GormokCancelHandOfProtectionAction(
        PlayerbotAI* botAI, std::string const name = "gormok cancel hand of protection") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class GormokBringSnoboldToMeleeAction : public NorthrendBeastsWalkAction
{
public:
    GormokBringSnoboldToMeleeAction(PlayerbotAI* botAI, std::string const name = "gormok bring snobold to melee")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;
};

class GormokLeaveStompRangeAction : public NorthrendBeastsWalkAction
{
public:
    GormokLeaveStompRangeAction(PlayerbotAI* botAI, std::string const name = "gormok leave stomp range")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    // False when no spot cleared the young bombs and the walk took one clear of the stomp alone
    bool _spotClearsBombs = false;
};

class GormokDodgeFireBombAction : public NorthrendBeastsWalkAction
{
public:
    GormokDodgeFireBombAction(PlayerbotAI* botAI, std::string const name = "gormok dodge fire bomb")
        : NorthrendBeastsWalkAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ToCGormokActionContext : public NamedObjectContext<Action>
{
public:
    ToCGormokActionContext()
    {
        creators["gormok tank hold boss"] =
            &ToCGormokActionContext::gormok_tank_hold_boss;
        creators["gormok swap tank hold"] =
            &ToCGormokActionContext::gormok_swap_tank_hold;
        creators["gormok dps target"] =
            &ToCGormokActionContext::gormok_dps_target;
        creators["gormok tank swap taunt"] =
            &ToCGormokActionContext::gormok_tank_swap_taunt;
        creators["gormok tank defensive"] =
            &ToCGormokActionContext::gormok_tank_defensive;
        creators["gormok hand of protection"] =
            &ToCGormokActionContext::gormok_hand_of_protection;
        creators["gormok cancel hand of protection"] =
            &ToCGormokActionContext::gormok_cancel_hand_of_protection;
        creators["gormok bring snobold to melee"] =
            &ToCGormokActionContext::gormok_bring_snobold_to_melee;
        creators["gormok leave stomp range"] =
            &ToCGormokActionContext::gormok_leave_stomp_range;
        creators["gormok dodge fire bomb"] =
            &ToCGormokActionContext::gormok_dodge_fire_bomb;
    }

private:
    static Action* gormok_tank_hold_boss(PlayerbotAI* botAI) {
        return new GormokTankHoldBossAction(botAI);
    }

    static Action* gormok_swap_tank_hold(PlayerbotAI* botAI) {
        return new GormokSwapTankHoldAction(botAI);
    }

    static Action* gormok_dps_target(PlayerbotAI* botAI) {
        return new NorthrendBeastsDpsTargetAction(botAI, "gormok dps target");
    }

    static Action* gormok_tank_swap_taunt(PlayerbotAI* botAI) {
        return new GormokTankSwapTauntAction(botAI);
    }

    static Action* gormok_tank_defensive(PlayerbotAI* botAI) {
        return new GormokTankDefensiveAction(botAI);
    }

    static Action* gormok_hand_of_protection(PlayerbotAI* botAI) {
        return new GormokHandOfProtectionAction(botAI);
    }

    static Action* gormok_cancel_hand_of_protection(PlayerbotAI* botAI) {
        return new GormokCancelHandOfProtectionAction(botAI);
    }

    static Action* gormok_bring_snobold_to_melee(PlayerbotAI* botAI) {
        return new GormokBringSnoboldToMeleeAction(botAI);
    }

    static Action* gormok_leave_stomp_range(PlayerbotAI* botAI) {
        return new GormokLeaveStompRangeAction(botAI);
    }

    static Action* gormok_dodge_fire_bomb(PlayerbotAI* botAI) {
        return new GormokDodgeFireBombAction(botAI);
    }
};

#endif
