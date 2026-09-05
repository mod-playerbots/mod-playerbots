/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DEBUGACTION_H
#define PLAYERBOTS_DEBUGACTION_H

#include "Action.h"
#include "ObjectGuid.h"
#include "TravelMgr.h"

class PlayerbotAI;
class Unit;

class DebugAction : public Action
{
public:
    DebugAction(PlayerbotAI* botAI) : Action(botAI, "Debug") {}

    bool Execute(Event event) override;

    void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid::Empty,
                   GuidVector otherTargets = {}, GuidVector missTargets = {}, WorldPosition source = WorldPosition(),
                   WorldPosition dest = WorldPosition(), bool forceDest = false);
    void addAura(uint32 spellId, Unit* target);
};

#endif
