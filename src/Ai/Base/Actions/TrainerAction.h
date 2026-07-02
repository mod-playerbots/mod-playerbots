/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TRAINERACTION_H
#define PLAYERBOTS_TRAINERACTION_H

#include "Action.h"
#include "ChatHelper.h"

class Creature;
class PlayerbotAI;

struct TrainerSpell;

class TrainerAction : public Action
{
public:
    TrainerAction(PlayerbotAI* botAI) : Action(botAI, "trainer") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
    Unit* GetTarget() override;

private:
    Creature* GetCreatureTarget();
    void Iterate(Creature* creature, bool learnSpells, uint32 spellId);
    void Learn(SpellInfo const* spellInfo, uint32 cost, std::ostringstream& out);
    void TellHeader(Creature* creature);
    void TellFooter(uint32 totalCost);
};

class MaintenanceAction : public Action
{
public:
    MaintenanceAction(PlayerbotAI* botAI) : Action(botAI, "maintenance") {}
    bool Execute(Event event) override;
};

class RemoveGlyphAction : public Action
{
public:
    RemoveGlyphAction(PlayerbotAI* botAI) : Action(botAI, "remove glyph") {}
    bool Execute(Event event) override;
};

class AutoGearAction : public Action
{
public:
    AutoGearAction(PlayerbotAI* botAI) : Action(botAI, "autogear") {}
    bool Execute(Event event) override;
};

class BisGearAction : public Action
{
public:
    BisGearAction(PlayerbotAI* botAI) : Action(botAI, "autogear bis") {}
    bool Execute(Event event) override;

private:
    bool RunAutogearFallback(uint16 effectiveIlvl);
};

#endif
