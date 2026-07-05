#ifndef PLAYERBOTS_GDACTIONS_H
#define PLAYERBOTS_GDACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "GDTriggers.h"

class AvoidPoisonNovaAction : public MovementAction
{
public:
    AvoidPoisonNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid poison nova") {}
    bool Execute(Event event) override;
};

class AttackSnakeWrapAction : public AttackAction
{
public:
    AttackSnakeWrapAction(PlayerbotAI* ai) : AttackAction(ai, "attack snake wrap") {}
    bool Execute(Event event) override;
};

class AvoidWhirlingSlashAction : public MovementAction
{
public:
    AvoidWhirlingSlashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid whirling slash") {}
    bool Execute(Event event) override;
};

#endif
