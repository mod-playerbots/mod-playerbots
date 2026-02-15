#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsResetObjectiveForce : public BGTactics
{
public:
    BattlegroundTacticsResetObjectiveForce(PlayerbotAI* botAI) : BGTactics(botAI, "reset objective force") {}
    ~BattlegroundTacticsResetObjectiveForce() override = default;
};
