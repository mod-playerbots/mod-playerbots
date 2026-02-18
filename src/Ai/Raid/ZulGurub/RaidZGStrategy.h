#pragma once

#include "CreateNextAction.h"
#include "Strategy.h"
#include "Multiplier.h"
#include "Trash/GurubashiBatRider/GurubashiBatRiderUnstableConcoctionAction.h"
#include "ZulGurub/Trash/GurubashiBatRider/GurubashiBatRiderUnstableConcoctionMultiplier.h"

class RaidZGStrategy : public Strategy
{
public:
    RaidZGStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    const std::string getName() override
    {
        return "zg";
    }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override
    {
        triggers.push_back(
            new TriggerNode(
                "gurubashi bat rider unstable concoction",
                {
                    CreateNextAction<GurubashiBatRiderUnstableConcoctionAction>(ACTION_EMERGENCY + 10.0f)
                }
            )
        );
    }

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override
    {
        multipliers.push_back(new GurubashiBatRiderUnstableConcoctionMultiplier(botAI));
    }
};
