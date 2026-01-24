#include "BotRoleService.h"
#include "RaidOsMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "RaidOsActions.h"
#include "RaidOsTriggers.h"
#include "ReachTargetActions.h"
#include "ScriptedCreature.h"
#include "WarriorActions.h"

float SartharionMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sartharion");
    if (!boss) { return 1.0f; }

    Unit* target = action->GetTarget();

    if (BotRoleService::IsMainTankStatic(bot) && dynamic_cast<TankFaceAction*>(action))
    {
        // return 0.0f;
    }

    if (BotRoleService::IsDpsStatic(bot) && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    if (BotRoleService::IsMainTankStatic(bot) && target && target != boss &&
        (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action)))
    {
        return 0.0f;
    }

    if (BotRoleService::IsAssistTankStatic(bot) && target && target == boss &&
        (dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action)))
    {
        return 0.0f;
    }
    return 1.0f;
}
