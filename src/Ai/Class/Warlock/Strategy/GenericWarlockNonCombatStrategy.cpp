/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericWarlockNonCombatStrategy.h"
#include "AiFactory.h"
#include "CreateNextAction.h"
#include "GenericActions.h"
#include "WarlockActions.h"

class GenericWarlockNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarlockNonCombatStrategyActionNodeFactory()
    {
        creators["fel armor"] = &fel_armor;
        creators["demon armor"] = &demon_armor;
        creators["summon voidwalker"] = &summon_voidwalker;
        creators["summon felguard"] = &summon_felguard;
        creators["summon succubus"] = &summon_succubus;
        creators["summon felhunter"] = &summon_felhunter;
    }

    // Pet skills are setup in pass-through fashion, so if one fails, it attempts to cast the next one
    // The order goes Felguard -> Felhunter -> Succubus -> Voidwalker -> Imp
    // Pets are summoned based on the non-combat strategy you have active, the warlock's level, and if they have a soul shard available

private:
    static ActionNode* fel_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastDemonArmorAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* demon_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastDemonSkinAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* summon_voidwalker([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSummonImpAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* summon_succubus([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSummonVoidwalkerAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* summon_felhunter([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSummonSuccubusAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* summon_felguard([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastSummonFelhunterAction>(1.0f) },
            /*C*/ {}
        );
    }
};

GenericWarlockNonCombatStrategy::GenericWarlockNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarlockNonCombatStrategyActionNodeFactory());
}

void GenericWarlockNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);
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
            "no pet",
            {
                CreateNextAction<CastFelDominationAction>(30.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no soul shard",
            {
                CreateNextAction<CreateSoulShardAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "too many soul shards",
            {
                CreateNextAction<DestroySoulShardAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "soul link",
            {
                CreateNextAction<CastSoulLinkAction>(28.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "demon armor",
            {
                CreateNextAction<CastFelArmorAction>(27.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unending breath",
            {
                CreateNextAction<CastUnendingBreathAction>(12.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "unending breath on party",
            {
                CreateNextAction<CastUnendingBreathOnPartyAction>(11.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no healthstone",
            {
                CreateNextAction<CastCreateHealthstoneAction>(26.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "no soulstone",
            {
                CreateNextAction<CastCreateSoulstoneAction>(25.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "life tap",
            {
                CreateNextAction<CastLifeTapAction>(23.0f)
            }
        )
    );
}

// Non-combat strategy for summoning a Imp
// Enabled by default for the Destruction spec
// To enable, type "nc +imp"
// To disable, type "nc -imp"
SummonImpStrategy::SummonImpStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonImpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastSummonImpAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wrong pet",
            {
                CreateNextAction<CastSummonImpAction>(29.0f)
            }
        )
    );
}

// Non-combat strategy for summoning a Voidwalker
// Disabled by default
// To enable, type "nc +voidwalker"
// To disable, type "nc -voidwalker"
SummonVoidwalkerStrategy::SummonVoidwalkerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonVoidwalkerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastSummonVoidwalkerAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wrong pet",
            {
                CreateNextAction<CastSummonVoidwalkerAction>(29.0f)
            }
        )
    );
}

// Non-combat strategy for summoning a Succubus
// Disabled by default
// To enable, type "nc +succubus"
// To disable, type "nc -succubus"
SummonSuccubusStrategy::SummonSuccubusStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonSuccubusStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastSummonSuccubusAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wrong pet",
            {
                CreateNextAction<CastSummonSuccubusAction>(29.0f)
            }
        )
    );
}

// Non-combat strategy for summoning a Felhunter
// Enabled by default for the Affliction spec
// To enable, type "nc +felhunter"
// To disable, type "nc -felhunter"
SummonFelhunterStrategy::SummonFelhunterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelhunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastSummonFelhunterAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wrong pet",
            {
                CreateNextAction<CastSummonFelhunterAction>(29.0f)
            }
        )
    );
}

// Non-combat strategy for summoning a Felguard
// Enabled by default for the Demonology spec
// To enable, type "nc +felguard"
// To disable, type "nc -felguard"
SummonFelguardStrategy::SummonFelguardStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SummonFelguardStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no pet",
            {
                CreateNextAction<CastSummonFelguardAction>(29.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "wrong pet",
            {
                CreateNextAction<CastSummonFelguardAction>(29.0f)
            }
        )
    );
}

// Non-combat strategy for selecting themselves to receive soulstone
// Disabled by default
// To enable, type "nc +ss self"
// To disable, type "nc -ss self"
SoulstoneSelfStrategy::SoulstoneSelfStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneSelfStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "soulstone",
            {
                CreateNextAction<UseSoulstoneSelfAction>(24.0f)
            }
        )
    );
}

// Non-combat strategy for selecting the master to receive soulstone
// Disabled by default
// To enable, type "nc +ss master"
// To disable, type "nc -ss master"
SoulstoneMasterStrategy::SoulstoneMasterStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneMasterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "soulstone",
            {
                CreateNextAction<UseSoulstoneMasterAction>(24.0f)
            }
        )
    );
}

// Non-combat strategy for selecting tanks to receive soulstone
// Disabled by default
// To enable, type "nc +ss tank"
// To disable, type "nc -ss tank"
SoulstoneTankStrategy::SoulstoneTankStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneTankStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "soulstone",
            {
                CreateNextAction<UseSoulstoneTankAction>(24.0f)
            }
        )
    );
}

// Non-combat strategy for selecting healers to receive soulstone
// Disabled by default
// To enable, type "nc +ss healer"
// To disable, type "nc -ss healer"
SoulstoneHealerStrategy::SoulstoneHealerStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void SoulstoneHealerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "soulstone",
            {
                CreateNextAction<UseSoulstoneHealerAction>(24.0f)
            }
        )
    );
}

// Non-combat strategy for using Spellstone
// Enabled by default for Affliction and Demonology specs
// To enable, type "nc +spellstone"
// To disable, type "nc -spellstone"
UseSpellstoneStrategy::UseSpellstoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void UseSpellstoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no spellstone",
            {
                CreateNextAction<CastCreateSpellstoneAction>(24.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "spellstone",
            {
                CreateNextAction<UseSpellstoneAction>(24.0f)
            }
        )
    );
}

// Non-combat strategy for using Firestone
// Enabled by default for the Destruction spec
// To enable, type "nc +firestone"
// To disable, type "nc -firestone"
UseFirestoneStrategy::UseFirestoneStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai) {}

void UseFirestoneStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "no firestone",
            {
                CreateNextAction<CastCreateFirestoneAction>(24.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "firestone",
            {
                CreateNextAction<UseFirestoneAction>(24.0f)
            }
        )
    );
}
