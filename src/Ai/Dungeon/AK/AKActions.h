#ifndef PLAYERBOTS_AKACTIONS_H
#define PLAYERBOTS_AKACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "AKTriggers.h"

class AttackNadoxGuardianAction : public AttackAction
{
public:
    AttackNadoxGuardianAction(PlayerbotAI* ai) : AttackAction(ai, "attack nadox guardian") {}
    bool Execute(Event event) override;
};

class AttackJedogaVolunteerAction : public AttackAction
{
public:
    AttackJedogaVolunteerAction(PlayerbotAI* ai) : AttackAction(ai, "attack jedoga volunteer") {}
    bool Execute(Event event) override;
};

class AvoidShadowCrashAction : public MovementAction
{
public:
    AvoidShadowCrashAction(PlayerbotAI* ai) : MovementAction(ai, "avoid shadow crash") {}
    bool Execute(Event event) override;
};

#endif
