/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PRIESTNONCOMBATSTRATEGYACTIONNODEFACTORY_H
#define PLAYERBOTS_PRIESTNONCOMBATSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "NamedObjectContext.h"

class PlayerbotAI;

class PriestNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    PriestNonCombatStrategyActionNodeFactory()
    {
        creators["holy nova"] = &holy_nova;
        creators["power word: shield"] = &power_word_shield;
        creators["power word: shield on party"] = &power_word_shield_on_party;
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
        creators["circle of healing on party"] = &circle_of_healing;
    }

private:
    static ActionNode* holy_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("holy nova",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* power_word_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("power word: shield",
                              /*P*/ {},
                              /*A*/ { NextAction("renew", 50.0f) },
                              /*C*/ {});
    }
    static ActionNode* power_word_shield_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("power word: shield on party",
                              /*P*/ {},
                              /*A*/ { NextAction("renew on party", 50.0f) },
                              /*C*/ {});
    }
    static ActionNode* renew([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("renew",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* renew_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("renew on party",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* greater_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("greater heal",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ { NextAction("heal") },
                              /*C*/ {});
    }
    static ActionNode* greater_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("greater heal on party",
                              /*P*/ {},
                              /*A*/ { NextAction("heal on party") },
                              /*C*/ {});
    }
    static ActionNode* heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heal",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ { NextAction("lesser heal") },
                              /*C*/ {});
    }
    static ActionNode* heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("heal on party",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ { NextAction("lesser heal on party") },
                              /*C*/ {});
    }
    static ActionNode* lesser_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser heal",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* lesser_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lesser heal on party",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* flash_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flash heal",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* flash_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flash heal on party",
                              /*P*/ { NextAction("remove shadowform") },
                              /*A*/ {},
                              /*C*/ {});
    }
    static ActionNode* circle_of_healing([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("circle of healing on party",
                              /*P*/ { NextAction("remove shadowform") },
                              // /*A*/ { NextAction("flash heal on party") },
                              /*A*/ {},
                              /*C*/ {});
    }
};

#endif
