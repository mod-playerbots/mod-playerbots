/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCACTIONS_FACTIONCHAMPIONS_H

#include "NamedObjectContext.h"
#include "RaidAntiFear.h"
#include "RaidDirectTargeting.h"

class FactionChampionsAntiFearAction : public RaidAntiFearAction
{
public:
    FactionChampionsAntiFearAction(PlayerbotAI* botAI) : RaidAntiFearAction(botAI, "faction champions anti fear") {}

protected:
    bool FearWindowActive() override;
};

class FactionChampionsFocusPriorityAction : public RaidDirectAttackAction
{
public:
    FactionChampionsFocusPriorityAction(PlayerbotAI* botAI)
        : RaidDirectAttackAction(botAI, "faction champions focus priority") {}
    bool Execute(Event event) override;
};

class FactionChampionsCounterspellKillTargetAction : public Action
{
public:
    FactionChampionsCounterspellKillTargetAction(
        PlayerbotAI* botAI) : Action(botAI, "faction champions counterspell kill target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

class FactionChampionsPetAutocastAction : public Action
{
public:
    FactionChampionsPetAutocastAction(PlayerbotAI* botAI) : Action(botAI, "faction champions pet autocast") {}
    bool Execute(Event event) override;
};

class ToCFactionChampionsActionContext : public NamedObjectContext<Action>
{
public:
    ToCFactionChampionsActionContext()
    {
        creators["faction champions anti fear"] =
            &ToCFactionChampionsActionContext::faction_champions_anti_fear;
        creators["faction champions focus priority"] =
            &ToCFactionChampionsActionContext::faction_champions_focus_priority;
        creators["faction champions counterspell kill target"] =
            &ToCFactionChampionsActionContext::faction_champions_counterspell_kill_target;
        creators["faction champions pet autocast"] =
            &ToCFactionChampionsActionContext::faction_champions_pet_autocast;
    }

private:
    static Action* faction_champions_anti_fear(PlayerbotAI* botAI) {
        return new FactionChampionsAntiFearAction(botAI);
    }

    static Action* faction_champions_focus_priority(PlayerbotAI* botAI) {
        return new FactionChampionsFocusPriorityAction(botAI);
    }

    static Action* faction_champions_counterspell_kill_target(PlayerbotAI* botAI) {
        return new FactionChampionsCounterspellKillTargetAction(botAI);
    }

    static Action* faction_champions_pet_autocast(PlayerbotAI* botAI) {
        return new FactionChampionsPetAutocastAction(botAI);
    }
};

#endif
