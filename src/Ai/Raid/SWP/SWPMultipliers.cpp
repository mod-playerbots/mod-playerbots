/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include <ctime>

#include "SWPMultipliers.h"
#include "SWPActions.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "NonCombatActions.h"
#include "PaladinActions.h"
#include "PartyMemberToDispel.h"
#include "PriestActions.h"
#include "RaidBossHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "TargetValue.h"
#include "Timer.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"

using namespace SunwellHelpers;

namespace
{

bool IsDpsCooldownAction(Action* action)
{
    return dynamic_cast<CastHeroismAction*>(action) ||
           dynamic_cast<CastBloodlustAction*>(action) ||
           dynamic_cast<CastMetamorphosisAction*>(action) ||
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
           dynamic_cast<CastBloodFuryAction*>(action);
}

}

// Kalecgos

float KalecgosControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float KalecgosWaitToDecurseMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_DRUID && bot->getClass() != CLASS_MAGE &&
        bot->getClass() != CLASS_SHAMAN)
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "kalecgos") &&
        !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
    {
        return 1.0f;
    }

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_CURSE);
    if (!target)
        return 1.0f;

    Aura* aura = target->GetAura(
        static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!aura)
    {
        aura = target->GetAura(
            static_cast<uint32>(SunwellSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC));
    }

    if (!aura || aura->GetDuration() < 15000) // 15 seconds remaining
        return 1.0f;

    if (dynamic_cast<CastRemoveCurseAction*>(action) ||
        dynamic_cast<CastRemoveCurseOnPartyAction*>(action) ||
        dynamic_cast<CastCleanseSpiritAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) ||
        dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (kalecgos && kalecgos->IsFriendlyTo(bot))
        return 1.0f;

    if (!kalecgos && !AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor"))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        (dynamic_cast<CombatFormationMoveAction*>(action) &&
         !dynamic_cast<SetBehindTargetAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosRestrictTauntMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (GetKalecgosCurrentTank(botAI, bot) == bot)
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosSuppressAssistTankPullThreatMultiplier::GetValue(Action* action)
{
    if (!botAI->IsAssistTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    KalecgosEncounterState& state = kalecgosEncounterStates[bot->GetInstanceId()];

    constexpr uint32 pullThreatSuppressionMs = 5000;
    if (getMSTimeDiff(state.encounterStartMs, getMSTime()) >=
        pullThreatSuppressionMs)
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<CastSpellAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KalecgosDelayCooldownsForSathrovarrMultiplier::GetValue(Action* action)
{
    if (bot->HasAura(static_cast<uint32>(SunwellSpells::SPELL_SPECTRAL_REALM)))
        return 1.0f;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (!kalecgos)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Brutallus

float BrutallusControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float BrutallusControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float BrutallusNoKillingSpreeWhenNearbyBurnMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(static_cast<uint32>(SunwellSpells::SPELL_BURN)) &&
            botAI->IsMelee(member) && !botAI->IsMainTank(member) &&
            !botAI->IsAssistTankOfIndex(member, 0, true))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float BrutallusRestrictTauntMultiplier::GetValue(Action* action)
{
    if (!botAI->IsTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    if (dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float BrutallusDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus || brutallus->GetHealthPct() < 95.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "felmyst"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystWaitForLandingDpsMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    uint32 const instanceId = bot->GetInstanceId();
    Position landingDestination;
    bool const isGoingToLand =
        felmyst->IsFlying() && TryGetFelmystLandingDestination(felmyst, landingDestination);

    if (isGoingToLand)
    {
        auto& state = felmystEncounterStates[instanceId];
        if (!state.landingDpsWaitTimer)
            state.landingDpsWaitTimer = std::time(nullptr);

        state.landingTouchdownTimer = 0;
    }
    else if (felmyst->IsFlying())
    {
        auto& state = felmystEncounterStates[instanceId];
        state.landingDpsWaitTimer = 0;
        state.landingTouchdownTimer = 0;
        return 1.0f;
    }

    time_t const now = std::time(nullptr);
    constexpr uint8 groundedDpsWaitSeconds = 3;
    auto& state = felmystEncounterStates[instanceId];
    if (!state.landingDpsWaitTimer)
        return 1.0f;

    if (!state.landingTouchdownTimer)
        state.landingTouchdownTimer = now;

    if (botAI->IsMainTank(bot) || dynamic_cast<FelmystMisdirectBossToMainTankAction*>(action))
        return 1.0f;

    if ((now - state.landingTouchdownTimer) >= groundedDpsWaitSeconds)
    {
        state.landingDpsWaitTimer = 0;
        state.landingTouchdownTimer = 0;
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) &&
         !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    if (!GetFelmystEncapsulateTarget(bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return 0.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FelmystFogOfCorruptionState fogState; // Fog phase active
    FelmystFogLane thirdPassLane = FelmystFogLane::None;
    bool const shouldRepositionAfterThirdPass =
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, fogState) &&
        !shouldRepositionAfterThirdPass)
    {
        return 1.0f;
    }

    // Bots switch to non-combat engine and try to drink during the Fog phase
    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<DrinkAction*>(action))
    {
        return 0.0f;
    }

    FelmystFogOfCorruptionState dangerousFogState; // Fog phase active & bot in danger
    bool needsFogAvoidance = TryGetActiveFelmystFogOfCorruptionState(
        bot, felmyst, dangerousFogState);

    std::array<Position, 3> destinations;
    uint8 destinationCount = 0;
    bool canRelocate = TryGetFelmystFogSafeDestinations(
        bot, needsFogAvoidance ? dangerousFogState.lane :
        shouldRepositionAfterThirdPass ? thirdPassLane :
        fogState.lane, destinations, destinationCount);

    if (needsFogAvoidance && canRelocate &&
        dynamic_cast<CastSpellAction*>(action) &&
        !dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action))
        return canRelocate ? 1.0f : 0.0f;

    if (dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeDemonicVaporKiteMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FelmystFogOfCorruptionState fogState;
    if (TryGetActiveFelmystFogOfCorruptionState(bot, felmyst, fogState))
        return 1.0f;

    if (!IsFelmystDemonicVaporHeadNearBot(bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<FelmystKiteDemonicVaporAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystFocusAttacksOnCharmedPlayerMultiplier::GetValue(Action* action)
{
    if (!botAI->IsDps(bot))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    Player* charmedTarget = GetFelmystCharmedTarget(botAI, bot, felmyst);
    if (!charmedTarget)
        return 1.0f;

    bool const isMelee = botAI->IsMelee(bot);

    if (isMelee && !felmyst->IsFlying() && !bot->IsWithinMeleeRange(charmedTarget))
        return 1.0f;

    if (!isMelee && bot->GetDistance2d(charmedTarget) > 30.0f)
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action) ||
        dynamic_cast<DropTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float FelmystDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || (!felmyst->IsFlying() && felmyst->GetHealthPct() < 95.0f))
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Eredar Twins

float EredarTwinsDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    if (botAI->GetState() == BOT_STATE_COMBAT &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

     if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
         return 0.0f;

    return 1.0f;
}

float EredarTwinsHoldDpsAtStartMultiplier::GetValue(Action* action)
{
    if (botAI->IsTank(bot))
        return 1.0f;

    if (botAI->IsMelee(bot) &&
        bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    uint32 const instanceId = bot->GetInstanceId();
    time_t const now = std::time(nullptr);
    auto const it = eredarTwinsDpsHoldTimer.try_emplace(instanceId, now).first;
    constexpr uint8 dpsHoldSeconds = 8;

    if ((now - it->second) >= dpsHoldSeconds)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) &&
         !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValue(Action* action)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    constexpr float alythessThreatRatio = 0.9f;
    constexpr float sacrolashThreatRatio = 0.8f;

    bool const shouldHoldSacrolashThreat = sacrolash &&
        ShouldHoldTwinThreat(botAI, bot, sacrolash, sacrolashThreatRatio, IsAnySacrolashTank);
    bool const shouldHoldAlythessThreat = alythess &&
        ShouldHoldTwinThreat(botAI, bot, alythess, alythessThreatRatio, IsAlythessTank);

    if (!shouldHoldSacrolashThreat && !shouldHoldAlythessThreat)
        return 1.0f;

    Unit* actionTarget = action->GetTarget();
    bool const suppressSacrolashAttack = shouldHoldSacrolashThreat &&
        (actionTarget == sacrolash || AI_VALUE(Unit*, "current target") == sacrolash);
    bool const suppressAlythessAttack = shouldHoldAlythessThreat &&
        (actionTarget == alythess || AI_VALUE(Unit*, "current target") == alythess);

    if (!suppressSacrolashAttack && !suppressAlythessAttack)
        return 1.0f;

    if ((dynamic_cast<AttackAction*>(action) &&
         !dynamic_cast<EredarTwinsDpsPrioritizeLadySacrolashAction*>(action)) ||
        (dynamic_cast<CastSpellAction*>(action) &&
         !dynamic_cast<CastHealingSpellAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    // Killing Spree hits Alythess during Phase 1 and puts bots in fire in Phase 2
    if (dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action) ||
        dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (!botAI->IsRanged(bot) && !IsAlythessTank(botAI, bot))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<ReachTargetAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float EredarTwinsNoMovingIntoConflagrationMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    Player* conflagTarget = GetEredarTwinsConflagrationTarget(bot);
    if (!conflagTarget)
        return 1.0f;

    if (conflagTarget == bot &&
        (dynamic_cast<CastReachTargetSpellAction*>(action) ||
         (dynamic_cast<MovementAction*>(action) &&
          !dynamic_cast<EredarTwinsConflagratedBotMoveFromGroupAction*>(action))))
    {
        return 0.0f;
    }

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return 1.0f;

    Unit* victim = sacrolash->GetVictim();
    if (victim && victim != bot && conflagTarget == victim)
    {
        if (dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (bot->GetDistance2d(victim) < 10.0f &&
            (dynamic_cast<MovementAction*>(action) &&
             !dynamic_cast<EredarTwinsMoveFromConflagSacrolashVictimAction*>(action)))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float EredarTwinsDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return 1.0f;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash || sacrolash->GetHealthPct() < 80.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_COMBAT &&
        dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }

    constexpr float searchRadius = 40.0f;
    Unit* voidSpawn = bot->FindNearestCreature(
        static_cast<uint32>(SunwellNpcs::NPC_VOID_SPAWN), searchRadius);
    if (voidSpawn && AI_VALUE(Unit*, "current target") == voidSpawn &&
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 0.0f;
    }

    if (AI_VALUE(Unit*, "current target") == muru)
        context->GetValue<bool>("neglect threat")->Set(true);

    if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel") &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float MuruControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return 1.0f;

    if (dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MuruControlMovementMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
    {
        return 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    // Remainder is checking only for validity of reach target actions
    if (!dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action))
    {
        return 1.0f;
    }

    if (botAI->IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        return 1.0f;
    }

    if (!TryGetMuruDarknessActiveState(bot, muru))
        return 1.0f;

    auto const isReachTargetSafeFromDarkness = [&](Action* action) -> bool
    {
        Unit* actionTarget = action->GetTarget();
        if (!actionTarget)
            return false;

        float const targetDistFromMuru = muru->GetExactDist2d(actionTarget);
        Position const refPosition = botAI->IsAssistTankOfIndex(bot, 1, true) ?
            MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
        float const targetDistFromRef = actionTarget->GetExactDist2d(
            refPosition.GetPositionX(), refPosition.GetPositionY());
        constexpr float targetDistThreshold = 20.0f;
        return targetDistFromMuru > targetDistThreshold &&
            targetDistFromRef < targetDistThreshold;
    };

    if (isReachTargetSafeFromDarkness(action))
        return 1.0f;

    if (botAI->IsTank(bot) && !TryGetMuruDarknessEarlyState(bot, muru))
        return 1.0f;
    else
        return 0.0f;
}

float MuruDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() < 95.0f)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (muru && muru->GetHealthPct() < 97.0f)
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenDelayCooldownsMultiplier::GetValue(Action* action)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if ((!kiljaeden && !AI_VALUE2(Unit*, "find target", "hand of the deceiver")) ||
        (kiljaeden && kiljaeden->GetHealthPct() < 25.0f)) // Phase 5
    {
        return 1.0f;
    }

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    if (kiljaeden && kiljaeden->GetHealthPct() < 85.0f) // Phase 3
        return 1.0f;

    if (IsDpsCooldownAction(action) ||
        (botAI->IsDps(bot) && dynamic_cast<UseTrinketAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KiljaedenTanksFocusAssignedHandOnlyMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action))
    {
        return 0.0f;
    }

    if (botAI->IsHeal(bot))
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    Player* secondAssistTank = GetGroupAssistTank(botAI, bot, 1);
    if (!mainTank || !firstAssistTank || !secondAssistTank)
        return 1.0f;

    if (botAI->IsDps(bot) && dynamic_cast<DpsAssistAction*>(action))
        return 0.0f;

    if (botAI->IsTank(bot) &&
        (dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) ||
         dynamic_cast<CastChallengingShoutAction*>(action) ||
         dynamic_cast<CastShockwaveAction*>(action) ||
         dynamic_cast<CastCleaveAction*>(action) ||
         dynamic_cast<CastGrowlAction*>(action) ||
         dynamic_cast<CastSwipeBearAction*>(action) ||
         dynamic_cast<CastChallengingRoarAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) ||
         dynamic_cast<CastRighteousDefenseAction*>(action) ||
         dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastDeathAndDecayAction*>(action) ||
         dynamic_cast<CastBloodBoilAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 0.0f;
    }

    if (!botAI->IsMainTank(bot))
        return 1.0f;

    if (botAI->GetState() == BOT_STATE_COMBAT &&
        dynamic_cast<TankAssistAction*>(action))
    {
        return 0.0f;
    }

    // Bots switch to non-combat engine and try to drink before KJ is attackable
    if (dynamic_cast<DrinkAction*>(action))
        return 0.0f;

    return 1.0f;
}

float KiljaedenPrioritizeDarknessProtectionMultiplier::GetValue(Action* action)
{
    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 1.0f;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) &&
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KiljaedenControlDragonMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (!HasKiljaedenDragonAura(bot))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (!dynamic_cast<KiljaedenControlDragonAction*>(action))
        return 0.0f;

    return 1.0f;
}
