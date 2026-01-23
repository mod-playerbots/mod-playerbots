#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsMoveToObjective : public BGTactics
{
    BattlegroundTacticsMoveToObjective(PlayerbotAI* botAI) : BGTactics(botAI, "move to objective") {}

    bool Execute(Event event) override;
};
