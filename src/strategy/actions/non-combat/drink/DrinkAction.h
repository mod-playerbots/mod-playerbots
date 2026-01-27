/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRINK_ACTION_H
#define _PLAYERBOT_DRINK_ACTION_H

#include <string>

#include "Event.h"
#include "UseItemAction.h"
#include "GlobalPlayerInspector.h"
#include "DrinkAuraEnum.h"


class PlayerbotAI;

class DrinkAction : public UseItemAction
{
public:
    DrinkAction(PlayerbotAI* botAI) : UseItemAction(botAI, "drink") {}

    bool isPossible() override
    {
        if (this->bot->IsInCombat())
        {
            return false;
        }

        if (this->bot->IsMounted())
        {
            return false;
        }

        if (this->botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form", "aquatic form","flight form", "swift flight form", nullptr))
        {
            return false;
        }

        if (this->bot->IsCrowdControlled())
        {
            return false;
        }

        return this->botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible();
    }

    bool isUseful() override
    {
        const GlobalPlayerInspector playerInspector(this->bot->GetGUID().GetRawValue());

        if (playerInspector.isDrinking())
            return false;

        return playerInspector.shouldBeDrinking(50.0f);
    }

    bool Execute(Event event) override
    {
        std::string name = this->bot->GetName().c_str();

        this->bot->ClearUnitState(UNIT_STATE_CHASE);
        this->bot->ClearUnitState(UNIT_STATE_FOLLOW);

        this->botAI->InterruptSpell();

        if (this->bot->isMoving())
            this->bot->StopMoving();

        this->bot->SetStandState(UNIT_STAND_STATE_SIT);

        if (this->botAI->HasCheat(BotCheatMask::food))
        {
            this->bot->AddAura(DRINK_AURA_GRACCU_MINCE_MEAT_FRUITCAKE, bot);

            this->botAI->SetNextCheckDelay(1000);

            return true;
        }

        this->botAI->SetNextCheckDelay(1000);

        return UseItemAction::Execute(event);
    }
};

#endif
