/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AiObjectContext.h"
#include "GDValueContext.h"
#include "GruulValueContext.h"
#include "MechValueContext.h"
#include "MgTValueContext.h"
#include "UBValueContext.h"
#include "ValueContext.h"

void AiObjectContext::BuildSharedValueContexts(
    SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new ValueContext());
    valueContexts.Add(new RaidGruulsLairValueContext());
    valueContexts.Add(new TbcDungeonMechValueContext());
    valueContexts.Add(new TbcDungeonMgTValueContext());
    valueContexts.Add(new TbcDungeonUnderbogValueContext());
    valueContexts.Add(new WotlkDungeonGDValueContext());
}
