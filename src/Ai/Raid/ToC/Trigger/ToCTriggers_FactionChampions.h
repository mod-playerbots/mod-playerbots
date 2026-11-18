/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONS_H

#include <vector>

#include "NamedObjectContext.h"
#include "RaidAntiFear.h"
#include "Trigger.h"

class FactionChampionsAntiFearTrigger : public RaidAntiFearTrigger
{
public:
    FactionChampionsAntiFearTrigger(PlayerbotAI* botAI) : RaidAntiFearTrigger(botAI, "faction champions anti fear") {}

protected:
    bool FearWindowActive() override;
};

class FactionChampionsShouldFocusTrigger : public Trigger
{
public:
    FactionChampionsShouldFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions should focus") {}
    bool IsActive() override;
};

class FactionChampionsKillTargetHealingTrigger : public Trigger
{
public:
    FactionChampionsKillTargetHealingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "faction champions kill target healing") {}
    bool IsActive() override;
};

class FactionChampionsPetAutocastTrigger : public Trigger
{
public:
    FactionChampionsPetAutocastTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions pet autocast") {}
    bool IsActive() override;
};

class ToCFactionChampionsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCFactionChampionsTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["faction champions anti fear"] =
            &ToCFactionChampionsTriggerContext::faction_champions_anti_fear;
        creators["faction champions should focus"] =
            &ToCFactionChampionsTriggerContext::faction_champions_should_focus;
        creators["faction champions kill target healing"] =
            &ToCFactionChampionsTriggerContext::faction_champions_kill_target_healing;
        creators["faction champions pet autocast"] =
            &ToCFactionChampionsTriggerContext::faction_champions_pet_autocast;
    }

private:
    static Trigger* faction_champions_anti_fear(PlayerbotAI* botAI) {
        return new FactionChampionsAntiFearTrigger(botAI);
    }

    static Trigger* faction_champions_should_focus(PlayerbotAI* botAI) {
        return new FactionChampionsShouldFocusTrigger(botAI);
    }

    static Trigger* faction_champions_kill_target_healing(PlayerbotAI* botAI) {
        return new FactionChampionsKillTargetHealingTrigger(botAI);
    }

    static Trigger* faction_champions_pet_autocast(PlayerbotAI* botAI) {
        return new FactionChampionsPetAutocastTrigger(botAI);
    }
};

void AddToCFactionChampionsTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
