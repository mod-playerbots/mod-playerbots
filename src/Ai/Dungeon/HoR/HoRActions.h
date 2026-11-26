/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORACTIONS_H
#define PLAYERBOTS_HORACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"

class PlayerbotAI;

// NPC entries specific to the Halls of Reflection escape event.
constexpr uint32 NPC_HOR_LICH_KING    = 36954;  // The Lich King chasing the group
constexpr uint32 NPC_HOR_RAGING_GHOUL = 36940;  // Melee trash
constexpr uint32 NPC_HOR_WITCH_DOCTOR = 36941;  // Ranged caster -> interrupt target
constexpr uint32 NPC_HOR_ABOMINATION  = 37069;  // Cleaving elite -> tank faces away from group

// True while the escape event is running: the Lich King is spawned, alive and
// in combat. During the roleplay intro he stands on the platform but is not in
// combat, so the bots keep their normal behaviour until the chase begins.
bool HorEscapeActive(PlayerbotAI* botAI);

// Returns the highest priority living hostile of the escape event within 60y.
// DPS order: Witch Doctor (ranged caster) > Abomination > Ghoul.
// Tank order: Abomination (hold away from group) > Witch Doctor > Ghoul.
Unit* HorFindPriorityMob(PlayerbotAI* botAI, bool forTank);

// DPS/healer/ranged tank: attack the priority mob, mark the Witch Doctor with
// the skull icon so every bot focuses the interrupt target.
class HorEscapeAttackAction : public AttackAction
{
public:
    HorEscapeAttackAction(PlayerbotAI* botAI) : AttackAction(botAI, "hor escape attack") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// Tank: pull the Lumbering Abomination and position it with its back to the
// group (tank stands on the far side of the abomination so its cleave cone
// points away from the raid). Without an abomination up, help on priority mobs.
class HorEscapeTankAction : public AttackAction
{
public:
    HorEscapeTankAction(PlayerbotAI* botAI) : AttackAction(botAI, "hor escape tank") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

// No priority mob left -> follow the group leader along the escape tunnel.
class HorEscapeFollowAction : public MovementAction
{
public:
    HorEscapeFollowAction(PlayerbotAI* botAI) : MovementAction(botAI, "hor escape follow") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
