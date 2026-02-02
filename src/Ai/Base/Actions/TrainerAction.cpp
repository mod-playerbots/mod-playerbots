/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TrainerAction.h"

#include "BudgetValues.h"
#include "Event.h"
#include "PlayerbotFactory.h"
#include "Playerbots.h"

void TrainerAction::Learn(uint32 cost, const Trainer::Spell tSpell, std::ostringstream& msg)
{
    if (sPlayerbotAIConfig.autoTrainSpells != "free" && !botAI->HasCheat(BotCheatMask::gold))
    {
        if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::spells) < cost)
        {
            msg << " - too expensive";
            return;
        }

        bot->ModifyMoney(-int32(cost));
    }

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(tSpell.SpellId);
    if (!spellInfo)
        return;

    bool learned = false;
    for (uint8 j = 0; j < 3; ++j)
    {
        if (spellInfo->Effects[j].Effect == SPELL_EFFECT_LEARN_SPELL)
        {
            uint32 learnedSpell = spellInfo->Effects[j].TriggerSpell;
            if (!bot->HasSpell(learnedSpell))
            {
                bot->learnSpell(learnedSpell);
                learned = true;
            }
        }
    }

    if (!learned && !bot->HasSpell(tSpell.SpellId))
        bot->learnSpell(tSpell.SpellId);

    msg << " - learned";
}

void TrainerAction::Iterate(Creature* creature, TrainerSpellAction action, SpellIds& spells)
{
    TellHeader(creature);

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(creature->GetEntry());

    if (!trainer)
        return;

    float fDiscountMod = bot->GetReputationPriceDiscount(creature);
    uint32 totalCost = 0;

    for (auto& spell : trainer->GetSpells())
    {
        if (!trainer->CanTeachSpell(bot, trainer->GetSpell(spell.SpellId)))
            continue;

        if (!spells.empty() && spells.find(spell.SpellId) == spells.end())
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell.SpellId);

        if (!spellInfo)
            continue;

        uint32 cost = uint32(floor(spell.MoneyCost * fDiscountMod));
        totalCost += cost;

        std::ostringstream out;
        out << chat->FormatSpell(spellInfo) << chat->formatMoney(cost);

        if (action)
            (this->*action)(cost, spell, out);

        botAI->TellMaster(out);
    }

    TellFooter(totalCost);
}

bool TrainerAction::Execute(Event event)
{
    std::string const text = event.getParam();

    Player* master = GetMaster();

    Creature* creature = botAI->GetCreature(bot->GetTarget());

    if (master)
    {
        creature = master->GetSelectedUnit() ? master->GetSelectedUnit()->ToCreature() : nullptr;
    }
    // if (AI_VALUE(GuidPosition, "rpg target") != bot->GetTarget())
    //     if (master)
    //         creature = botAI->GetCreature(master->GetTarget());
    //     else
    //         return false;

    if (!creature || !creature->IsTrainer())
        return false;

    Trainer::Trainer* trainer = sObjectMgr->GetTrainer(creature->GetEntry());

    if (!trainer || !trainer->IsTrainerValidForPlayer(bot))
        return false;

    std::vector<Trainer::Spell> trainer_spells = trainer->GetSpells();

    if (trainer_spells.empty())
    {
        botAI->TellError("No spells can be learned from this trainer");
        return false;
    }

    uint32 spell = chat->parseSpell(text);
    SpellIds spells;
    if (spell)
        spells.insert(spell);

    if (text.find("learn") != std::string::npos || sRandomPlayerbotMgr.IsRandomBot(bot) ||
        (sPlayerbotAIConfig.autoTrainSpells != "no" &&
         (trainer->GetTrainerType() != Trainer::Type::Tradeskill ||
          !botAI->HasActivePlayerMaster())))  // Todo rewrite to only exclude start primary profession skills and make
                                              // config dependent.
        Iterate(creature, &TrainerAction::Learn, spells);
    else
        Iterate(creature, nullptr, spells);

    return true;
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

        if (sPlayerbotAIConfig.altMaintenanceGemsEnchants && bot->GetLevel() >= sPlayerbotAIConfig.minEnchantingBotLevel)
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
