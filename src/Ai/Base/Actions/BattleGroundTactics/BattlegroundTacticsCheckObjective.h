#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsCheckObjective : public BGTactics
{
    BattlegroundTacticsCheckObjective(PlayerbotAI* botAI) : BGTactics(botAI, "check objective") {}

    bool Execute(Event event) override;
};
