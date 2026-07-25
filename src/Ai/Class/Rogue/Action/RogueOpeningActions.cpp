/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RogueOpeningActions.h"

#include "Playerbots.h"

Value<Unit*>* CastSapAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }
