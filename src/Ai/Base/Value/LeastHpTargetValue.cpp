/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LeastHpTargetValue.h"

#include "AttackersValue.h"
#include "Playerbots.h"

class FindLeastHpTargetStrategy : public FindNonCcTargetStrategy
{
public:
    FindLeastHpTargetStrategy(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI), minHealth(0) {}

    void CheckAttacker(Unit* attacker, ThreatManager* /*threatMgr*/) override
    {
        if (IsCcTarget(attacker))
            return;

        if (!result || result->GetHealth() > attacker->GetHealth())
            result = attacker;
    }

protected:
    float minHealth;
};

Unit* LeastHpTargetValue::Calculate()
{
    FindLeastHpTargetStrategy strategy(botAI);

    return FindTarget(&strategy);
}
