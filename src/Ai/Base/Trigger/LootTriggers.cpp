/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LootTriggers.h"
#include <algorithm>

#include "LootObjectStack.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool LootAvailableTrigger::IsActive()
{
    if (!AI_VALUE(bool, "has available loot"))
        return false;

    LootObject lootTarget = AI_VALUE(LootObject, "loot target");
    if (lootTarget.IsEmpty())
        return true;

    // A different nearest entry may be worth switching to; LootAction::isUseful applies the margin.
    if (AI_VALUE(LootObject, "nearest loot").guid != lootTarget.guid)
        return true;

    bool const inRange = botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT)
                             ? ServerFacade::instance().IsDistanceLessOrEqualThan(
                                   AI_VALUE2(float, "distance", "loot target"), CONTACT_DISTANCE)
                             : AI_VALUE(bool, "can loot");
    if (inRange || !AI_VALUE(LootObjectStack*, "available loot")->CanAttemptLoot(lootTarget.guid) ||
        !lootTarget.IsLootPossible(bot))
        return true;

    // No hostile targets to deal with first. Last because "all targets" is an uncached grid search.
    return AI_VALUE(GuidVector, "all targets").empty();
}

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (loot.IsEmpty() || AI_VALUE(bool, "can loot") ||
        !AI_VALUE(LootObjectStack*, "available loot")->CanAttemptLoot(loot.guid))
        return false;

    bool const possible = loot.IsLootPossible(bot);
    if (!possible)
        LOG_DEBUG("playerbots", "swim-trace {}: far-trigger: target {} not loot-possible", bot->GetName(),
                  loot.guid.ToString());
    return possible;
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }

bool UnderwaterSafetyTrigger::IsActive()
{
    // Bot-only concern; players drive themselves.
    if (!bot->IsAlive() || bot->HasWaterBreathingAura())
        return false;

    uint32 const now = getMSTime();
    uint32 elapsed = 0;
    if (botAI->diveLastTickMs)
        elapsed = std::min<uint32>(now - botAI->diveLastTickMs, 1000);
    botAI->diveLastTickMs = now;

    int32 const maxAir = int32(sWorld->getIntConfig(CONFIG_WATER_BREATH_TIMER)) * IN_MILLISECONDS
                         * bot->GetTotalAuraMultiplier(SPELL_AURA_MOD_WATER_BREATHING);

    if (bot->IsUnderWater())
    {
        if (!botAI->diveSubmergedSince)
        {
            botAI->diveSubmergedSince = now;
            botAI->diveAirMs = maxAir;
        }
        botAI->diveAirMs -= int32(elapsed);

        constexpr int32 diveAirSafetyMs = 15 * IN_MILLISECONDS;
        bool const idle = AI_VALUE(LootObject, "loot target").IsEmpty();
        bool const active = idle || botAI->diveAirMs <= diveAirSafetyMs;
        if (active)
            LOG_DEBUG("playerbots", "swim-trace {}: dive safety (air {:.0f}s, target {})", bot->GetName(),
                      float(botAI->diveAirMs) / IN_MILLISECONDS, idle ? "none" : "pending");
        return active;
    }

    // At the surface / on land: air regenerates at the mirror timer's 10x rate.
    botAI->diveSubmergedSince = 0;
    botAI->diveSurfacing = false;
    if (botAI->diveAirMs < maxAir)
        botAI->diveAirMs = std::min(maxAir, botAI->diveAirMs + int32(10 * elapsed));
    return false;
}
