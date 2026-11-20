/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AssassinationRogueStrategy.h"
#include "Playerbots.h"

class AssassinationRogueStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    AssassinationRogueStrategyActionNodeFactory()
    {
        creators["mutilate"] = &mutilate;
        creators["envenom"] = &envenom;
        creators["backstab"] = &backstab;
    }

private:
    static ActionNode* mutilate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "mutilate",
            /*P*/ {},
            /*A*/ { NextAction("backstab") },
            /*C*/ {}
        );
    }
    static ActionNode* envenom([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "envenom",
            /*P*/ {},
            /*A*/ { NextAction("eviscerate") },
            /*C*/ {}
        );
    }
    static ActionNode* backstab([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "backstab",
            /*P*/ {},
            /*A*/ { NextAction("sinister strike") },
            /*C*/ {}
        );
    }
};

AssassinationRogueStrategy::AssassinationRogueStrategy(PlayerbotAI* botAI)
    : GenericRogueStrategy(botAI)
{
    actionNodeFactories.Add(new AssassinationRogueStrategyActionNodeFactory());
}

std::vector<NextAction> AssassinationRogueStrategy::getDefaultActions()
{
    return {
        NextAction("melee", ACTION_DEFAULT)
    };
}

void AssassinationRogueStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericRogueStrategy::InitTriggers(triggers);

    // ---------------------------------------------------------------
    // Rotation (opening and steady-state priority)
    //
    // Opening: Stealth -> Garrote -> Hunger for Blood -> Slice and Dice -> Mutilate -> Envenom
    // Steady: Mutilate -> Envenom (>=4 CP), Cold Blood before the first Envenom
    // Refresh: HBF expiring and target not bleeding -> low-CP Rupture -> refresh HBF
    //
    // Ordered high to low priority; each tick only the highest-priority ready action runs.
    // ---------------------------------------------------------------

    // [1] Highest: HBF expiring + target not bleeding -> Rupture (apply a bleed)
    //     Must come before HBF, otherwise the bot loops trying to cast it with no bleed.
    triggers.push_back(
        new TriggerNode(
            "rupture for hunger",
            {
                NextAction("rupture", ACTION_HIGH + 10)
            }
        )
    );

    // [2] Garrote opener (stealthed + enough energy)
    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                NextAction("garrote", ACTION_HIGH + 7),
                NextAction("ambush", ACTION_HIGH + 6)
            }
        )
    );

    // [3] Hunger for Blood: triggers when absent or within 8s remaining (duration handled in trigger)
    triggers.push_back(
        new TriggerNode(
            "hunger for blood",
            {
                NextAction("hunger for blood", ACTION_HIGH + 6),
            }
        )
    );

    // [4] Slice and Dice: maintain the buff
    triggers.push_back(
        new TriggerNode(
            "slice and dice",
            {
                NextAction("slice and dice", ACTION_HIGH + 5),
            }
        )
    );

    // [5] 4 CP: Cold Blood first (only if off CD), then Envenom.
    //     3-minute CD naturally means it is not used on every Envenom, matching "before the first Envenom".
    triggers.push_back(
        new TriggerNode(
            "combo points 4 available",
            {
                NextAction("cold blood", ACTION_HIGH + 6),
                NextAction("envenom", ACTION_HIGH + 5)
            }
        )
    );

    // [6] Execute on low-HP target
    triggers.push_back(
        new TriggerNode(
            "target with combo points almost dead",
            {
                NextAction("envenom", ACTION_HIGH + 4)
            }
        )
    );

    // [7] Expose Armor
    triggers.push_back(
        new TriggerNode(
            "expose armor",
            {
                NextAction("expose armor", ACTION_HIGH + 3),
            }
        )
    );

    // [8] Filler: Mutilate to build combo points
    triggers.push_back(
        new TriggerNode(
            "high energy available",
            {
                NextAction("mutilate", ACTION_NORMAL + 3)
            }
        )
    );
}
