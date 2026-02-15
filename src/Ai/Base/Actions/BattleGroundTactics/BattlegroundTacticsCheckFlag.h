#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsCheckFlag : public BGTactics
{
public:
    BattlegroundTacticsCheckFlag(PlayerbotAI* botAI) : BGTactics(botAI, "check flag") {}
    ~BattlegroundTacticsCheckFlag() override = default;
};
