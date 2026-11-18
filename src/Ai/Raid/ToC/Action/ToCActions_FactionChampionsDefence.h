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
#include "Position.h"
#include "RaidDirectTargeting.h"
#include "ToCActions_NorthrendBeasts.h"

// Walks out of a Bladestorm or Hellfire, the two champion AoEs a bot can see coming
class FactionChampionsAvoidAoeAction : public MovementAction
{
public:
    static constexpr char const* Name = "faction champions avoid aoe";

    FactionChampionsAvoidAoeAction(PlayerbotAI* botAI) : MovementAction(botAI, Name) {}
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
    static constexpr char const* Name = "faction champions mass dispel";

    FactionChampionsMassDispelAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsDispelCcAction : public Action
{
public:
    static constexpr char const* Name = "faction champions dispel cc";

    FactionChampionsDispelCcAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsKillTotemAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "faction champions kill totem";

    FactionChampionsKillTotemAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsPhysicalSwitchAction : public AttackAction
{
public:
    static constexpr char const* Name = "faction champions physical switch";

    FactionChampionsPhysicalSwitchAction(PlayerbotAI* botAI) : AttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsPurgeKillTargetAction : public Action
{
public:
    static constexpr char const* Name = "faction champions purge kill target";

    FactionChampionsPurgeKillTargetAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Walks the DK to its latched stand, then grips the kill target to it
class FactionChampionsDeathGripAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions death grip";

    FactionChampionsDeathGripAction(PlayerbotAI* botAI, std::string const name = Name)
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
    static constexpr char const* Name = "faction champions chain grip";

    FactionChampionsChainGripAction(PlayerbotAI* botAI) : FactionChampionsDeathGripAction(botAI, Name) {}
    bool Execute(Event event) override;
};

// A bot DK opens the grip chain on the player's target when the player types pull
class FactionChampionsChainRequestAction : public Action
{
public:
    static constexpr char const* Name = "faction champions chain request";

    FactionChampionsChainRequestAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

// Walks the DK to its wall stand, grips the melee champion there, then slows it with Chains of Ice
class FactionChampionsWallGripAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions wall grip";

    FactionChampionsWallGripAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsFreedomAction : public Action
{
public:
    static constexpr char const* Name = "faction champions freedom";

    FactionChampionsFreedomAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsChannelInterruptAction : public Action
{
public:
    static constexpr char const* Name = "faction champions channel interrupt";

    FactionChampionsChannelInterruptAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsSnareAction : public Action
{
public:
    static constexpr char const* Name = "faction champions snare";

    FactionChampionsSnareAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// Walks into spell range first when the duty was dealt from out of it
class FactionChampionsCrowdControlAction : public NorthrendBeastsWalkAction
{
public:
    static constexpr char const* Name = "faction champions crowd control";

    FactionChampionsCrowdControlAction(PlayerbotAI* botAI) : NorthrendBeastsWalkAction(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsPeelAction : public Action
{
public:
    static constexpr char const* Name = "faction champions peel";

    FactionChampionsPeelAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsProtectAction : public Action
{
public:
    static constexpr char const* Name = "faction champions protect";

    FactionChampionsProtectAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsBattleResAction : public Action
{
public:
    static constexpr char const* Name = "faction champions battle res";

    FactionChampionsBattleResAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
    std::vector<NextAction> getPrerequisites() override;
};

class FactionChampionsPersonalDefensiveAction : public Action
{
public:
    static constexpr char const* Name = "faction champions personal defensive";

    FactionChampionsPersonalDefensiveAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
