#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsMoveToStart : public BGTactics
{
public:
    BattlegroundTacticsMoveToStart(PlayerbotAI* botAI) : BGTactics(botAI, "move to start") {}
    ~BattlegroundTacticsMoveToStart() override = default;
};
