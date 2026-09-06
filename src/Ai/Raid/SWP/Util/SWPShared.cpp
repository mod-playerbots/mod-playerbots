/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPShared.h"
#include "Playerbots.h"

namespace SwpHelpers
{

ObjectGuid FindSwpVolatileFiendGuid(Player* bot)
{
    Creature* fiend = bot->FindNearestCreature(
        Id(SwpNpcs::NPC_VOLATILE_FIEND), VOLATILE_FIEND_SEARCH_RADIUS);

    return fiend ? fiend->GetGUID() : ObjectGuid::Empty;
}

float GetCenteredArcSlotAngleOffset(uint8 slotIndex, uint8 slotCount, float arcWidth)
{
    if (slotCount <= 1)
        return 0.0f;

    float const angleStep = arcWidth / static_cast<float>(slotCount - 1);
    if (slotCount % 2 == 1)
    {
        if (slotIndex == 0)
            return 0.0f;

        uint8 const stepIndex = (slotIndex + 1) / 2;
        float angleOffset = angleStep * stepIndex;
        if (slotIndex % 2 == 0)
            angleOffset = -angleOffset;

        return angleOffset;
    }

    float const halfStep = angleStep / 2.0f;
    uint8 const pairIndex = slotIndex / 2;
    float angleOffset = halfStep + angleStep * pairIndex;
    if (slotIndex % 2 == 1)
        angleOffset = -angleOffset;

    return angleOffset;
}

uint32 GetManualCastCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    uint32 cooldownMs = spellInfo->GetRecoveryTime();
    if (spellInfo->CategoryRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->CategoryRecoveryTime;
    if (spellInfo->StartRecoveryTime > cooldownMs)
        cooldownMs = spellInfo->StartRecoveryTime;

    return cooldownMs ? cooldownMs : minGlobalCooldown;
}

uint32 GetManualCastGlobalCooldown(uint32 spellId)
{
    constexpr uint32 minGlobalCooldown = 1000; // Spell.cpp MIN_GCD

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return minGlobalCooldown;

    if (spellInfo->StartRecoveryTime)
        return spellInfo->StartRecoveryTime;

    // A charmed caster still gets MIN_GCD for a cooldownless spell.
    return spellInfo->RecoveryTime || spellInfo->CategoryRecoveryTime ? 0 : minGlobalCooldown;
}

}
