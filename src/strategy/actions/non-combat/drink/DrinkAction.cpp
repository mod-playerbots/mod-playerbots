#include "DrinkAction.h"
#include <string>
#include "Event.h"

#include "DrinkAuraEnum.h"
#include "isEatingOrDrinking.h"
#include "shouldKeepDrinking.h"


bool DrinkAction::isPossible()
{
    bool possible = !this->bot->IsInCombat() &&
        !this->bot->IsMounted() &&
        !this->botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form",
            "aquatic form","flight form", "swift flight form", nullptr) &&
        (this->botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible());

    return possible;
}

bool DrinkAction::isUseful()
{
    if (isEatingOrDrinking(bot))
        return false;

    bool keepDrinking = shouldKeepDrinking(botAI);

    return keepDrinking;
}

bool DrinkAction::Execute(Event event)
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
