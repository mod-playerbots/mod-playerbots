/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDTRIGGERS_IRONASSEMBLY_H
#define PLAYERBOTS_ULDTRIGGERS_IRONASSEMBLY_H

#include "EventMap.h"
#include "GenericTriggers.h"
#include "RaidResistAura.h"
#include "Trigger.h"
#include "UldEncounter_IronAssembly.h"

// Iron Assembly

// All three back at full health, which only happens on a fresh pull.
class IronAssemblyResetEncounterStateTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly reset encounter state trigger";

    IronAssemblyResetEncounterStateTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, Name)
    {
    }
    bool IsActive() override;
};

class IronAssemblyLightningTendrilsTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly lightning tendrils trigger";

    IronAssemblyLightningTendrilsTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyOverloadTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly overload trigger";

    IronAssemblyOverloadTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// 200 ms rather than every tick: the rune lives 30s on a 30-40s cadence, so a fraction of a second
// late costs one tick of damage and the sweep is a grid search across the whole raid.
class IronAssemblyRuneOfDeathTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly rune of death trigger";

    IronAssemblyRuneOfDeathTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name, 200) {}
    bool IsActive() override;
};

// Brundir is the only member that can be interrupted or stunned at all; Steelbreaker and Molgeim are
// immune to both.
class IronAssemblyInterruptTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly interrupt trigger";

    IronAssemblyInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// One tank per living member, Brundir's spot first.
class IronAssemblyTankAssignmentTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly tank assignment trigger";

    IronAssemblyTankAssignmentTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Shield of Runes pays Molgeim +50% damage for 15s if it is drained rather than removed, so
// stripping it is worth more than the global costs.
class IronAssemblyShieldOfRunesTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly shield of runes trigger";

    IronAssemblyShieldOfRunesTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyFusionPunchDispelTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly fusion punch dispel trigger";

    IronAssemblyFusionPunchDispelTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly redirect threat trigger";

    IronAssemblyRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Melee step into the focus's Rune of Power behind him. Ranged have no node: the rune is their raid spot,
// and the tank's spot backs his boss off it.
class IronAssemblyRuneOfPowerMeleeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly rune of power melee trigger";

    IronAssemblyRuneOfPowerMeleeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// The other tank has Steelbreaker off an Overwhelming Power carrier, who walks to the Meltdown point.
class IronAssemblyMeltdownRunOutTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly meltdown run out trigger";

    IronAssemblyMeltdownRunOutTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

// Empowered Steelbreaker is casting Fusion Punch at this tank and it has a magic defensive for it.
class IronAssemblyFusionPunchDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly fusion punch defensive trigger";

    IronAssemblyFusionPunchDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyNatureAspectTrigger : public RaidResistAuraTrigger
{
public:
    static constexpr char const* Name = "iron assembly nature aspect trigger";

    IronAssemblyNatureAspectTrigger(PlayerbotAI* botAI)
        : RaidResistAuraTrigger(botAI, Name, ULD_IRON_ASSEMBLY_RESIST_AURA)
    {
    }
};

// The totem shaman's air button or totem isn't Nature Resistance, or it walked away from the totem.
class IronAssemblyNatureTotemTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly nature totem trigger";

    IronAssemblyNatureTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyUnheldTauntTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly unheld taunt trigger";

    IronAssemblyUnheldTauntTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblySetDpsPriorityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly set dps priority trigger";

    IronAssemblySetDpsPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class IronAssemblyRaidPositionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "iron assembly raid position trigger";

    IronAssemblyRaidPositionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
