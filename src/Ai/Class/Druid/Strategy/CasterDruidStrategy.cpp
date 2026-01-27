/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "CasterDruidStrategy.h"

#include "ActionNode.h"
#include "CreateNextAction.h"
#include "AiObjectContext.h"
#include "MovementActions.h"
#include "CancelChannelAction.h"
#include "DruidActions.h"
#include "DruidShapeshiftActions.h"
#include "FeralDruidStrategy.h"

class CasterDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterDruidStrategyActionNodeFactory()
    {
        creators["faerie fire"] = &faerie_fire;
        creators["hibernate"] = &hibernate;
        creators["entangling roots"] = &entangling_roots;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["wrath"] = &wrath;
        creators["starfall"] = &starfall;
        creators["insect swarm"] = &insect_swarm;
        creators["moonfire"] = &moonfire;
        creators["starfire"] = &starfire;
        creators["moonkin form"] = &moonkin_form;
    }

private:
    static ActionNode* faerie_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* hibernate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ { CreateNextAction<CastEntanglingRootsAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* entangling_roots([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* starfall([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* insect_swarm([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* moonfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* starfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastMoonkinFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* moonkin_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastCasterFormAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

CasterDruidStrategy::CasterDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CasterDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

std::vector<NextAction> CasterDruidStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastStarfallAction>(ACTION_HIGH + 1.0f),
        CreateNextAction<CastForceOfNatureAction>(ACTION_DEFAULT + 1.0f),
        CreateNextAction<CastWrathAction>(ACTION_DEFAULT + 0.1f),
    };
}

void CasterDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "eclipse (lunar) cooldown",
            {
                CreateNextAction<CastStarfireAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "eclipse (solar) cooldown",
            {
                CreateNextAction<CastWrathAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "insect swarm",
            {
                CreateNextAction<CastInsectSwarmAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "moonfire",
            {
                CreateNextAction<CastMoonfireAction>(ACTION_NORMAL + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "eclipse (solar)",
            {
                CreateNextAction<CastWrathAction>(ACTION_NORMAL + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "eclipse (lunar)",
            {
                CreateNextAction<CastStarfireAction>(ACTION_NORMAL + 6.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium mana",
            {
                CreateNextAction<CastInnervateAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell",
            {
                CreateNextAction<FleeAction>(ACTION_MOVE + 9.0f)
            }
        )
    );
}

void CasterDruidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "hurricane channel check",
            {
                CreateNextAction<CancelChannelAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastHurricaneAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "light aoe",
            {
                CreateNextAction<CastInsectSwarmOnAttackerAction>(ACTION_NORMAL + 3.0f),
                CreateNextAction<CastMoonfireOnAttackerAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
}

void CasterDruidDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "faerie fire",
            {
                CreateNextAction<CastFaerieFireAction>(ACTION_HIGH)
            }
        )
    );
}
