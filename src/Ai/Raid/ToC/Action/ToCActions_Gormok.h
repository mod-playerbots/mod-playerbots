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
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"
#include "ToCActions_Shared.h"

class GormokTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    static constexpr char const* Name = "gormok tank hold boss";

    GormokTankHoldBossAction(PlayerbotAI* botAI) : ToCMainTankHoldAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// Never taunts, that would pull him off the holder between swaps. The swap itself is
// "gormok tank swap taunt".
class GormokSwapTankHoldAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "gormok swap tank hold";

    GormokSwapTankHoldAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class GormokTankSwapTauntAction : public AttackAction
{
public:
    static constexpr char const* Name = "gormok tank swap taunt";

    GormokTankSwapTauntAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class GormokTankDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "gormok tank defensive";

    GormokTankDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Hand of Protection on the tank dealt to this paladin bot: the immunity purges Impale
class GormokHandOfProtectionAction : public Action
{
public:
    static constexpr char const* Name = "gormok hand of protection";

    GormokHandOfProtectionAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class GormokCancelHandOfProtectionAction : public Action
{
public:
    static constexpr char const* Name = "gormok cancel hand of protection";

    GormokCancelHandOfProtectionAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class GormokBringSnoboldToMeleeAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "gormok bring snobold to melee";

    GormokBringSnoboldToMeleeAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class GormokLeaveStompRangeAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "gormok leave stomp range";

    GormokLeaveStompRangeAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    // False when no spot cleared the young bombs and the walk took one clear of the stomp alone
    bool _spotClearsBombs = false;
};

class GormokDodgeFireBombAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "gormok dodge fire bomb";

    GormokDodgeFireBombAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
