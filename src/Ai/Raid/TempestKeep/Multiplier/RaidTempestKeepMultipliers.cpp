#include "RaidTempestKeepMultipliers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepKaelthasBossAI.h"
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
#include "RogueActions.h"
#include "ShamanActions.h"
#include "WarlockActions.h"
#include "WarriorActions.h"

// Al'ar <Phoenix God>

float AlarMoveBetweenPlatformsMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return 1.0f;

    if (dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<TankFaceAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsDps(bot) &&
        dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AlarDisableDisperseMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AlarDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if (!botAI->IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "al'ar"))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AlarStayAwayFromRebirthMultiplier::GetValue(Action* action)
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return 1.0f;

    Creature* alarCreature = alar->ToCreature();
    if (!alarCreature || alarCreature->GetReactState() != REACT_PASSIVE)
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AlarMoveAwayFromRebirthAction*>(action) &&
        !dynamic_cast<AlarAvoidFlamePatchesAndDiveBombsAction*>(action))
        return 0.0f;

    return 1.0f;
}

float AlarPhase2NoTankingIfArmorMeltedMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_MELT_ARMOR))
        return 1.0f;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || AI_VALUE(Unit*, "current target") != alar)
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Void Reaver

float VoidReaverMaintainPositionsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "void reaver"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// High Astromancer Solarian

float HighAstromancerSolarianMaintainPositionMultiplier::GetValue(Action* action)
{
    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer || astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM))
        return 1.0f;

    if (botAI->IsRanged(bot) &&
        (dynamic_cast<CombatFormationMoveAction*>(action) ||
         dynamic_cast<FleeAction*>(action) ||
         dynamic_cast<CastBlinkBackAction*>(action) ||
         dynamic_cast<CastDisengageAction*>(action)))
        return 0.0f;

    if (!bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<HighAstromancerSolarianMoveAwayFromGroupAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float HighAstromancerSolarianDisableTankAssistMultiplier::GetValue(Action* action)
{
    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if (!botAI->IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "solarium priest"))
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

float KaelthasSunstriderWaitForDpsMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    if (dynamic_cast<KaelthasSunstriderMisdirectAdvisorsToTanksAction*>(action))
        return 1.0f;

    const time_t now = std::time(nullptr);
    constexpr uint8 dpsWaitSeconds = 10;

    auto it = advisorDpsWaitTimer.find(kaelthas->GetMap()->GetInstanceId());
    if (it == advisorDpsWaitTimer.end() || (now - it->second) < dpsWaitSeconds)
    {
        Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
        Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
        Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");

        auto isAdvisorActive = [](Unit* advisor)
        {
            return advisor && !advisor->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
                   !advisor->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
        };

        if ((isAdvisorActive(sanguinar) && botAI->IsMainTank(bot)) ||
            (isAdvisorActive(telonicus) && botAI->IsAssistTankOfIndex(bot, 0, true)) ||
            (isAdvisorActive(capernian) && (botAI->IsMainTank(bot) || GetCapernianTank(bot) == bot)))
            return 1.0f;

        bool shouldHoldDps =
            (isAdvisorActive(sanguinar) && !botAI->IsMainTank(bot)) ||
            (isAdvisorActive(telonicus) && !botAI->IsAssistTankOfIndex(bot, 0, true)) ||
            (isAdvisorActive(capernian) && !botAI->IsMainTank(bot) && GetCapernianTank(bot) != bot);

        if (shouldHoldDps &&
            (dynamic_cast<AttackAction*>(action) ||
             (dynamic_cast<CastSpellAction*>(action) &&
              !dynamic_cast<CastHealingSpellAction*>(action))))
            return 0.0f;
    }

    return 1.0f;
}

float KaelthasSunstriderKiteThaladredMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (botAI->IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
        return 1.0f;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot ||
        thaladred->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<KaelthasSunstriderKiteThaladredAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() == PHASE_FINAL)
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderKeepDistanceFromCapernianMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return 1.0f;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderManageWeaponTankingMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (kaelAI->GetPhase() != PHASE_WEAPONS &&
        dynamic_cast<TankFaceAction*>(action))
        return 0.0f;

    if (!botAI->IsMainTank(bot))
        return 1.0f;

    // Try to keep main tank from grabbing aggro on any weapon other than the axe
    if (kaelAI->GetPhase() == PHASE_WEAPONS &&
        (dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) ||
         dynamic_cast<CastChallengingShoutAction*>(action) ||
         dynamic_cast<CastThunderClapAction*>(action) ||
         dynamic_cast<CastShockwaveAction*>(action) ||
         dynamic_cast<CastCleaveAction*>(action) ||
         dynamic_cast<CastGrowlAction*>(action) ||
         dynamic_cast<CastSwipeAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) ||
         dynamic_cast<CastAvengersShieldAction*>(action) ||
         dynamic_cast<CastConsecrationAction*>(action) ||
         dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastDeathAndDecayAction*>(action) ||
         dynamic_cast<CastPestilenceAction*>(action) ||
         dynamic_cast<CastBloodBoilAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderDisableAdvisorTankAssistMultiplier::GetValue(Action* action)
{
    if (bot->GetVictim() == nullptr || !botAI->IsTank(bot))
        return 1.0f;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return 1.0f;

    if (kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR &&
        kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderDisableDisperseMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankFaceAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Bloodlust/Heroism and other major cooldowns should be used at the start of Phase 3
float KaelthasSunstriderDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return 1.0f;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() == PHASE_ALL_ADVISORS ||
        kaelAI->GetPhase() == PHASE_FINAL)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) ||
         dynamic_cast<CastHeroismAction*>(action)))
        return 0.0f;

    if (botAI->IsDps(bot) &&
        (dynamic_cast<CastMetamorphosisAction*>(action) ||
         dynamic_cast<CastAdrenalineRushAction*>(action) ||
         dynamic_cast<CastBladeFlurryAction*>(action) ||
         dynamic_cast<CastIcyVeinsAction*>(action) ||
         dynamic_cast<CastColdSnapAction*>(action) ||
         dynamic_cast<CastArcanePowerAction*>(action) ||
         dynamic_cast<CastPresenceOfMindAction*>(action) ||
         dynamic_cast<CastCombustionAction*>(action) ||
         dynamic_cast<CastRapidFireAction*>(action) ||
         dynamic_cast<CastReadinessAction*>(action) ||
         dynamic_cast<CastAvengingWrathAction*>(action) ||
         dynamic_cast<CastElementalMasteryAction*>(action) ||
         dynamic_cast<CastFeralSpiritAction*>(action) ||
         dynamic_cast<CastFireElementalTotemAction*>(action) ||
         dynamic_cast<CastFireElementalTotemMeleeAction*>(action) ||
         dynamic_cast<CastForceOfNatureAction*>(action) ||
         dynamic_cast<CastArmyOfTheDeadAction*>(action) ||
         dynamic_cast<CastSummonGargoyleAction*>(action) ||
         dynamic_cast<CastBerserkingAction*>(action) ||
         dynamic_cast<CastBloodFuryAction*>(action) ||
         dynamic_cast<UseTrinketAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_GRAVITY_LAPSE))
        return 1.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<KaelthasSunstriderSpreadOutInMidairAction*>(action))
        return 0.0f;

    return 1.0f;
}
