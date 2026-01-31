/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DKActions.h"

#include "CreateNextAction.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "DKActions.h"

std::vector<NextAction> CastDeathchillAction::getPrerequisites()
{
    return {
        CreateNextAction<CastFrostPresenceAction>(1.0f)
    };
}

std::vector<NextAction> CastUnholyMeleeSpellAction::getPrerequisites()
{
    return {
        CreateNextAction<CastUnholyPresenceAction>(1.0f)
    };
}

std::vector<NextAction> CastFrostMeleeSpellAction::getPrerequisites()
{
    return {
        CreateNextAction<CastFrostPresenceAction>(1.0f)
    };
}

std::vector<NextAction> CastBloodMeleeSpellAction::getPrerequisites()
{
    return {
        CreateNextAction<CastBloodPresenceAction>(1.0f)
    };
}

bool CastRaiseDeadAction::Execute(Event event)
{
    const bool result = CastBuffSpellAction::Execute(event);

    if (!result)
        return false;

    const uint32_t spellId = AI_VALUE2(uint32_t, "spell id", spell);

    bot->AddSpellCooldown(spellId, 0, 3 * 60 * 1000);

    return true;
}
