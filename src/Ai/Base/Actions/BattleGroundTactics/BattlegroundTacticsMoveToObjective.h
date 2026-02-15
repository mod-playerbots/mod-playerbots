#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsMoveToObjective : public BGTactics
{
public:
    BattlegroundTacticsMoveToObjective(PlayerbotAI* botAI) : BGTactics(botAI, "move to objective") {}
    ~BattlegroundTacticsMoveToObjective() override = default;
};
