/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "OnyHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "Spell.h"
#include "ThreatManager.h"
#include <array>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace OnyxiaHelpers
{

namespace
{

struct HoldState
{
    ObjectGuid onyxia;
    uint32 startedMs = 0;
    bool expired = false;
};

std::shared_mutex holdStateMutex;
std::unordered_map<uint32, HoldState> holdStates;

bool IsHoldExpired(Player* bot, Unit* onyxia)
{
    if (onyxia->IsFlying())
        return false;

    std::shared_lock lock(holdStateMutex);
    auto const it = holdStates.find(bot->GetInstanceId());
    return it != holdStates.end() && it->second.onyxia == onyxia->GetGUID() && it->second.expired;
}

bool IsHoldTimeUp(Player* bot, Unit* onyxia)
{
    uint32 const instanceId = bot->GetInstanceId();
    if (onyxia->IsFlying())
    {
        ResetHoldState(instanceId);
        return false;
    }

    uint32 const now = getMSTime();
    {
        std::shared_lock lock(holdStateMutex);
        auto const it = holdStates.find(instanceId);
        if (it != holdStates.end() && it->second.onyxia == onyxia->GetGUID())
        {
            if (it->second.expired)
                return true;

            if (getMSTimeDiff(it->second.startedMs, now) < HOLD_LIMIT_MS)
                return false;
        }
    }

    std::unique_lock lock(holdStateMutex);
    HoldState& state = holdStates[instanceId];
    if (state.onyxia != onyxia->GetGUID())
    {
        state = { onyxia->GetGUID(), now, false };
        return false;
    }

    state.expired = state.expired || getMSTimeDiff(state.startedMs, now) >= HOLD_LIMIT_MS;
    return state.expired;
}

constexpr std::array<CurrentSpellTypes, 3> ATTACK_SPELL_TYPES = { CURRENT_AUTOREPEAT_SPELL, CURRENT_CHANNELED_SPELL,
                                                                  CURRENT_GENERIC_SPELL };

bool IsSpellAimedAt(Player* bot, CurrentSpellTypes type, Unit const& target)
{
    Spell* spell = bot->GetCurrentSpell(type);
    return spell && spell->m_targets.GetUnitTargetGUID() == target.GetGUID();
}

}

bool IsOnyxia(Unit* unit)
{
    return unit && (unit->GetEntry() == Id(OnyNpcs::NPC_ONYXIA) || unit->GetEntry() == Id(OnyNpcs::NPC_ONYXIA_40));
}

Unit* FindOnyxia(Player* bot)
{
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (IsOnyxia(unit))
            return unit->IsInWorld() ? unit : nullptr;
    }

    return nullptr;
}

bool IsAttackingOnyxia(Player* bot, Unit const& onyxia)
{
    for (CurrentSpellTypes const type : ATTACK_SPELL_TYPES)
    {
        if (IsSpellAimedAt(bot, type, onyxia))
            return true;
    }

    return bot->GetVictim() == &onyxia && !PlayerbotAI::IsHeal(bot);
}

bool StopAttackingOnyxia(Player* bot, Unit const& onyxia)
{
    bool isStopped = false;
    if (bot->GetVictim() == &onyxia && !PlayerbotAI::IsHeal(bot))
    {
        bot->AttackStop();
        isStopped = true;
    }

    for (CurrentSpellTypes const type : ATTACK_SPELL_TYPES)
    {
        if (IsSpellAimedAt(bot, type, onyxia))
        {
            bot->InterruptSpell(type);
            isStopped = true;
        }
    }

    return isStopped;
}

bool MustHoldPhaseThreeThreat(Player* bot, Unit* onyxia, float requiredLead)
{
    if (!IsOnyxia(onyxia) || onyxia->GetHealthPct() > ONYXIA_PHASE_THREE_HEALTH_PCT)
        return false;

    if (IsHoldExpired(bot, onyxia))
        return false;

    Player* mainTank = EncounterHelpers::GetGroupMainTank(bot);
    if (!mainTank || mainTank == bot || mainTank->GetMap() != onyxia->GetMap())
        return false;

    ThreatManager const& threatMgr = onyxia->GetThreatMgr();
    if (threatMgr.GetThreat(mainTank) - threatMgr.GetThreat(bot) >= requiredLead)
        return false;

    return !IsHoldTimeUp(bot, onyxia);
}

bool ResetHoldState(uint32 instanceId)
{
    {
        std::shared_lock lock(holdStateMutex);
        if (!holdStates.contains(instanceId))
            return false;
    }

    std::unique_lock lock(holdStateMutex);
    return holdStates.erase(instanceId) > 0;
}

}
