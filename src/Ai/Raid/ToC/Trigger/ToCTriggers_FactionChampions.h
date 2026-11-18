/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONS_H

#include "RaidAntiFear.h"
#include "Trigger.h"

class FactionChampionsAntiFearTrigger : public RaidAntiFearTrigger
{
public:
    static constexpr char const* Name = "faction champions anti fear";

    FactionChampionsAntiFearTrigger(PlayerbotAI* botAI) : RaidAntiFearTrigger(botAI, Name) {}

protected:
    bool FearWindowActive() override;
};

class FactionChampionsShouldFocusTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions should focus";

    FactionChampionsShouldFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsKillTargetHealingTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions kill target healing";

    FactionChampionsKillTargetHealingTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class FactionChampionsPetAutocastTrigger : public Trigger
{
public:
    static constexpr char const* Name = "faction champions pet autocast";

    FactionChampionsPetAutocastTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
