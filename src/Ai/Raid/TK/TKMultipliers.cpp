/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKMultipliers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "EquipAction.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include <ctime>

namespace
{

bool IsSingleTargetTauntAction(Action* action)
{
    return dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastDeathGripAction*>(action);
}

}

// Al'ar <Phoenix God>

float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    bool const isBlockedMovement =
        dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action);

    if (!isBlockedMovement && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    int8 const currentLocationIndex = GetAlarCurrentLocationIndex(alar);
    if (currentLocationIndex < PLATFORM_0_IDX || currentLocationIndex > PLATFORM_3_IDX)
        return 0.0f;

    return 1.0f;
}

float AlarControlMovementMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<TankFaceAction*>(action) || dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    bool const isDisperseOrFlee =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action);

    if (!isDisperseOrFlee && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (isDisperseOrFlee)
        return 0.0f;

    if (!IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    // Enable FollowAction only in non-combat engine in Phase 2
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 0.0f;

    return 1.0f;
}

float AlarDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "al'ar"))
        return 0.0f;

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AlarMoveAwayFromRebirthAction*>(action))
        return 1.0f;

    // Don't block Flame Quills avoidance in case of bad timing for the transition
    if (dynamic_cast<AlarJumpFromPlatformAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return 0.0f;

    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (alar->GetHealthPct() <= 5.0f) // Melee dps activate logic for the P2 transition at 5% HP
        return 0.0f;

    return 1.0f;
}

float AlarControlTauntingMultiplier::GetValue(Action* action)
{
    if (!IsSingleTargetTauntAction(action))
        return 1.0f;

    bool const isFirstAlarTank = IsFirstAlarTank(bot);

    if (!isFirstAlarTank && !IsSecondAlarTank(bot))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (bot->HasAura(Id(TkSpells::SPELL_MELT_ARMOR)) && AI_VALUE(Unit*, "current target") == alar)
        return 0.0f;

    if (IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return 1.0f;

    int8 platformIndex = GetAlarPlatformIndex(alar);
    if (isFirstAlarTank)
    {
        if (platformIndex != PLATFORM_0_IDX && platformIndex != PLATFORM_2_IDX)
            return 0.0f;
    }
    else // isSecondAlarTank
    {
        if (platformIndex != PLATFORM_1_IDX && platformIndex != PLATFORM_3_IDX)
            return 0.0f;
    }

    return 1.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "void reaver"))
        return 0.0f;

    return 1.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianWrathStayAwayMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<MovementAction*>(action))
    {
        return 1.0f;
    }

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(Id(TkSpells::SPELL_SOLARIAN_TRANSFORM)))
        return 1.0f;

    if (HasWrathOfTheAstromancer(bot))
        return 0.0f;

    return 1.0f;
}

float HighAstromancerSolarianDisableMeleeTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsMelee(bot))
        return 1.0f;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
    {
        Creature* astromancerCreature = astromancer->ToCreature();
        if (astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE)
            return 0.0f;
    }
    else if (AI_VALUE2(Unit*, "find target", "solarium priest"))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) && !dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    time_t const now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 10;

    auto it = advisorDpsWaitTimer.find(kaelthas->GetMap()->GetInstanceId());
    if (it != advisorDpsWaitTimer.end() && it->second != -1 &&
        (now - it->second) >= dpsWaitSeconds)
    {
        return 1.0f;
    }

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

    auto isAdvisorActive = [](Unit* advisor)
    {
        return advisor && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
            !IsFeigningDeath(advisor);
    };

    bool isMainTank = PlayerbotAI::IsMainTank(bot);
    bool isFirstAssistTank = PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
    bool isWarlockTank = GetCapernianTank(bot) == bot;

    if ((isAdvisorActive(sanguinar) && isMainTank) ||
        (isAdvisorActive(telonicus) && isFirstAssistTank) ||
        (isAdvisorActive(capernian) && (isMainTank || isWarlockTank)))
    {
        return 1.0f;
    }

    bool shouldHoldDps =
        (isAdvisorActive(sanguinar) && !isMainTank) ||
        (isAdvisorActive(telonicus) && !isFirstAssistTank) ||
        (isAdvisorActive(capernian) && !isMainTank && !isWarlockTank);

    if (shouldHoldDps)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (thaladred && thaladred->GetVictim() == bot)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI && kaelAI->GetPhase() != PHASE_FINAL)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<MovementAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderManageWeaponTankingMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe
    if (!IsSingleTargetTauntAction(action) &&
        !dynamic_cast<CastChallengingShoutAction*>(action) &&
        !dynamic_cast<CastThunderClapAction*>(action) &&
        !dynamic_cast<CastShockwaveAction*>(action) &&
        !dynamic_cast<CastCleaveAction*>(action) &&
        !dynamic_cast<CastSwipeBearAction*>(action) &&
        !dynamic_cast<CastChallengingRoarAction*>(action) &&
        !dynamic_cast<CastAvengersShieldAction*>(action) &&
        !dynamic_cast<CastConsecrationAction*>(action) &&
        !dynamic_cast<CastDeathAndDecayAction*>(action) &&
        !dynamic_cast<CastPestilenceAction*>(action) &&
        !dynamic_cast<CastBloodBoilAction*>(action))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI && kaelAI->GetPhase() == PHASE_WEAPONS)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderSuppressEquipUpgradeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<EquipUpgradeAction*>(action) &&
        !dynamic_cast<EquipUpgradesPacketAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderManageAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);

    if (!isDpsAssist && !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    // TankAssistAction
    if (PlayerbotAI::IsMainTank(bot))
        return 0.0f;

    if (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
        kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
    {
        return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderPrepareForPhase3Multiplier::GetValue(Action* action)
{
    if (dynamic_cast<KaelthasSunstriderHandleAdvisorRolesInPhase3Action*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return 1.0f;

    // Proxy for revival/Kael talk phase (could pick any advisor here)
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, false) ||
        (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be saved until Phase 3
float KaelthasSunstriderDelayCooldownsMultiplier::GetValue(Action* action)
{
    bool const isLustAction = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

    if (!isLustAction &&
        !dynamic_cast<CastMetamorphosisAction*>(action) &&
        !dynamic_cast<CastAdrenalineRushAction*>(action) &&
        !dynamic_cast<CastBladeFlurryAction*>(action) &&
        !dynamic_cast<CastIcyVeinsAction*>(action) &&
        !dynamic_cast<CastColdSnapAction*>(action) &&
        !dynamic_cast<CastArcanePowerAction*>(action) &&
        !dynamic_cast<CastPresenceOfMindAction*>(action) &&
        !dynamic_cast<CastCombustionAction*>(action) &&
        !dynamic_cast<CastRapidFireAction*>(action) &&
        !dynamic_cast<CastReadinessAction*>(action) &&
        !dynamic_cast<CastAvengingWrathAction*>(action) &&
        !dynamic_cast<CastElementalMasteryAction*>(action) &&
        !dynamic_cast<CastFeralSpiritAction*>(action) &&
        !dynamic_cast<CastFireElementalTotemAction*>(action) &&
        !dynamic_cast<CastFireElementalTotemMeleeAction*>(action) &&
        !dynamic_cast<CastForceOfNatureAction*>(action) &&
        !dynamic_cast<CastArmyOfTheDeadAction*>(action) &&
        !dynamic_cast<CastSummonGargoyleAction*>(action) &&
        !dynamic_cast<CastBerserkingAction*>(action) &&
        !dynamic_cast<CastBloodFuryAction*>(action) &&
        !(PlayerbotAI::IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 1.0f;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI && kaelAI->GetPhase() != PHASE_ALL_ADVISORS && kaelAI->GetPhase() != PHASE_FINAL)
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(Id(TkSpells::SPELL_GRAVITY_LAPSE)))
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) && PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}
