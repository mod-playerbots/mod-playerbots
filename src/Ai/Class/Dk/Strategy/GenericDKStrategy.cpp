/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GenericDKStrategy.h"

#include "DKAiObjectContext.h"
#include "Playerbots.h"

class GenericDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKStrategyActionNodeFactory()
    {
        creators["killing machine"] = &killing_machine;
        creators["anti magic zone"] = &anti_magic_zone;
        creators["death grip"] = &death_grip;
    }

private:
    static ActionNode* death_grip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death grip",
                              /*P*/ {},
                              /*A*/ { NextAction("icy touch") },
                              /*C*/ {});
    }

    static ActionNode* killing_machine([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("killing machine",
                              /*P*/ {},
                              /*A*/ { NextAction("improved icy talons") },
                              /*C*/ {});
    }

    static ActionNode* anti_magic_zone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("anti magic zone",
                              /*P*/ {},
                              /*A*/ { NextAction("anti magic shell") },
                              /*C*/ {});
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
        new TriggerNode("mind freeze", { NextAction("mind freeze", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("mind freeze on enemy healer",
                        { NextAction("mind freeze on enemy healer", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "horn of winter", { NextAction("horn of winter", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("critical health",
                                       { NextAction("raise dead", ACTION_HIGH + 6),
                                         NextAction("death pact", ACTION_HIGH + 5) }));

    triggers.push_back(
        new TriggerNode("low health", { NextAction("icebound fortitude", ACTION_HIGH + 5),
                                                        NextAction("rune tap", ACTION_HIGH + 4) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { NextAction("death and decay", ACTION_HIGH + 9),
                                                        NextAction("pestilence", ACTION_NORMAL + 4),
                                                        NextAction("blood boil", ACTION_NORMAL + 3) }));
    triggers.push_back(
        new TriggerNode("pestilence glyph", { NextAction("pestilence", ACTION_HIGH + 9) }));
    triggers.push_back(
        new TriggerNode("no rune",
            {
                NextAction("empower rune weapon", ACTION_HIGH + 1)
            }
        )
    );
}
