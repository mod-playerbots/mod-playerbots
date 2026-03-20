/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SurvivalHunterStrategy.h"
#include "Playerbots.h"

// ===== Action Node Factory =====
class SurvivalHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    SurvivalHunterStrategyActionNodeFactory()
    {
        creators["explosive shot rank 4"] = &explosive_shot_rank_4;
        creators["explosive shot rank 3"] = &explosive_shot_rank_3;
        creators["explosive shot rank 2"] = &explosive_shot_rank_2;
    }

private:
    static ActionNode* explosive_shot_rank_4([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 4",
                              /*P*/ {},
                              /*A*/ { NextAction("explosive shot rank 3") },
                              /*C*/ {});
    }
    static ActionNode* explosive_shot_rank_3([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 3",
                              /*P*/ {},
                              /*A*/ { NextAction("explosive shot rank 2") },
                              /*C*/ {});
    }
    static ActionNode* explosive_shot_rank_2([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 2",
                              /*P*/ {},
                              /*A*/ { NextAction("explosive shot rank 1") },
                              /*C*/ {});
    }
};

SurvivalHunterStrategy::SurvivalHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new SurvivalHunterStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> SurvivalHunterStrategy::getDefaultActions()
{
    return {
        NextAction("kill command", 5.9f),
        NextAction("kill shot", 5.8f),
        NextAction("explosive shot", 5.7f),
        NextAction("black arrow", 5.6f),
        NextAction("serpent sting", 5.5f),
        NextAction("aimed shot", 5.4f),
        NextAction("arcane shot", 5.3f),
        NextAction("steady shot", 5.2f),
        NextAction("auto shot", 5.1f)
    };
}

// ===== Trigger Initialization ===
void SurvivalHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "lock and load",
            {
                NextAction("explosive shot rank 4", 28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kill command",
            {
                NextAction("kill command", 18.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                NextAction("kill shot", 18.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "explosive shot",
            {
                NextAction("explosive shot", 17.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "black arrow",
            {
                NextAction("black arrow", 16.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                NextAction("viper sting", 16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no stings",
            {
                NextAction("serpent sting", 15.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "serpent sting on attacker",
            {
                NextAction("serpent sting on attacker", 15.0f)
            }
        )
    );
}
