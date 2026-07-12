#ifndef PLAYERBOTS_MECHANARMULTIPLIERS_H
#define PLAYERBOTS_MECHANARMULTIPLIERS_H

#include "Multiplier.h"

// Movement whitelist for the fixated or fire-dodging bot: while a flame is kiting the bot
// (or it must escape a trail or Inferno right now), only the three Sepethrea movers may
// issue a step. Every other movement action (stock chase/formation/follow/flee, rogue
// behind-target, MoveOutOfEnemyContact, the stock AvoidAoe, and the DungeonClear
// assist/regroup/follow movers) is zeroed so nothing fights the kite or dodge for control.
// Spellcasting and healing are never MovementActions, so they are left fully intact and the
// bot keeps doing damage or healing on the hold ticks between hops.
class SepethreaKiteFlameMultiplier : public Multiplier
{
public:
    SepethreaKiteFlameMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea kite flame") {}
    float GetValue(Action* action) override;
};

// While Sepethrea is engaged, zero the tank's stock target-acquisition actions ("tank
// assist" and "aggressive target") so it cannot grab the un-tankable Raging Flames as its
// "loose add". SepethreaFocusBossAction keeps the tank on the boss; the tank's ability
// rotation (not AttackActions) is untouched. Scoped to the boss fight only, so ordinary
// trash tanking on the approach still works.
class SepethreaTankFocusMultiplier : public Multiplier
{
public:
    SepethreaTankFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sepethrea tank focus") {}
    float GetValue(Action* action) override;
};

#endif
