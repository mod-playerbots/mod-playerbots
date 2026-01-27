/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "EmoteStrategy.h"

#include "PlayerbotAIConfig.h"
#include "CreateNextAction.h"
#include "EmoteAction.h"
#include "SuggestWhatToDoAction.h"
#include "GreetAction.h"
#include "RpgSubActions.h"

void EmoteStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (PlayerbotAIConfig::instance().randomBotEmote)
    {
        triggers.push_back(
            new TriggerNode(
                "often",
                {
                    CreateNextAction<TalkAction>(1.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "seldom",
                {
                    CreateNextAction<EmoteAction>(1.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "receive text emote",
                {
                    CreateNextAction<EmoteAction>(10.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "receive emote",
                {
                    CreateNextAction<EmoteAction>(10.0f)
                }
            )
        );
    }

    if (PlayerbotAIConfig::instance().randomBotTalk)
    {
        triggers.push_back(
            new TriggerNode(
                "often",
                {
                    CreateNextAction<SuggestWhatToDoAction>(10.0f),
                    CreateNextAction<SuggestDungeonAction>(3.0f),
                    CreateNextAction<SuggestTradeAction>(3.0f)
                }
            )
        );
    }

    if (PlayerbotAIConfig::instance().enableGreet)
    {
        triggers.push_back(
            new TriggerNode(
                "new player nearby",
                {
                    CreateNextAction<GreetAction>(1.0f)
                }
            )
        );
    }

    triggers.push_back(
        new TriggerNode(
            "often",
            {
                CreateNextAction<RpgMountAnimAction>(1.0f)
            }
        )
    );
}
