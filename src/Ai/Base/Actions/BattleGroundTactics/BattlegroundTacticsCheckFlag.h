#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsCheckFlag : public BGTactics
{
    BattlegroundTacticsCheckFlag(PlayerbotAI* botAI) : BGTactics(botAI, "check flag") {}

    bool Execute(Event event) override;
};
