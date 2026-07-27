/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

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
#include <ctime>

using namespace SwpHelpers;

namespace
{

bool IsDpsCooldownAction(Action* action, PlayerbotAI* botAI)
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
        dynamic_cast<CastBloodFuryAction*>(action) ||
        (dynamic_cast<UseTrinketAction*>(action) && PlayerbotAI::IsDps(botAI->GetBot()));
}

bool IsTauntAction(Action* action)
{
    return dynamic_cast<CastTauntAction*>(action) ||
        dynamic_cast<CastChallengingShoutAction*>(action) ||
        dynamic_cast<CastGrowlAction*>(action) ||
        dynamic_cast<CastChallengingRoarAction*>(action) ||
        dynamic_cast<CastHandOfReckoningAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ||
        dynamic_cast<CastDarkCommandAction*>(action) ||
        dynamic_cast<CastDeathGripAction*>(action);
}

}

// Kalecgos

float KalecgosControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "kalecgos"))
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

    if (!dynamic_cast<CastRemoveCurseAction*>(action) &&
        !dynamic_cast<CastRemoveCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    Unit* target = AI_VALUE2(Unit*, "party member to dispel", DISPEL_CURSE);
    if (!target)
        return 1.0f;

    Aura* aura = target->GetAura(static_cast<uint32>(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!aura)
        aura = target->GetAura(static_cast<uint32>(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC));

    if (aura && aura->GetDuration() >= 15000) // 15 seconds remaining
        return 0.0f;

    return 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    if (kalecgos && !kalecgos->IsFriendlyTo(bot))
        return 0.0f;

    return 1.0f;
}

float KalecgosRestrictTauntMultiplier::GetValue(Action* action)
{
    if (!IsTauntAction(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (IsInSpectralRealm(bot))
        return 1.0f;

    if (GetKalecgosCurrentTank(bot) != bot)
        return 0.0f;

    return 1.0f;
}

float KalecgosSuppressAssistTankPullThreatMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsAssistTank(bot))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    KalecgosEncounterState& state = kalecgosEncounterStates[bot->GetInstanceId()];
    constexpr uint32 pullThreatSuppressionMs = 5000;
    if (getMSTimeDiff(state.encounterStartMs, getMSTime()) < pullThreatSuppressionMs)
        return 0.0f;

    return 1.0f;
}

float KalecgosDelayCooldownsForSathrovarrMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(action, botAI))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!IsInSpectralRealm(bot))
        return 0.0f;

    return 1.0f;
}

// Brutallus

float BrutallusControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "brutallus"))
        return 0.0f;

    return 1.0f;
}

float BrutallusControlMovementMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "brutallus"))
        return 0.0f;

    return 1.0f;
}

// Don't use KS if any melee member (other than the Brutallus tanks) has Burn
float BrutallusNoKillingSpreeWhenNearbyBurnMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!dynamic_cast<CastKillingSpreeAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "brutallus"))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(static_cast<uint32>(SwpSpells::SPELL_BURN)) &&
            PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float BrutallusRestrictTauntMultiplier::GetValue(Action* action)
{
    if (!IsTauntAction(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "brutallus"))
        return 0.0f;

    return 1.0f;
}

float BrutallusDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(action, botAI))
        return 1.0f;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (brutallus && brutallus->GetHealthPct() > 95.0f)
        return 0.0f;

    return 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "felmyst"))
        return 0.0f;

    return 1.0f;
}

float FelmystWaitForLandingDpsMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<FelmystMisdirectBossToMainTankAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    auto& state = felmystEncounterStates[felmyst->GetMap()->GetInstanceId()];
    if (state.landingDpsWaitTimer != 0)
        return 0.0f;

    return 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    if (GetFelmystEncapsulateTarget(bot))
        return 0.0f;

    return 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValue(Action* action)
{
    bool const isReachOrDrink =
        dynamic_cast<CastReachTargetSpellAction*>(action) ||
        dynamic_cast<DrinkAction*>(action);

    bool const isBlockedMovement =
        dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action);

    bool const isFogLaneMove =
        dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action);

    bool const isDpsSpell =
        dynamic_cast<CastSpellAction*>(action) &&
        !dynamic_cast<CastHealingSpellAction*>(action);

    if (!isReachOrDrink && !isBlockedMovement && !isFogLaneMove && !isDpsSpell)
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FogOfCorruptionState fogState;
    FogLane thirdPassLane = FogLane::None;
    bool const shouldRepositionAfterThirdPass =
        TryGetFelmystPostThirdPassWindow(felmyst, thirdPassLane);

    if (!TryGetFelmystFogOfCorruptionStageState(felmyst, fogState) &&
        !shouldRepositionAfterThirdPass)
    {
        return 1.0f;
    }

    // Fog is active — always-blocked action types
    if (isReachOrDrink || isBlockedMovement)
        return 0.0f;

    // Remaining: fog lane move or DPS spell — these need danger state
    FogOfCorruptionState dangerousFogState;
    bool needsFogAvoidance = TryGetActiveFogOfCorruptionState(
        bot, felmyst, dangerousFogState);

    FogLane const activeLane = needsFogAvoidance ? dangerousFogState.lane :
        shouldRepositionAfterThirdPass ? thirdPassLane :
        fogState.lane;

    if (isFogLaneMove)
    {
        std::array<Position, 3> destinations;
        uint8 destinationCount = 0;
        bool canRelocate = TryGetFelmystFogSafeDestinations(
            bot, activeLane, destinations, destinationCount);
        return canRelocate ? 1.0f : 0.0f;
    }

    // DPS spell — blocked when in active danger and can relocate
    if (needsFogAvoidance)
    {
        std::array<Position, 3> destinations;
        uint8 destinationCount = 0;
        bool canRelocate = TryGetFelmystFogSafeDestinations(
            bot, dangerousFogState.lane, destinations, destinationCount);
        if (canRelocate)
            return 0.0f;
    }

    return 1.0f;
}

float FelmystPrioritizeDemonicVaporKiteMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<FelmystKiteDemonicVaporAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(bot, felmyst, fogState))
        return 1.0f;

    if (IsFelmystDemonicVaporHeadNearBot(bot))
        return 0.0f;

    return 1.0f;
}

float FelmystFocusAttacksOnCharmedPlayerMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<DropTargetAction*>(action))
    {
        return 1.0f;
    }

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    Player* charmedTarget = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedTarget)
        return 1.0f;

    if (PlayerbotAI::IsMelee(bot) && !felmyst->IsFlying() && bot->IsWithinMeleeRange(charmedTarget))
        return 0.0f;

    if (!PlayerbotAI::IsMelee(bot) && bot->GetDistance2d(charmedTarget) > 30.0f)
        return 0.0f;

    return 1.0f;
}

float FelmystDontDotAddsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    if (action->GetTarget() != felmyst)
        return 0.0f;

    return 1.0f;
}

float FelmystDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(action, botAI))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (felmyst && (felmyst->IsFlying() || felmyst->GetHealthPct() > 95.0f))
        return 0.0f;

    return 1.0f;
}

// Eredar Twins

float EredarTwinsDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 0.0f;

    return 1.0f;
}

float EredarTwinsControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 0.0f;

    return 1.0f;
}

float EredarTwinsHoldDpsAtStartMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<EredarTwinsMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (PlayerbotAI::IsMelee(bot) && bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    uint32 const instanceId = bot->GetInstanceId();
    time_t const now = std::time(nullptr);
    auto const it = eredarTwinsDpsHoldTimer.try_emplace(instanceId, now).first;
    constexpr uint8 dpsHoldSeconds = 8;

    if ((now - it->second) < dpsHoldSeconds)
        return 0.0f;

    return 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsDpsPrioritizeLadySacrolashAction*>(action))
        return 1.0f;

    if (!dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    constexpr float alythessThreatRatio = 0.9f;
    constexpr float sacrolashThreatRatio = 0.8f;

    bool const shouldHoldSacrolashThreat = sacrolash &&
        ShouldHoldTwinThreat(bot, sacrolash, sacrolashThreatRatio, IsAnySacrolashTank);
    bool const shouldHoldAlythessThreat = alythess &&
        ShouldHoldTwinThreat(bot, alythess, alythessThreatRatio, IsAlythessTank);

    if (!shouldHoldSacrolashThreat && !shouldHoldAlythessThreat)
        return 1.0f;

    Unit* actionTarget = action->GetTarget();
    bool const suppressSacrolashAttack = shouldHoldSacrolashThreat &&
        (actionTarget == sacrolash || AI_VALUE(Unit*, "current target") == sacrolash);
    bool const suppressAlythessAttack = shouldHoldAlythessThreat &&
        (actionTarget == alythess || AI_VALUE(Unit*, "current target") == alythess);

    if (suppressSacrolashAttack || suppressAlythessAttack)
        return 0.0f;

    return 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValue(Action* action)
{
    bool const isAlwaysBlocked =
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) ||
        dynamic_cast<CastKillingSpreeAction*>(action);

    bool const isTankAvoidAoe =
        PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action);

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isAlwaysBlocked && !isTankAvoidAoe && !isReachAction)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (isAlwaysBlocked || isTankAvoidAoe)
        return 0.0f;

    if (isReachAction && (PlayerbotAI::IsRanged(bot) || IsAlythessTank(bot)))
        return 0.0f;

    return 1.0f;
}

float EredarTwinsNoMovingIntoConflagrationMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<EredarTwinsConflagratedBotMoveFromGroupAction*>(action) ||
        dynamic_cast<EredarTwinsMoveFromConflagSacrolashVictimAction*>(action))
    {
        return 1.0f;
    }

    bool const isReachSpell = dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    Player* conflagTarget = GetEredarTwinsConflagrationTarget(bot);
    if (!conflagTarget)
        return 1.0f;

    // Block movement for bot targeted by Conflagration
    if (conflagTarget == bot)
        return 0.0f;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return 1.0f;

    // When Sacrolash's target is targeted by Conflagration, block actions that move toward them
    Unit* victim = sacrolash->GetVictim();
    if (!victim || victim == bot || conflagTarget != victim)
        return 1.0f;

    if (isReachSpell)
        return 0.0f;

    if (bot->GetDistance2d(victim) < 10.0f) // For MovementAction generally
        return 0.0f;

    return 1.0f;
}

float EredarTwinsDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (!IsDpsCooldownAction(action, botAI))
        return 1.0f;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return 1.0f;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (sacrolash && sacrolash->GetHealthPct() > 80.0f)
        return 0.0f;

    return 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    bool isDpsAssist =
        botAI->GetState() == BOT_STATE_COMBAT && dynamic_cast<DpsAssistAction*>(action);
    bool isTankAssist =
        botAI->GetState() == BOT_STATE_COMBAT && dynamic_cast<TankAssistAction*>(action);
    bool isCastDotOnAddSpell = dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);

    if (!isDpsAssist && !isTankAssist && !isCastDotOnAddSpell)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    if (isTankAssist && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        AI_VALUE2(Unit*, "find target", "void sentinel"))
    {
        return 0.0f;
    }

    constexpr float searchRadius = 40.0f;
    Unit* voidSpawn = bot->FindNearestCreature(
        static_cast<uint32>(SwpNpcs::NPC_VOID_SPAWN), searchRadius);
    if (isCastDotOnAddSpell && voidSpawn && AI_VALUE(Unit*, "current target") == voidSpawn)
        return 0.0f;

    return 1.0f;
}

float MuruControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "entropius"))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "m'uru"))
        return 0.0f;

    return 1.0f;
}

float MuruControlMovementMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    bool const isBlockedMovement =
        dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<CastDisengageAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action);

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isBlockedMovement && !isReachAction)
        return 1.0f;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    if (isBlockedMovement)
        return 0.0f;

    // Remainder is checking only for validity of reach actions

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
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
        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
        float const targetDistFromRef = actionTarget->GetExactDist2d(
            refPosition.GetPositionX(), refPosition.GetPositionY());
        constexpr float targetDistThreshold = 20.0f;

        return targetDistFromMuru > targetDistThreshold &&
            targetDistFromRef < targetDistThreshold;
    };

    if (isReachTargetSafeFromDarkness(action))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) && !TryGetMuruDarknessEarlyState(bot, muru))
        return 1.0f;
    else
        return 0.0f;
}

float MuruDelayCooldownsMultiplier::GetValue(Action* action)
{
    bool const isLustSpell = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action));

    bool const isDpsCooldown = IsDpsCooldownAction(action, botAI);

    if (!isLustSpell && !isDpsCooldown)
        return 1.0f;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() < 95.0f)
        return 1.0f;

    if (isLustSpell)
        return 0.0f;

    if (muru->GetHealthPct() < 97.0f)
        return 1.0f;

    if (isDpsCooldown)
        return 0.0f;

    return 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenDelayCooldownsMultiplier::GetValue(Action* action)
{
    bool const isLustSpell = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) ||
         dynamic_cast<CastBloodlustAction*>(action));

    bool const isDpsCooldown = IsDpsCooldownAction(action, botAI);

    if (!isLustSpell && !isDpsCooldown)
        return 1.0f;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden && !AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
        return 1.0f;

    if (kiljaeden && kiljaeden->GetHealthPct() < 25.0f) // Phase 5
        return 1.0f;

    if (isLustSpell)
        return 0.0f;

    if (kiljaeden && kiljaeden->GetHealthPct() < 85.0f) // Phase 3
        return 1.0f;

    if (isDpsCooldown)
        return 0.0f;

    return 1.0f;
}

float KiljaedenTanksFocusAssignedHandOnlyMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    bool const isHighAggroAction =
        IsTauntAction(action) ||
        dynamic_cast<CastShockwaveAction*>(action) ||
        dynamic_cast<CastCleaveAction*>(action) ||
        dynamic_cast<CastSwipeBearAction*>(action) ||
        dynamic_cast<CastDeathAndDecayAction*>(action) ||
        dynamic_cast<CastBloodBoilAction*>(action);

    if (!isHighAggroAction && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) && !dynamic_cast<DpsAssistAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
        return 1.0f;

    // Apply this multiplier only if there are 3 bot tanks
    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || !GET_PLAYERBOT_AI(mainTank))
        return 1.0f;

    Player* firstAssistTank = GetGroupAssistTank(botAI, bot, 0);
    if (!firstAssistTank || !GET_PLAYERBOT_AI(firstAssistTank))
        return 1.0f;

    Player* secondAssistTank = GetGroupAssistTank(botAI, bot, 1);
    if (!secondAssistTank || !GET_PLAYERBOT_AI(secondAssistTank))
        return 1.0f;

    return 0.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValue(Action* action)
{
    bool const isMainTankAssist =
        botAI->GetState() == BOT_STATE_COMBAT && PlayerbotAI::IsMainTank(bot) &&
        dynamic_cast<TankAssistAction*>(action);

    if (!isMainTankAssist &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    if (AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 0.0f;

    return 1.0f;
}

float KiljaedenPrioritizeDarknessProtectionMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<AttackAction*>(action))
    {
        return 1.0f;
    }

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 1.0f;

    if (IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden))
        return 0.0f;

    return 1.0f;
}

float KiljaedenControlDragonMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    if (dynamic_cast<KiljaedenControlDragonAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 0.0f;

    return 1.0f;
}
