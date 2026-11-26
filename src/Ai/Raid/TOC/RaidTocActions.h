/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCACTIONS_H
#define PLAYERBOTS_RAIDTOCACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class PlayerbotAI;

//
// Twin Val'kyr (encounter 4)
//
// Touch of Light (67297) is cast on players carrying the Dark Essence, and
// Touch of Darkness (67282) on players carrying the Light Essence. The debuff
// is removed by touching the portal of the opposite colour, after which the
// player has to go back to their original portal and return to their spot.
// Tanks never get a Touch (the bosses pick only non-victims), and they must
// never move because the twin follows its victim - hence the tank guard.
class TocTwinValkyrTouchAction : public MovementAction
{
public:
    TocTwinValkyrTouchAction(PlayerbotAI* botAI) : MovementAction(botAI, "toc twin valkyr touch") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    enum Phase
    {
        PHASE_IDLE,
        PHASE_TO_CLEANSING_PORTAL,
        PHASE_TO_ORIGINAL_PORTAL,
        PHASE_RETURN_TO_SPOT,
    };

    Phase phase = PHASE_IDLE;
    uint32 cleansingPortalEntry = 0;  // portal that removes the current Touch
    uint32 originalPortalEntry = 0;   // portal carrying the original essence
    Position originalSpot;

    bool HasLightTouch() const;
    bool HasDarkTouch() const;
    bool HasLightEssence() const;
    bool HasDarkEssence() const;
    Unit* FindPortal(uint32 entry) const;
};

//
// Twin Val'kyr: while a healer is busy running the portal dance, the hybrids
// that can off-heal (retribution paladin, cat druid) switch to their offheal
// strategy so the group is not left without heals. The toggle is automatic:
// it turns on when any healing-class group member carries a Touch and off
// again once the touches are gone.
//
class TocOffhealAction : public Action
{
public:
    TocOffhealAction(PlayerbotAI* botAI) : Action(botAI, "toc offheal") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    bool HealerHasTouch() const;
    bool GroupHasBetterOffhealer() const;
};

//
// Icehowl: Ferocious Butt picks a random target and the boss charges its lane.
// The picked player sidesteps, the rest of the raid clears the lane.
//
class TocIcehowlDodgeAction : public MovementAction
{
public:
    TocIcehowlDodgeAction(PlayerbotAI* botAI) : MovementAction(botAI, "toc icehowl dodge") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

//
// Anub'arak submerge: tank walks north (up), the raid walks south (down) so the
// boss breaks its own spikes on the southern frost spheres, holy paladins
// follow the tank. On surfacing everybody returns to the middle.
//
class TocAnubSubmergeAction : public MovementAction
{
public:
    TocAnubSubmergeAction(PlayerbotAI* botAI) : MovementAction(botAI, "toc anub submerge move") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

//
// Lord Jaraxxus: the Nether Portal and the Infernal Volcano do not attack, so
// nobody targets them on their own. Ranged/caster dps and the tanks stay on the
// boss; melee interrupt classes also stay on the boss so Fel Fireball keeps
// getting kicked - only the rest of the dps swaps to the portal/volcano.
//
class TocJaraxxusAddsAction : public AttackAction
{
public:
    TocJaraxxusAddsAction(PlayerbotAI* botAI) : AttackAction(botAI, "toc jaraxxus adds") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    static bool HasInterruptMelee(Player* player);
};

//
// Gormok's two jormungar (Acidmaw / Dreadscale): the tank holding Dreadscale
// carries Burning Bite. Burning damage removes Paralytic Toxin from a poisoned
// player, so that tank has to walk over them, then drag the boss away from the
// raid so its Burning Spray does not hit the group.
//
class TocBurningBileAction : public MovementAction
{
public:
    TocBurningBileAction(PlayerbotAI* botAI) : MovementAction(botAI, "toc burning bile") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
