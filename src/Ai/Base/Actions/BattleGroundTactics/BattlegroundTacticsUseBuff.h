#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsUseBuff : public BGTactics
{
public:
    BattlegroundTacticsUseBuff(PlayerbotAI* botAI) : BGTactics(botAI, "use buff") {}
    ~BattlegroundTacticsUseBuff() override = default;
};
