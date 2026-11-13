/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Multipliers.h"
#include "Aq40Actions.h"
#include "Aq40Helpers.h"
#include "ChooseTargetActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include <cmath>

using namespace Aq40Helpers;
using namespace EncounterHelpers;

namespace
{

bool IsOwnAction(Action* action) { return dynamic_cast<Aq40ActionTag*>(action) != nullptr; }

// Actions that pick and attack a target on their own.
bool IsTargetPicker(Action* action)
{
    return dynamic_cast<AttackAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action) ||
           dynamic_cast<AttackLeastHpTargetAction*>(action);
}

bool IsHostileTarget(Player* bot, Action* action)
{
    Unit* target = action->GetTarget();
    return target && bot->IsValidAttackTarget(target);
}

SpellInfo const* ActionSpell(PlayerbotAI* botAI, Action* action)
{
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    return cast ? SpellFor(botAI, cast->getSpell()) : nullptr;
}

bool IsGenericMovement(Action* action) { return dynamic_cast<MovementAction*>(action) && !IsTargetPicker(action); }

bool IsRepositionAction(Player* bot, Action* action)
{
    return (bot->getClass() == CLASS_HUNTER && dynamic_cast<CastDisengageAction*>(action)) ||
           (bot->getClass() == CLASS_MAGE && dynamic_cast<CastBlinkBackAction*>(action));
}

}  // namespace

// Shared

float Aq40WaitForKnockbackMultiplier::GetValueInEncounter(Action* /*action*/)
{
    // Attack/cast actions can stop a movement spline even when CanMove() is false.
    return IsInEncounter(bot, Aq40Encounter::Twins) && !IsSelfBot(bot) &&
                   bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == EFFECT_MOTION_TYPE &&
                   !bot->movespline->Finalized()
               ? 0.0f
               : 1.0f;
}

float Aq40DontAttackMindControlledMultiplier::GetValueInEncounter(Action* action)
{
    // Only the explicit nonlethal crowd-control action may target raid members.
    if (IsOwnAction(action))
        return 1.0f;
    Unit* target = action->GetTarget();
    return HasTrueFulfillment(target) && IsHostileTarget(bot, action) ? 0.0f : 1.0f;
}

float Aq40ControlTauntingMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action))
        return 1.0f;
    bool taunt = IsTauntAction(bot, action);
    if (!taunt && IsInEncounter(bot, Aq40Encounter::Twins))
        if (SpellInfo const* spell = ActionSpell(botAI, action))
            taunt = spell->HasEffect(SPELL_EFFECT_ATTACK_ME);
    if (!taunt)
        return 1.0f;
    std::string const name = action->getName();
    if (name == "challenging shout" || name == "challenging roar" || name == "righteous defense")
        return 0.0f;
    Unit* target = action->GetTarget();
    return target && GetTankFor(bot, target) == bot && !Whirling(target) ? 1.0f : 0.0f;
}

float Aq40TanksDisableTankAssistMultiplier::GetValueInEncounter(Action* action)
{
    if (!PlayerbotAI::IsTank(bot) || !dynamic_cast<TankAssistAction*>(action))
        return 1.0f;
    // On a standard-assist boss only the boss's tank stays out of tank assist, so it never leaves the boss for an
    // add; the off-tanks use it to grab adds loose on the raid.
    return UsesAssignedTarget(bot) || GetTankFor(bot, GetBoss(bot)) == bot ? 0.0f : 1.0f;
}

float Aq40DisableAutoTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !UsesAssignedTarget(bot))
        return 1.0f;
    bool const petAttack = action->getName() == "pet attack";
    if (!IsTargetPicker(action) && !petAttack)
        return 1.0f;
    Unit* desired = GetAssignedTarget(botAI);
    if (petAttack)
    {
        if (!desired)
            return 0.0f;
        return IsInEncounter(bot, Aq40Encounter::Twins) && bot->getClass() == CLASS_WARLOCK && bot->GetPet() ? 0.0f
                                                                                                             : 1.0f;
    }
    return desired && action->GetTarget() == desired ? 1.0f : 0.0f;
}

float Aq40AllowedDamageMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !UsesAssignedTarget(bot) || !IsHostileTarget(bot, action))
        return 1.0f;
    Unit* target = action->GetTarget();
    if (!IsAllowedDamage(bot, target))
        return 0.0f;
    return PlayerbotAI::IsTank(bot) || target == GetAssignedTarget(botAI) ? 1.0f : 0.0f;
}

float Aq40DisableAoeMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || PlayerbotAI::IsTank(bot) || !UsesAssignedTarget(bot))
        return 1.0f;
    // Healing also generates area threat. Only harmful actions belong in the damage hold.
    bool areaThreat = action->getThreatType() == Action::ActionThreatType::Aoe;
    SpellInfo const* spell = ActionSpell(botAI, action);
    bool areaDamage = spell ? !spell->IsPositive() && (areaThreat || spell->IsAffectingArea())
                            : IsHostileTarget(bot, action) && areaThreat;
    if (!areaDamage)
        return 1.0f;
    Aq40Encounter encounter = GetCurrentEncounter(bot);
    bool hold = encounter == Aq40Encounter::Twins || encounter == Aq40Encounter::Skeram || !GetAssignedTarget(botAI);
    return hold ? 0.0f : 1.0f;
}

float Aq40RangedHoldBossRangeMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;
    Position spot;
    return GetBossRangeGoal(botAI, spot) ? 0.0f : 1.0f;
}

// Trash

float AnubisathDefenderAvoidReflectedSchoolsMultiplier::GetValue(Action* action)
{
    // An Anubisath Defender reflects two schools for its whole fight (Shadow+Frost or Fire+Arcane, picked on aggro).
    // Casting into the reflect is how bots kill themselves.
    if (!action || bot->GetMapId() != AQ40_MAP_ID || IsEncounterInProgress(bot, AQ40_MAP_ID))
        return 1.0f;
    Unit* target = action->GetTarget();
    if (!target || target->GetEntry() != Id(Aq40Npcs::NPC_ANUBISATH_DEFENDER) || !bot->IsValidAttackTarget(target))
        return 1.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    if (!spell || spell->IsPositive())
        return 1.0f;
    uint32 school = spell->GetSchoolMask();
    if (target->HasAura(Id(Aq40Spells::SPELL_SHADOW_FROST_REFLECT)) &&
        (school & (SPELL_SCHOOL_MASK_SHADOW | SPELL_SCHOOL_MASK_FROST)))
        return 0.0f;
    if (target->HasAura(Id(Aq40Spells::SPELL_FIRE_ARCANE_REFLECT)) &&
        (school & (SPELL_SCHOOL_MASK_FIRE | SPELL_SCHOOL_MASK_ARCANE)))
        return 0.0f;
    return 1.0f;
}

// The Prophet Skeram

float TheProphetSkeramControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const reach = dynamic_cast<ReachTargetAction*>(action) != nullptr;
    if (!reach && !dynamic_cast<CombatFormationMoveAction*>(action) && !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;
    Position platform;
    if (!GetSkeramPlatform(botAI, platform))
        return 1.0f;
    Unit* target = action->GetTarget();
    bool const localReach =
        reach && target && target->GetExactDist2d(platform) <= SKERAM_LOCAL_REACH &&
        std::abs(target->GetPositionZ() - platform.GetPositionZ()) <= SKERAM_PLATFORM_HEIGHT_TOLERANCE;
    return localReach ? 1.0f : 0.0f;
}

float TheProphetSkeramHoldDamageInTransitMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !IsHostileTarget(bot, action))
        return 1.0f;
    Position spot;
    return GetSkeramPlatformGoal(botAI, spot) ? 0.0f : 1.0f;
}

// Bug Trio

float BugTrioControlTankMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;
    Position spot;
    return GetTrioTankGoal(botAI, spot) ? 0.0f : 1.0f;
}

// Viscidus

float ViscidusControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;
    Position spot;
    return GetViscidusShatterGoal(botAI, spot) || GetViscidusRingGoal(botAI, spot) ? 0.0f : 1.0f;
}

float ViscidusFrostOnlyMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !IsHostileTarget(bot, action))
        return 1.0f;
    Unit* target = action->GetTarget();
    if (target->GetEntry() != Id(Aq40Npcs::NPC_VISCIDUS))
        return 1.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    if (target->HasAura(Id(Aq40Spells::SPELL_VISCIDUS_FREEZE)))
    {
        std::string const name = action->getName();
        bool shoot = name == "shoot" || name == "shoot bow" || name == "shoot gun" || name == "shoot crossbow";
        return (spell && spell->CalcCastTime() > 0) || shoot ? 0.0f : 1.0f;
    }
    return spell && !(spell->GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) && !PlayerbotAI::IsTank(bot) ? 0.0f : 1.0f;
}

// Princess Huhuran

float PrincessHuhuranControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const rangedReach = dynamic_cast<ReachTargetAction*>(action) && !PlayerbotAI::IsMelee(bot);
    if (!rangedReach && !dynamic_cast<CombatFormationMoveAction*>(action) && !dynamic_cast<FollowAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;
    if (dynamic_cast<SetBehindTargetAction*>(action) || PlayerbotAI::IsTank(bot))
        return 1.0f;
    // Soakers and everyone at range hold a spot; the other melee stay on the boss.
    return IsHuhuranSoaker(bot) || !PlayerbotAI::IsMelee(bot) ? 0.0f : 1.0f;
}

float PrincessHuhuranControlWyvernStingDispelMultiplier::GetValueInEncounter(Action* action)
{
    // The dedicated selective dispel handles tanks above the damage safety margin.
    Unit* target = action->GetTarget();
    SpellInfo const* spell = ActionSpell(botAI, action);
    if (!spell || !target || !target->HasAura(Id(Aq40Spells::SPELL_WYVERN_STING)))
        return 1.0f;
    for (SpellEffectInfo const& effect : spell->GetEffects())
        if (effect.Effect == SPELL_EFFECT_DISPEL)
            return 0.0f;
    return 1.0f;
}

float PrincessHuhuranDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;
    Creature* boss = GetBoss(bot);
    return boss && boss->GetHealthPct() > HUHURAN_COOLDOWN_HEALTH_PCT ? 0.0f : 1.0f;
}

// Twin Emperors

float TwinEmperorsOpeningHealHoldMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !PlayerbotAI::IsHeal(bot))
        return 1.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    if (!spell || !spell->IsPositive())
        return 1.0f;
    Unit* target = action->GetTarget();
    return TwinOpeningHold(bot, target ? target : bot) ? 0.0f : 1.0f;
}

float TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier::GetValueInEncounter(Action* action)
{
    // Instant priest spells cast on anyone drain the priests' mana within about two minutes. In this fight Shield
    // and Renew are for the emperors' targets only.
    if (IsOwnAction(action) || bot->getClass() != CLASS_PRIEST)
        return 1.0f;
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    if (!cast || (cast->getSpell() != "power word: shield" && cast->getSpell() != "renew"))
        return 1.0f;
    Unit* who = action->GetTarget() ? action->GetTarget() : bot;
    bool emperorTarget = false;
    for (Creature* emperor : GetUnits(bot))
        if (IsTwinEmperor(emperor) && (emperor->GetVictim() == who || GetTankFor(bot, emperor) == who))
            emperorTarget = true;
    return emperorTarget && who->GetHealthPct() <= 80.0f ? 1.0f : 0.0f;
}

float TwinEmperorsControlHealerMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action))
        return 1.0f;
    Position spot;
    return GetTwinHealerGoal(botAI, spot) ? 0.0f : 1.0f;
}

float TwinEmperorsSkipImmuneTargetsMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !IsHostileTarget(bot, action))
        return 1.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    return spell && action->GetTarget()->IsImmunedToDamage(bot, spell) ? 0.0f : 1.0f;
}

float TwinEmperorsVeklorThreatCapMultiplier::GetValueInEncounter(Action* action)
{
    // Vek'lor's tank holds him from outside Arcane Burst with taunt threat only, and his Shadow Bolt hits for ~4k.
    // Damage pauses before it would out-threaten that tank.
    if (IsOwnAction(action) || PlayerbotAI::IsTank(bot) || !IsHostileTarget(bot, action))
        return 1.0f;
    Unit* target = action->GetTarget();
    if (target->GetEntry() != Id(Aq40Npcs::NPC_VEKLOR))
        return 1.0f;
    Player* owner = GetTankFor(bot, target);
    if (!owner || owner == bot)
        return 1.0f;
    return target->GetThreatMgr().GetThreat(bot) + TWIN_CASTER_THREAT_MARGIN >=
                   1.3f * target->GetThreatMgr().GetThreat(owner)
               ? 0.0f
               : 1.0f;
}

float TwinEmperorsSpareTankThreatCapMultiplier::GetValueInEncounter(Action* action)
{
    // The spare tank stays second on Vek'nilash's threat: close behind his tank, never above him.
    if (IsOwnAction(action) || !IsTwinSpareTank(bot, bot) || !IsHostileTarget(bot, action))
        return 1.0f;
    Unit* target = action->GetTarget();
    if (target->GetEntry() != Id(Aq40Npcs::NPC_VEKNILASH))
        return 1.0f;
    Unit* victim = target->GetVictim();
    return victim && victim != bot &&
                   target->GetThreatMgr().GetThreat(bot) >= 0.9f * target->GetThreatMgr().GetThreat(victim)
               ? 0.0f
               : 1.0f;
}

float TwinEmperorsTanksHoldAnchorMultiplier::GetValueInEncounter(Action* action)
{
    // A Twin tank belongs to a platform, including while its boss is displaced. Ordinary chase and charge must not
    // undo the anchor as soon as the positioning action reaches its goal.
    if (IsOwnAction(action) || !PlayerbotAI::IsTank(bot))
        return 1.0f;
    if (IsGenericMovement(action))
        return 0.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    return spell && spell->HasEffect(SPELL_EFFECT_CHARGE) ? 0.0f : 1.0f;
}

// C'Thun

float CthunControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    // Every bot's place comes from the strategy. A bot briefly out of combat would otherwise follow the raid leader,
    // possibly back up the entrance ramp during phase 1.
    if (dynamic_cast<FollowAction*>(action))
        return 0.0f;
    if (!dynamic_cast<ReachTargetAction*>(action) && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;
    Position spot;
    return GetCthunPositionGoal(botAI, spot) ? 0.0f : 1.0f;
}

float CthunStomachExitOnlyMultiplier::GetValueInEncounter(Action* action)
{
    // A swallowed bot heading out does nothing else: each heal cast stops its walk to the exit and it dies in the
    // pool.
    if (IsOwnAction(action))
        return 1.0f;
    return StomachExitNeeded(bot) ? 0.0f : 1.0f;
}

float CthunStomachNoTreeOfLifeMultiplier::GetValueInEncounter(Action* action)
{
    // Stays out of Tree of Life to damage the Flesh Tentacles.
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    if (!cast || cast->getSpell() != "tree of life")
        return 1.0f;
    return InStomach(bot) ? 0.0f : 1.0f;
}

float CthunStomachNoFloorHealsMultiplier::GetValueInEncounter(Action* action)
{
    // Nobody on the floor is in range of a healer in the stomach; those casts only fail and cost its turns.
    if (IsOwnAction(action) || !InStomach(bot))
        return 1.0f;
    SpellInfo const* spell = ActionSpell(botAI, action);
    Unit* target = action->GetTarget();
    return spell && spell->IsPositive() && target && target != bot && target->GetPositionZ() > CTHUN_STOMACH_Z ? 0.0f
                                                                                                               : 1.0f;
}

float CthunStomachHoldMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (IsOwnAction(action) || !IsGenericMovement(action) || !InStomach(bot))
        return 1.0f;
    Unit* target = action->GetTarget();
    return dynamic_cast<ReachTargetAction*>(action) && target && target->GetPositionZ() <= CTHUN_STOMACH_Z ? 1.0f
                                                                                                           : 0.0f;
}

float CthunMeleeBurnRangeMultiplier::GetValueInEncounter(Action* action)
{
    // Weakened burn: no melee strikes on C'Thun from beyond 10 yd.
    if (IsOwnAction(action) || !IsHostileTarget(bot, action))
        return 1.0f;
    Unit* target = action->GetTarget();
    return target->GetEntry() == Id(Aq40Npcs::NPC_CTHUN) && CthunMeleeBurn(botAI) &&
                   bot->GetExactDist2d(target) > CTHUN_BURN_MELEE_REACH
               ? 0.0f
               : 1.0f;
}
