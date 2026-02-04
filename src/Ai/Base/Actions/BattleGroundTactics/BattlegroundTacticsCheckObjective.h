#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsCheckObjective : public BGTactics
{
public:
    BattlegroundTacticsCheckObjective(PlayerbotAI* botAI) : BGTactics(botAI, "check objective") {}
    ~BattlegroundTacticsCheckObjective() override = default;
};
