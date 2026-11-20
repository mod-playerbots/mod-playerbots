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
#include "FoodActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "InstanceScript.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "SWPActions.h"
#include "SWPEncounter_Brut.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "ThreatHelpers.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "WipeAction.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

// Shared

namespace
{

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
        (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

bool IsKillingSpree(Player* bot, Action* action)
{
    return bot->getClass() == CLASS_ROGUE && dynamic_cast<CastKillingSpreeAction*>(action);
}

} // end anonymous namespace (Shared)

// Without this, bots are able to drink after Kil'jaeden's Hands go down. I assume it's due to the
// issue reported here: https://github.com/azerothcore/azerothcore-wotlk/issues/22122.
float SunwellNoEncounterDrinkingMultiplier::GetValueInEncounter(Action* action)
{
    return dynamic_cast<DrinkAction*>(action) || dynamic_cast<EatAction*>(action) ? 0.0f : 1.0f;
}

// Trash

float VolatileFiendRestrictApproachMultiplier::GetValue(Action* action)
{
    if (IsEncounterInProgress(bot, SwpHelpers::SWP_MAP_ID))
        return 1.0f;

    if (!dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action))
    {
        return 1.0f;
    }

    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    Creature* volatileFiend = botAI->GetCreature(AI_VALUE(ObjectGuid, "swp volatile fiend"));
    if (!volatileFiend || !volatileFiend->IsAlive())
        return 1.0f;

    return bot->GetExactDist(volatileFiend) < VOLATILE_FIEND_APPROACH_SUPPRESSION_RADIUS ?
        0.0f : 1.0f;
}

// Shared Boss

float SunwellControlMisdirectionMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER)
        return 1.0f;

    if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action))
        return 1.0f;

    bool isHeldBoss = false;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit)
            continue;

        switch (unit->GetEntry())
        {
            case Id(SwpNpcs::NPC_ENTROPIUS):
                return 1.0f;
            case Id(SwpNpcs::NPC_KALECGOS_DRAGON):
            case Id(SwpNpcs::NPC_BRUTALLUS):
            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
            case Id(SwpNpcs::NPC_MURU):
            case Id(SwpNpcs::NPC_KILJAEDEN):
                isHeldBoss = true;
                break;
            default:
                break;
        }
    }

    return isHeldBoss ? 0.0f : 1.0f;
}

float SunwellDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    std::optional<uint32> const bossId = GetBossInProgress(bot, SWP_MAP_ID);
    if (!bossId)
        return 1.0f;

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastHeroismAction*>(action) || dynamic_cast<CastBloodlustAction*>(action));

    // If the boss cannot be found but the encounter is in progress, it is probably because the
    // bot was rezzed and zone combat hasn't reapplied, so block cooldowns.
    switch (static_cast<SwpBosses>(*bossId))
    {
        case SwpBosses::DATA_KILJAEDEN:
        {
            // Kil'jaeden is summoned only when the last Hand dies.
            if (FindThreatTarget(bot, "hand of the deceiver"))
                return 0.0f;

            Unit* kiljaeden = GetSwpBoss(bot, SwpBosses::DATA_KILJAEDEN);
            if (!kiljaeden)
                return 0.0f;

            if (kiljaeden->GetHealthPct() <= KILJAEDEN_PHASE5_HP_THRESHOLD)
                return 1.0f;

            if (isBloodlust)
                return 0.0f;

            return kiljaeden->GetHealthPct() > KILJAEDEN_PHASE3_HP_THRESHOLD ? 0.0f : 1.0f;
        }
        case SwpBosses::DATA_MURU:
        {
            Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
            if (!muru)
                return 0.0f;

            Unit* entropius = FindThreatTarget(bot, "entropius");
            if (entropius && entropius->GetHealthPct() < BOSS_ENGAGED_HEALTH_PCT)
                return 1.0f;

            if (isBloodlust) // Bloodlust is saved for Entropius.
                return 0.0f;

            // Other dps cooldowns can be used on M'uru after the pull.
            return muru->GetHealthPct() > MURU_MAX_DPS_HP_PERCENT ? 0.0f : 1.0f;
        }
        case SwpBosses::DATA_EREDAR_TWINS:
        {
            if (!FindAlythess(bot))
                return 0.0f;

            Unit* sacrolash = FindSacrolash(bot);
            return sacrolash && sacrolash->GetHealthPct() > EREDAR_TWINS_MAX_DPS_HP_PERCENT ?
                0.0f : 1.0f;
        }
        case SwpBosses::DATA_FELMYST:
        {
            Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
            if (!felmyst || felmyst->IsFlying())
                return 0.0f;

            return felmyst->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }
        case SwpBosses::DATA_BRUTALLUS:
        {
            Unit* brutallus = GetSwpBoss(bot, SwpBosses::DATA_BRUTALLUS);
            return !brutallus || brutallus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ?
                0.0f : 1.0f;
        }
        case SwpBosses::DATA_KALECGOS:
        {
            if (!GetSwpBoss(bot, SwpBosses::DATA_KALECGOS))
                return 0.0f;

            // Use cooldowns against Sathrovarr only, and not while he is held.
            return IsInSpectralRealm(bot) &&
                GetKalecgosDpsHoldWindow(bot) == KalecgosDpsHold::None ? 1.0f : 0.0f;
        }
        default:
            return 1.0f;
    }
}

// Kalecgos

float KalecgosWaitToDecurseMultiplier::GetValueInEncounter(Action* action)
{
    uint8 const botClass = bot->getClass();
    if (botClass != CLASS_DRUID && botClass != CLASS_MAGE && botClass != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastRemoveCurseAction*>(action) &&
        !dynamic_cast<CastRemoveCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritAction*>(action) &&
        !dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) &&
        !dynamic_cast<CastDruidRemoveCurseOnPartyAction*>(action))
    {
        return 1.0f;
    }

    Unit* target = dynamic_cast<CurePartyMemberAction*>(action) ?
        action->GetTarget() : AI_VALUE2(Unit*, "party member to dispel", DISPEL_MAGIC);
    if (!target)
        return 1.0f;

    // Like with Illidan's Shadowfiends, the spread from player-to-player is a separate spell.
    Aura* curse = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY));
    if (!curse)
        curse = target->GetAura(Id(SwpSpells::SPELL_CURSE_OF_BOUNDLESS_AGONY_SEC));

    return curse && curse->GetDuration() >= KALECGOS_DISPEL_REMAINING_MS ? 0.0f : 1.0f;
}

float KalecgosControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* kalecgos = GetEncounterBoss();
    return kalecgos && !kalecgos->IsFriendlyTo(bot) ? 0.0f : 1.0f;
}

// Avoid dueling taunts in the surface and spectral realms
float KalecgosRestrictTauntMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!IsTauntAction(bot, action))
        return 1.0f;

    if (!IsInSpectralRealm(bot))
        return FindKalecgosDesignatedTank(bot) == bot ? 1.0f : 0.0f;

    Unit* sathrovarr = FindThreatTarget(bot, "sathrovarr the corruptor");
    if (!sathrovarr)
        return 1.0f;

    Unit* victim = sathrovarr->GetVictim();
    Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
    return victimPlayer && PlayerbotAI::IsTank(victimPlayer) ? 0.0f : 1.0f;
}

float KalecgosSuppressAssistTankPullThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsAssistTank(bot))
        return 1.0f;

    // Until the tracker stamps the start time, the pull has only just happened.
    KalecgosEncounterState const* state = FindKalecgosState(bot->GetInstanceId());
    if (!state || !state->encounterStartMs)
        return 0.0f;

    return getMSTimeDiff(*state->encounterStartMs, getMSTime()) <
        KALECGOS_PULL_THREAT_SUPPRESSION_MS ? 0.0f : 1.0f;
}

float KalecgosEnterSpectralRiftMultiplier::GetValueInEncounter(Action* action)
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

    if (!ShouldEnterKalecgosPortal(bot))
        return 1.0f;

    return botAI->GetGameObject(AI_VALUE(ObjectGuid, "kalecgos spectral rift")) ? 0.0f : 1.0f;
}

float KalecgosHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    return GetKalecgosDpsHold(bot) != KalecgosDpsHold::None ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Brutallus

float BrutallusControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    // Melee positions are all within reach of Brutallus, so melee can be allowed to use reach
    // target. The exception is the active tank because it needs to temporarily back out of melee
    // range to pull Brutallus into the designated position.
    if (dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        if (!PlayerbotAI::IsMelee(bot))
            return 0.0f;

        Unit* brutallus = GetEncounterBoss();
        return brutallus && brutallus->GetVictim() == bot ? 0.0f : 1.0f;
    }

    return dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action) ? 0.0f : 1.0f;
}

// Don't use KS if any melee member (other than the Brutallus tanks) has Burn
float BrutallusNoKillingSpreeWhenNearbyBurnMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_ROGUE)
        return 1.0f;

    if (!IsKillingSpree(bot, action))
        return 1.0f;

    Group* group = bot->GetGroup();
    if (!group)
        return 1.0f;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !HasBrutallusBurn(member))
            continue;

        if (PlayerbotAI::IsMelee(member) && !PlayerbotAI::IsMainTank(member) &&
            !PlayerbotAI::IsAssistTankOfIndex(member, 0, true))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float BrutallusRestrictTauntMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsTauntAction(bot, action))
        return 1.0f;

    Unit* brutallus = GetEncounterBoss();
    if (!brutallus)
        return 1.0f;

    Unit* victim = brutallus->GetVictim();
    if (!victim)
        return 1.0f;

    Player* playerVictim = victim->ToPlayer();
    return playerVictim && PlayerbotAI::IsTank(playerVictim) ? 0.0f : 1.0f;
}

float BrutallusNoRebirthWhileBurningMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_DRUID)
        return 1.0f;

    if (!HasBrutallusBurn(bot))
        return 1.0f;

    return dynamic_cast<CastRebirthAction*>(action) ? 0.0f : 1.0f;
}

// Felmyst

float FelmystControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    return dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FleeAction*>(action) || IsRepositionAction(bot, action) ? 0.0f : 1.0f;
}

float FelmystWaitForLandingDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    return IsFelmystLandingDpsHoldActive(botAI) ? GetDpsHoldValue(bot, action) : 1.0f;
}

float FelmystPrioritizeEncapsulateAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<FelmystRunAwayFromEncapsulatedPlayerAction*>(action))
        return 1.0f;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || felmyst->IsFlying())
        return 1.0f;

    Player* encapsulateTarget = GetFelmystEncapsulateTarget(bot);
    if (!encapsulateTarget)
        return 1.0f;

    // Healers may move to heal the target: the reach stops about 40 yd away, clear of its 20 yd.
    return dynamic_cast<ReachPartyMemberToHealAction*>(action) &&
        action->GetTarget() == encapsulateTarget ? 1.0f : 0.0f;
}

// Healers at the side stacks would otherwise keep reaching for players in the other side stack.
// This does not impact healers at the rear stack with melee.
float FelmystKeepHealersInSideStacksMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachPartyMemberToHealAction*>(action))
        return 1.0f;

    return IsFelmystRangedStackActive(bot, GetEncounterBoss()) ? 0.0f : 1.0f;
}

float FelmystPrioritizeFogAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<FelmystMoveToSafeFogLaneAction*>(action))
        return 1.0f;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return IsFelmystFogMovementSuppressed(felmyst) ? 0.0f : 1.0f;
}

float FelmystPrioritizeDemonicVaporAvoidanceMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    if (IsFelmystFogActiveForBot(bot, felmyst))
        return 1.0f;

    return IsFelmystLanding(felmyst) ? 1.0f : 0.0f;
}

float FelmystFocusAttacksOnCharmedPlayerMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    // The charmed player is still friendly to group members so is considered to be an
    // invalid target by bots; blocking "drop target" allows them to be attacked
    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    Unit* felmyst = GetEncounterBoss();
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

float FelmystDisableDotsOnAddsMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    Unit* felmyst = GetEncounterBoss();
    if (!felmyst || !felmyst->IsFlying())
        return 1.0f;

    return action->GetTarget() == felmyst ? 1.0f : 0.0f;
}

// Eredar Twins

// The Twins actions pick every bot's target. Drop target would also send a bot whose target died
// or went out of sight to its non-combat engine (a shadow priest drops Shadowform there). The twins
// are immune to taunts: the bots' own immunity check stops the others, but Righteous Defense is
// cast on a friendly, so it would pass and waste its cooldown.
float EredarTwinsDisableAutoTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    return dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        dynamic_cast<DropTargetAction*>(action) ||
        dynamic_cast<CastRighteousDefenseAction*>(action) ? 0.0f : 1.0f;
}

float EredarTwinsHoldDpsAtStartMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) ||
        dynamic_cast<EredarTwinsDpsPrioritizeSacrolashAction*>(action))
    {
        return 1.0f;
    }

    return IsEredarTwinsStartHoldActive(botAI) ? GetDpsHoldValue(bot, action) : 1.0f;
}

float EredarTwinsControlThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsHeal(bot))
        return 1.0f;

    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    if (dynamic_cast<EredarTwinsDpsPrioritizeSacrolashAction*>(action))
        return 1.0f;

    EredarTwinsThreatHolds const holds =
        AI_VALUE(EredarTwinsThreatHolds, "eredar twins threat holds");
    if (holds.sacrolash.IsEmpty() && holds.alythess.IsEmpty())
        return 1.0f;

    auto const isHeldTwin = [&holds](Unit* unit)
    {
        return unit && (unit->GetGUID() == holds.sacrolash || unit->GetGUID() == holds.alythess);
    };

    if (!isHeldTwin(action->GetTarget()) && !isHeldTwin(AI_VALUE(Unit*, "current target")))
        return 1.0f;

    return GetDpsHoldValue(bot, action);
}

// Avoid-AoE is off: the only hazards are the Blaze patches, which land at the Alythess tank and
// step-aside positions. The tanks and a bot holding Alythess stand in their own patches, and the
// moves to the stacks pass them outside Burn but inside avoid-AoE's flee distance.
float EredarTwinsControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    // Killing Spree would carry a Conflagration target back to the group, or a rogue in phase 2
    // onto the Blaze patches; on Sacrolash it reaches nothing else.
    if (IsKillingSpree(bot, action))
    {
        Unit* sacrolash = FindSacrolash(bot);
        return !sacrolash || IsEredarTwinsConflagrationTarget(bot, FindAlythess(bot), sacrolash) ?
            0.0f : 1.0f;
    }

    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    // Follow too: every bot has its own position all fight, and follow would take one in its
    // non-combat engine back to the group.
    if (!isReachAction &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<AvoidAoeAction*>(action) && !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (!isReachAction)
        return 0.0f;

    // In phase 2 the stack is in Alythess's melee range, and a reach or charge at her would cross
    // the Alythess tank positions.
    return PlayerbotAI::IsRanged(bot) || IsAlythessTank(bot) || !FindSacrolash(bot) ? 0.0f : 1.0f;
}

float EredarTwinsIsolateConflagrationMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<EredarTwinsConflagrationTargetMoveFromGroupAction*>(action) ||
        dynamic_cast<EredarTwinsMoveAwayFromSacrolashVictimAction*>(action))
    {
        return 1.0f;
    }

    Unit* sacrolash = FindSacrolash(bot);
    ObjectGuid const conflagTargetGuid =
        GetEredarTwinsConflagrationTargetGuid(FindAlythess(bot), sacrolash);
    if (conflagTargetGuid.IsEmpty())
        return 1.0f;

    // Block movement for the bot targeted by Conflagration.
    if (conflagTargetGuid == bot->GetGUID())
        return 0.0f;

    if (IsAlythessTank(bot))
        return 1.0f;

    // If Sacrolash's victim is targeted by Conflagration, block actions that move toward Sacrolash.
    if (!sacrolash)
        return 1.0f;

    Unit* victim = sacrolash->GetVictim();
    return victim && victim->GetGUID() == conflagTargetGuid ? 0.0f : 1.0f;
}

// M'uru

float MuruDisableDefaultTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);
    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);

    if (!isDpsAssist && !isTankAssist && !dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
        return 1.0f;

    if (isDpsAssist)
        return 0.0f;

    if (isTankAssist && PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        FindThreatTarget(bot, "void sentinel"))
    {
        return 0.0f;
    }

    // Disable secondary dots on void spawn
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    return currentTarget && currentTarget->GetEntry() == Id(SwpNpcs::NPC_VOID_SPAWN) ? 0.0f : 1.0f;
}

float MuruControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isReachAction =
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);

    if (!isReachAction &&
        !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    Unit* muru = GetEncounterBoss();
    if (!muru)
        return 1.0f;

    if (!isReachAction)
        return 0.0f;

    // Remainder is checking only for validity of reach actions

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) &&
        FindThreatTarget(bot, "void sentinel"))
    {
        return 1.0f;
    }

    if (!IsMuruDarknessActive(muru))
        return 1.0f;

    auto const isReachTargetSafeFromDarkness = [&](Action* action) -> bool
    {
        Unit* actionTarget = action->GetTarget();
        if (!actionTarget)
            return false;

        Position const& refPosition = PlayerbotAI::IsAssistTankOfIndex(bot, 1, true) ?
            MURU_ENTRANCE_POSITION : MURU_STACK_POSITION;
        float const targetDistFromRef = actionTarget->GetExactDist2d(refPosition);

        return !IsMeleeTargetInMuruDarkness(muru, actionTarget) &&
            targetDistFromRef < MURU_HOLDING_POSITION_RADIUS;
    };

    if (isReachTargetSafeFromDarkness(action))
        return 1.0f;

    return PlayerbotAI::IsTank(bot) && !IsMuruDarknessEarly(muru) ? 1.0f : 0.0f;
}

// Only attacks on Entropius are held; adds may still be attacked.
float MuruHoldDpsOnEntropiusMultiplier::GetValueInEncounter(Action* action)
{
    if (PlayerbotAI::IsTank(bot))
        return 1.0f;

    if (!IsDpsHoldCandidate(bot, action) || dynamic_cast<MuruAssignDpsPriorityAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!target || target->GetEntry() != Id(SwpNpcs::NPC_ENTROPIUS))
        return 1.0f;

    return IsEntropiusDpsHoldActive(bot) ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Standard Purge and Tranquilizing Shot would otherwise take Spell Fury before a mage steals it.
float MuruLetMagesStealSpellFuryMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastPurgeAction*>(action) &&
        !dynamic_cast<CastTranquilizingShotAction*>(action))
    {
        return 1.0f;
    }

    Unit* target = action->GetTarget();
    if (!target || target->GetEntry() != Id(SwpNpcs::NPC_SHADOWSWORD_FURY_MAGE))
        return 1.0f;

    return IsSpellFuryReservedForMages(target) ? 0.0f : 1.0f;
}

// Kil'jaeden <The Deceiver>

float KiljaedenSingleTargetHandsMultiplier::GetValueInEncounter(Action* action)
{
    // The only Shaman spell classified as ActionThreatType::Aoe is Chain Lightning, which is a
    // strong single-target spell in addition to providing AoE damage.
    if (bot->getClass() == CLASS_SHAMAN)
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot))
        return 1.0f;

    if (!dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) &&
        action->getThreatType() != Action::ActionThreatType::Aoe)
    {
        return 1.0f;
    }

    return FindThreatTarget(bot, "hand of the deceiver") ? 0.0f : 1.0f;
}

float KiljaedenControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    return dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        IsRepositionAction(bot, action) ||
        (dynamic_cast<TankAssistAction*>(action) && PlayerbotAI::IsMainTank(bot)) ? 0.0f : 1.0f;
}

float KiljaedenPrioritizeDarknessProtectionMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (dynamic_cast<KiljaedenStackForShieldOfTheBlueAction*>(action))
        return 1.0f;

    Unit* kiljaeden = GetEncounterBoss();
    if (!kiljaeden)
        return 1.0f;

    if (HasKiljaedenDragonAura(bot))
        return 1.0f;

    return IsKiljaedenCastingDarknessOfAThousandSouls(kiljaeden) ? 0.0f : 1.0f;
}

float KiljaedenControlDragonMultiplier::GetValueInEncounter(Action* action)
{
    if (!HasKiljaedenDragonAura(bot))
        return 1.0f;

    if (dynamic_cast<KiljaedenDragonBuffAndProtectRaidAction*>(action))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}
