/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZGStrategy.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "ZGHelpers.h"
#include "ZGMultipliers.h"

using namespace ZulGurubHelpers;

namespace
{
bool IsThekalTrioMember(uint32 entry)
{
    return entry == static_cast<uint32>(ZulGurubNpcs::NPC_HIGH_PRIEST_THEKAL) ||
           entry == static_cast<uint32>(ZulGurubNpcs::NPC_ZEALOT_ZATH) ||
           entry == static_cast<uint32>(ZulGurubNpcs::NPC_ZEALOT_LORKHAN);
}
}  // namespace

void RaidZulGurubStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Bloodlord Mandokir
    triggers.push_back(new TriggerNode("bloodlord mandokir threatening gaze",
        { NextAction("bloodlord mandokir stop causing threat", ACTION_EMERGENCY + 6) }));
}

void RaidZulGurubStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HakkarKeepPoisonousBloodMultiplier(botAI));
    multipliers.push_back(new BloodlordMandokirCauseNoThreatUnderGazeMultiplier(botAI));
}

void RaidZulGurubStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::DpsTarget && type != TargetValueExclusionType::DebuffTarget)
        return;

    Player* bot = botAI->GetBot();
    if (!EncounterHelpers::IsEncounterInProgress(bot, ZG_MAP_ID))
        return;

    // Thekal's first phase: a low member of the trio is left alone while another is still high.
    AiObjectContext* context = botAI->GetAiObjectContext();
    std::vector<Unit*> trio;
    for (ObjectGuid const guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (IsThekalTrioMember(unit->GetEntry()))
            trio.push_back(unit);
    }

    if (trio.size() < 2)
        return;

    bool anyHigh = false;
    for (Unit* unit : trio)
    {
        if (unit->GetHealthPct() > THEKAL_TRIO_SYNC_HEALTH_PCT)
            anyHigh = true;
    }

    if (!anyHigh)
        return;

    for (Unit* unit : trio)
    {
        if (unit->GetHealthPct() <= THEKAL_TRIO_SYNC_HEALTH_PCT)
            exclusions.insert(unit->GetGUID());
    }
}
