/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ULDACTIONS_IRONASSEMBLY_H
#define PLAYERBOTS_ULDACTIONS_IRONASSEMBLY_H

#include "Action.h"
#include "AttackAction.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RaidResistAura.h"
#include "UldEncounter_IronAssembly.h"
#include "UldTriggers.h"

class IronAssemblyResetEncounterStateAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly reset encounter state action";

    IronAssemblyResetEncounterStateAction(PlayerbotAI* botAI)
        : Action(botAI, Name)
    {
    }
    bool Execute(Event event) override;
};

class IronAssemblyLightningTendrilsAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly lightning tendrils action";

    IronAssemblyLightningTendrilsAction(PlayerbotAI* botAI)
        : MovementAction(botAI, Name)
    {
    }
    bool Execute(Event event) override;
};

class IronAssemblyOverloadAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly overload action";

    IronAssemblyOverloadAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyRuneOfDeathAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly rune of death action";

    IronAssemblyRuneOfDeathAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyInterruptAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly interrupt action";

    IronAssemblyInterruptAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Target, taunt and park in one node: a tank that has its boss but is standing in the wrong place is
// as wrong as one that does not have it.
class IronAssemblyTankAssignmentAction : public AttackAction
{
public:
    static constexpr char const* Name = "iron assembly tank assignment action";

    IronAssemblyTankAssignmentAction(PlayerbotAI* botAI) : AttackAction(botAI, Name)
    {
    }
    bool Execute(Event event) override;

private:
    bool _spotReached = false;
};

class IronAssemblyShieldOfRunesAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly shield of runes action";

    IronAssemblyShieldOfRunesAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyFusionPunchDispelAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly fusion punch dispel action";

    IronAssemblyFusionPunchDispelAction(PlayerbotAI* botAI)
        : Action(botAI, Name)
    {
    }
    bool Execute(Event event) override;
};

// Standalone rather than built on the Naxx redirect base: Ulduar has no other Naxx include, and
// Freya's redirect is the same shape one file over.
class IronAssemblyRedirectThreatAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly redirect threat action";

    IronAssemblyRedirectThreatAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Player* GetRedirectTank();
};

class IronAssemblyRuneOfPowerMeleeAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly rune of power melee action";

    IronAssemblyRuneOfPowerMeleeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyMeltdownRunOutAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly meltdown run out action";

    IronAssemblyMeltdownRunOutAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// The weakest magic defensive that covers the cast's remaining time.
class IronAssemblyFusionPunchDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly fusion punch defensive action";

    IronAssemblyFusionPunchDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyNatureAspectAction : public RaidRaiseResistAuraAction
{
public:
    static constexpr char const* Name = "iron assembly nature aspect action";

    IronAssemblyNatureAspectAction(PlayerbotAI* botAI)
        : RaidRaiseResistAuraAction(botAI, Name, ULD_IRON_ASSEMBLY_RESIST_AURA)
    {
    }
};

// Cast directly, not through ChangeStrategy: nothing removes an added "nature resistance", so it would
// outlive the fight. The totem hold rule keeps the shaman's own air nodes off the slot meanwhile.
class IronAssemblyNatureTotemAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly nature totem action";

    IronAssemblyNatureTotemAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblyUnheldTauntAction : public Action
{
public:
    static constexpr char const* Name = "iron assembly unheld taunt action";

    IronAssemblyUnheldTauntAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class IronAssemblySetDpsPriorityAction : public AttackAction
{
public:
    static constexpr char const* Name = "iron assembly set dps priority action";

    IronAssemblySetDpsPriorityAction(PlayerbotAI* botAI) : AttackAction(botAI, Name)
    {
    }
    bool Execute(Event event) override;
};

class IronAssemblyRaidPositionAction : public MovementAction
{
public:
    static constexpr char const* Name = "iron assembly raid position action";

    IronAssemblyRaidPositionAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    bool _spotReached = false;
};

#endif
