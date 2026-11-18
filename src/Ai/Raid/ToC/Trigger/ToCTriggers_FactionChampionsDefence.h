/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSDEFENCE_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSDEFENCE_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

class FactionChampionsCrowdControlTrigger : public Trigger
{
public:
    FactionChampionsCrowdControlTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "faction champions crowd control") {}
    bool IsActive() override;
};

class FactionChampionsPeelTrigger : public Trigger
{
public:
    FactionChampionsPeelTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions peel") {}
    bool IsActive() override;
};

class FactionChampionsAvoidAoeTrigger : public Trigger
{
public:
    FactionChampionsAvoidAoeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions avoid aoe") {}
    bool IsActive() override;
};

class FactionChampionsMassDispelTrigger : public Trigger
{
public:
    FactionChampionsMassDispelTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions mass dispel") {}
    bool IsActive() override;
};

class FactionChampionsDispelCcTrigger : public Trigger
{
public:
    FactionChampionsDispelCcTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions dispel cc") {}
    bool IsActive() override;
};

class FactionChampionsKillTotemTrigger : public Trigger
{
public:
    FactionChampionsKillTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions kill totem") {}
    bool IsActive() override;
};

class FactionChampionsPhysicalSwitchTrigger : public Trigger
{
public:
    FactionChampionsPhysicalSwitchTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "faction champions physical switch") {}
    bool IsActive() override;
};

class FactionChampionsPurgeKillTargetTrigger : public Trigger
{
public:
    FactionChampionsPurgeKillTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "faction champions purge kill target") {}
    bool IsActive() override;
};

class FactionChampionsChannelInterruptTrigger : public Trigger
{
public:
    FactionChampionsChannelInterruptTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "faction champions channel interrupt") {}
    bool IsActive() override;
};

class FactionChampionsDeathGripTrigger : public Trigger
{
public:
    FactionChampionsDeathGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions death grip") {}
    bool IsActive() override;
};

class FactionChampionsChainGripTrigger : public Trigger
{
public:
    FactionChampionsChainGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions chain grip") {}
    bool IsActive() override;
};

class FactionChampionsWallGripTrigger : public Trigger
{
public:
    FactionChampionsWallGripTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions wall grip") {}
    bool IsActive() override;
};

class FactionChampionsFreedomTrigger : public Trigger
{
public:
    FactionChampionsFreedomTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions freedom") {}
    bool IsActive() override;
};

class FactionChampionsSnareTrigger : public Trigger
{
public:
    FactionChampionsSnareTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions snare") {}
    bool IsActive() override;
};

class FactionChampionsProtectTrigger : public Trigger
{
public:
    FactionChampionsProtectTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions protect") {}
    bool IsActive() override;
};

class FactionChampionsBattleResTrigger : public Trigger
{
public:
    FactionChampionsBattleResTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions battle res") {}
    bool IsActive() override;
};

class FactionChampionsPersonalDefensiveTrigger : public Trigger
{
public:
    FactionChampionsPersonalDefensiveTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "faction champions personal defensive") {}
    bool IsActive() override;
};

class ToCFactionChampionsDefenceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCFactionChampionsDefenceTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["faction champions personal defensive"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_personal_defensive;
        creators["faction champions avoid aoe"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_avoid_aoe;
        creators["faction champions mass dispel"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_mass_dispel;
        creators["faction champions dispel cc"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_dispel_cc;
        creators["faction champions kill totem"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_kill_totem;
        creators["faction champions physical switch"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_physical_switch;
        creators["faction champions purge kill target"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_purge_kill_target;
        creators["faction champions channel interrupt"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_channel_interrupt;
        creators["faction champions death grip"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_death_grip;
        creators["faction champions chain grip"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_chain_grip;
        creators["faction champions wall grip"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_wall_grip;
        creators["faction champions freedom"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_freedom;
        creators["faction champions snare"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_snare;
        creators["faction champions crowd control"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_crowd_control;
        creators["faction champions peel"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_peel;
        creators["faction champions protect"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_protect;
        creators["faction champions battle res"] =
            &ToCFactionChampionsDefenceTriggerContext::faction_champions_battle_res;
    }

private:
    static Trigger* faction_champions_avoid_aoe(PlayerbotAI* botAI) {
        return new FactionChampionsAvoidAoeTrigger(botAI);
    }

    static Trigger* faction_champions_mass_dispel(PlayerbotAI* botAI) {
        return new FactionChampionsMassDispelTrigger(botAI);
    }

    static Trigger* faction_champions_dispel_cc(PlayerbotAI* botAI) {
        return new FactionChampionsDispelCcTrigger(botAI);
    }

    static Trigger* faction_champions_kill_totem(PlayerbotAI* botAI) {
        return new FactionChampionsKillTotemTrigger(botAI);
    }

    static Trigger* faction_champions_physical_switch(PlayerbotAI* botAI) {
        return new FactionChampionsPhysicalSwitchTrigger(botAI);
    }

    static Trigger* faction_champions_purge_kill_target(PlayerbotAI* botAI) {
        return new FactionChampionsPurgeKillTargetTrigger(botAI);
    }

    static Trigger* faction_champions_channel_interrupt(PlayerbotAI* botAI) {
        return new FactionChampionsChannelInterruptTrigger(botAI);
    }

    static Trigger* faction_champions_death_grip(PlayerbotAI* botAI) {
        return new FactionChampionsDeathGripTrigger(botAI);
    }

    static Trigger* faction_champions_chain_grip(PlayerbotAI* botAI) {
        return new FactionChampionsChainGripTrigger(botAI);
    }

    static Trigger* faction_champions_wall_grip(PlayerbotAI* botAI) {
        return new FactionChampionsWallGripTrigger(botAI);
    }

    static Trigger* faction_champions_freedom(PlayerbotAI* botAI) {
        return new FactionChampionsFreedomTrigger(botAI);
    }

    static Trigger* faction_champions_snare(PlayerbotAI* botAI) {
        return new FactionChampionsSnareTrigger(botAI);
    }

    static Trigger* faction_champions_crowd_control(PlayerbotAI* botAI) {
        return new FactionChampionsCrowdControlTrigger(botAI);
    }

    static Trigger* faction_champions_peel(PlayerbotAI* botAI) {
        return new FactionChampionsPeelTrigger(botAI);
    }

    static Trigger* faction_champions_protect(PlayerbotAI* botAI) {
        return new FactionChampionsProtectTrigger(botAI);
    }

    static Trigger* faction_champions_battle_res(PlayerbotAI* botAI) {
        return new FactionChampionsBattleResTrigger(botAI);
    }

    static Trigger* faction_champions_personal_defensive(PlayerbotAI* botAI) {
        return new FactionChampionsPersonalDefensiveTrigger(botAI);
    }
};

void AddToCFactionChampionsDefenceTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
