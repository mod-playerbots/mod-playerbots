/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCTRIGGERS_H
#define PLAYERBOTS_RAIDTOCTRIGGERS_H

#include "GenericTriggers.h"
#include "Trigger.h"

class PlayerbotAI;

// Trial of the Crusader, Faction Champions: true when a low-health non-tank bot
// is being meleed by a champion (the champions are player-class NPCs that pick
// their own targets instead of following tank threat).
class TocChampionsChasedTrigger : public Trigger
{
public:
    TocChampionsChasedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc champions chased", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Twin Val'kyr: true while either sister is alive and in
// combat. The portal dance itself is driven by the touch debuff inside the
// action, this trigger just gates it to the encounter.
class TocTwinValkyrFightTrigger : public Trigger
{
public:
    TocTwinValkyrFightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc twin valkyr fight", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Icehowl: someone got picked for Ferocious Butt (the
// charge). Everybody has to sidestep away from the charge lane.
class TocIcehowlChargeTrigger : public Trigger
{
public:
    TocIcehowlChargeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc icehowl charge", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Gormok: Rising Anger (66636) stacks on the current
// tank; swap at 3 stacks before it becomes unhealable.
class TocGormokTankSwapTrigger : public TankSwapTriggerBase
{
public:
    TocGormokTankSwapTrigger(PlayerbotAI* botAI)
        : TankSwapTriggerBase(botAI, "toc gormok tank swap", 66636, 3, "gormok the impaler")
    {
    }
};

// Trial of the Crusader, jormungar pair: the Dreadscale tank carries Burning
// Bite and must walk it over paralysed players, then pull the boss away.
class TocBurningBileTrigger : public Trigger
{
public:
    TocBurningBileTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc burning bile", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Anub'arak: true while the boss is submerged (phase 2),
// when the group has to split - tank up, raid down - and the spikes are chased
// around the frost spheres.
class TocAnubSubmergeTrigger : public Trigger
{
public:
    TocAnubSubmergeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc anub submerge", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Lord Jaraxxus: the Nether Portal and the Infernal
// Volcano do not attack anyone, so nobody in the raid picks them up
// automatically. They must be killed by the dps.
class TocJaraxxusAddsTrigger : public Trigger
{
public:
    TocJaraxxusAddsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc jaraxxus adds", 1) {}
    bool IsActive() override;
};

// Trial of the Crusader, Lord Jaraxxus: Nether Power stacked on the boss means
// the cast was neither kicked nor stolen/dispelled. The tank then has to pop a
// defensive before the boosted spell damage lands. Mages steal and shamans
// purge on their own (the base triggers already cover magic buffs), so this is
// strictly the "nobody handled it" fallback for the tank.
class TocJaraxxusNetherPowerTrigger : public Trigger
{
public:
    TocJaraxxusNetherPowerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "toc jaraxxus nether power", 1) {}
    bool IsActive() override;
};

#endif
