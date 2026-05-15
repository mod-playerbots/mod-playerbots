/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "WaitForAttackStrategy.h"

#include "Action.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Strategy.h"

void WaitForAttackStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wait for attack safe distance",
        {
            NextAction("wait for attack keep safe distance", ACTION_RAID)
        }
    ));
}

void WaitForAttackStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new WaitForAttackMultiplier(botAI));
}

bool WaitForAttackStrategy::ShouldWait(PlayerbotAI* botAI)
{
    if (botAI->HasStrategy("wait for attack", BOT_STATE_COMBAT))
    {
        Player* bot = botAI->GetBot();
        if (bot->GetGroup() && botAI->HasRealPlayerMaster())
        {
            // Don't wait if the current target is an enemy player
            Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
            if (target && target->IsPlayer())
                return false;

            AiObjectContext* context = botAI->GetAiObjectContext();
            time_t combatStartTime = context->GetValue<time_t>("combat start time")->Get();

            if (bot->IsInCombat())
            {
                if (combatStartTime == 0)
                {
                    combatStartTime = time(nullptr);
                    context->GetValue<time_t>("combat start time")->Set(combatStartTime);
                }

                time_t elapsedTime = time(nullptr) - combatStartTime;
                return elapsedTime < GetWaitTime(botAI);
            }
            else
            {
                if (combatStartTime != 0)
                    context->GetValue<time_t>("combat start time")->Set(0);
            }
        }
    }

    return false;
}

uint8 WaitForAttackStrategy::GetWaitTime(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<uint8>("wait for attack time")->Get();
}

float WaitForAttackStrategy::GetSafeDistance()
{
    return sPlayerbotAIConfig.spellDistance;
}

float WaitForAttackMultiplier::GetValue(Action* action)
{
    std::string const& actionName = action->getName();

    if (actionName != "wait for attack keep safe distance" &&
        actionName != "dps assist" &&
        actionName != "set facing" &&
        actionName != "pull my target" &&
        actionName != "pull rti target" &&
        actionName != "reach pull" &&
        actionName != "pull start" &&
        actionName != "pull action" &&
        actionName != "pull end")
    {
        return WaitForAttackStrategy::ShouldWait(botAI) ? 0.0f : 1.0f;
    }

    return 1.0f;
}
