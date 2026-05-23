#include "RaidBwlTriggers.h"

#include "Playerbots.h"
#include "RaidBwlHelpers.h"

using namespace BlackwingLairHelpers;

// General

bool BwlSuppressionDeviceTrigger::IsActive()
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto i = gos.begin(); i != gos.end(); ++i)
    {
        const GameObject* go = botAI->GetGameObject(*i);
        if (IsActiveSuppressionDeviceInRange(go, bot))
        {
            return true;
        }
    }
    return false;
}

// Chromaggus

bool BwlAfflictionBronzeTrigger::IsActive()
{
    return bot->HasAura(SPELL_BROOD_AFFLICTION_BRONZE);
}

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
