/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CombatStrategy.h"

#include "Strategy.h"
#include "CreateNextAction.h"
#include "ReachTargetActions.h"
#include "ChooseTargetActions.h"
#include "CheckMountStateAction.h"
#include "MovementActions.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "enemy out of spell",
            {
                CreateNextAction<ReachSpellAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "invalid target",
            {
                CreateNextAction<DropTargetAction>(99.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mounted",
            {
                CreateNextAction<CheckMountStateAction>(54.0f)
            }
        )
    );
    // "reset" is seemingly not linked to any creator
    // triggers.push_back(
    //     new TriggerNode(
    //         "combat stuck",
    //         {
    //             CreateNextAction("reset", 1.0f)
    //         }
    //     )
    // );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                CreateNextAction<SetFacingTargetAction>(ACTION_MOVE + 7.0f)
            }
        )
    );
    // The pet-attack trigger is commented out because it was forcing the bot's pet to attack, overriding stay and follow commands.
    // Pets will automatically attack the bot's enemy if they are in "defensive" or "aggressive"
    // stance, or if the master issues an attack command.
}

AvoidAoeStrategy::AvoidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> AvoidAoeStrategy::getDefaultActions()
{
    return {
        CreateNextAction<AvoidAoeAction>(ACTION_EMERGENCY)
    };
}

void AvoidAoeStrategy::InitTriggers(std::vector<TriggerNode*>&)
{
}

void AvoidAoeStrategy::InitMultipliers(std::vector<Multiplier*>&)
{
}

TankFaceStrategy::TankFaceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TankFaceStrategy::getDefaultActions()
{
    return {
        CreateNextAction<TankFaceAction>(ACTION_MOVE)
    };
}

void TankFaceStrategy::InitTriggers(std::vector<TriggerNode*>&)
{
}

std::vector<NextAction> CombatFormationStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CombatFormationMoveAction>(ACTION_NORMAL)
    };
}
