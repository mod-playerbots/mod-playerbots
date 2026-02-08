/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "DpsPaladinStrategy.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"

class DpsPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    DpsPaladinStrategyActionNodeFactory()
    {
        creators["sanctity aura"] = &sanctity_aura;
        creators["retribution aura"] = &retribution_aura;
        creators["seal of corruption"] = &seal_of_corruption;
        creators["seal of vengeance"] = &seal_of_vengeance;
        creators["seal of command"] = &seal_of_command;
        creators["blessing of might"] = &blessing_of_might;
        creators["crusader strike"] = &crusader_strike;
        creators["repentance"] = &repentance;
        creators["repentance on enemy healer"] = &repentance_on_enemy_healer;
        creators["repentance on snare target"] = &repentance_on_snare_target;
        creators["repentance of shield"] = &repentance_or_shield;
    }

private:
    static ActionNode* seal_of_corruption([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfVengeanceAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* seal_of_vengeance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfCommandAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* seal_of_command([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSealOfRighteousnessAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* blessing_of_might([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastBlessingOfKingsAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* crusader_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* repentance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHammerOfJusticeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_on_enemy_healer([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHammerOfJusticeOnEnemyHealerAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_on_snare_target([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastHammerOfJusticeSnareAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* sanctity_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRetributionAuraAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* retribution_aura([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastDevotionAuraAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* repentance_or_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastDivineShieldAction>(1.0f) },
            /*C*/ {}
        );
    }
};

DpsPaladinStrategy::DpsPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new DpsPaladinStrategyActionNodeFactory());
}

std::vector<NextAction> DpsPaladinStrategy::getDefaultActions()
{
    return {
        CreateNextAction<CastHammerOfWrathAction>(ACTION_DEFAULT + 0.6f),
        CreateNextAction<CastJudgementOfWisdomAction>(ACTION_DEFAULT + 0.5f),
        CreateNextAction<CastCrusaderStrikeAction>(ACTION_DEFAULT + 0.4f),
        CreateNextAction<CastDivineStormAction>(ACTION_DEFAULT + 0.3f),
        CreateNextAction<CastConsecrationAction>(ACTION_DEFAULT + 0.1f),
        CreateNextAction<MeleeAction>(ACTION_DEFAULT)
    };
}

void DpsPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "art of war",
            {
                CreateNextAction<CastExorcismAction>(ACTION_DEFAULT + 0.2f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "seal",
            {
                CreateNextAction<CastSealOfCorruptionAction>(ACTION_HIGH)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastSealOfWisdomAction>(ACTION_HIGH + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "avenging wrath",
            {
                CreateNextAction<CastAvengingWrathAction>(ACTION_HIGH + 2.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastDivineStormAction>(ACTION_HIGH + 4.0f),
                CreateNextAction<CastConsecrationAction>(ACTION_HIGH + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of melee",
            {
                CreateNextAction<ReachMeleeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
}
