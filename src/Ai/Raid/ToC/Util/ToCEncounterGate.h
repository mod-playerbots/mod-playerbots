/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCENCOUNTERGATE_H
#define PLAYERBOTS_RAID_TOCENCOUNTERGATE_H

#include <string>
#include <vector>

#include "Define.h"
#include "RaidEncounter.h"
#include "Trigger.h"

class PlayerbotAI;

// Mirrors trial_of_the_crusader.h, out of reach in the core's scripts: INSTANCE_PROGRESS a GetData key, the
// rest GetGuidData keys from TrialOfTheCrusaderObjectData, implicit values to recheck when the core adds one
enum ToCInstanceData : uint32
{
    TOC_DATA_INSTANCE_PROGRESS = 1,
    TOC_DATA_GORMOK = 5,
    TOC_DATA_DREADSCALE = 6,
    TOC_DATA_ACIDMAW = 7,
    TOC_DATA_ICEHOWL = 8,
    TOC_DATA_JARAXXUS = 10,
    TOC_DATA_FJOLA = 11,  // DATA_LIGHTBANE
    TOC_DATA_EYDIS = 12,  // DATA_DARKBANE
    TOC_DATA_ANUBARAK = 14,
};

// GetData(TOC_DATA_INSTANCE_PROGRESS). Never moves back on a wipe.
enum ToCInstanceProgress : uint32
{
    TOC_PROGRESS_INITIAL = 0,
    TOC_PROGRESS_INTRO_DONE = 1,
    TOC_PROGRESS_BEASTS_DEAD = 2,
    TOC_PROGRESS_JARAXXUS_INTRO_DONE = 3,
    TOC_PROGRESS_JARAXXUS_DEAD = 4,
    TOC_PROGRESS_FACTION_CHAMPIONS_DEAD = 6,
    TOC_PROGRESS_VALKYR_DEAD = 8,
    TOC_PROGRESS_ANUB_ARAK = 9,
    TOC_PROGRESS_DONE = 10,
};

enum class ToCEncounter : uint8
{
    None,
    NorthrendBeasts,
    Jaraxxus,
    FactionChampions,
    TwinValkyr,
    Anubarak
};

// The encounter a GetData(TOC_DATA_INSTANCE_PROGRESS) stage belongs to, None for 8, 10 and the rest.
ToCEncounter ToCEncounterOfStage(uint32 stage);

// The whole stage, pulled or not, so pre-pull prep still runs; Beasts and Jaraxxus also the stage after
// their kill, for leftover adds, until the next is live. True off a ToC instance: ungated beats dead.
bool ToCEncounterGateOpen(PlayerbotAI* botAI, ToCEncounter encounter);

// Stage matches, IsEncounterInProgress, and one of its units alive and in combat: the script flags most
// encounters in progress before the pull. False off a ToC instance.
bool ToCEncounterIsLive(PlayerbotAI* botAI, ToCEncounter encounter);

// None between pulls and off a ToC instance.
ToCEncounter ToCLiveEncounter(PlayerbotAI* botAI);

// EncounterGate over ToCEncounterGateOpen, for a ToC EncounterDefinition. encounterId is
// static_cast<uint32>(ToCEncounter::X).
bool ToCGateOpen(PlayerbotAI* botAI, uint32 encounterId);

// Always open: for a raid-wide stem whose rule predicates do their own gating. Its definition takes
// static_cast<uint32>(ToCEncounter::None).
bool ToCRaidWideGateOpen(PlayerbotAI* botAI, uint32 encounterId);

// False for a name with no encounter prefix, which then stays ungated.
bool ToCEncounterOfTrigger(std::string const& triggerName, ToCEncounter& encounter);

// Wraps where the context builds each trigger, so no stem trigger class needs editing.
class ToCGatedTrigger : public Trigger
{
public:
    ToCGatedTrigger(PlayerbotAI* botAI, Trigger* inner, ToCEncounter encounter);
    ~ToCGatedTrigger() override;

    Event Check() override;
    bool IsActive() override;
    bool IsBuffTrigger() override;
    bool IsDebuffTrigger() override;
    std::vector<NextAction> getHandlers() override;
    void Reset() override;
    Unit* GetTarget() override;
    Value<Unit*>* GetTargetValue() override;
    std::string const GetTargetName() override;
    void ExternalEvent(std::string const param, Player* owner = nullptr) override;
    void ExternalEvent(WorldPacket& packet, Player* owner = nullptr) override;

private:
    Trigger* _inner;
    ToCEncounter _encounter;
};

#endif
