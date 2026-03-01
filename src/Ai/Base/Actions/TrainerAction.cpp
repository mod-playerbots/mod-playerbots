/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TrainerAction.h"

#include "BudgetValues.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"
#include "Trainer.h"

bool TrainerAction::Execute(Event event)
{
    std::string const param = event.getParam();

    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(target->GetEntry());
    if (!trainer)
        return false;

    // NOTE: Original version uses SpellIds here, but occasionally only inserts
    // a single spell ID value from parameters. If someone wants to impl multiple
    // spells as parameters, check SkipSpellsListAction::parseIds as an example.
    uint32 spellId = chat->parseSpell(param);

    bool learnSpells = param.find("learn") != std::string::npos || sRandomPlayerbotMgr.IsRandomBot(bot) ||
                       (sPlayerbotAIConfig.allowLearnTrainerSpells &&
                        // TODO: Rewrite to only exclude start primary profession skills and make config dependent.
                        (trainer->GetTrainerType() != Trainer::Type::Tradeskill || !botAI->HasActivePlayerMaster()));

    Iterate(target, learnSpells, spellId);

    return true;
}

bool TrainerAction::isUseful()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld() || !target->IsAlive())
        return false;

    return target->IsTrainer();
}

bool TrainerAction::isPossible()
{
    Creature* target = GetCreatureTarget();
    if (!target)
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(target->GetEntry());
    if (!trainer)
        return false;

    if (!trainer->IsTrainerValidForPlayer(bot))
        return false;

    if (trainer->GetSpells().empty())
        return false;

    return true;
}

Unit* TrainerAction::GetTarget()
{
    // There are just two scenarios: the bot has a master or it doesn't. If the
    // bot has a master, the master should target a unit; otherwise, the bot
    // should target the unit itself.
    if (Player* master = GetMaster())
        return master->GetSelectedUnit();

    return bot->GetSelectedUnit();
}

Creature* TrainerAction::GetCreatureTarget()
{
    Unit* target = GetTarget();
    return target ? target->ToCreature() : nullptr;
}

void TrainerAction::Iterate(Creature* creature, bool learnSpells, uint32 spellId)
{
    TellHeader(creature);

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(creature->GetEntry());
    if (!trainer)
        return;

    float reputationDiscount = bot->GetReputationPriceDiscount(creature);
    uint32 totalCost = 0;

    for (auto& spell : trainer->GetSpells())
    {
        // simplified version of Trainer::TeachSpell method

        Trainer::Spell const* trainerSpell = trainer->GetSpell(spell.SpellId);
        if (!trainerSpell)
            continue;

        if (!trainer->CanTeachSpell(bot, trainerSpell))
            continue;

        if (spellId && trainerSpell->SpellId != spellId)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(trainerSpell->SpellId);
        if (!spellInfo)
            continue;

        uint32 cost = static_cast<uint32>(floor(trainerSpell->MoneyCost * reputationDiscount));
        totalCost += cost;

        std::ostringstream out;
        out << chat->FormatSpell(spellInfo) << chat->formatMoney(cost);

        if (learnSpells)
            Learn(spellInfo, cost, out);

        botAI->TellMaster(out);
    }

    TellFooter(totalCost);
}

void TrainerAction::Learn(SpellInfo const* spellInfo, uint32 cost, std::ostringstream& out)
{
    if (!botAI->HasCheat(BotCheatMask::gold))
    {
        if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) < cost)
        {
            out << " - too expensive";
            return;
        }

        bot->ModifyMoney(-static_cast<int32>(cost));
    }

    if (spellInfo->HasEffect(SPELL_EFFECT_LEARN_SPELL))
        bot->CastSpell(bot, spellInfo->Id, true);
    else
        bot->learnSpell(spellInfo->Id, false);

    out << " - learned";
}

void TrainerAction::TellHeader(Creature* creature)
{
    std::ostringstream out;
    out << "--- Can learn from " << creature->GetName() << " ---";
    botAI->TellMaster(out);
}

void TrainerAction::TellFooter(uint32 totalCost)
{
    if (totalCost)
    {
        std::ostringstream out;
        out << "Total cost: " << chat->formatMoney(totalCost);
        botAI->TellMaster(out);
    }
}

bool MaintenanceAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig.maintenanceCommand)
    {
        botAI->TellError("maintenance command is not allowed, please check the configuration.");
        return false;
    }

    botAI->TellMaster("I'm maintaining");
    PlayerbotFactory factory(bot, bot->GetLevel());

    if (!botAI->IsAlt())
    {
        factory.InitAttunementQuests();
        factory.InitBags(false);
        factory.InitAmmo();
        factory.InitFood();
        factory.InitReagents();
        factory.InitConsumables();
        factory.InitPotions();
        factory.InitTalentsTree(true);
        factory.InitPet();
        factory.InitPetTalents();
        factory.InitSkills();
        factory.InitClassSpells();
        factory.InitAvailableSpells();
        factory.InitReputation();
        factory.InitSpecialSpells();
        factory.InitMounts();
        factory.InitGlyphs(false);
        factory.InitKeyring();
        if (bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();
    }
    else
    {
        if (sPlayerbotAIConfig.altMaintenanceAttunementQs)
            factory.InitAttunementQuests();

        if (sPlayerbotAIConfig.altMaintenanceBags)
            factory.InitBags(false);

        if (sPlayerbotAIConfig.altMaintenanceAmmo)
            factory.InitAmmo();

        if (sPlayerbotAIConfig.altMaintenanceFood)
            factory.InitFood();

        if (sPlayerbotAIConfig.altMaintenanceReagents)
            factory.InitReagents();

        if (sPlayerbotAIConfig.altMaintenanceConsumables)
            factory.InitConsumables();

        if (sPlayerbotAIConfig.altMaintenancePotions)
            factory.InitPotions();

        if (sPlayerbotAIConfig.altMaintenanceTalentTree)
            factory.InitTalentsTree(true);

        if (sPlayerbotAIConfig.altMaintenancePet)
            factory.InitPet();

        if (sPlayerbotAIConfig.altMaintenancePetTalents)
            factory.InitPetTalents();

        if (sPlayerbotAIConfig.altMaintenanceSkills)
            factory.InitSkills();

        if (sPlayerbotAIConfig.altMaintenanceClassSpells)
            factory.InitClassSpells();

        if (sPlayerbotAIConfig.altMaintenanceAvailableSpells)
            factory.InitAvailableSpells();

        if (sPlayerbotAIConfig.altMaintenanceReputation)
            factory.InitReputation();

        if (sPlayerbotAIConfig.altMaintenanceSpecialSpells)
            factory.InitSpecialSpells();

        if (sPlayerbotAIConfig.altMaintenanceMounts)
            factory.InitMounts();

        if (sPlayerbotAIConfig.altMaintenanceGlyphs)
            factory.InitGlyphs(false);

        if (sPlayerbotAIConfig.altMaintenanceKeyring)
            factory.InitKeyring();

        if (sPlayerbotAIConfig.altMaintenanceGemsEnchants &&
            bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
            factory.ApplyEnchantAndGemsNew();
    }

    bot->DurabilityRepairAll(false, 1.0f, false);
    bot->SendTalentsInfoData(false);

    return true;
}

bool RemoveGlyphAction::Execute(Event /*event*/)
{
    for (uint32 slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
    {
        bot->SetGlyph(slotIndex, 0, true);
    }
    bot->SendTalentsInfoData(false);
    return true;
}

bool AutoGearAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig.autoGearCommand)
    {
        botAI->TellError("autogear command is not allowed, please check the configuration.");
        return false;
    }

    if (!sPlayerbotAIConfig.autoGearCommandAltBots &&
        !sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
    {
        botAI->TellError("You cannot use autogear on alt bots.");
        return false;
    }

    botAI->TellMaster("I'm auto gearing");
    uint32 gs = sPlayerbotAIConfig.autoGearScoreLimit == 0
                    ? 0
                    : PlayerbotFactory::CalcMixedGearScore(sPlayerbotAIConfig.autoGearScoreLimit,
                                                           sPlayerbotAIConfig.autoGearQualityLimit);
    PlayerbotFactory factory(bot, bot->GetLevel(), sPlayerbotAIConfig.autoGearQualityLimit, gs);
    factory.InitEquipment(true);
    factory.InitAmmo();
    if (bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
    {
        factory.ApplyEnchantAndGemsNew();
    }
    bot->DurabilityRepairAll(false, 1.0f, false);
    return true;
}
