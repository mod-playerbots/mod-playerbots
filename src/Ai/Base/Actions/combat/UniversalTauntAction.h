#pragma once

#include "CreateNextAction.h"
#include "GenericSpellActions.h"
#include "AiFactory.h"
#include "DKActions.h"
#include "PaladinActions.h"
#include "WarriorActions.h"
#include "DruidBearActions.h"

class UniversalTauntAction : public CastSpellAction
{
public:
    UniversalTauntAction(PlayerbotAI* botAI, std::string spell) : CastSpellAction(botAI, spell) {}

    bool isPossible() override
    {
        const uint8_t tab = AiFactory::GetPlayerSpecTab(this->bot);

        switch (this->bot->getClass())
        {
            case CLASS_DEATH_KNIGHT:
                return tab == DEATH_KNIGHT_TAB_BLOOD;
            case CLASS_PALADIN:
                return tab == PALADIN_TAB_PROTECTION;
            case CLASS_DRUID:
                return tab == DRUID_TAB_FERAL;
            case CLASS_WARRIOR:
                return tab == WARRIOR_TAB_PROTECTION;
        }

        return false;
    }

    bool isUseful() override
    {
        const Unit* const target = this->GetTarget();

        if (target == nullptr)
        {
            return false;
        }

        const ObjectGuid targetTargetGUID = target->GetTarget();

        if (targetTargetGUID.GetEntry() == this->bot->GetGUID().GetEntry())
        {
            return false;
        }

        return true;
    }

    bool Execute(Event) override
    {
        switch (this->bot->getClass())
        {
            case CLASS_DEATH_KNIGHT:
                return this->botAI->DoSpecificAction(CreateNextAction<CastDarkCommandAction>(ACTION_EMERGENCY).factory);
            case CLASS_PALADIN:
                return this->botAI->DoSpecificAction(CreateNextAction<CastHandOfReckoningAction>(ACTION_EMERGENCY).factory);
            case CLASS_DRUID:
                return this->botAI->DoSpecificAction(CreateNextAction<CastGrowlAction>(ACTION_EMERGENCY).factory);
            case CLASS_WARRIOR:
                return this->botAI->DoSpecificAction(CreateNextAction<CastTauntAction>(ACTION_EMERGENCY).factory);
        }

        return false;
    }

};
