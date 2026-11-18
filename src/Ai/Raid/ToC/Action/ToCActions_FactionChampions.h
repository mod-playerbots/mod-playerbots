/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONS_H

#include "RaidAntiFear.h"
#include "RaidDirectTargeting.h"

class FactionChampionsAntiFearAction : public RaidAntiFearAction
{
public:
    static constexpr char const* Name = "faction champions anti fear";

    FactionChampionsAntiFearAction(PlayerbotAI* botAI) : RaidAntiFearAction(botAI, Name) {}

protected:
    bool FearWindowActive() override;
};

class FactionChampionsFocusPriorityAction : public RaidDirectAttackAction
{
public:
    static constexpr char const* Name = "faction champions focus priority";

    FactionChampionsFocusPriorityAction(PlayerbotAI* botAI) : RaidDirectAttackAction(botAI, Name) {}
    bool Execute(Event event) override;
};

class FactionChampionsCounterspellKillTargetAction : public Action
{
public:
    static constexpr char const* Name = "faction champions counterspell kill target";

    FactionChampionsCounterspellKillTargetAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsPetAutocastAction : public Action
{
public:
    static constexpr char const* Name = "faction champions pet autocast";

    FactionChampionsPetAutocastAction(PlayerbotAI* botAI) : Action(botAI, Name) {}
    bool Execute(Event event) override;
};

#endif
