/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRAINERACTION_H
#define _PLAYERBOT_TRAINERACTION_H

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
    [[nodiscard]] Creature* getCreatureTarget() noexcept;
    void iterate(const Creature* const creature, const bool learnSpells, const uint32_t spellId);
    [[nodiscard]] const std::string learn(const SpellInfo& spellInfo, const uint32_t cost);
    void tellHeader(const Creature* const creature) const;
    void tellFooter(const uint32_t totalCost);
};

class MaintenanceAction : public Action
{
public:
    MaintenanceAction(PlayerbotAI* botAI) : Action(botAI, "maintenance") {}
    bool Execute(Event event) override;

private:
    void performAltMaintenance();
    void performRandomBotMaintenance();
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

#endif
