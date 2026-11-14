/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AKMultipliers.h"
#include "AKActions.h"
#include "AKTriggers.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "ThreatHelpers.h"

float ElderNadoxMultiplier::GetValue(Action* action)
{
    if (!FindThreatTarget(bot, "elder nadox")) { return 1.0f; }

    Unit* guardian = FindThreatTarget(bot, "ahn'kahar guardian");
    if (guardian)
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float JedogaShadowseekerMultiplier::GetValue(Action* action)
{
    if (!FindThreatTarget(bot, "jedoga shadowseeker")) { return 1.0f; }

    Unit* volunteer = nullptr;
    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_TWILIGHT_VOLUNTEER)
        {
            volunteer = unit;
            break;
        }
    }

    if (volunteer)
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

float ForgottenOneMultiplier::GetValue(Action* action)
{
    if (!FindThreatTarget(bot, "forgotten one")) { return 1.0f; }

    if (bot->isMoving())
    {
        if (dynamic_cast<MovementAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
