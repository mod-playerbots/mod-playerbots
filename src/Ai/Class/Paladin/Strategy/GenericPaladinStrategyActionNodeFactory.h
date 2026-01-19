/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPALADINSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_GENERICPALADINSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class GenericPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericPaladinStrategyActionNodeFactory()
    {
        // creators["seal of light"] = &seal_of_light;
        creators["cleanse poison"] = &cleanse_poison;
        creators["cleanse disease"] = &cleanse_disease;
        creators["cleanse magic"] = &cleanse_magic;
        creators["cleanse poison on party"] = &cleanse_poison_on_party;
        creators["cleanse disease on party"] = &cleanse_disease_on_party;
        creators["seal of wisdom"] = &seal_of_wisdom;
        creators["seal of justice"] = &seal_of_justice;
        creators["hand of reckoning"] = &hand_of_reckoning;
        creators["judgement"] = &judgement;
        creators["judgement of wisdom"] = &judgement_of_wisdom;
        creators["divine shield"] = &divine_shield;
        creators["flash of light"] = &flash_of_light;
        creators["flash of light on party"] = &flash_of_light_on_party;
        creators["holy wrath"] = &holy_wrath;
        creators["lay on hands"] = &lay_on_hands;
        creators["lay on hands on party"] = &lay_on_hands_on_party;
        creators["hammer of wrath"] = &hammer_of_wrath;
        creators["retribution aura"] = &retribution_aura;
        creators["blessing of kings"] = &blessing_of_kings;
        creators["blessing of wisdom"] = &blessing_of_wisdom;
        creators["blessing of kings on party"] = &blessing_of_kings_on_party;
        creators["blessing of wisdom on party"] = &blessing_of_wisdom_on_party;
        creators["blessing of sanctuary on party"] = &blessing_of_sanctuary_on_party;
        creators["blessing of sanctuary"] = &blessing_of_sanctuary;
        creators["seal of command"] = &seal_of_command;
        creators["taunt spell"] = &hand_of_reckoning;
        creators["righteous defense"] = &righteous_defense;
        creators["avenger's shield"] = &avengers_shield;
        creators["divine sacrifice"] = &divine_sacrifice;
    }

private:
    static ActionNode* blessing_of_sanctuary(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of sanctuary",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* blessing_of_kings(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of kings",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* blessing_of_wisdom(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of wisdom",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* blessing_of_kings_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of kings on party",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* blessing_of_wisdom_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of wisdom on party",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* blessing_of_sanctuary_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("blessing of sanctuary on party",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* retribution_aura(PlayerbotAI* /* ai */)
    {
        return new ActionNode("retribution aura",
                              /*P*/ {},
                              /*A*/ { NextAction("devotion aura") },
                              /*C*/ {});
    }
    static ActionNode* lay_on_hands(PlayerbotAI* /* ai */)
    {
        return new ActionNode("lay on hands",
                              /*P*/ {},
                              /*A*/ {},  // { NextAction("divine shield"), new
                                              // NextAction("flash of light"), NULL),
                              /*C*/ {});
    }
    static ActionNode* lay_on_hands_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("lay on hands on party",
                              /*P*/ {},
                              /*A*/ {},  // { NextAction("flash of light"), NULL),
                              /*C*/ {});
    }
    // static ActionNode* seal_of_light(PlayerbotAI* /* ai */)
    // {
    //     return new ActionNode ("seal of light",
    //         /*P*/ NULL,
    //         /*A*/ { NextAction("seal of justice"), NULL),
    //         /*C*/ NULL);
    // }
    static ActionNode* cleanse_poison(PlayerbotAI* /* ai */)
    {
        return new ActionNode("cleanse poison",
                              /*P*/ {},
                              /*A*/ { NextAction("purify poison") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_magic(PlayerbotAI* /* ai */)
    {
        return new ActionNode("cleanse magic",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* cleanse_disease(PlayerbotAI* /* ai */)
    {
        return new ActionNode("cleanse disease",
                              /*P*/ {},
                              /*A*/ { NextAction("purify disease") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_poison_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("cleanse poison on party",
                              /*P*/ {},
                              /*A*/ { NextAction("purify poison on party") },
                              /*C*/ {});
    }
    static ActionNode* cleanse_disease_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("cleanse disease on party",
                              /*P*/ {},
                              /*A*/ { NextAction("purify disease on party") },
                              /*C*/ {});
    }
    static ActionNode* seal_of_wisdom(PlayerbotAI* /* ai */)
    {
        return new ActionNode ("seal of wisdom",
            /*P*/ {},
            /*A*/ { NextAction("seal of righteousness") },
            /*C*/ {});
    }
    static ActionNode* seal_of_justice(PlayerbotAI* /* ai */)
    {
        return new ActionNode("seal of justice",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of righteousness") },
                              /*C*/ {});
    }
    static ActionNode* hand_of_reckoning(PlayerbotAI* /* ai */)
    {
        return new ActionNode("hand of reckoning",
                              /*P*/ {},
                              /*A*/ { NextAction("righteous defense") },
                              /*C*/ {});
    }
    static ActionNode* righteous_defense(PlayerbotAI* /* ai */)
    {
        return new ActionNode("righteous defense",
                              /*P*/ {},
                              /*A*/ { NextAction("avenger's shield") },
                              /*C*/ {});
    }
    static ActionNode* avengers_shield(PlayerbotAI* /* ai */)
    {
        return new ActionNode("avenger's shield",
                              /*P*/ {},
                              /*A*/ { NextAction("judgement of wisdom") },
                              /*C*/ {});
    }
    static ActionNode* divine_sacrifice(PlayerbotAI* /* ai */)
    {
        return new ActionNode("divine sacrifice",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ { NextAction("cancel divine sacrifice") });
    }
    static ActionNode* judgement_of_wisdom(PlayerbotAI* /* ai */)
    {
        return new ActionNode("judgement of wisdom",
                              /*P*/ {},
                              /*A*/ { NextAction("judgement of light") },
                              /*C*/ {});
    }
    static ActionNode* judgement(PlayerbotAI* /* ai */)
    {
        return new ActionNode("judgement",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* divine_shield(PlayerbotAI* /* ai */)
    {
        return new ActionNode("divine shield",
                              /*P*/ {},
                              /*A*/ { NextAction("divine protection") },
                              /*C*/ {});
    }
    static ActionNode* flash_of_light(PlayerbotAI* /* ai */)
    {
        return new ActionNode("flash of light",
                              /*P*/ {},
                              /*A*/ { NextAction("holy light") },
                              /*C*/ {});
    }
    static ActionNode* flash_of_light_on_party(PlayerbotAI* /* ai */)
    {
        return new ActionNode("flash of light on party",
                              /*P*/ {},
                              /*A*/ { NextAction("holy light on party") },
                              /*C*/ {});
    }
    static ActionNode* holy_wrath(PlayerbotAI* /* ai */)
    {
        return new ActionNode("holy wrath",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* hammer_of_wrath(PlayerbotAI* /* ai */)
    {
        return new ActionNode("hammer of wrath",
                              /*P*/ {},
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* seal_of_command(PlayerbotAI* /* ai */)
    {
        return new ActionNode("seal of command",
                              /*P*/ {},
                              /*A*/ { NextAction("seal of righteousness") },
                              /*C*/ {});
    }
};

#endif
