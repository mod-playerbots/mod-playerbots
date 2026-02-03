/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H
#define _PLAYERBOT_GENERICPRIESTSTRATEGYACTIONNODEFACTORY_H

#include "Action.h"
#include "ActionNode.h"
#include "CreateNextAction.h"
#include "NamedObjectContext.h"
#include "PriestActions.h"

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
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* holy_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_fortitude([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_fortitude_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* divine_spirit([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* divine_spirit_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_shield([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            // /*A*/ { CreateNextAction("renew", 50.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* power_word_shield_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* renew([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* renew_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* greater_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastHealAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* greater_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastHealOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastLesserHealAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastLesserHealOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* lesser_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* lesser_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ {},
            /*C*/ {}
        );
    }
    static ActionNode* flash_heal([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastGreaterHealAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* flash_heal_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ { CreateNextAction<CastRemoveShadowformAction>(1.0f) },
            /*A*/ { CreateNextAction<CastGreaterHealOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* psychic_scream([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFadeAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* shadowfiend([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
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
            /*P*/ {},
            /*A*/ { CreateNextAction<CastCureDiseaseAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* abolish_disease_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastCureDiseaseOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
};

#endif
