#include "BWLTriggers.h"

#include "Playerbots.h"
#include "BWLHelpers.h"

using namespace BlackwingLairHelpers;

// General

bool BwlSuppressionDeviceTrigger::IsActive()
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto i = gos.begin(); i != gos.end(); ++i)
    {
        const GameObject* go = botAI->GetGameObject(*i);
        if (IsActiveSuppressionDeviceInRange(go, bot))
            return true;
    }
    return false;
}

// Razorgore the Untamed

bool BwlRazorgoreNotMindControlledTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "razorgore the untamed"))
        return !boss->HasAura(SPELL_MINDCONTROL);
    return false;
}

// Vaelastrasz the Corrupt

bool BwlVaelastraszPositioningTrigger::IsActive()
{
    // Prevent non-tanks from rotating the boss while the tanks gain thread.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "vaelastrasz the corrupt"))
        return boss->GetVictim() != bot;
    return false;
}

bool BwlVaelastraszBurningAdrenaline::IsActive()
{
    // No check for Vaelastrasz, because bots may still have burning adrenaline even after Vaelastrasz died.
    return bot->HasAura(SPELL_BURNING_ADRENALINE);
}

// Chromaggus

bool BwlAfflictionBronzeTrigger::IsActive()
{
    return bot->HasAura(SPELL_BROOD_AFFLICTION_BRONZE);
}

// Nefarian

bool BwlWildMagicTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE && bot->HasAura(SPELL_WILD_MAGIC);
}

bool BwlNefarianFearWardTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* nefarian = AI_VALUE2(Unit*, "find target", "nefarian");
    if (!nefarian || !nefarian->IsInCombat())
        return false;

    Unit* victim = nefarian->GetVictim();
    if (!victim)
        return false;

    return !botAI->HasAura("fear ward", victim);
}

// Trash

bool BwlDeathTalonWyrmguardTankTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "death talon wyrmguard");
}

bool BwlDeathTalonWyrmguardRangedTrigger::IsActive()
{
    return PlayerbotAI::IsRanged(bot) && AI_VALUE2(Unit*, "find target", "death talon wyrmguard");
}
