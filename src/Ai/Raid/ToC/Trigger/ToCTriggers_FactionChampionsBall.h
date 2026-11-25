/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSBALL_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSBALL_H

#include "Trigger.h"

class FactionChampionsIceTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions ice target";

    FactionChampionsIceTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsChainsOfIceTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions chains of ice";

    FactionChampionsChainsOfIceTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsIceReachTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions ice reach";

    FactionChampionsIceReachTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsCycloneStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions cyclone stand";

    FactionChampionsCycloneStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsStepOffTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions step off";

    FactionChampionsStepOffTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsChasedTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions chased";

    FactionChampionsChasedTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsKiteTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions kite";

    FactionChampionsKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsSpeedBoostTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions speed boost";

    FactionChampionsSpeedBoostTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsOpenerTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions opener";

    FactionChampionsOpenerTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsEscapeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions escape";

    FactionChampionsEscapeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsDemonicCircleTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions demonic circle";

    FactionChampionsDemonicCircleTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
