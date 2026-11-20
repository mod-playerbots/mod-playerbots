/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RaidBurstWindow.h"

#include "Action.h"
#include "BurstCooldowns.h"
#include "Playerbots.h"
#include "Timer.h"

float RaidBurstWindowMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    auto const classify = [this, action]
    {
        std::string const name = action->getName();
        Facts result;
        result.burst = IsBurstCooldownAction(name) && !IsManaReturnCooldown(bot, name);
        result.lust = name == "bloodlust" || name == "heroism";
        return result;
    };

    Facts const& facts = _facts.Get(action, classify);
    if (!facts.burst)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    uint32 const now = getMSTime();
    if (now != _cachedAtMs || !_cachedAtMs)
    {
        _cachedAtMs = now;
        _cachedValue = _window(botAI);
    }

    bool const allowed = facts.lust ? _cachedValue.allowLust : _cachedValue.allowAll;

    return allowed ? 1.0f : 0.0f;
}
