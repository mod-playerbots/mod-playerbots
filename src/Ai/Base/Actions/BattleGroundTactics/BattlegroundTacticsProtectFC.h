#pragma once

#include "BattleGroundTactics.h"

class BattlegroundTacticsProtectFC : public BGTactics
{
public:
    BattlegroundTacticsProtectFC(PlayerbotAI* botAI) : BGTactics(botAI, "protect fc") {}
    ~BattlegroundTacticsProtectFC() override = default;
};
