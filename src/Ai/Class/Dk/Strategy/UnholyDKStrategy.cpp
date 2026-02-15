/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "UnholyDKStrategy.h"
#include "CreateNextAction.h"
#include "DKActions.h"
#include "GenericActions.h"

class UnholyDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    UnholyDKStrategyActionNodeFactory()
    {
        creators["death strike"] = &death_strike;
        creators["scourge strike"] = &scourge_strike;
        creators["ghoul frenzy"] = &ghoul_frenzy;
        creators["corpse explosion"] = &corpse_explosion;
        creators["icy touch"] = &icy_touch;
    }

private:
    static ActionNode* death_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* ghoul_frenzy([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* scourge_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastBloodPresenceAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
};

UnholyDKStrategy::UnholyDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new UnholyDKStrategyActionNodeFactory());
}

std::vector<NextAction> UnholyDKStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastDeathAndDecayAction>(ACTION_HIGH + 5.0f),
        CreateNextAction<CastSummonGargoyleAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastHornOfWinterAction>(ACTION_DEFAULT + 0.2f),
        CreateNextAction<CastDeathCoilAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void UnholyDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "death and decay cooldown",
            {
                CreateNextAction<CastGhoulFrenzyAction>(ACTION_DEFAULT + 0.9f),
                CreateNextAction<CastScourgeStrikeAction>(ACTION_DEFAULT + 0.8f),
                CreateNextAction<CastIcyTouchAction>(ACTION_DEFAULT + 0.7f),
                CreateNextAction<CastBloodStrikeAction>(ACTION_DEFAULT + 0.6f),
                CreateNextAction<CastPlagueStrikeAction>(ACTION_DEFAULT + 0.5f),
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "dd cd and no desolation",
            {
                CreateNextAction<CastBloodStrikeAction>(ACTION_DEFAULT + 0.75f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high frost rune",
            {
                CreateNextAction<CastIcyTouchAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high blood rune",
            {
                CreateNextAction<CastBloodStrikeAction>(ACTION_NORMAL + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "high unholy rune",
            {
                CreateNextAction<CastPlagueStrikeAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dd cd and plague strike 3s",
            {
                CreateNextAction<CastPlagueStrikeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "dd cd and icy touch 3s",
            {
                CreateNextAction<CastIcyTouchAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no rune",
            {
                CreateNextAction<CastEmpowerRuneWeaponAction>(ACTION_HIGH + 1.0f)
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
            "bone shield",
            {
                CreateNextAction<CastBoneShieldAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
}

void UnholyDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "loot available",
            {
                CreateNextAction<CastCorpseExplosionAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastDeathAndDecayAction>(ACTION_NORMAL + 3.0f),
                CreateNextAction<CastCorpseExplosionAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
}
