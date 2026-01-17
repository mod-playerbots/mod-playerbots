#include "RaidGruulsLairMultipliers.h"
#include "RaidGruulsLairActions.h"
#include "RaidGruulsLairHelpers.h"
#include "ChooseTargetActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "WarriorActions.h"

using namespace GruulsLairHelpers;

static bool IsChargeAction(Action* action)
{
    return dynamic_cast<CastChargeAction*>(action) ||
           dynamic_cast<CastInterceptAction*>(action) ||
           dynamic_cast<CastFeralChargeBearAction*>(action) ||
           dynamic_cast<CastFeralChargeCatAction*>(action);
}

float HighKingMaulgarDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (IsAnyOgreBossAlive(botAI) && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Don't run back in during Whirlwind
float HighKingMaulgarAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    if (maulgar && maulgar->HasAura(SPELL_WHIRLWIND) &&
        (!kiggler || !kiggler->IsAlive()) &&
        (!krosh || !krosh->IsAlive()) &&
        (!olm || !olm->IsAlive()) &&
        (!blindeye || !blindeye->IsAlive()))
    {
        if (IsChargeAction(action) || (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action)))
            return 0.0f;
    }

    return 1.0f;
}

// Arcane Shot will remove Spell Shield, which the mage tank needs to survive
float HighKingMaulgarDisableArcaneShotOnKroshMultiplier::GetValue(Action* action)
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* target = AI_VALUE(Unit*, "current target");

    if (krosh && target && target->GetGUID() == krosh->GetGUID() && dynamic_cast<CastArcaneShotAction*>(action))
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarDisableMageTankAOEMultiplier::GetValue(Action* action)
{
    if (IsKroshMageTank(botAI, bot) &&
        (dynamic_cast<CastFrostNovaAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastConeOfColdAction*>(action) || dynamic_cast<CastFlamestrikeAction*>(action) ||
        dynamic_cast<CastDragonsBreathAction*>(action) || dynamic_cast<CastBlastWaveAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float GruulTheDragonkillerMainTankMovementMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return 1.0f;

    if (botAI->IsMainTank(bot))
    {
        if (gruul->GetVictim() == bot && dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;

        if (dynamic_cast<AvoidAoeAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerGroundSlamMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul)
        return 1.0f;

    if (bot->HasAura(SPELL_GROUND_SLAM_1) ||
        bot->HasAura(SPELL_GROUND_SLAM_2))
    {
        if ((dynamic_cast<MovementAction*>(action) && !dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action)) ||
            IsChargeAction(action))
            return 0.0f;
    }

    return 1.0f;
}
