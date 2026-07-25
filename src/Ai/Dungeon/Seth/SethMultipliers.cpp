/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SethMultipliers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "SethActions.h"
#include "SethData.h"
#include "ShamanActions.h"

using namespace SethData;

float SethekkProphetSetTremorTotemMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "sethekk prophet"))
        return 1.0f;

    if (dynamic_cast<CastStrengthOfEarthTotemAction*>(action) ||
        dynamic_cast<CastStoneskinTotemAction*>(action) ||
        dynamic_cast<CastStoneclawTotemAction*>(action) ||
        dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float AnzuControlSpellCastingWithSpellBombMultiplier::GetValue(Action* action)
{
    if (bot->getPowerType() != POWER_MANA || botAI->IsTank(bot))
        return 1.0f;

    if (!bot->HasAura(static_cast<uint32>(SethSpells::SPELL_SPELL_BOMB)))
        return 1.0f;

    if (botAI->IsDps(bot) && dynamic_cast<CastSpellAction*>(action))
        return 0.0f;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (mainTank && mainTank->GetHealthPct() > 50.0f &&
        dynamic_cast<CastSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float TalonKingIkissDelayBloodlustAndHeroismMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
        ikiss && ikiss->GetHealthPct() > 95.0f)
    {
        return 0.0f;
    }

    if (!dynamic_cast<CastHeroismAction*>(action) &&
        !dynamic_cast<CastBloodlustAction*>(action))
    {
        return 1.0f;
    }

    return 1.0f;
}

float TalonKingIkissControlMovementMultiplier::GetValue(Action* action)
{
    Unit* ikiss = AI_VALUE2(Unit*, "find target", "talon king ikiss");
    if (!ikiss)
        return 1.0f;

    if (/*dynamic_cast<FleeAction*>(action) ||*/
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action))
    {
        return 0.0f;
    }

    if (ikiss->HasAura(static_cast<uint32>(SethSpells::SPELL_ARCANE_BUBBLE)))
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<TalonKingIkissLosArcaneExplosionAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
