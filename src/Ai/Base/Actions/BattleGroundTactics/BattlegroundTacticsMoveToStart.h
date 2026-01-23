#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsMoveToStart : public BGTactics
{
    BattlegroundTacticsMoveToStart(PlayerbotAI* botAI) : BGTactics(botAI, "move to start") {}

    bool Execute(Event event) override;
};
