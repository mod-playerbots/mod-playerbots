/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "FrostDKStrategy.h"

#include "CreateNextAction.h"
#include "DKActions.h"
#include "GenericActions.h"

class FrostDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostDKStrategyActionNodeFactory()
    {
        creators["icy touch"] = &icy_touch;
        creators["obliterate"] = &obliterate;
        creators["howling blast"] = &howling_blast;
        creators["frost strike"] = &frost_strike;
        creators["rune strike"] = &rune_strike;
        creators["unbreakable armor"] = &unbreakable_armor;
    }

private:
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* obliterate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ { CreateNextAction<MeleeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* frost_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* howling_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* unbreakable_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodTapAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

FrostDKStrategy::FrostDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new FrostDKStrategyActionNodeFactory());
}

std::vector<NextAction> FrostDKStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastObliterateAction>(ACTION_DEFAULT + 0.7f),
        CreateNextAction<CastFrostStrikeAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastEmpowerRuneWeaponAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastHornOfWinterAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void FrostDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "unbreakable armor",
            {
                CreateNextAction<CastUnbreakableArmorAction>(ACTION_DEFAULT + 0.6f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "freezing fog",
            {
                CreateNextAction<CastHowlingBlastAction>(ACTION_DEFAULT + 0.5f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high blood rune",
            {
                CreateNextAction<CastBloodStrikeAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "army of the dead",
            {
                CreateNextAction<CastArmyOfTheDeadAction>(ACTION_HIGH + 6.0f)
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

void FrostDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastHowlingBlastAction>(ACTION_HIGH + 4)
            }
        )
    );
}
