#include "RaidAq20Triggers.h"

#include "RaidAq20Utils.h"

bool Aq20MoveToCrystalTrigger::IsActive()
{
    Unit* const boss = context->GetValue<Unit*>("find target", "ossirian the unscarred")->Get();

    if (boss == nullptr)
    {
        return false;
    }

    if (!boss->IsInCombat())
    {
        return false;
    }

    // if buff is active move to crystal
    if (RaidAq20Utils::IsOssirianBuffActive(*boss))
    {
        return true;
    }

    // if buff is not active a debuff will be, buff becomes active once debuff expires
    // so move to crystal when debuff almost done, or based debuff time left and
    // distance bot is from crystal (ie: start moving early enough to make it)
    int32_t debuffTimeRemaining = RaidAq20Utils::GetOssirianDebuffTimeRemaining(*boss);

    if (debuffTimeRemaining < 5000)
    {
        return true;
    }

    if (debuffTimeRemaining < 30000)
    {
        const GameObject* const crystal = RaidAq20Utils::GetNearestCrystal(*boss);

        if (crystal == nullptr)
        {
            return false;
        }

        float botDist = this->bot->GetDistance(crystal);
        float timeToReach = botDist / this->bot->GetSpeed(MOVE_RUN);
        // bot should ideally activate crystal a ~5 seconds early (due to time it takes for crystal
        // to activate) so aim to get there in time to do so
        return debuffTimeRemaining - 5000 < timeToReach * 1000.0f;
    }

    return false;
}
