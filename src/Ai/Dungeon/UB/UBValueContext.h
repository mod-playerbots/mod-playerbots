/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBVALUECONTEXT_H
#define PLAYERBOTS_UBVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "UBShared.h"
#include "Value.h"

class UnderbogMushroomsValue : public CalculatedValue<GuidVector>
{
public:
    UnderbogMushroomsValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "ub mushrooms", 200) {}

protected:
    GuidVector Calculate() override { return UnderbogHungarfen::FindMushroomGuids(bot); }
};

class TbcDungeonUnderbogValueContext : public NamedObjectContext<UntypedValue>
{
public:
    TbcDungeonUnderbogValueContext() { creators["ub mushrooms"] = &TbcDungeonUnderbogValueContext::ub_mushrooms; }

private:
    static UntypedValue* ub_mushrooms(PlayerbotAI* botAI) { return new UnderbogMushroomsValue(botAI); }
};

#endif
