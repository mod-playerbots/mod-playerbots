/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_GORMOK_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_GORMOK_H

#include <vector>

#include "NamedObjectContext.h"
#include "ToCTriggers_NorthrendBeasts.h"
#include "Trigger.h"

class GormokTankDutyTrigger : public Trigger
{
public:
    GormokTankDutyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok tank duty") {}
    bool IsActive() override;
};

class GormokSwapTankDutyTrigger : public Trigger
{
public:
    GormokSwapTankDutyTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok swap tank duty") {}
    bool IsActive() override;
};

class GormokTankSwapNeededTrigger : public Trigger
{
public:
    GormokTankSwapNeededTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok tank swap needed") {}
    bool IsActive() override;
};

class GormokTankDefensiveTrigger : public Trigger
{
public:
    GormokTankDefensiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok tank defensive") {}
    bool IsActive() override;
};

// This paladin bot is dealt a Hand of Protection to clear a tank's Impale
class GormokImpaleResetTrigger : public Trigger
{
public:
    GormokImpaleResetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok impale reset") {}
    bool IsActive() override;
};

class GormokCancelHandOfProtectionTrigger : public Trigger
{
public:
    GormokCancelHandOfProtectionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok cancel hand of protection") {}
    bool IsActive() override;
};

class GormokSnobolledTrigger : public Trigger
{
public:
    GormokSnobolledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok snobolled") {}
    bool IsActive() override;
};

class GormokStompRangeTrigger : public Trigger
{
public:
    GormokStompRangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok stomp range") {}
    bool IsActive() override;
};

class GormokFireBombIncomingTrigger : public Trigger
{
public:
    GormokFireBombIncomingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gormok fire bomb incoming") {}
    bool IsActive() override;
};

class ToCGormokTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCGormokTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["gormok tank duty"] =
            &ToCGormokTriggerContext::gormok_tank_duty;
        creators["gormok swap tank duty"] =
            &ToCGormokTriggerContext::gormok_swap_tank_duty;
        creators["gormok dps target"] =
            &ToCGormokTriggerContext::gormok_dps_target;
        creators["gormok tank swap needed"] =
            &ToCGormokTriggerContext::gormok_tank_swap_needed;
        creators["gormok tank defensive"] =
            &ToCGormokTriggerContext::gormok_tank_defensive;
        creators["gormok impale reset"] =
            &ToCGormokTriggerContext::gormok_impale_reset;
        creators["gormok cancel hand of protection"] =
            &ToCGormokTriggerContext::gormok_cancel_hand_of_protection;
        creators["gormok snobolled"] =
            &ToCGormokTriggerContext::gormok_snobolled;
        creators["gormok stomp range"] =
            &ToCGormokTriggerContext::gormok_stomp_range;
        creators["gormok fire bomb incoming"] =
            &ToCGormokTriggerContext::gormok_fire_bomb_incoming;
    }

private:
    static Trigger* gormok_tank_duty(PlayerbotAI* botAI) {
        return new GormokTankDutyTrigger(botAI);
    }

    static Trigger* gormok_swap_tank_duty(PlayerbotAI* botAI) {
        return new GormokSwapTankDutyTrigger(botAI);
    }

    static Trigger* gormok_dps_target(PlayerbotAI* botAI) {
        return new NorthrendBeastsDpsTargetTrigger(botAI, "gormok dps target", NorthrendBeastsStem::Gormok);
    }

    static Trigger* gormok_tank_swap_needed(PlayerbotAI* botAI) {
        return new GormokTankSwapNeededTrigger(botAI);
    }

    static Trigger* gormok_tank_defensive(PlayerbotAI* botAI) {
        return new GormokTankDefensiveTrigger(botAI);
    }

    static Trigger* gormok_impale_reset(PlayerbotAI* botAI) {
        return new GormokImpaleResetTrigger(botAI);
    }

    static Trigger* gormok_cancel_hand_of_protection(PlayerbotAI* botAI) {
        return new GormokCancelHandOfProtectionTrigger(botAI);
    }

    static Trigger* gormok_snobolled(PlayerbotAI* botAI) {
        return new GormokSnobolledTrigger(botAI);
    }

    static Trigger* gormok_stomp_range(PlayerbotAI* botAI) {
        return new GormokStompRangeTrigger(botAI);
    }

    static Trigger* gormok_fire_bomb_incoming(PlayerbotAI* botAI) {
        return new GormokFireBombIncomingTrigger(botAI);
    }
};

void AddToCGormokTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
