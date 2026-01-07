#/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericDKStrategy.h"

#include "DKAiObjectContext.h"
#include "Playerbots.h"

class GenericDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKStrategyActionNodeFactory()
    {
        // blood
        // creators["rune tap"] = &rune_tap; cd
        // creators["vampiric blood"] = &vampiric_blood;
        // creators["death pact"] = &death_pact;
        // creators["hysteria"] = &hysteria; boost party
        // creators["dancing rune weapon"] = &dancing_rune_weapon; //cd
        // creators["dark command"] = &dark_command; taunt

        // frost
        // creators["chains of ice"] = &chains_of_ice;
        // creators["icy clutch"] = &icy_clutch;
        creators["horn of winter"] = &horn_of_winter;
        creators["killing machine"] = &killing_machine;  // buff
        // creators["deathchill"] = &deathchill;        //boost
        creators["icebound fortitude"] = &icebound_fortitude;
        // creators["mind freeze"] = &mind_freeze; interrupt
        // creators["empower rune weapon"] = &empower_rune_weapon; boost
        // creators["hungering cold"] = &hungering_cold; snare
        // creators["unbreakable armor"] = &unbreakable_armor; boost +cd
        // creators["improved icy talons"] = &improved_icy_talons; boost party

        // unholy
        creators["death and decay"] = &death_and_decay;
        // creators["raise dead"] = &raise_dead;
        // creators["army of the dead"] = &army of the dead;
        // creators["summon gargoyle"] = &army of the dead;
        // creators["anti magic shell"] = &anti_magic_shell; cd
        creators["anti magic zone"] = &anti_magic_zone;
        // creators["ghoul frenzy"] = &ghoul_frenzy;
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
        return new ActionNode("death coil",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* death_grip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death grip",
                              /*P*/ {},
                              /*A*/ { NextAction("icy touch") },
                              /*C*/ {});
    }

    static ActionNode* plague_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("plague strike",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy touch",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* heart_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heart strike",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* pestilence([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("pestilence",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* horn_of_winter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("horn of winter",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* bone_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("bone shield",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* killing_machine([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("killing machine",
                              /*P*/ {},
                              /*A*/ { NextAction("improved icy talons") },
                              /*C*/ {});
    }

    static ActionNode* corpse_explosion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("corpse explosion",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* death_and_decay([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("death and decay",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }

    static ActionNode* anti_magic_zone([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("anti magic zone",
                              /*P*/ {},
                              /*A*/ { NextAction("anti magic shell") },
                              /*C*/ {});
    }

    static ActionNode* icebound_fortitude([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icebound fortitude",
                              /*P*/ {},
                              /*A*/ {},
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
        new TriggerNode("no pet", { NextAction("raise dead", ACTION_NORMAL + 5) }));
    triggers.push_back(
        new TriggerNode("has pet", { NextAction("toggle pet spell", 60.0f) }));
    triggers.push_back(
        new TriggerNode("new pet", { NextAction("set pet stance", 60.0f) }));
    triggers.push_back(
        new TriggerNode("mind freeze", { NextAction("mind freeze", ACTION_HIGH + 1) }));
    triggers.push_back(
        new TriggerNode("mind freeze on enemy healer",
                        { NextAction("mind freeze on enemy healer", ACTION_HIGH + 1) }));
    triggers.push_back(new TriggerNode(
        "horn of winter", { NextAction("horn of winter", ACTION_NORMAL + 1) }));
    triggers.push_back(new TriggerNode("critical health",
                                       { NextAction("death pact", ACTION_HIGH + 5) }));

    triggers.push_back(
        new TriggerNode("low health", { NextAction("icebound fortitude", ACTION_HIGH + 5),
                                                        NextAction("rune tap", ACTION_HIGH + 4) }));
    triggers.push_back(
        new TriggerNode("medium aoe", { NextAction("death and decay", ACTION_HIGH + 9),
                                                        NextAction("pestilence", ACTION_NORMAL + 4),
                                                        NextAction("blood boil", ACTION_NORMAL + 3) }));
    triggers.push_back(
        new TriggerNode("pestilence glyph", { NextAction("pestilence", ACTION_HIGH + 9) }));
}
