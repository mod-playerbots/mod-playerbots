/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_JARAXXUS_H
#define PLAYERBOTS_RAID_TOCACTIONS_JARAXXUS_H

#include <string>

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_Shared.h"

class JaraxxusInterruptFelFireballAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus interrupt fel fireball";

    JaraxxusInterruptFelFireballAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusAvoidLegionFlameAction : public MovementAction
{
public:
    static constexpr char const* Name = "jaraxxus avoid legion flame";

    JaraxxusAvoidLegionFlameAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;

private:
    void ClearLatches();
    bool IssueLeg(Position const& spot, uint32 now);

    Position _spot;
    Position _legStart;
    Position _lastIssued;
    bool _hasSpot = false;
    uint32 _issuedMs = 0;
    float _heading = 0.0f;
    bool _hasHeading = false;
    uint32 _lastTickMs = 0;
};

class JaraxxusBreakPinnedCastAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus break pinned cast";

    JaraxxusBreakPinnedCastAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusLeavePortalAction : public MovementAction
{
public:
    static constexpr char const* Name = "jaraxxus leave portal";

    JaraxxusLeavePortalAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {};
    bool Execute(Event event) override;

private:
    Position _spot;
    bool _hasSpot = false;
    uint32 _issuedMs = 0;
};

class JaraxxusCastRaidCoverAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus cast raid cover";

    JaraxxusCastRaidCoverAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusCastPersonalDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus cast personal defensive";

    JaraxxusCastPersonalDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusMainTankHoldBossAction : public ToCMainTankHoldAction
{
public:
    static constexpr char const* Name = "jaraxxus main tank hold boss";

    JaraxxusMainTankHoldBossAction(PlayerbotAI* botAI) : ToCMainTankHoldAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusAssistTankHoldAction : public AttackAction
{
public:
    JaraxxusAssistTankHoldAction(PlayerbotAI* botAI, std::string const name) : AttackAction(botAI, name) {};

protected:
    bool HoldAdd(uint8 index);
};

class JaraxxusAssistTankHoldAddAction : public JaraxxusAssistTankHoldAction
{
public:
    static constexpr char const* Name = "jaraxxus assist tank hold add";

    JaraxxusAssistTankHoldAddAction(PlayerbotAI* botAI) : JaraxxusAssistTankHoldAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusAssistTankHoldSecondAddAction : public JaraxxusAssistTankHoldAction
{
public:
    static constexpr char const* Name = "jaraxxus assist tank hold second add";

    JaraxxusAssistTankHoldSecondAddAction(PlayerbotAI* botAI) : JaraxxusAssistTankHoldAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusRemoveNetherPowerAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus remove nether power";

    JaraxxusRemoveNetherPowerAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusDpsTargetAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "jaraxxus dps target";

    JaraxxusDpsTargetAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {};
    bool Execute(Event event) override;
};

class JaraxxusHealIncinerateTargetAction : public Action
{
public:
    static constexpr char const* Name = "jaraxxus heal incinerate target";

    JaraxxusHealIncinerateTargetAction(PlayerbotAI* botAI) : Action(botAI, Name) {};
    bool Execute(Event event) override;
};

#endif
