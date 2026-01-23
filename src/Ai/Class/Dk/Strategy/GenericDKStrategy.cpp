#/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDKStrategy.h"

#include "DKAiObjectContext.h"
#include "Playerbots.h"
#include "CreateNextAction.h"
#include "ActionNode.h"
#include "DKActions.h"
#include "GenericActions.h"

class GenericDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKStrategyActionNodeFactory()
    {
        creators["horn of winter"] = &horn_of_winter;
        creators["killing machine"] = &killing_machine;
        creators["icebound fortitude"] = &icebound_fortitude;
        creators["death and decay"] = &death_and_decay;
        creators["anti magic zone"] = &anti_magic_zone;
        creators["corpse explosion"] = &corpse_explosion;
        creators["bone shield"] = &bone_shield;
        creators["heart strike"] = &heart_strike;
        creators["death grip"] = &death_grip;
        creators["plague strike"] = &plague_strike;
        creators["pestilence"] = &pestilence;
        creators["icy touch"] = &icy_touch;
    }

private:
    static ActionNode* death_coil([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* death_grip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastIcyTouchAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* plague_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* pestilence([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* horn_of_winter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* bone_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* killing_machine([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastImprovedIcyTalonsAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* death_and_decay([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* anti_magic_zone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastAntiMagicShellAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* icebound_fortitude([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

GenericDKStrategy::GenericDKStrategy(PlayerbotAI* botAI) : MeleeCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDKStrategyActionNodeFactory());
}

void GenericDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    MeleeCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastRaiseDeadAction>(ACTION_NORMAL + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "has pet",
            {
                CreateNextAction<TogglePetSpellAutoCastAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "new pet",
            {
                CreateNextAction<SetPetStanceAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mind freeze",
            {
                CreateNextAction<CastMindFreezeAction>(ACTION_HIGH + 1.0f)
            }
        )
    );
    // "mind freeze on enemy healer" does not exist as an action
    // triggers.push_back(
    //     new TriggerNode(
    //         "mind freeze on enemy healer",
    //         {
    //             CreateNextAction("mind freeze on enemy healer", ACTION_HIGH + 1.0f)
    //         }
    //     )
    // );
    triggers.push_back(
        new TriggerNode(
            "horn of winter",
            {
                CreateNextAction<CastHornOfWinterAction>(ACTION_NORMAL + 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastDeathPactAction>(ACTION_HIGH + 5.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastIceboundFortitudeAction>(ACTION_HIGH + 5.0f),
                CreateNextAction<CastRuneTapAction>(ACTION_HIGH + 4.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium aoe",
            {
                CreateNextAction<CastDeathAndDecayAction>(ACTION_HIGH + 9.0f),
                CreateNextAction<CastPestilenceAction>(ACTION_NORMAL + 4.0f),
                CreateNextAction<CastBloodBoilAction>(ACTION_NORMAL + 3.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "pestilence glyph",
            {
                CreateNextAction<CastPestilenceAction>(ACTION_HIGH + 9.0f)
            }
        )
    );
}
