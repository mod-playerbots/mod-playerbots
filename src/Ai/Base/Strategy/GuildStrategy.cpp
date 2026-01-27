/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GuildStrategy.h"

#include "CreateNextAction.h"
#include "GuildCreateActions.h"
#include "GuildManagementActions.h"

void GuildStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<PetitionOfferNearbyAction>(4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<GuildManageNearbyAction>(4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "petition signed",
            {
                CreateNextAction<PetitionTurnInAction>(10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "buy tabard",
            {
                CreateNextAction<BuyTabardAction>(10.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "leave large guild",
            {
                CreateNextAction<GuildLeaveAction>(4.0f)
            }
        )
    );
}
