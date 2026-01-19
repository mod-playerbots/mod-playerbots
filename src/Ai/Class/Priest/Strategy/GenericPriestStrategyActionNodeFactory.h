/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class GenericPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericPriestStrategyActionNodeFactory()
    {
        creators["inner fire"] = &inner_fire;
        creators["holy nova"] = &holy_nova;
        creators["power word: fortitude"] = &power_word_fortitude;
        creators["power word: fortitude on party"] = &power_word_fortitude_on_party;
        creators["divine spirit"] = &divine_spirit;
        creators["divine spirit on party"] = &divine_spirit_on_party;
        creators["power word: shield"] = &power_word_shield;
        // creators["power word: shield on party"] = &power_word_shield_on_party;
        creators["renew"] = &renew;
        creators["renew on party"] = &renew_on_party;
        creators["greater heal"] = &greater_heal;
        creators["greater heal on party"] = &greater_heal_on_party;
        creators["heal"] = &heal;
        creators["heal on party"] = &heal_on_party;
        creators["lesser heal"] = &lesser_heal;
        creators["lesser heal on party"] = &lesser_heal_on_party;
        creators["flash heal"] = &flash_heal;
        creators["flash heal on party"] = &flash_heal_on_party;
        creators["psychic scream"] = &psychic_scream;
        // creators["fade"] = &fade;
        creators["shadowfiend"] = &shadowfiend;
    }

private:
    static ActionNode* inner_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "inner fire",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* holy_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "holy nova",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_fortitude([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "power word: fortitude",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_fortitude_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "power word: fortitude on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* divine_spirit([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "divine spirit",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* divine_spirit_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "divine spirit on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "power word: shield",
            /*P*/ { NextAction("remove shadowform") },
            // /*A*/ { NextAction("renew", 50.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_shield_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "power word: shield on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* renew([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "renew",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* renew_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "renew on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* greater_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "greater heal",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("heal") },
            /*C*/ {}
        );
    }
    static ActionNode* greater_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "greater heal on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("heal on party") },
            /*C*/ {}
        );
    }
    static ActionNode* heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "heal",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("lesser heal") },
            /*C*/ {}
        );
    }
    static ActionNode* heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "heal on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("lesser heal on party") },
            /*C*/ {}
        );
    }
    static ActionNode* lesser_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "lesser heal",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* lesser_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "lesser heal on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* flash_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "flash heal",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("greater heal") },
            /*C*/ {}
        );
    }
    static ActionNode* flash_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "flash heal on party",
            /*P*/ { NextAction("remove shadowform") },
            /*A*/ { NextAction("greater heal on party") },
            /*C*/ {}
        );
    }
    static ActionNode* psychic_scream([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "psychic scream",
            /*P*/ {},
            /*A*/ { NextAction("fade") },
            /*C*/ {}
        );
    }

    static ActionNode* shadowfiend([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "shadowfiend",
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }
};

class CurePriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CurePriestStrategyActionNodeFactory()
    {
        creators["abolish disease"] = &abolish_disease;
        creators["abolish disease on party"] = &abolish_disease_on_party;
    }

private:

    static ActionNode* abolish_disease([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "abolish disease",
            /*P*/ {},
            /*A*/ { NextAction("cure disease") },
            /*C*/ {}
        );
    }

    static ActionNode* abolish_disease_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            "abolish disease on party",
            /*P*/ {},
            /*A*/ { NextAction("cure disease on party") },
            /*C*/ {}
        );
    }
};

#endif
