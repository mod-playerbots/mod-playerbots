/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RpgSubActions.h"

#include "AcceptQuestAction.h"
#include "BattleGroundJoinAction.h"
#include "BuyAction.h"
#include "CastCustomSpellAction.h"
#include "BudgetValues.h"
#include "ChooseRpgTargetAction.h"
#include "DruidActions.h"
#include "EmoteAction.h"
#include "Formations.h"
#include "GossipDef.h"
#include "GuildCreateActions.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "PriestActions.h"
#include "RepairAllAction.h"
#include "SellAction.h"
#include "SetHomeAction.h"
#include "ShamanActions.h"
#include "SocialMgr.h"
#include "TalkToQuestGiverAction.h"
#include "TradeAction.h"
#include "TrainerAction.h"

void RpgHelper::onExecute(std::string nextAction)
{
    if (botAI->HasRealPlayerMaster() && nextAction == "rpg")
        nextAction = "rpg cancel";

    SET_AI_VALUE(std::string, "next rpg action", nextAction);
}

void RpgHelper::BeforeExecute()
{
    onExecute();

    bot->SetTarget(guidP());

    setFacingTo(guidP());
}

void RpgHelper::afterExecute(const bool doDelay, const bool waitForGroup)
{
    this->onExecute();

    this->bot->SetTarget(guidP());

    this->setFacingTo(this->guidP());

    if (doDelay)
    {
        this->setDelay(waitForGroup);
    }

    this->setFacing(this->guidP());
}

GuidPosition RpgHelper::guidP()
{
    Value<GuidPosition>* value = this->context->GetValue<GuidPosition>("rpg target");

    if (value == nullptr)
    {
        return GuidPosition{};
    }

    return value->Get();
}

ObjectGuid RpgHelper::guid()
{
    return (ObjectGuid)this->guidP();
}

bool RpgHelper::InRange()
{
    GuidPosition gp = guidP();
    if (!gp)
        return false;

    return gp.sqDistance2d(bot) < INTERACTION_DISTANCE * INTERACTION_DISTANCE;
}

void RpgHelper::setFacingTo(GuidPosition guidPosition)
{
    bot->SetFacingTo(guidPosition.getAngleTo(bot) + static_cast<float>(M_PI));
}

void RpgHelper::setFacing(GuidPosition guidPosition)
{
    if (!guidPosition.IsUnit())
        return;

    if (guidPosition.IsPlayer())
        return;

    //    Unit* unit = guidPosition.GetUnit();

    //    unit->SetFacingTo(unit->GetAngle(bot));
}

void RpgHelper::setDelay(bool waitForGroup)
{
    if (!botAI->HasRealPlayerMaster() || (waitForGroup && botAI->GetGroupLeader() == bot && bot->GetGroup()))
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay);
    else
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay / 5);
}

bool RpgSubAction::isPossible()
{
    return this->rpg->guidP() && this->rpg->guidP().GetWorldObject();
}

bool RpgSubAction::isUseful()
{
    return this->rpg->InRange();
}

bool RpgSubAction::Execute(Event event)
{
    bool doAction = botAI->DoSpecificAction(this->getActionFactory(), this->ActionEvent(event), true);
    this->rpg->afterExecute(doAction, true);
    return doAction;
}

// @TODO: This is a simple fallback. It should never be triggered.
NextAction::Factory RpgSubAction::getActionFactory() const
{
    LOG_ERROR("playerbots.rpg.rpgsubaction", "Bot {} - No action factory defined for RpgSubAction", this->bot->GetName());

    return CreateNextAction<EmoteAction>(1.0f).factory;
}

Event RpgSubAction::ActionEvent(Event event)
{
    return event;
}

bool RpgStayAction::isUseful()
{
    return this->rpg->InRange() && !this->botAI->HasRealPlayerMaster();
}

bool RpgStayAction::Execute(Event)
{
    this->bot->PlayerTalkClass->SendCloseGossip();

    this->rpg->afterExecute();

    return true;
}

bool RpgWorkAction::isUseful()
{
    return this->rpg->InRange() && !this->botAI->HasRealPlayerMaster();
}

bool RpgWorkAction::Execute(Event)
{
    this->bot->HandleEmoteCommand(EMOTE_STATE_USE_STANDING);
    this->rpg->afterExecute();
    return true;
}

bool RpgEmoteAction::isUseful()
{
    return this->rpg->InRange() && !this->botAI->HasRealPlayerMaster();
}

bool RpgEmoteAction::Execute(Event)
{
    uint32 type = TalkAction::GetRandomEmote(this->rpg->guidP().GetUnit());

    WorldPacket p1;
    p1 << this->rpg->guid();

    this->bot->GetSession()->HandleGossipHelloOpcode(p1);
    this->bot->HandleEmoteCommand(type);
    this->rpg->afterExecute();

    return true;
}

bool RpgCancelAction::Execute(Event)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");
    this->rpg->onExecute("");
    return true;
}

bool RpgTaxiAction::isUseful()
{
    return this->rpg->InRange() && !this->botAI->HasRealPlayerMaster();
}

bool RpgTaxiAction::Execute(Event)
{
    GuidPosition guidP = this->rpg->guidP();

    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeamId());

    std::vector<uint32> nodes;
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i);
        if (entry && entry->from == node && (this->bot->m_taxi.IsTaximaskNodeKnown(entry->to) || this->bot->isTaxiCheater()))
        {
            nodes.push_back(i);
        }
    }

    if (nodes.empty())
    {
        LOG_ERROR("playerbots", "Bot {} - No flight paths available", bot->GetName());
        return false;
    }

    uint32 path = nodes[urand(0, nodes.size() - 1)];
    uint32 money = this->bot->GetMoney();
    this->bot->SetMoney(money + 100000);

    TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(path);
    if (!entry)
        return false;

    TaxiNodesEntry const* nodeFrom = sTaxiNodesStore.LookupEntry(entry->from);
    TaxiNodesEntry const* nodeTo = sTaxiNodesStore.LookupEntry(entry->to);

    Creature* flightMaster = bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
    {
        LOG_ERROR("playerbots", "Bot {} cannot talk to flightmaster ({} location available)", this->bot->GetName(),
                  nodes.size());
        return false;
    }

    if (!this->bot->ActivateTaxiPathTo({entry->from, entry->to}, flightMaster, 0))
    {
        LOG_ERROR("playerbots", "Bot {} cannot fly {} ({} location available)", this->bot->GetName(), path, nodes.size());
        return false;
    }

    LOG_INFO("playerbots", "Bot {} <{}> is flying from {} to {} ({} location available)",
             this->bot->GetGUID().ToString().c_str(), this->bot->GetName(), nodeFrom->name[0], nodeTo->name[0], nodes.size());

    this->bot->SetMoney(money);

    this->rpg->afterExecute();

    return true;
}

bool RpgDiscoverAction::Execute(Event)
{
    GuidPosition guidP = this->rpg->guidP();

    uint32 node =
        sObjectMgr->GetNearestTaxiNode(guidP.getX(), guidP.getY(), guidP.getZ(), guidP.getMapId(), bot->GetTeamId());

    if (!node)
        return false;

    Creature* flightMaster = this->bot->GetNPCIfCanInteractWith(guidP, UNIT_NPC_FLAG_FLIGHTMASTER);
    if (!flightMaster)
        return false;

    return this->bot->GetSession()->SendLearnNewTaxiNode(flightMaster);
}

NextAction::Factory RpgStartQuestAction::getActionFactory() const
{
    return CreateNextAction<AcceptAllQuestsAction>(1.0f).factory;
}

Event RpgStartQuestAction::ActionEvent(Event)
{
    WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
    p << this->rpg->guid();
    p.rpos(0);

    return Event("rpg action", p);
}

NextAction::Factory RpgEndQuestAction::getActionFactory() const
{
    return CreateNextAction<TalkToQuestGiverAction>(1.0f).factory;
}

Event RpgEndQuestAction::ActionEvent(Event)
{
    WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST);
    p << this->rpg->guid();
    p.rpos(0);

    return Event("rpg action", p);
}

NextAction::Factory RpgBuyAction::getActionFactory() const
{
    return CreateNextAction<BuyAction>(1.0f).factory;
}

Event RpgBuyAction::ActionEvent(Event)
{
    return Event("rpg action", "vendor");
}

NextAction::Factory RpgSellAction::getActionFactory() const
{
    return CreateNextAction<SellAction>(1.0f).factory;
}

Event RpgSellAction::ActionEvent(Event)
{
    return Event("rpg action", "vendor");
}

NextAction::Factory RpgRepairAction::getActionFactory() const
{
    return CreateNextAction<RepairAllAction>(1.0f).factory;
}

bool RpgTrainAction::isUseful()
{
    if (!this->rpg->InRange())
    {
        return false;
    }

    const Creature* const creature = this->rpg->guidP().GetCreature();

    if (creature == nullptr)
    {
        return false;
    }

    if (!creature->IsInWorld() || creature->IsDuringRemoveFromWorld() || !creature->IsAlive())
    {
        return false;
    }

    return true;
}

bool RpgTrainAction::isPossible()
{
    const GuidPosition gp = this->rpg->guidP();

    const CreatureTemplate* const cinfo = gp.GetCreatureTemplate();

    if (cinfo == nullptr)
    {
        return false;
    }

    ObjectMgr* const objectMgr = ObjectMgr::instance();

    if (objectMgr == nullptr)
    {
        return false;
    }

    Trainer::Trainer* const trainer = objectMgr->GetTrainer(cinfo->Entry);

    if (trainer == nullptr)
    {
        return false;
    }

    if (!trainer->IsTrainerValidForPlayer(this->bot))
    {
        return false;
    }

    const FactionTemplateEntry* const factionTemplate = sFactionTemplateStore.LookupEntry(cinfo->faction);
    const float reputationDiscount = this->bot->GetReputationPriceDiscount(factionTemplate);

    Value<uint32_t>* const freeMoneyFor = this->context->GetValue<uint32_t>("free money for", uint32_t(NeedMoneyFor::spells));

    if (freeMoneyFor == nullptr)
    {
        return false;
    }

    const uint32_t currentGold = freeMoneyFor->Get();

    for (const Trainer::Spell& spell : trainer->GetSpells())
    {
        const Trainer::Spell* const trainerSpell = trainer->GetSpell(spell.SpellId);

        if (trainerSpell == nullptr)
        {
            continue;
        }

        if (!trainer->CanTeachSpell(bot, trainerSpell))
        {
            continue;
        }

        const uint32_t realCost = uint32_t(floor(trainerSpell->MoneyCost * reputationDiscount));

        if (currentGold < realCost)
        {
            continue;
        }

        // we only check if at least one spell can be learned from the trainer;
        // otherwise, the train action should not be allowed
        return true;
    }

    return false;
}

NextAction::Factory RpgTrainAction::getActionFactory() const
{
    return CreateNextAction<TrainerAction>(1.0f).factory;
}

bool RpgHealAction::Execute(Event)
{
    bool retVal = false;

    switch (bot->getClass())
    {
        case CLASS_PRIEST:
            retVal = botAI->DoSpecificAction(CreateNextAction<CastLesserHealOnPartyAction>(1.0f).factory, Event(), true);
            break;
        case CLASS_DRUID:
            retVal = botAI->DoSpecificAction(CreateNextAction<CastHealingTouchOnPartyAction>(1.0f).factory, Event(), true);
            break;
        case CLASS_PALADIN:
            retVal = botAI->DoSpecificAction(CreateNextAction<CastHolyLightOnPartyAction>(1.0f).factory, Event(), true);
            break;
        case CLASS_SHAMAN:
            retVal = botAI->DoSpecificAction(CreateNextAction<CastHealingWaveOnPartyAction>(1.0f).factory, Event(), true);
            break;
    }

    return retVal;
}

NextAction::Factory RpgHomeBindAction::getActionFactory() const
{
    return CreateNextAction<SetHomeAction>(1.0f).factory;
}

NextAction::Factory RpgQueueBgAction::getActionFactory() const
{
    SET_AI_VALUE(uint32, "bg type", (uint32)AI_VALUE(BattlegroundTypeId, "rpg bg type"));

    return CreateNextAction<FreeBGJoinAction>(1.0f).factory;
}

NextAction::Factory RpgBuyPetitionAction::getActionFactory() const
{
    return CreateNextAction<BuyPetitionAction>(1.0f).factory;
}

NextAction::Factory RpgUseAction::getActionFactory() const
{
    return CreateNextAction<UseItemAction>(1.0f).factory;
}

Event RpgUseAction::ActionEvent(Event)
{
    return Event("rpg action", chat->FormatWorldobject(this->rpg->guidP().GetWorldObject()));
}

NextAction::Factory RpgSpellAction::getActionFactory() const
{
    return CreateNextAction<CastRandomSpellAction>(1.0f).factory;
}

Event RpgSpellAction::ActionEvent(Event)
{
    return Event("rpg action", chat->FormatWorldobject(this->rpg->guidP().GetWorldObject()));
}

NextAction::Factory RpgCraftAction::getActionFactory() const
{
    return CreateNextAction<CraftRandomItemAction>(1.0f).factory;
}

Event RpgCraftAction::ActionEvent(Event)
{
    return Event("rpg action", chat->FormatWorldobject(this->rpg->guidP().GetWorldObject()));
}

std::vector<Item*> RpgTradeUsefulAction::CanGiveItems(GuidPosition guidPosition)
{
    Player* player = guidPosition.GetPlayer();

    std::vector<Item*> giveItems;

    if (botAI->HasActivePlayerMaster() || !GET_PLAYERBOT_AI(player))
        return giveItems;

    std::vector<ItemUsage> myUsages = {ITEM_USAGE_NONE, ITEM_USAGE_VENDOR, ITEM_USAGE_AH, ITEM_USAGE_DISENCHANT};

    for (auto& myUsage : myUsages)
    {
        std::vector<Item*> myItems =
            AI_VALUE2(std::vector<Item*>, "inventory items", "usage " + std::to_string(myUsage));
        std::reverse(myItems.begin(), myItems.end());

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
                continue;

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", item->GetEntry());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}

bool RpgTradeUsefulAction::Execute(Event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    std::vector<Item*> items = CanGiveItems(guidP);

    if (items.empty())
        return false;

    Item* item = items.front();

    std::ostringstream param;

    param << chat->FormatWorldobject(player);
    param << " ";
    param << chat->FormatItem(item->GetTemplate());

    bool hasTraded = botAI->DoSpecificAction(CreateNextAction<TradeAction>(1.0f).factory, Event("rpg action", param.str().c_str()), true);

    if (hasTraded || bot->GetTradeData())
    {
        if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
        {
            if (bot->GetGroup() && bot->GetGroup()->IsMember(guidP) && botAI->HasRealPlayerMaster())
                botAI->TellMasterNoFacing(
                    "You can use this " + chat->FormatItem(item->GetTemplate()) + " better than me, " +
                    guidP.GetPlayer()->GetName() /*chat->FormatWorldobject(guidP.GetPlayer())*/ + ".");
            else
                bot->Say("You can use this " + chat->FormatItem(item->GetTemplate()) + " better than me, " +
                             player->GetName() /*chat->FormatWorldobject(player)*/ + ".",
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

            if (!urand(0, 4) || items.size() < 2)
            {
                // bot->Say("End trade with" + chat->FormatWorldobject(player), (bot->GetTeamId() == TEAM_ALLIANCE ?
                // LANG_COMMON : LANG_ORCISH));
                WorldPacket p;
                uint32 status = TRADE_STATUS_TRADE_ACCEPT;
                p << status;
                bot->GetSession()->HandleAcceptTradeOpcode(p);
            }
        }
        else
            bot->Say("Start trade with" + chat->FormatWorldobject(player),
                     (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

        botAI->SetNextCheckDelay(sPlayerbotAIConfig.rpgDelay);
        return true;
    }

    return false;
}

bool RpgDuelAction::isUseful()
{
    // do not offer duel in non pvp areas
    if (sPlayerbotAIConfig.IsInPvpProhibitedZone(bot->GetZoneId()))
        return false;

    // Players can only fight a duel with each other outside (=not inside dungeons and not in capital cities)
    AreaTableEntry const* casterAreaEntry = sAreaTableStore.LookupEntry(bot->GetAreaId());
    if (casterAreaEntry && !(casterAreaEntry->flags & AREA_FLAG_ALLOW_DUELS))
    {
        // Dueling isn't allowed here
        return false;
    }

    return true;
}

bool RpgDuelAction::Execute(Event)
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    if (!player)
        return false;

    return botAI->DoSpecificAction(CreateNextAction<CastCustomSpellAction>(1.0f).factory, Event("rpg action", chat->FormatWorldobject(player) + " 7266"),
                                   true);
}

bool RpgMountAnimAction::isUseful()
{
    return AI_VALUE2(bool, "mounted", "self target") && !AI_VALUE2(bool, "moving", "self target");
}

bool RpgMountAnimAction::Execute(Event)
{
    WorldPacket p;
    bot->GetSession()->HandleMountSpecialAnimOpcode(p);

    return true;
}