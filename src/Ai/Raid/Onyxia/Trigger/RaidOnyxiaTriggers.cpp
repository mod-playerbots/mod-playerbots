#include "RaidOnyxiaTriggers.h"
#include "BotRoleService.h"

#include "GenericTriggers.h"
#include "ObjectAccessor.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "NearestNpcsValue.h"

// Onyxia Deep Breath spell IDs
enum OnyxiaDeepBreathSpells
{
    SPELL_DEEP_BREATH_N_TO_S  = 17086,  // North to South
    SPELL_DEEP_BREATH_S_TO_N  = 18351,  // South to North
    SPELL_DEEP_BREATH_E_TO_W  = 18576,  // East to West
    SPELL_DEEP_BREATH_W_TO_E  = 18609,  // West to East
    SPELL_DEEP_BREATH_SE_TO_NW = 18564, // Southeast to Northwest
    SPELL_DEEP_BREATH_NW_TO_SE = 18584, // Northwest to Southeast
    SPELL_DEEP_BREATH_SW_TO_NE = 18596, // Southwest to Northeast
    SPELL_DEEP_BREATH_NE_TO_SW = 18617, // Northeast to Southwest
    SPELL_ONYXIA_FIREBALL     = 18392   // Onyxia Fireball
};

OnyxiaDeepBreathTrigger::OnyxiaDeepBreathTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony deep breath warning") {}

bool OnyxiaDeepBreathTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    // Check if Onyxia is casting
    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    if (!currentSpell)
        return false;

    uint32 spellId = currentSpell->m_spellInfo->Id;

    if (spellId == SPELL_DEEP_BREATH_N_TO_S ||
        spellId == SPELL_DEEP_BREATH_S_TO_N ||
        spellId == SPELL_DEEP_BREATH_E_TO_W ||
        spellId == SPELL_DEEP_BREATH_W_TO_E ||
        spellId == SPELL_DEEP_BREATH_SE_TO_NW ||
        spellId == SPELL_DEEP_BREATH_NW_TO_SE ||
        spellId == SPELL_DEEP_BREATH_SW_TO_NE ||
        spellId == SPELL_DEEP_BREATH_NE_TO_SW
    )
    {
        return true;
    }

    return false;
}

OnyxiaNearTailTrigger::OnyxiaNearTailTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony near tail") {}

bool OnyxiaNearTailTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss || BotRoleService::IsTankStatic(bot))
        return false;

    // Skip if Onyxia is in air or transitioning
    if (!boss->IsInCombat() || boss->IsFlying() || !boss->GetVictim())
        return false;

    return true;
}
RaidOnyxiaFireballSplashTrigger::RaidOnyxiaFireballSplashTrigger(PlayerbotAI* botAI)
    : Trigger(botAI, "ony fireball splash incoming")
{
}

bool RaidOnyxiaFireballSplashTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss || !boss->HasUnitState(UNIT_STATE_CASTING))
        return false;

    // Check if Onyxia is casting Fireball
    Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!currentSpell || currentSpell->m_spellInfo->Id != SPELL_ONYXIA_FIREBALL)
        return false;

    GuidVector nearbyUnits = AI_VALUE(GuidVector, "nearest friendly players");

    for (ObjectGuid guid : nearbyUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || unit == bot || !unit->IsAlive())
            continue;

        if (bot->GetDistance(unit) < 8.0f)
            return true;
    }

    return false;
}

RaidOnyxiaWhelpsSpawnTrigger::RaidOnyxiaWhelpsSpawnTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony whelps spawn") {}

bool RaidOnyxiaWhelpsSpawnTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "onyxia");
    if (!boss)
        return false;

    return !BotRoleService::IsHealStatic(bot) && boss->IsFlying();  // DPS + Tanks only
}

OnyxiaAvoidEggsTrigger::OnyxiaAvoidEggsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ony avoid eggs") {}

bool OnyxiaAvoidEggsTrigger::IsActive()
{
    Position botPos = Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

    if (botPos.GetExactDist2d(-35.0f, -165.0f) <= 5.0f)
        return true;

    if (botPos.GetExactDist2d(-35.0f, -260.0f) <= 5.0f)
        return true;

    return false;
}
