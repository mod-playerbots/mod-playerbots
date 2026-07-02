/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GUILDCREATEACTIONS_H
#define PLAYERBOTS_GUILDCREATEACTIONS_H

#include "ChooseTravelTargetAction.h"
#include "InventoryAction.h"

class PlayerbotAI;

class BuyPetitionAction : public InventoryAction
{
public:
    BuyPetitionAction(PlayerbotAI* botAI) : InventoryAction(botAI, "buy petition") {}

    bool Execute(Event event) override;
    bool isUseful() override;

    static bool canBuyPetition(Player* bot);
};

class PetitionOfferAction : public Action
{
public:
    PetitionOfferAction(PlayerbotAI* botAI, std::string const name = "petition offer") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class PetitionOfferNearbyAction : public PetitionOfferAction
{
public:
    PetitionOfferNearbyAction(PlayerbotAI* botAI) : PetitionOfferAction(botAI, "petition offer nearby") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class PetitionTurnInAction : public ChooseTravelTargetAction
{
public:
    PetitionTurnInAction(PlayerbotAI* botAI) : ChooseTravelTargetAction(botAI, "turn in petitn") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class BuyTabardAction : public ChooseTravelTargetAction
{
public:
    BuyTabardAction(PlayerbotAI* botAI) : ChooseTravelTargetAction(botAI, "buy tabard") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
