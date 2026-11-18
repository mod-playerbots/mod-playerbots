/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSDEFENCE_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSDEFENCE_H

#include "Trigger.h"

class FactionChampionsCrowdControlTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions crowd control";

    FactionChampionsCrowdControlTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsPeelTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions peel";

    FactionChampionsPeelTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsAvoidAoeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions avoid aoe";

    FactionChampionsAvoidAoeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsMassDispelTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions mass dispel";

    FactionChampionsMassDispelTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsDispelCcTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions dispel cc";

    FactionChampionsDispelCcTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsKillTotemTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions kill totem";

    FactionChampionsKillTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsPhysicalSwitchTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions physical switch";

    FactionChampionsPhysicalSwitchTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsPurgeKillTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions purge kill target";

    FactionChampionsPurgeKillTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsChannelInterruptTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions channel interrupt";

    FactionChampionsChannelInterruptTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsDeathGripTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions death grip";

    FactionChampionsDeathGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsChainGripTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions chain grip";

    FactionChampionsChainGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsWallGripTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions wall grip";

    FactionChampionsWallGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsFreedomTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions freedom";

    FactionChampionsFreedomTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsSnareTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions snare";

    FactionChampionsSnareTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsProtectTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions protect";

    FactionChampionsProtectTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsBattleResTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions battle res";

    FactionChampionsBattleResTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsPersonalDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions personal defensive";

    FactionChampionsPersonalDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
