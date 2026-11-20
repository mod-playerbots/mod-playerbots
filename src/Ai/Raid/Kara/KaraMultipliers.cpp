/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraMultipliers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidCatActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PetsAction.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "ThreatHelpers.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"
#include <array>

using namespace KaraHelpers;
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

}

float KarazhanKeepTremorTotemMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastStrengthOfEarthTotemAction*>(action) &&
        !dynamic_cast<CastStoneskinTotemAction*>(action) &&
        !dynamic_cast<CastStoneclawTotemAction*>(action) &&
        !dynamic_cast<CastEarthbindTotemAction*>(action))
    {
        return 1.0f;
    }

    return IsKarazhanFearCasterActive(bot) ? 0.0f : 1.0f;
}

float KarazhanDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    bool const isBloodlust = bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastBloodlustAction*>(action) || dynamic_cast<CastHeroismAction*>(action));

    std::optional<uint32> const bossId = GetBossInProgress(bot, KARA_MAP_ID);
    if (!bossId)
        return 1.0f;

    Unit* boss = nullptr;
    switch (static_cast<KaraBosses>(*bossId))
    {
        case KaraBosses::DATA_ATTUMEN:
            boss = FindThreatTarget(bot, "midnight");
            break;
        case KaraBosses::DATA_MOROES:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MOROES);
            break;
        case KaraBosses::DATA_MAIDEN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MAIDEN);
            break;
        case KaraBosses::DATA_OPERA_PERFORMANCE:
        {
            static constexpr std::array operaLeads = {
                "the big bad wolf",
                "dorothee",
                "romulo",
                "julianne",
            };
            for (char const* name : operaLeads)
            {
                boss = GetOperaActor(bot, name);
                if (boss)
                    break;
            }
            break;
        }
        case KaraBosses::DATA_CURATOR:
            boss = GetKaraBoss(bot, KaraBosses::DATA_CURATOR);
            break;
        case KaraBosses::DATA_TERESTIAN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_TERESTIAN);
            break;
        case KaraBosses::DATA_ARAN:
            boss = GetKaraBoss(bot, KaraBosses::DATA_ARAN);
            break;
        case KaraBosses::DATA_NETHERSPITE:
            boss = GetKaraBoss(bot, KaraBosses::DATA_NETHERSPITE);
            break;
        case KaraBosses::DATA_MALCHEZAAR:
            boss = GetKaraBoss(bot, KaraBosses::DATA_MALCHEZAAR);
            break;
        case KaraBosses::DATA_NIGHTBANE:
            boss = GetKaraBoss(bot, KaraBosses::DATA_NIGHTBANE);
            break;
        default:
            return 1.0f;
    }

    // If the boss cannot be found but the encounter is in progress, it is probably because the bot
    // was rezzed and zone combat hasn't reapplied, so block cooldowns.
    if (!boss)
        return 0.0f;

    if (isBloodlust)
    {
        if (boss->GetEntry() == Id(KaraNpcs::NPC_PRINCE_MALCHEZAAR))
        {
            constexpr float finalPhaseHealthPct = 30.0f;
            return boss->GetHealthPct() > finalPhaseHealthPct ? 0.0f : 1.0f;
        }

        if (boss->GetEntry() == Id(KaraNpcs::NPC_THE_CURATOR))
            return !boss->HasAura(Id(KaraSpells::SPELL_CURATOR_EVOCATION)) ? 0.0f : 1.0f;
    }

    if (boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return 0.0f;

    if (boss->GetEntry() == Id(KaraNpcs::NPC_NIGHTBANE))
        return IsNightbaneFlying(boss) ? 0.0f : 1.0f;

    if (boss->GetEntry() == Id(KaraNpcs::NPC_NETHERSPITE))
        return IsBanishPhase(boss) ? 0.0f : 1.0f;

    return 1.0f;
}

float KarazhanDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return dynamic_cast<SetBehindTargetAction*>(action) ? 1.0f : 0.0f;
}

// Trash

// Some stun abilities are used in standard rotations. Every stunner should save them for the mana
// warps at the right time. Block all stuns used by the stun action just for belt-and-suspenders.
float ManaWarpSaveStunsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT || !IsNearManaWarpPacks(bot))
        return 1.0f;

    bool isStun = false;
    switch (bot->getClass())
    {
        case CLASS_DRUID:
        {
            isStun = dynamic_cast<CastBashAction*>(action) ||
                dynamic_cast<CastBashOnEnemyHealerAction*>(action) ||
                dynamic_cast<CastMaimAction*>(action);
            break;
        }
        case CLASS_PALADIN:
        {
            isStun = dynamic_cast<CastHammerOfJusticeAction*>(action) ||
                dynamic_cast<CastHammerOfJusticeOnEnemyHealerAction*>(action) ||
                dynamic_cast<CastHammerOfJusticeSnareAction*>(action);
            break;
        }
        case CLASS_WARLOCK:
        {
            isStun = dynamic_cast<CastShadowfuryAction*>(action);
            break;
        }
        case CLASS_WARRIOR:
        {
            isStun = dynamic_cast<CastConcussionBlowAction*>(action) ||
                dynamic_cast<CastShockwaveAction*>(action) ||
                dynamic_cast<CastShockwaveSnareAction*>(action);
            break;
        }
        default:
            break;
    }

    if (isStun)
        return 0.0f;

    return bot->getRace() == RACE_TAUREN && dynamic_cast<CastWarStompAction*>(action) ? 0.0f : 1.0f;
}

// Attumen the Huntsman

float AttumenTheHuntsmanDisableAutoTargetingMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    // Include Blade Flurry because it's also used as a pure DPS cooldown, not just for AoE, so the
    // target exclusion doesn't cover it entirely. Save it for phase 2.
    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action) &&
        !(bot->getClass() == CLASS_ROGUE && dynamic_cast<CastBladeFlurryAction*>(action)))
    {
        return 1.0f;
    }

    // Mounted Attumen is the only valid target in phase 2, so standard targeting is fine.
    Unit* midnight = FindThreatTarget(bot, "midnight");
    return midnight && !IsAttumenMounted(midnight) ? 0.0f : 1.0f;
}

float AttumenTheHuntsmanStayStackedMultiplier::GetValueInEncounter(Action* action)
{
    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<AttumenTheHuntsmanStackBehindAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;

    Unit* midnight = FindThreatTarget(bot, "midnight");
    if (!midnight || !IsAttumenMounted(midnight))
        return 1.0f;

    Unit* attumen = GetMountedAttumen(bot);
    if (attumen && attumen->GetVictim() == bot)
        return 1.0f;

    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;
}

float AttumenTheHuntsmanHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    return IsAttumenDpsHeld(bot) ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Maiden of Virtue

float MaidenOfVirtueKeepGroundingTotemMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    // Call of the Elements drops whatever air totem the totem bar holds.
    return dynamic_cast<CastWrathOfAirTotemAction*>(action) ||
        dynamic_cast<CastNatureResistanceTotemAction*>(action) ||
        dynamic_cast<CastWindfuryTotemAction*>(action) ||
        dynamic_cast<CastCallOfTheElementsAction*>(action) ? 0.0f : 1.0f;
}

// The Big Bad Wolf

// Feign Death must be held through the entire 20s chase sequence to keep the boss away. Moving,
// turning, or casting ends Feign Death, so a feigning Hood does nothing until Hood ends.
float BigBadWolfRedHoodHunterStayDownMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_HUNTER || !bot->HasAura(Id(KaraSpells::SPELL_FEIGN_DEATH)))
        return 1.0f;

    if (!IsLittleRedRidingHood(bot))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

// The Curator

float TheCuratorDisableTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    return dynamic_cast<TankAssistAction*>(action) ? 0.0f : 1.0f;
}

// Terestian Illhoof

float TerestianIllhoofDisableDotsOnFiendishImpsMultiplier::GetValueInEncounter(Action* action)
{
    // Also the on-attacker DoTs, which pick their own target.
    if (!dynamic_cast<CastDebuffSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    return target && target->GetEntry() == Id(KaraNpcs::NPC_FIENDISH_IMP) ? 0.0f : 1.0f;
}

// Shade of Aran

float ShadeOfAranArcaneExplosionDisableMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action) &&
        !IsKillingSpree(bot, action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranRunAwayFromArcaneExplosionAction*>(action))
    {
        return 1.0f;
    }

    Unit* aran = GetEncounterBoss();
    return aran && IsAranCastingArcaneExplosion(aran) ? 0.0f : 1.0f;
}

// Includes set behind, as he faces a random player with each cast.
float ShadeOfAranDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    bool const isReactiveMovement =
        dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action);

    return isReactiveMovement ? 0.0f : 1.0f;
}

// I will not move when Flame Wreath is cast or the raid blows up.
float ShadeOfAranFlameWreathDisableMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !IsRepositionAction(bot, action) &&
        !IsKillingSpree(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<ShadeOfAranStopMovingAction*>(action))
    {
        return 1.0f;
    }

    Unit* aran = GetEncounterBoss();
    if (!aran || !IsFlameWreathInProgress(botAI, aran))
        return 1.0f;

    bool const isReachToAttack =
        dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachSpellAction*>(action);

    return isReachToAttack && CanReachTargetDuringFlameWreath(botAI, aran) ? 1.0f : 0.0f;
}

// Netherspite

namespace
{

// Where a standard reach toward the target stops, worked out the way ReachCombatTo does it: range
// plus both combat reaches from the target. The reach action doesn't expose that position, and the
// void zone hold needs it before the reach runs.
Position GetNetherspiteReachEndPosition(Unit& bot, Unit& target, float range)
{
    float const distance = bot.GetExactDist2d(target);
    float const end = range + bot.GetCombatReach() + target.GetCombatReach();
    if (distance <= end)
        return bot.GetPosition();

    float const fraction = end / distance;
    return Position(
        target.GetPositionX() + (bot.GetPositionX() - target.GetPositionX()) * fraction,
        target.GetPositionY() + (bot.GetPositionY() - target.GetPositionY()) * fraction);
}

} // end anonymous namespace (Netherspite)

// Positioning in and around the beams decides the outcome of the fight, and a tight formation is
// kept during the Banish phase, so reactive and unpredictable movement is blocked.
float NetherspiteDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isReactiveMovement =
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action) ||
        dynamic_cast<FleeAction*>(action) || dynamic_cast<FollowAction*>(action);
    if (isReactiveMovement || IsKillingSpree(bot, action) || IsRepositionAction(bot, action))
        return 0.0f;

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action))
    {
        return 1.0f;
    }

    KaraInstanceState const* state = FindKaraState(bot->GetInstanceId());
    if (!state || state->netherspiteBeams[Id(NetherspiteBeam::Red)].blocker == bot->GetGUID())
        return 1.0f;

    // Active blue and green blockers and those up next are blocked from reaching targets. Their
    // primary purpose is to keep blocking (or remain ready to block) a beam, so they need to
    // maintain their position along the beam while also staying alive (i.e., out of void zones),
    // which means the ability to DPS or heal might have to be sacrificed at times. The red blocker
    // does not have this constraint since it does not avoid void zones.
    if (IsNetherspiteBlockerOrNext(*state, bot->GetGUID()))
        return 0.0f;

    // Other bots' reach and charge are blocked if they would go through a void zone, since the
    // standard actions don't avoid them.
    Unit* target = action->GetTarget();
    if (!target)
        return 1.0f;

    // A void zone past where the move stops, between that point and the target, doesn't count.
    float range = 0.0f; // Charges go all the way to the target.
    if (dynamic_cast<ReachMeleeAction*>(action))
        range = sPlayerbotAIConfig.MeleeDistance;
    else if (dynamic_cast<ReachPartyMemberToHealAction*>(action))
        range = botAI->GetRange("heal");
    else if (dynamic_cast<ReachTargetAction*>(action))
        range = botAI->GetRange("spell");

    Position const end = GetNetherspiteReachEndPosition(*bot, *target, range);
    return CrossesVoidZone(*state, bot->GetPosition(), end) ? 0.0f : 1.0f;
}

float NetherspiteHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* netherspite = GetEncounterBoss();
    if (!netherspite || !IsNetherspiteDpsHeld(bot, netherspite))
        return 1.0f;

    return GetDpsHoldValue(bot, action);
}

// Prince Malchezaar

float PrinceMalchezaarDisableReactiveMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<AvoidAoeAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    if (!dynamic_cast<SetBehindTargetAction*>(action))
        return 0.0f;

    Unit* malchezaar = GetEncounterBoss();
    if (!malchezaar)
        return 1.0f;

    return IsClearOfInfernals(*malchezaar, GetLandedInfernals(botAI), MALCHEZAAR_MELEE_CLEARANCE) ?
        1.0f : 0.0f;
}

float PrinceMalchezaarEnfeebledBotFocusOnEscapeMultiplier::GetValueInEncounter(Action* action)
{
    // Enfeeble's aura ends before Malchezaar's boss AI restores the player's health. Players update
    // every world tick, but dungeon creatures only about every third tick, and after players within
    // the same tick. So a bot can drop Enfeeble but still have 1 HP, and an aura check alone would
    // end the hold too early. Of course, this has the side effect of enforcing the hold in the
    // unlikely event that a bot legitimately has 1 HP.
    if (!bot->HasAura(Id(KaraSpells::SPELL_ENFEEBLE)) && bot->GetHealth() > 1)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<PrinceMalchezaarEnfeebledBotRunToSafetyAction*>(action))
    {
        return 1.0f;
    }

    // Disable movement other than escaping Shadow Nova range.
    if (dynamic_cast<MovementAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action) || IsRepositionAction(bot, action))
    {
        return 0.0f;
    }

    // Hold cooldowns used at low or critical health.
    bool isHeld = false;
    switch (bot->getClass())
    {
        case CLASS_MAGE:
        {
            isHeld = dynamic_cast<CastIceBlockAction*>(action) ||
                dynamic_cast<CastManaShieldAction*>(action);
            break;
        }
        case CLASS_PALADIN:
        {
            isHeld = dynamic_cast<CastDivineShieldAction*>(action) ||
                dynamic_cast<CastLayOnHandsAction*>(action);
            break;
        }
        case CLASS_ROGUE:
        {
            isHeld = dynamic_cast<CastCloakOfShadowsAction*>(action) ||
                dynamic_cast<CastEvasionAction*>(action);
            break;
        }
        case CLASS_SHAMAN:
        {
            isHeld = dynamic_cast<CastShamanisticRageAction*>(action);
            break;
        }
        case CLASS_PRIEST:
        {
            isHeld = dynamic_cast<CastBindingHealAction*>(action) ||
                dynamic_cast<CastDesperatePrayerAction*>(action) ||
                dynamic_cast<CastPainSuppressionAction*>(action) ||
                dynamic_cast<CastDispersionAction*>(action);
            break;
        }
        case CLASS_WARRIOR:
        {
            isHeld = dynamic_cast<CastEnragedRegenerationAction*>(action) ||
                dynamic_cast<CastLastStandAction*>(action) ||
                dynamic_cast<CastShieldWallAction*>(action);
            break;
        }
        case CLASS_DRUID:
        {
            isHeld = dynamic_cast<CastSurvivalInstinctsAction*>(action);
            break;
        }
        case CLASS_HUNTER:
        {
            isHeld = dynamic_cast<CastDeterrenceAction*>(action);
            break;
        }
        case CLASS_DEATH_KNIGHT:
        {
            isHeld = dynamic_cast<CastIceboundFortitudeAction*>(action) ||
                dynamic_cast<CastRuneTapAction*>(action) ||
                dynamic_cast<CastVampiricBloodAction*>(action) ||
                dynamic_cast<CastDeathStrikeAction*>(action) ||
                dynamic_cast<CastDeathPactAction*>(action);
            break;
        }
        default:
            break;
    }

    return isHeld || dynamic_cast<UseItemAction*>(action) ? 0.0f : 1.0f;
}

// Nightbane

// A unique multiplier to manage pet issues with Nightbane (see NightbaneSetPetStanceAction).
float NightbaneDelaySummonsAndPetAttacksMultiplier::GetValueInEncounter(Action* action)
{
    bool isFireElementalTotem = false;
    bool isSummon = false;
    switch (bot->getClass())
    {
        case CLASS_SHAMAN:
        {
            isFireElementalTotem = dynamic_cast<CastFireElementalTotemAction*>(action) ||
                dynamic_cast<CastFireElementalTotemMeleeAction*>(action);
            isSummon = isFireElementalTotem || dynamic_cast<CastFeralSpiritAction*>(action);
            break;
        }
        case CLASS_PRIEST:
            isSummon = dynamic_cast<CastShadowfiendAction*>(action);
            break;
        case CLASS_MAGE:
            isSummon = dynamic_cast<CastSummonWaterElementalAction*>(action);
            break;
        case CLASS_DRUID:
            isSummon = dynamic_cast<CastForceOfNatureAction*>(action);
            break;
        default:
            break;
    }

    if (!isSummon && !dynamic_cast<PetAttackAction*>(action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (!isSummon)
        return IsNightbaneFlying(nightbane) ? 0.0f : 1.0f;

    if (IsNightbaneDpsHeld(bot, nightbane))
        return 0.0f;

    // The Fire Elemental is the only summon that cannot be called back to the summoner, so it is
    // held until the final ground phase.
    return (!isFireElementalTotem ||
        nightbane->GetHealthPct() <= NIGHTBANE_FINAL_PHASE_HEALTH_PCT) ? 1.0f : 0.0f;
}

float NightbaneHoldDpsMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDropTarget = dynamic_cast<DropTargetAction*>(action);
    if (!isDropTarget && !IsDpsHoldCandidate(bot, action))
        return 1.0f;

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (isDropTarget)
        return IsNightbaneFlightPhaseActive(nightbane) ? 0.0f : 1.0f;

    if (dynamic_cast<CastHealingSpellAction*>(action) || !IsNightbaneDpsHeld(bot, nightbane))
        return 1.0f;

    return action->GetTarget() == nightbane ? GetDpsHoldValue(bot, action) : 1.0f;
}

// Movement is highly controlled on Nightbane. The most important note is that during the ground
// phase, melee targeting Nightbane (other than the tank) and ranged move only via raid actions.
float NightbaneDisableMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isReposition = IsRepositionAction(bot, action);
    bool const isReachTargetSpell = dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!dynamic_cast<MovementAction*>(action) && !isReachTargetSpell && !isReposition)
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<NightbaneRangedMoveFromCharredEarthAction*>(action) ||
        dynamic_cast<NightbaneMeleeMoveFromCharredEarthAction*>(action) ||
        dynamic_cast<NightbaneStackAwayFromRainOfBonesAction*>(action))
    {
        return 1.0f;
    }

    bool const isSetBehind = dynamic_cast<SetBehindTargetAction*>(action);
    if ((dynamic_cast<CombatFormationMoveAction*>(action) && !isSetBehind) ||
        isReposition || dynamic_cast<FleeAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
    {
        return 0.0f;
    }

    Unit* nightbane = GetEncounterBoss();
    if (!nightbane)
        return 1.0f;

    if (!IsNightbaneFlying(nightbane))
        return HoldsNightbaneGroundPosition(botAI, *nightbane) ? 0.0f : 1.0f;

    if (isReachTargetSpell || dynamic_cast<ReachTargetAction*>(action))
        return 0.0f;

    if (isSetBehind || dynamic_cast<FollowAction*>(action))
        return IsNightbaneFlightPhaseActive(nightbane) ? 0.0f : 1.0f;

    return 1.0f;
}
