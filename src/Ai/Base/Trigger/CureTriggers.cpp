/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CureTriggers.h"

#include "Playerbots.h"
#include "WorldBuffAction.h"

bool NeedCureTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && target->IsInWorld() && botAI->HasAuraToDispel(target, dispelType);
}

Value<Unit*>* PartyMemberNeedCureTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party member to dispel", dispelType);
}

bool PartyMemberNeedCureTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && target->IsInWorld();
}

bool NeedWorldBuffTrigger::IsActive() { return !WorldBuffAction::NeedWorldBuffs(bot).empty(); }
