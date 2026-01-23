#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsUseBuff : public BGTactics
{
    BattlegroundTacticsUseBuff(PlayerbotAI* botAI) : BGTactics(botAI, "use buff") {}

    bool Execute(Event event) override;
};
