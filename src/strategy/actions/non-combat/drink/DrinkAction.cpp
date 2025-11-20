#include "DrinkAction.h"
#include <string>
#include "Event.h"

#include "DrinkAuraEnum.h"
#include "isEatingOrDrinking.h"
#include "shouldKeepDrinking.h"


bool DrinkAction::isPossible()
{
    bool possible = !bot->IsInCombat() &&
        !bot->IsMounted() &&
        !botAI->HasAnyAuraOf(GetTarget(), "dire bear form", "bear form", "cat form", "travel form",
            "aquatic form","flight form", "swift flight form", nullptr) &&
        (botAI->HasCheat(BotCheatMask::food) || UseItemAction::isPossible());

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
    std::string name = bot->GetName().c_str();

    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    botAI->InterruptSpell();

    if (bot->isMoving())
        bot->StopMoving();

    bot->SetStandState(UNIT_STAND_STATE_SIT);

    if (botAI->HasCheat(BotCheatMask::food))
    {
        bot->AddAura(DRINK_AURA_GRACCU_MINCE_MEAT_FRUITCAKE, bot);

        botAI->SetNextCheckDelay(1000);

        return true;
    }

    botAI->SetNextCheckDelay(1000);

    return UseItemAction::Execute(event);
}
