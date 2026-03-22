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

class FleeFocusFireAction : public MovementAction
{
public:
    FleeFocusFireAction(PlayerbotAI* botAI) : MovementAction(botAI, "flee focus fire") {}
    bool Execute(Event event) override;
};

#endif
