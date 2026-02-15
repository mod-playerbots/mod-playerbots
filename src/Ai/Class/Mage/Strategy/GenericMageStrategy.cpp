/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericMageStrategy.h"
#include "AiFactory.h"
#include "CancelChannelAction.h"
#include "CreateNextAction.h"
#include "GenericSpellActions.h"
#include "MageActions.h"
#include "RangedCombatStrategy.h"

class GenericMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageStrategyActionNodeFactory()
    {
        creators["frostbolt"] = &frostbolt;
        creators["frostfire bolt"] = &frostfire_bolt;
        creators["ice lance"] = &ice_lance;
        creators["fire blast"] = &fire_blast;
        creators["scorch"] = &scorch;
        creators["frost nova"] = &frost_nova;
        creators["cone of cold"] = &cone_of_cold;
        creators["icy veins"] = &icy_veins;
        creators["combustion"] = &combustion;
        creators["evocation"] = &evocation;
        creators["dragon's breath"] = &dragons_breath;
        creators["blast wave"] = &blast_wave;
        creators["remove curse"] = &remove_curse;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["fireball"] = &fireball;
    }

private:
    static ActionNode* frostbolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShootAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* frostfire_bolt([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastFireballAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* ice_lance([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* fire_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* scorch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShootAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* frost_nova([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* cone_of_cold([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* icy_veins([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* combustion([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* evocation([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<UseManaPotion>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* dragons_breath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* blast_wave([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ {},
            /*C*/ {}
        );
    }

    static ActionNode* remove_curse([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRemoveLesserCurseAction>(1.0f) },
            /*C*/ {}
        );
    }

    static ActionNode* remove_curse_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastRemoveLesserCurseOnPartyAction>(1.0f) },
            /*C*/ {}
        );
    }
    static ActionNode* fireball([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode(
            /*P*/ {},
            /*A*/ { CreateNextAction<CastShootAction>(1.0f) },
            /*C*/ {}
        );
    }
};

GenericMageStrategy::GenericMageStrategy(PlayerbotAI* botAI) : RangedCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageStrategyActionNodeFactory());
}

void GenericMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RangedCombatStrategy::InitTriggers(triggers);

    // Threat Triggers
    triggers.push_back(
        new TriggerNode(
            "high threat",
            {
                CreateNextAction<CastMirrorImageAction>(60.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "medium threat",
            {
                CreateNextAction<CastInvisibilityAction>(30.0f)
            }
        )
    );

    // Defensive Triggers
    triggers.push_back(
        new TriggerNode(
            "critical health",
            {
                CreateNextAction<CastIceBlockAction>(90.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low health",
            {
                CreateNextAction<CastManaShieldAction>(85.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "fire ward",
            {
                CreateNextAction<CastFireWardAction>(90.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "frost ward",
            {
                CreateNextAction<CastFrostWardAction>(90.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy is close and no firestarter strategy",
            {
                CreateNextAction<CastFrostNovaAction>(50.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy too close for spell and no firestarter strategy",
            {
                CreateNextAction<CastBlinkBackAction>(35.0f)
            }
        )
    );

    // Mana Threshold Triggers
    Player* bot = botAI->GetBot();
    if (bot->HasSpell(42985))  // Mana Sapphire
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaSapphireAction>(90.0f)
                }
            )
        );
    else if (bot->HasSpell(27101))  // Mana Emerald
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaEmeraldAction>(90.0f)
                }
            )
        );
    else if (bot->HasSpell(10054))  // Mana Ruby
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaRubyAction>(90.0f)
                }
            )
        );
    else if (bot->HasSpell(10053))  // Mana Citrine
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaCitrineAction>(90.0f)
                }
            )
        );
    else if (bot->HasSpell(3552))  // Mana Jade
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaJadeAction>(90.0f)
                }
            )
        );
    else if (bot->HasSpell(759))  // Mana Agate
        triggers.push_back(
            new TriggerNode(
                "high mana",
                {
                    CreateNextAction<UseManaAgateAction>(90.0f)
                }
            )
        );

    triggers.push_back(
        new TriggerNode(
            "medium mana", { CreateNextAction<UseManaPotion>(90.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "low mana",
            {
                CreateNextAction<CastEvocationAction>(90.0f)
            }
        )
    );

    // Counterspell / Spellsteal Triggers
    triggers.push_back(
        new TriggerNode(
            "spellsteal",
            {
                CreateNextAction<CastSpellstealAction>(40.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "counterspell on enemy healer",
            {
                CreateNextAction<CastCounterspellOnEnemyHealerAction>(40.0f)
            }
        )
    );
}

void MageCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "remove curse",
            {
                CreateNextAction<CastRemoveCurseAction>(41.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "remove curse on party",
            {
                CreateNextAction<CastRemoveCurseOnPartyAction>(40.0f)
            }
        )
    );
}

void MageBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    int tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == 0)  // Arcane
    {
        triggers.push_back(
            new TriggerNode(
                "arcane power",
                {
                    CreateNextAction<CastArcanePowerAction>(29.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "icy veins",
                {
                    CreateNextAction<CastIcyVeinsAction>(28.5f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "mirror image",
                {
                    CreateNextAction<CastMirrorImageAction>(28.0f)
                }
            )
        );
    }
    else if (tab == 1)
    {
        if (bot->HasSpell(44614) /*Frostfire Bolt*/ && bot->HasAura(15047) /*Ice Shards*/)
        { // Frostfire
            triggers.push_back(
                new TriggerNode(
                    "combustion",
                    {
                        CreateNextAction<CastCombustionAction>(18.0f)
                    }
                )
            );
            triggers.push_back(
                new TriggerNode(
                    "icy veins",
                    {
                        CreateNextAction<CastIcyVeinsAction>(17.5f)
                    }
                )
            );
            triggers.push_back(
                new TriggerNode(
                    "mirror image",
                    {
                        CreateNextAction<CastMirrorImageAction>(17.0f)
                    }
                )
            );
        }
        else
        { // Fire
            triggers.push_back(
                new TriggerNode(
                    "combustion",
                    {
                        CreateNextAction<CastCombustionAction>(18.0f)
                    }
                )
            );
            triggers.push_back(
                new TriggerNode(
                    "mirror image",
                    {
                        CreateNextAction<CastMirrorImageAction>(17.5f)
                    }
                )
            );
        }
    }
    else if (tab == 2)  // Frost
    {
        triggers.push_back(
            new TriggerNode(
                "cold snap",
                {
                    CreateNextAction<CastColdSnapAction>(28.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "icy veins",
                {
                    CreateNextAction<CastIcyVeinsAction>(27.5f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "mirror image",
                {
                    CreateNextAction<CastMirrorImageAction>(26.0f)
                }
            )
        );
    }
}

void MageCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "polymorph",
            {
                CreateNextAction<CastPolymorphAction>(30.0f)
            }
        )
    );
}

void MageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode(
            "blizzard channel check",
            {
                CreateNextAction<CancelChannelAction>(26.0f)
            }
        )
    );

    Player* bot = botAI->GetBot();
    const uint8_t tab = AiFactory::GetPlayerSpecTab(bot);

    // Arcane
    if (tab == MAGE_TAB_ARCANE)
    {
        triggers.push_back(
            new TriggerNode(
                "flamestrike active and medium aoe",
                {
                    CreateNextAction<CastBlizzardAction>(24.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "medium aoe",
                {
                    CreateNextAction<CastFlamestrikeAction>(23.0f),
                    CreateNextAction<CastBlizzardAction>(22.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "light aoe",
                {
                    CreateNextAction<CastArcaneExplosionAction>(21.0f)
                }
            )
        );
    }

    // Fire and Frostfire
    if (tab == MAGE_TAB_FIRE)
    {
        triggers.push_back(
            new TriggerNode(
                "medium aoe",
                {
                    CreateNextAction<CastDragonsBreathAction>(39.0f),
                    CreateNextAction<CastBlastWaveAction>(38.0f),
                    CreateNextAction<CastFlamestrikeAction>(23.0f),
                    CreateNextAction<CastBlizzardAction>(22.0f)
                }
            )
        );

        triggers.push_back(
            new TriggerNode(
                "flamestrike active and medium aoe",
                {
                    CreateNextAction<CastBlizzardAction>(24.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "firestarter",
                {
                    CreateNextAction<CastFlamestrikeAction>(40.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "living bomb on attackers",
                {
                    CreateNextAction<CastLivingBombOnAttackersAction>(21.0f)
                }
            )
        );
    }

    // Frost
    if (tab == MAGE_TAB_FROST)
    {
        triggers.push_back(
            new TriggerNode(
                "flamestrike active and medium aoe",
                {
                    CreateNextAction<CastBlizzardAction>(24.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "medium aoe",
                {
                    CreateNextAction<CastFlamestrikeAction>(23.0f),
                    CreateNextAction<CastBlizzardAction>(22.0f)
                }
            )
        );
        triggers.push_back(
            new TriggerNode(
                "light aoe",
                {
                    CreateNextAction<CastConeOfColdAction>(21.0f)
                }
            )
        );
    }
}
