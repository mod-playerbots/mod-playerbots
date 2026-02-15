/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SurvivalHunterStrategy.h"
#include "CreateNextAction.h"
#include "HunterActions.h"

// ===== Action Node Factory =====
class SurvivalHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    SurvivalHunterStrategyActionNodeFactory()
    {
        creators["auto shot"] = &auto_shot;
        creators["kill command"] = &kill_command;
        creators["kill shot"] = &kill_shot;
        creators["explosive shot"] = &explosive_shot;
        creators["black arrow"] = &black_arrow;
        creators["viper sting"] = &viper_sting;
        creators["serpent sting"] = serpent_sting;
        creators["aimed shot"] = &aimed_shot;
        creators["arcane shot"] = &arcane_shot;
        creators["steady shot"] = &steady_shot;
        creators["multi-shot"] = &multi_shot;
        creators["volley"] = &volley;
    }

private:
    static ActionNode* auto_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* kill_command(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* kill_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* explosive_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* black_arrow(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* viper_sting(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* serpent_sting(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* aimed_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* arcane_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* steady_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* multi_shot(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

    static ActionNode* volley(PlayerbotAI*)
    {
        return new ActionNode(
            {},
            {},
            {}
        );
    }

};

// ===== Single Target Strategy =====
SurvivalHunterStrategy::SurvivalHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new SurvivalHunterStrategyActionNodeFactory());
}

// ===== Default Actions =====
std::vector<NextAction> SurvivalHunterStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastKillCommandAction>(5.9f),
        CreateNextAction<CastKillShotAction>(5.8f),
        CreateNextAction<CastExplosiveShotAction>(5.7f),
        CreateNextAction<CastBlackArrow>(5.6f),
        CreateNextAction<CastSerpentStingAction>(5.5f),
        CreateNextAction<CastAimedShotAction>(5.4f),
        CreateNextAction<CastArcaneShotAction>(5.3f),
        CreateNextAction<CastSteadyShotAction>(5.2f),
        CreateNextAction<CastAutoShotAction>(5.1f)
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
                CreateNextAction<CastExplosiveShotRank4Action>(28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lock and load",
            {
                CreateNextAction<CastExplosiveShotRank3Action>(27.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lock and load",
            {
                CreateNextAction<CastExplosiveShotRank2Action>(27.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lock and load",
            {
                CreateNextAction<CastExplosiveShotRank1Action>(26.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "kill command",
            {
                CreateNextAction<CastKillCommandAction>(18.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "target critical health",
            {
                CreateNextAction<CastKillShotAction>(18.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "explosive shot",
            {
                CreateNextAction<CastExplosiveShotAction>(17.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "black arrow",
            {
                CreateNextAction<CastBlackArrow>(16.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastViperStingAction>(16.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no stings",
            {
                CreateNextAction<CastSerpentStingAction>(15.5f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "serpent sting on attacker",
            {
                CreateNextAction<CastSerpentStingOnAttackerAction>(15.0f)
            }
        )
    );
}
