/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_TWINVALKYR_H
#define PLAYERBOTS_RAID_TOCACTIONS_TWINVALKYR_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "RaidDirectTargeting.h"
#include "RaidRedirectThreat.h"
#include "ToCActions_Shared.h"
#include "ToCHelpers_TwinValkyr.h"

class TwinValkyrInterruptPactAction : public Action
{
public:
    static constexpr char const* Name = "twin valkyr interrupt pact";

    TwinValkyrInterruptPactAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Runs only while GetWantedEssence names this action's reason, and walks to that colour's portal.
class TwinValkyrEssenceActionBase : public MovementAction
{
public:
    TwinValkyrEssenceActionBase(PlayerbotAI* botAI, std::string const name,
                                TrialOfTheCrusaderHelpers::TwinEssenceReason reason)
        : MovementAction(botAI, name), _reason(reason) {}
    bool Execute(Event event) override;

protected:
    bool AcquireEssence(TrialOfTheCrusaderHelpers::TwinColour colour);

private:
    TrialOfTheCrusaderHelpers::TwinEssenceReason const _reason;
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrSwapEssenceForTouchAction : public TwinValkyrEssenceActionBase
{
public:
    static constexpr char const* Name = "twin valkyr swap essence for touch";

    TwinValkyrSwapEssenceForTouchAction(PlayerbotAI* botAI)
        : TwinValkyrEssenceActionBase(botAI, Name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Touch) {}
};

class TwinValkyrSwapEssenceForVortexAction : public TwinValkyrEssenceActionBase
{
public:
    static constexpr char const* Name = "twin valkyr swap essence for vortex";

    TwinValkyrSwapEssenceForVortexAction(PlayerbotAI* botAI)
        : TwinValkyrEssenceActionBase(botAI, Name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Vortex) {}
};

class TwinValkyrTakeBaseEssenceAction : public TwinValkyrEssenceActionBase
{
public:
    static constexpr char const* Name = "twin valkyr take base essence";

    TwinValkyrTakeBaseEssenceAction(PlayerbotAI* botAI)
        : TwinValkyrEssenceActionBase(botAI, Name, TrialOfTheCrusaderHelpers::TwinEssenceReason::Base) {}
};

class TwinValkyrWalkToNextPortalAction : public MovementAction
{
public:
    static constexpr char const* Name = "twin valkyr walk to next portal";

    TwinValkyrWalkToNextPortalAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrCastVortexRaidCoverAction : public Action
{
public:
    static constexpr char const* Name = "twin valkyr cast vortex raid cover";

    TwinValkyrCastVortexRaidCoverAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class TwinValkyrCastVortexDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "twin valkyr cast vortex defensive";

    TwinValkyrCastVortexDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

class TwinValkyrMoveToDoorStandAction : public MovementAction
{
public:
    static constexpr char const* Name = "twin valkyr move to door stand";

    TwinValkyrMoveToDoorStandAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    TrialOfTheCrusaderHelpers::TwinWaitBudget _orbWait;
};

class TwinValkyrDodgeOrbAction : public MovementAction
{
public:
    static constexpr char const* Name = "twin valkyr dodge orb";

    TwinValkyrDodgeOrbAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
    bool Execute(Event event) override;

private:
    Position _dodgeSpot;
    uint32 _dodgeSpotMs = 0;
};

// Waits on its door spot for its twin and taunts her in. Walks out only to fetch one that stopped
// short of taunt range, and back by a point 5 yd out of the spot so she parks on the arena side.
class TwinValkyrTankHoldAction : public ToCMainTankHoldAction
{
public:
    TwinValkyrTankHoldAction(PlayerbotAI* botAI, std::string const name) : ToCMainTankHoldAction(botAI, name) {}

protected:
    bool HoldTwin(Unit* twin);

private:
    bool FetchTwin(Unit* twin);
    bool WalkToDoor(Unit* twin);
    bool WalkFacingTwin(Position const& to, Unit* twin);

    bool _doorWalk = false;
    // back from a fetch, headed for the lead point first
    bool _leading = false;
    uint32 _leadReachedMs = 0;
};

class TwinValkyrMainTankHoldLightTwinAction : public TwinValkyrTankHoldAction
{
public:
    static constexpr char const* Name = "twin valkyr main tank hold light twin";

    TwinValkyrMainTankHoldLightTwinAction(PlayerbotAI* botAI) : TwinValkyrTankHoldAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class TwinValkyrAssistTankHoldDarkTwinAction : public TwinValkyrTankHoldAction
{
public:
    static constexpr char const* Name = "twin valkyr assist tank hold dark twin";

    TwinValkyrAssistTankHoldDarkTwinAction(PlayerbotAI* botAI) : TwinValkyrTankHoldAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class TwinValkyrRedirectThreatAction : public RaidRedirectThreatAction
{
public:
    static constexpr char const* Name = "twin valkyr redirect threat";

    TwinValkyrRedirectThreatAction(PlayerbotAI* botAI) : RaidRedirectThreatAction(botAI, Name) {}

protected:
    Player* GetRedirectTank() override;
    Unit* GetThreatDumpTarget() override;
};

class TwinValkyrFocusTwinAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "twin valkyr focus twin";

    TwinValkyrFocusTwinAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
