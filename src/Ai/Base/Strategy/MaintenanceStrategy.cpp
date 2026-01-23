/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "MaintenanceStrategy.h"

#include "Playerbots.h"
#include "CreateNextAction.h"
#include "DropQuestAction.h"
#include "UseItemAction.h"
#include "CastCustomSpellAction.h"
#include "DestroyItemAction.h"
#include "ShareQuestAction.h"

std::vector<NextAction> MaintenanceStrategy::getDefaultActions() { return {}; }

void MaintenanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<CleanQuestLogAction>(6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<UseRandomRecipe>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<DisEnchantRandomItemAction>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<EnchantRandomItemAction>(1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<SmartDestroyItemAction>(1.0f)
            }
        )
    );
    // It seems "reset" has no attached creator
    // triggers.push_back(
    //     new TriggerNode(
    //         "move stuck",
    //         {
    //             CreateNextAction("reset", 1.0f)
    //         }
    //     )
    // );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<UseRandomQuestItem>(0.9f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "random",
            {
                CreateNextAction<AutoShareQuestAction>(0.9f)
            }
        )
    );
}
