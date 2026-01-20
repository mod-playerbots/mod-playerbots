/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DKActions.h"

#include "Duration.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

std::vector<NextAction> CastDeathchillAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("frost presence") },
                             CastSpellAction::getPrerequisites());
}

std::vector<NextAction> CastUnholyMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("unholy presence") },
                             CastMeleeSpellAction::getPrerequisites());
}

std::vector<NextAction> CastFrostMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("frost presence") },
                             CastMeleeSpellAction::getPrerequisites());
}

std::vector<NextAction> CastBloodMeleeSpellAction::getPrerequisites()
{
    return NextAction::merge({ NextAction("blood presence") },
                             CastMeleeSpellAction::getPrerequisites());
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
