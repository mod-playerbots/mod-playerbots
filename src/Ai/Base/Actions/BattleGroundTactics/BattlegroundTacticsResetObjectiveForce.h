#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsResetObjectiveForce : public BGTactics
{
    BattlegroundTacticsResetObjectiveForce(PlayerbotAI* botAI) : BGTactics(botAI, "reset objective force") {}

    bool Execute(Event event) override;
};
