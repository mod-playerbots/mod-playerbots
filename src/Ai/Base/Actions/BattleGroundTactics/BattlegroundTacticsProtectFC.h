#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsProtectFC : public BGTactics
{
    BattlegroundTacticsProtectFC(PlayerbotAI* botAI) : BGTactics(botAI, "protect fc") {}

    bool Execute(Event event) override;
};
