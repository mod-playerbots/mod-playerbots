/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TrainerAction.h"

#include "AiObjectContext.h"
#include "BudgetValues.h"
#include "RandomPlayerbotMgr.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "SpellMgr.h"
#include "Trainer.h"

bool TrainerAction::Execute(Event event)
{
    const std::string param = event.getParam();

    Creature* const target = this->getCreatureTarget();

    if (target == nullptr)
    {
        return false;
    }

    ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return false;
    }

    const Trainer::Trainer* const trainer = objectMgr->GetTrainer(target->GetEntry());

    if (trainer == nullptr)
    {
        return false;
    }

    // NOTE: Original version uses SpellIds here, but occasionally only inserts
    // a single spell ID value from parameters. If someone wants to impl multiple
    // spells as parameters, check SkipSpellsListAction::parseIds as an example.
    const uint32_t spellId = this->chat->parseSpell(param);

    // @TODO: Move to a dedicated method instead of this boolean hell.
    const bool wasAskedToLearn = param.find("learn") != std::string::npos;
    const bool isRandomBot = RandomPlayerbotMgr::instance().IsRandomBot(bot);
    const bool isTradeSkillTrainer = trainer->GetTrainerType() == Trainer::Type::Tradeskill;
    const bool hasMaster = this->botAI->HasActivePlayerMaster();
    const bool allowLearnTrainerSpells = PlayerbotAIConfig::instance().allowLearnTrainerSpells;
    // TODO: Rewrite to only exclude start primary profession skills and make config dependent.
    const bool isAllowedToAutomaticallyLearnSpells = allowLearnTrainerSpells && (!isTradeSkillTrainer || !hasMaster);

    const bool learnSpells = wasAskedToLearn || isRandomBot || isAllowedToAutomaticallyLearnSpells;

    this->iterate(target, learnSpells, spellId);

    return true;
}

bool TrainerAction::isUseful()
{
    const Creature* const target = this->getCreatureTarget();

    if (target == nullptr)
    {
        return false;
    }

    if (!target->IsInWorld() || target->IsDuringRemoveFromWorld() || !target->IsAlive())
    {
        return false;
    }

    return target->IsTrainer();
}

bool TrainerAction::isPossible()
{
    const Creature* const target = this->getCreatureTarget();

    if (target == nullptr)
    {
        return false;
    }

    ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return false;
    }

    const Trainer::Trainer* const trainer = objectMgr->GetTrainer(target->GetEntry());

    if (trainer == nullptr)
    {
        return false;
    }

    if (!trainer->IsTrainerValidForPlayer(this->bot))
    {
        return false;
    }

    if (trainer->GetSpells().empty())
    {
        return false;
    }

    return true;
}

// There are just two scenarios: the bot has a master or it doesn't. If the
// bot has a master, the master should target a unit; otherwise, the bot
// should target the unit itself.
Unit* TrainerAction::GetTarget()
{
    const Player* const master = this->GetMaster();

    if (master == nullptr)
    {
        return this->bot->GetSelectedUnit();
    }

    return master->GetSelectedUnit();
}

Creature* TrainerAction::getCreatureTarget() noexcept
{
    Unit* const target = this->GetTarget();

    if (target == nullptr)
    {
        return nullptr;
    }

    return dynamic_cast<Creature*>(target);
}

void TrainerAction::iterate(const Creature* const creature, const bool learnSpells, const uint32_t spellId)
{
    ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return;
    }

    Trainer::Trainer* const trainer = objectMgr->GetTrainer(creature->GetEntry());

    if (trainer == nullptr)
    {
        return;
    }

    this->tellHeader(creature);

    const float reputationDiscount = this->bot->GetReputationPriceDiscount(creature);

    uint32_t totalCost = 0;

    const SpellMgr* const spellMgr = SpellMgr::instance();

    if (spellMgr == nullptr)
    {
        return;
    }

    // simplified version of Trainer::TeachSpell method
    for (const Trainer::Spell& spell : trainer->GetSpells())
    {
        if (!trainer->CanTeachSpell(this->bot, &spell))
        {
            continue;
        }

        if (spellId && spell.SpellId != spellId)
        {
            continue;
        }

        const SpellInfo* const spellInfo = spellMgr->GetSpellInfo(spell.SpellId);

        if (spellInfo == nullptr)
        {
            continue;
        }

        const uint32_t cost = uint32_t(floor(spell.MoneyCost * reputationDiscount));
        totalCost += cost;

        std::ostringstream out{};
        out << chat->FormatSpell(spellInfo) << chat->formatMoney(cost);

        if (learnSpells)
        {
            out << this->learn(*spellInfo, cost);
        }

        this->botAI->TellMaster(out);
    }

    this->tellFooter(totalCost);
}

const std::string TrainerAction::learn(const SpellInfo& spellInfo, const uint32_t cost)
{
    if (!this->botAI->HasCheat(BotCheatMask::gold))
    {
        Value<uint32_t>* const freeMoneyFor = this->context->GetValue<uint32_t>("free money for", uint32_t(NeedMoneyFor::spells));

        if (freeMoneyFor == nullptr)
        {
           return " - cannot determine if I can afford it";
        }

        const uint32_t freeMoneyForSpells = freeMoneyFor->Get();

        if (freeMoneyForSpells < cost)
        {
            return " - too expensive";
        }

        this->bot->ModifyMoney(-uint32_t(cost));
    }

    if (spellInfo.HasEffect(SPELL_EFFECT_LEARN_SPELL))
    {
        this->bot->CastSpell(bot, spellInfo.Id, true);

        return " - learned";
    }

    this->bot->learnSpell(spellInfo.Id, false);

    return " - learned";
}

void TrainerAction::tellHeader(const Creature* const creature) const
{
    std::ostringstream out{};

    out << "--- Can learn from " << creature->GetName() << " ---";

    this->botAI->TellMaster(out);
}

void TrainerAction::tellFooter(const uint32_t totalCost)
{
    if (totalCost == 0)
    {
        return;
    }

    std::ostringstream out{};

    out << "Total cost: " << this->chat->formatMoney(totalCost);

    this->botAI->TellMaster(out);
}

void MaintenanceAction::performAltMaintenance()
{
    const PlayerbotAIConfig& configuration = PlayerbotAIConfig::instance();
    PlayerbotFactory factory{this->bot, this->bot->GetLevel()};

    if (configuration.altMaintenanceAttunementQs)
    {
        factory.InitAttunementQuests();
    }

    if (configuration.altMaintenanceBags)
    {
        factory.InitBags(false);
    }

    if (configuration.altMaintenanceAmmo)
    {
        factory.InitAmmo();
    }

    if (configuration.altMaintenanceFood)
    {
        factory.InitFood();
    }

    if (configuration.altMaintenanceReagents)
    {
        factory.InitReagents();
    }

    if (configuration.altMaintenanceConsumables)
    {
        factory.InitConsumables();
    }

    if (configuration.altMaintenancePotions)
    {
        factory.InitPotions();
    }

    if (configuration.altMaintenanceTalentTree)
    {
        factory.InitTalentsTree(true);
    }

    if (configuration.altMaintenancePet)
    {
        factory.InitPet();
    }

    if (configuration.altMaintenancePetTalents)
    {
        factory.InitPetTalents();
    }

    if (configuration.altMaintenanceSkills)
    {
        factory.InitSkills();
    }

    if (configuration.altMaintenanceClassSpells)
    {
        factory.InitClassSpells();
    }

    if (configuration.altMaintenanceAvailableSpells)
    {
        factory.InitAvailableSpells();
    }

    if (configuration.altMaintenanceReputation)
    {
        factory.InitReputation();
    }

    if (configuration.altMaintenanceSpecialSpells)
    {
        factory.InitSpecialSpells();
    }

    if (configuration.altMaintenanceMounts)
    {
        factory.InitMounts();
    }

    if (configuration.altMaintenanceGlyphs)
    {
        factory.InitGlyphs(false);
    }

    if (configuration.altMaintenanceKeyring)
    {
        factory.InitKeyring();
    }

    if (configuration.altMaintenanceGemsEnchants && this->bot->GetLevel() >= configuration.minEnchantingBotLevel)
    {
        factory.ApplyEnchantAndGemsNew();
    }

    this->bot->DurabilityRepairAll(false, 1.0f, false);
    this->bot->SendTalentsInfoData(false);
}

void MaintenanceAction::performRandomBotMaintenance()
{
    const PlayerbotAIConfig& configuration = PlayerbotAIConfig::instance();
    PlayerbotFactory factory{this->bot, this->bot->GetLevel()};

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

    if (bot->GetLevel() >= configuration.minEnchantingBotLevel)
    {
        factory.ApplyEnchantAndGemsNew();
    }

    this->bot->DurabilityRepairAll(false, 1.0f, false);
    this->bot->SendTalentsInfoData(false);
}

bool MaintenanceAction::Execute(Event)
{
    const PlayerbotAIConfig& configuration = PlayerbotAIConfig::instance();

    if (!configuration.maintenanceCommand)
    {
        this->botAI->TellError("maintenance command is not allowed, please check the configuration.");

        return false;
    }

    this->botAI->TellMaster("I'm maintaining");

    PlayerbotFactory factory{this->bot, this->bot->GetLevel()};

    if (this->botAI->IsAlt())
    {
        this->performAltMaintenance();

        return true;
    }

    this->performRandomBotMaintenance();

    return true;
}

bool RemoveGlyphAction::Execute(Event)
{
    for (uint32_t slotIndex = 0; slotIndex < MAX_GLYPH_SLOT_INDEX; ++slotIndex)
    {
        this->bot->SetGlyph(slotIndex, 0, true);
    }

    this->bot->SendTalentsInfoData(false);

    return true;
}

bool AutoGearAction::Execute(Event)
{
    PlayerbotAIConfig& configuration = PlayerbotAIConfig::instance();

    if (!configuration.autoGearCommand)
    {
        this->botAI->TellError("autogear command is not allowed, please check the configuration.");

        return false;
    }

    if (!configuration.autoGearCommandAltBots &&
        !configuration.IsInRandomAccountList(this->bot->GetSession()->GetAccountId()))
    {
        this->botAI->TellError("You cannot use autogear on alt bots.");
        return false;
    }

    this->botAI->TellMaster("I'm auto gearing");

    uint32_t gearscore = 0;

    if (configuration.autoGearScoreLimit > 0)
    {
        gearscore = PlayerbotFactory::CalcMixedGearScore(configuration.autoGearScoreLimit, configuration.autoGearQualityLimit);
    }

    PlayerbotFactory factory{this->bot, this->bot->GetLevel(), configuration.autoGearQualityLimit, gearscore};

    factory.InitEquipment(true);
    factory.InitAmmo();

    if (this->bot->GetLevel() >= configuration.minEnchantingBotLevel)
    {
        factory.ApplyEnchantAndGemsNew();
    }

    this->bot->DurabilityRepairAll(false, 1.0f, false);

    return true;
}
