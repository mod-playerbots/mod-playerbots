#ifndef _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONS_H
#define _PLAYERBOT_TBCDUNGEONAUCHENAICRYPTSACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "AuchenaiCryptsTriggers.h"

// Shirrak the Dead Watcher

class ShirrakTankPositionBossAction : public AttackAction
{
public:
    ShirrakTankPositionBossAction(PlayerbotAI* botAI) : AttackAction(botAI, "shirrak tank position boss") {}
    bool Execute(Event event) override;
};

class ShirrakFleeFocusFireAction : public MovementAction
{
public:
    ShirrakFleeFocusFireAction(PlayerbotAI* botAI) : MovementAction(botAI, "shirrak flee focus fire") {}
    bool Execute(Event event) override;
};

class ShirrakRangedKeepDistanceAction : public MovementAction
{
public:
    ShirrakRangedKeepDistanceAction(PlayerbotAI* botAI) : MovementAction(botAI, "shirrak ranged keep distance") {}
    bool Execute(Event event) override;
};

#endif
