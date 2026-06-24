#include "GruulMultipliers.h"
#include "GruulActions.h"
#include "GruulHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "ShamanActions.h"

using namespace GruulsLairHelpers;

float HighKingMaulgarDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    if (!blindeye || blindeye->GetHealthPct() < 75.0f)
        return 1.0f;

    if (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float HighKingMaulgarControlTankActionsMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "high king maulgar"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        (bot->GetVictim() != nullptr && dynamic_cast<TankAssistAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float HighKingMaulgarAvoidWhirlwindMultiplier::GetValue(Action* action)
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    if (!maulgar ||
        !maulgar->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_WHIRLWIND)))
    {
        return 1.0f;
    }

    if (AI_VALUE(Unit*, "current target") != maulgar)
        return 1.0f;

    if (botAI->IsMainTank(bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<HighKingMaulgarRunAwayFromWhirlwindAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Arcane Shot will remove Spell Shield, which the mage tank needs to survive
float HighKingMaulgarDisableArcaneShotOnKroshMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    if (!krosh || AI_VALUE(Unit*, "current target") != krosh)
        return 1.0f;

    if (dynamic_cast<CastArcaneShotAction*>(action))
        return 0.0f;

    return 1.0f;
}

float HighKingMaulgarDisableMageTankAoeMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "krosh firehand"))
        return 1.0f;

    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (castSpellAction &&
        castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul || gruul->GetHealthPct() < 95.0f)
        return 1.0f;

    if (dynamic_cast<CastHeroismAction*>(action) ||
        dynamic_cast<CastBloodlustAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerControlTankMovementMultiplier::GetValue(Action* action)
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    if (!gruul || gruul->GetVictim() != bot)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float GruulTheDragonkillerStaySpreadForShatterMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_GROUND_SLAM_1)) &&
        !bot->HasAura(static_cast<uint32>(GruulsLairSpells::SPELL_GROUND_SLAM_2)))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<GruulTheDragonkillerShatterSpreadAction*>(action)))
    {
         return 0.0f;
    }

    return 1.0f;
}
