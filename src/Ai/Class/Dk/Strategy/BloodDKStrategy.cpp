/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "BloodDKStrategy.h"

#include "CreateNextAction.h"
#include "DKActions.h"
#include "ActionNode.h"
#include "GenericActions.h"

class BloodDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BloodDKStrategyActionNodeFactory()
    {
        creators["rune strike"] = &rune_strike;
        creators["heart strike"] = &heart_strike;
        creators["death strike"] = &death_strike;
        creators["icy touch"] = &icy_touch;
        creators["dark command"] = &dark_command;
        creators["taunt spell"] = &dark_command;
    }

private:
    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            {
                CreateNextAction<CastFrostPresenceAction>(1.0f)
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            {
                CreateNextAction<CastFrostPresenceAction>(1.0f)
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            {
                CreateNextAction<CastFrostPresenceAction>(1.0f)
            },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            {
                CreateNextAction<CastFrostPresenceAction>(1.0f)
            },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* dark_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            {
                CreateNextAction<CastFrostPresenceAction>(1.0f)
            },
            /*A*/ {
                CreateNextAction<CastDeathGripAction>(1.0f)
            },
            /*C*/ {}
        );
    }
};

BloodDKStrategy::BloodDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new BloodDKStrategyActionNodeFactory());
}

std::vector<NextAction> BloodDKStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastRuneStrikeAction>(ACTION_DEFAULT + 0.8f),
        CreateNextAction<CastIcyTouchAction>(ACTION_DEFAULT + 0.7f),
        CreateNextAction<CastHeartStrikeAction>(ACTION_DEFAULT + 0.6f),
        CreateNextAction<CastBloodStrikeAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastDancingRuneWeaponAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastDeathCoilAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastPlagueStrikeAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastHornOfWinterAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void BloodDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "rune strike",
            {
                CreateNextAction<CastRuneStrikeAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "blood tap",
            {
                CreateNextAction<CastBloodTapAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "lose aggro",
            {
                CreateNextAction<CastDarkCommandAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastArmyOfTheDeadAction>(ACTION_HIGH + 4.0f),
                CreateNextAction<CastDeathStrikeAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastVampiricBloodAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "icy touch",
            {
                CreateNextAction<CastIcyTouchAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "plague strike",
            {
                CreateNextAction<CastPlagueStrikeAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
}
