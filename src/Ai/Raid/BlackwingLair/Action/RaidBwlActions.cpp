#include "RaidBwlActions.h"

#include "Playerbots.h"
#include "RaidBwlHelpers.h"

using namespace BlackwingLairHelpers;

// General

bool BwlOnyxiaScaleCloakAuraCheckAction::Execute(Event /*event*/)
{
    bot->AddAura(SPELL_ONYXIA_SCALE_CLOAK, bot);
    return true;
}

bool BwlOnyxiaScaleCloakAuraCheckAction::isUseful()
{
    return !bot->HasAura(SPELL_ONYXIA_SCALE_CLOAK);
}

bool BwlTurnOffSuppressionDeviceAction::Execute(Event /*event*/)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (IsActiveSuppressionDeviceInRange(go, bot))
        {
            go->SetGoState(GO_STATE_ACTIVE);
        }
    }
    return true;
}

// Chromaggus

bool BwlUseHourglassSandAction::Execute(Event /*event*/)
{
    return botAI->CastSpell(SPELL_HOURGLASS_SAND, bot);
}

bool BwlNefarianFearWardAction::Execute(Event /*event*/)
{
    Unit* nefarian = AI_VALUE2(Unit*, "find target", "nefarian");
    if (!nefarian)
        return false;

    Unit* victim = nefarian->GetVictim();
    if (!victim)
        return false;

    return botAI->CastSpell("fear ward", victim);
}
