/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSBALL_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_FACTIONCHAMPIONSBALL_H

#include <vector>

#include "NamedObjectContext.h"
#include "Trigger.h"

class FactionChampionsIceTargetTrigger : public Trigger
{
public:
    FactionChampionsIceTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions ice target") {}
    bool IsActive() override;
};

class FactionChampionsChainsOfIceTrigger : public Trigger
{
public:
    FactionChampionsChainsOfIceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions chains of ice") {}
    bool IsActive() override;
};

class FactionChampionsIceReachTrigger : public Trigger
{
public:
    FactionChampionsIceReachTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions ice reach") {}
    bool IsActive() override;
};

class FactionChampionsCycloneStandTrigger : public Trigger
{
public:
    FactionChampionsCycloneStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions cyclone stand") {}
    bool IsActive() override;
};

class FactionChampionsStepOffTrigger : public Trigger
{
public:
    FactionChampionsStepOffTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions step off") {}
    bool IsActive() override;
};

class FactionChampionsChasedTrigger : public Trigger
{
public:
    FactionChampionsChasedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions chased") {}
    bool IsActive() override;
};

class FactionChampionsKiteTrigger : public Trigger
{
public:
    FactionChampionsKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions kite") {}
    bool IsActive() override;
};

class FactionChampionsSpeedBoostTrigger : public Trigger
{
public:
    FactionChampionsSpeedBoostTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions speed boost") {}
    bool IsActive() override;
};

class FactionChampionsOpenerTrigger : public Trigger
{
public:
    FactionChampionsOpenerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions opener") {}
    bool IsActive() override;
};

class FactionChampionsEscapeTrigger : public Trigger
{
public:
    FactionChampionsEscapeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions escape") {}
    bool IsActive() override;
};

class FactionChampionsDemonicCircleTrigger : public Trigger
{
public:
    FactionChampionsDemonicCircleTrigger(PlayerbotAI* botAI) : Trigger(botAI, "faction champions demonic circle") {}
    bool IsActive() override;
};

class ToCFactionChampionsBallTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ToCFactionChampionsBallTriggerContext() : NamedObjectContext<Trigger>()
    {
        creators["faction champions escape"] = &ToCFactionChampionsBallTriggerContext::faction_champions_escape;
        creators["faction champions demonic circle"] =
            &ToCFactionChampionsBallTriggerContext::faction_champions_demonic_circle;
        creators["faction champions ice target"] = &ToCFactionChampionsBallTriggerContext::faction_champions_ice_target;
        creators["faction champions chains of ice"] =
            &ToCFactionChampionsBallTriggerContext::faction_champions_chains_of_ice;
        creators["faction champions ice reach"] = &ToCFactionChampionsBallTriggerContext::faction_champions_ice_reach;
        creators["faction champions cyclone stand"] =
            &ToCFactionChampionsBallTriggerContext::faction_champions_cyclone_stand;
        creators["faction champions step off"] = &ToCFactionChampionsBallTriggerContext::faction_champions_step_off;
        creators["faction champions opener"] = &ToCFactionChampionsBallTriggerContext::faction_champions_opener;
        creators["faction champions chased"] = &ToCFactionChampionsBallTriggerContext::faction_champions_chased;
        creators["faction champions kite"] = &ToCFactionChampionsBallTriggerContext::faction_champions_kite;
        creators["faction champions speed boost"] =
            &ToCFactionChampionsBallTriggerContext::faction_champions_speed_boost;
    }

private:
    static Trigger* faction_champions_ice_target(PlayerbotAI* botAI)
    {
        return new FactionChampionsIceTargetTrigger(botAI);
    }

    static Trigger* faction_champions_chains_of_ice(PlayerbotAI* botAI)
    {
        return new FactionChampionsChainsOfIceTrigger(botAI);
    }

    static Trigger* faction_champions_ice_reach(PlayerbotAI* botAI)
    {
        return new FactionChampionsIceReachTrigger(botAI);
    }

    static Trigger* faction_champions_cyclone_stand(PlayerbotAI* botAI)
    {
        return new FactionChampionsCycloneStandTrigger(botAI);
    }

    static Trigger* faction_champions_step_off(PlayerbotAI* botAI) { return new FactionChampionsStepOffTrigger(botAI); }

    static Trigger* faction_champions_opener(PlayerbotAI* botAI) { return new FactionChampionsOpenerTrigger(botAI); }

    static Trigger* faction_champions_chased(PlayerbotAI* botAI) { return new FactionChampionsChasedTrigger(botAI); }

    static Trigger* faction_champions_kite(PlayerbotAI* botAI) { return new FactionChampionsKiteTrigger(botAI); }

    static Trigger* faction_champions_speed_boost(PlayerbotAI* botAI)
    {
        return new FactionChampionsSpeedBoostTrigger(botAI);
    }

    static Trigger* faction_champions_escape(PlayerbotAI* botAI) { return new FactionChampionsEscapeTrigger(botAI); }

    static Trigger* faction_champions_demonic_circle(PlayerbotAI* botAI)
    {
        return new FactionChampionsDemonicCircleTrigger(botAI);
    }
};

void AddToCFactionChampionsBallTriggerNodes(std::vector<TriggerNode*>& triggers);

#endif
