#include <unordered_map>
#include <ctime>

#include "RaidMagtheridonMultipliers.h"
#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "WarlockActions.h"
#include "WipeAction.h"

using namespace MagtheridonHelpers;

// Don't do anything other than clicking cubes when Magtheridon is casting Blast Nova
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        auto it = botToCubeAssignment.find(bot->GetGUID());
        if (it != botToCubeAssignment.end())
        {
            if (dynamic_cast<WipeAction*>(action))
                return 1.0f;
            else if (!dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Bots will wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return 1.0f;

    if (botAI->IsMainTank(bot))
        return 1.0f;

    const uint8 dpsWaitSeconds = 6;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it == dpsWaitTimer.end() ||
        (time(nullptr) - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

float MagtheridonDisableOffTankAssistMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon)
        return 1.0f;

    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if ((botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1)) &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}
