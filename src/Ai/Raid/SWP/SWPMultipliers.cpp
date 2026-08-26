/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPMultipliers.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "SWPActions.h"
#include "SWPSharedConstants.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "Timer.h"
#include "WipeAction.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

// Kalecgos

float KalecgosControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "kalecgos") ? 0.0f : 1.0f;
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

    // Like Illidan's Shadowfiends, the spread from player-to-player is a separate spell
    Aura* aura = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!aura)
        aura = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC));

    return aura && aura->GetDuration() >= KALECGOS_DISPEL_REMAINING_MS ? 0.0f : 1.0f;
}

float KalecgosControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* kalecgos = AI_VALUE2(Unit*, "find target", "kalecgos");
    return kalecgos && !kalecgos->IsFriendlyTo(bot) ? 0.0f : 1.0f;
}

// Avoid dueling taunts in the surface and spectral realms
float KalecgosRestrictTauntMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!IsInSpectralRealm(bot))
        return FindKalecgosDesignatedTank(bot) == bot ? 1.0f : 0.0f;

    Unit* sathrovarr = AI_VALUE2(Unit*, "find target", "sathrovarr the corruptor");
    if (!sathrovarr)
        return 1.0f;

    Unit* victim = sathrovarr->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    return victimPlayer && PlayerbotAI::IsTank(victimPlayer) ? 0.0f : 1.0f;
}

float KalecgosSuppressAssistTankPullThreatMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!PlayerbotAI::IsAssistTank(bot))
        return 1.0f;

    auto const stateItr = kalecgosEncounterStates.find(bot->GetInstanceId());
    if (stateItr == kalecgosEncounterStates.end() || !stateItr->second.encounterStartMs)
        return 1.0f;

    return getMSTimeDiff(stateItr->second.encounterStartMs, getMSTime()) <
        KALECGOS_PULL_THREAT_SUPPRESSION_MS ? 0.0f : 1.0f;
}

float KalecgosEnterSpectralRiftMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KalecgosEnterSpectralRiftAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    if (!ShouldEnterKalecgosPortal(bot))
        return 1.0f;

    return botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift")) ? 0.0f : 1.0f;
}

float KalecgosDelayCooldownsForSathrovarrMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "kalecgos"))
        return 1.0f;

    return IsInSpectralRealm(bot) ? 1.0f : 0.0f;
}

// Brutallus

float BrutallusControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "brutallus") ? 0.0f : 1.0f;
}

float BrutallusControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "brutallus") ? 0.0f : 1.0f;
}

// Don't use KS if any melee member (other than the Brutallus tanks) has Burn
float BrutallusNoKillingSpreeWhenNearbyBurnMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

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
        if (!member || !member->HasAura(Id(SwpSpells::SPELL_BURN)))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float BrutallusRestrictTauntMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return 1.0f;

    Unit* victim = brutallus->GetVictim();
    if (!victim)
        return 1.0f;

    Player* playerVictim = victim->ToPlayer();
    return playerVictim && PlayerbotAI::IsTank(playerVictim) ? 0.0f : 1.0f;
}

float BrutallusDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* brutallus = AI_VALUE2(Unit*, "find target", "brutallus");
    if (!brutallus)
        return 1.0f;

    return brutallus->GetHealthPct() > SWP_PULL_COMPLETE_HP_PERCENT ? 0.0f : 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "felmyst") ? 0.0f : 1.0f;
}

float FelmystWaitForLandingDpsMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    if (PlayerbotAI::IsMainTank(bot))
        return 1.0f;

    auto const stateItr = felmystEncounterStates.find(felmyst->GetInstanceId());
    return stateItr != felmystEncounterStates.end() && stateItr->second.landingDpsWaitStartMs ?
        0.0f : 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    return GetFelmystEncapsulateTarget(bot) ? 0.0f : 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return IsFelmystFogMovementSuppressed(felmyst) ? 0.0f : 1.0f;
}

float FelmystPrioritizeDemonicVaporAvoidanceMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    if (IsFelmystFogActiveForBot(bot, felmyst))
        return 1.0f;

    return IsFelmystLanding(felmyst) ? 1.0f : 0.0f;
}

float FelmystFocusAttacksOnCharmedPlayerMultiplier::GetValue(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    // The charmed player is still friendly to group members so is considered to be an
    // invalid target by bots; blocking "drop target" allows them to be attacked
    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    Player* charmedPlayer = GetFelmystCharmedTarget(bot, felmyst);
    if (!charmedPlayer)
        return 1.0f;

    // Melee: attack only during flight phase when the charmed player is in melee range
    if (PlayerbotAI::IsMelee(bot) &&
        (!felmyst->IsFlying() || !bot->IsWithinMeleeRange(charmedPlayer)))
    {
        return 0.0f;
    }

    // Ranged: attack at any time the charmed player is in general spell range
    return PlayerbotAI::IsRanged(bot) &&
        bot->GetExactDist2d(charmedPlayer) < FELMYST_CHARMED_TARGET_RANGE ? 0.0f : 1.0f;
}

float FelmystDontDotAddsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return action->GetTarget() == felmyst ? 1.0f : 0.0f;
}

float FelmystDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (!felmyst)
        return 1.0f;

    if (felmyst->IsFlying())
        return 0.0f;

    return felmyst->GetHealthPct() > SWP_PULL_COMPLETE_HP_PERCENT ? 0.0f : 1.0f;
}

// Eredar Twins

float EredarTwinsDisableAutomaticTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") ? 0.0f : 1.0f;
}

float EredarTwinsControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "grand warlock alythess") ? 0.0f : 1.0f;
}

float EredarTwinsHoldDpsAtStartMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    // No AttackAction block. Commencing auto-attack gets bots positioned, but don't use abilities.
    if (!dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsMisdirectBossesToTanksAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsMelee(bot) && bot->GetPositionZ() > EREDAR_TWINS_BALCONY_Z)
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "lady sacrolash"))
        return 1.0f;

    auto const it = eredarTwinsDpsHoldStartMs.find(bot->GetInstanceId());
    if (it == eredarTwinsDpsHoldStartMs.end())
        return 0.0f;

    return getMSTimeDiff(it->second, getMSTime()) < EREDAR_TWINS_DPS_HOLD_MS ? 0.0f : 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValue(Action* action)
{
    if (PlayerbotAI::IsHeal(bot)) // early return; already excluded from ShouldHoldTwinThreat()
        return 1.0f;

    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsDpsPrioritizeSacrolashAction*>(action))
        return 1.0f;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");

    bool const shouldHoldSacrolashThreat = sacrolash && !PlayerbotAI::IsTank(bot) &&
        ShouldHoldTwinThreat(bot, sacrolash, SACROLASH_THREAT_HOLD_RATIO, IsAnySacrolashTank);
    bool const shouldHoldAlythessThreat = alythess &&
        ShouldHoldTwinThreat(bot, alythess, ALYTHESS_THREAT_HOLD_RATIO, IsAlythessTank);

    if (!shouldHoldSacrolashThreat && !shouldHoldAlythessThreat)
        return 1.0f;

    Unit* actionTarget = action->GetTarget();
    bool const suppressSacrolashAttack = shouldHoldSacrolashThreat &&
        (actionTarget == sacrolash || AI_VALUE(Unit*, "current target") == sacrolash);
    bool const suppressAlythessAttack = shouldHoldAlythessThreat &&
        (actionTarget == alythess || AI_VALUE(Unit*, "current target") == alythess);

    return suppressSacrolashAttack || suppressAlythessAttack ? 0.0f : 1.0f;
}

float EredarTwinsControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachAction &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !dynamic_cast<CastKillingSpreeAction*>(action) &&
        !(PlayerbotAI::IsTank(bot) && dynamic_cast<AvoidAoeAction*>(action)))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    if (!isReachAction)
        return 0.0f;

    return PlayerbotAI::IsRanged(bot) || IsAlythessTank(bot) ? 0.0f : 1.0f;
}

float EredarTwinsIsolateConflagrationMultiplier::GetValue(Action* action)
{
    bool const isReachTargetSpell = dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachTargetSpell && !dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsConflagrationTargetMoveFromGroupAction*>(action) ||
        dynamic_cast<EredarTwinsMoveAwayFromSacrolashVictimAction*>(action))
    {
        return 1.0f;
    }

    if (!AI_VALUE2(Unit*, "find target", "grand warlock alythess"))
        return 1.0f;

    Player* conflagTarget = GetEredarTwinsConflagrationTarget(bot);
    if (!conflagTarget)
        return 1.0f;

    // Block movement for bot targeted by Conflagration, unless the target is a Rogue that has
    // vanished and caused Alythess to drop the target.
    if (conflagTarget == bot)
        return bot->getClass() == CLASS_ROGUE && botAI->HasAura("vanish", bot) ? 1.0f : 0.0f;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return 1.0f;

    // If Sacrolash's victim is targeted by Conflagration, block actions that move toward Sacrolash.
    Unit* victim = sacrolash->GetVictim();
    if (!victim || victim == bot || conflagTarget != victim)
        return 1.0f;

    if (isReachTargetSpell)
        return 0.0f;

    // Block movement actions generally when too close to the Conflagration target.
    return bot->GetExactDist2d(victim) < CONFLAGRATION_SAFE_DISTANCE ? 0.0f : 1.0f;
}

float EredarTwinsDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* alythess = AI_VALUE2(Unit*, "find target", "grand warlock alythess");
    if (!alythess)
        return 1.0f;

    Unit* sacrolash = AI_VALUE2(Unit*, "find target", "lady sacrolash");
    if (!sacrolash)
        return 1.0f;

    return sacrolash->GetHealthPct() > MAX_DPS_HP_PERCENT ? 0.0f : 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);
    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);

    if (!isDpsAssist && !isTankAssist && !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
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

    // Disable secondary dots on void spawn
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    return currentTarget && currentTarget->GetEntry() == Id(SwpNpcs::NPC_VOID_SPAWN) ? 0.0f : 1.0f;
}

float MuruControlMisdirectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "entropius"))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "m'uru") ? 0.0f : 1.0f;
}

float MuruControlMovementMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachAction &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    if (!isReachAction)
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
        float const targetDistFromRef = actionTarget->GetExactDist2d(refPosition);

        return targetDistFromMuru > DARKNESS_SAFE_DISTANCE &&
            targetDistFromRef < MURU_HOLDING_POSITION_RADIUS;
    };

    if (isReachTargetSafeFromDarkness(action))
        return 1.0f;

    return PlayerbotAI::IsTank(bot) && !TryGetMuruDarknessEarlyState(bot, muru) ? 1.0f : 0.0f;
}

float MuruDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru)
        return 1.0f;

    Unit* entropius = AI_VALUE2(Unit*, "find target", "entropius");
    if (entropius && entropius->GetHealthPct() < SWP_PULL_COMPLETE_HP_PERCENT)
        return 1.0f;

    // Bloodlust is saved for Entropius
    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    // Other dps cooldowns can be used on M'uru after the pull
    return muru->GetHealthPct() > MURU_MAX_DPS_HP_PERCENT ? 0.0f : 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenDelayCooldownsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
        return 0.0f;

    if (kiljaeden->GetHealthPct() <= KILJAEDEN_PHASE5_HP_THRESHOLD)
        return 1.0f;

    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action)))
    {
        return 0.0f;
    }

    return kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD ? 0.0f : 1.0f;
}

float KiljaedenTanksFocusAssignedHandOnlyMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) &&
        !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !IsTauntAction(bot, action) && !IsAoeThreatAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!AI_VALUE2(Unit*, "find target", "hand of the deceiver"))
        return 1.0f;

    return HasAtLeastThreeBotTanks(bot) ? 0.0f : 1.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastDisengageAction*>(action) &&
        !dynamic_cast<CastBlinkBackAction*>(action) &&
        !(dynamic_cast<TankAssistAction*>(action) && PlayerbotAI::IsMainTank(bot)))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "kil'jaeden") ? 0.0f : 1.0f;
}

float KiljaedenPrioritizeDarknessProtectionMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
        return 1.0f;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden)
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 1.0f;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ? 0.0f : 1.0f;
}

float KiljaedenControlDragonMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return 1.0f;

    if (dynamic_cast<KiljaedenDragonBuffAndProtectRaidAction*>(action))
        return 1.0f;

    if (dynamic_cast<WipeAction*>(action))
        return 1.0f;

    return HasKiljaedenDragonAura(bot) ? 0.0f : 1.0f;
}
