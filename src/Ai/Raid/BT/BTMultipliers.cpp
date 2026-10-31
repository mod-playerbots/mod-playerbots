/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTMultipliers.h"
#include "BTActions.h"
#include "BTHelpers.h"
#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "DruidShapeshiftActions.h"
#include "EncounterHelpers.h"
#include "FollowActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "PaladinActions.h"
#include "PlayerbotAIConfig.h"
#include "PriestActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "ThreatManager.h"
#include "Timer.h"
#include "UseItemAction.h"
#include "WarlockActions.h"
#include "WarriorActions.h"
#include "WipeAction.h"
#include <algorithm>
#include <limits>
#include <vector>

using namespace BtHelpers;
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

bool IsReliquaryEssence(uint32 entry)
{
    return entry == Id(BtNpcs::NPC_ESSENCE_OF_SUFFERING) ||
        entry == Id(BtNpcs::NPC_ESSENCE_OF_DESIRE) || entry == Id(BtNpcs::NPC_ESSENCE_OF_ANGER);
}

bool IsDpsHoldCandidate(Player* bot, Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return false;

    if (!PlayerbotAI::IsTank(bot) && dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    return !dynamic_cast<AttackAction*>(action) || !PlayerbotAI::IsHeal(bot);
}

// Buffs, cures, protection, and resurrection are allowed during a DPS hold. Totems are not because
// some are offensive, and I don't think it's worth getting granular about which to allow.
float GetDpsHoldValue(Player* bot, Action* action)
{
    if (bot->getClass() == CLASS_SHAMAN && dynamic_cast<CastTotemAction*>(action))
        return 0.0f;

    bool const castOnRaidSpell = dynamic_cast<CastBuffSpellAction*>(action) ||
        dynamic_cast<CastCureSpellAction*>(action) ||
        dynamic_cast<CurePartyMemberAction*>(action) ||
        dynamic_cast<ResurrectPartyMemberAction*>(action) ||
        dynamic_cast<CastProtectSpellAction*>(action);

    return castOnRaidSpell ? 1.0f : 0.0f;
}

} // end anonymous namespace (Shared)

// Held for each boss's first 5% (excluding Shade of Akama and Illidan), and also through Supremus's
// kite phase and between Reliquary essences. See below for Illidan's rules.
float BlackTempleDelayDpsCooldownsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsDpsCooldownAction(bot, action))
        return 1.0f;

    std::optional<uint32> const bossId = GetBossInProgress(bot, BT_MAP_ID);
    if (!bossId)
        return 1.0f;

    switch (static_cast<BtBosses>(*bossId))
    {
        case BtBosses::DATA_ILLIDAN_STORMRAGE:
        {
            Unit* illidan = GetBtBoss(bot, BtBosses::DATA_ILLIDAN_STORMRAGE);
            if (!illidan)
                return 1.0f;

            // At 62%, Bloodlust and all cooldowns are permanently freed.
            if (illidan->GetHealthPct() <= ILLIDAN_DPS_COOLDOWN_RELEASE_HEALTH_PCT)
                return 1.0f;

            if (bot->getClass() == CLASS_SHAMAN &&
                (dynamic_cast<CastHeroismAction*>(action) ||
                 dynamic_cast<CastBloodlustAction*>(action)))
            {
                return 0.0f;
            }

            // Above 95%, cooldowns other than Bloodlust can be used, then disable them until 62%
            // (i.e., get one use in and then save until phase 3, when cooldown should be back up).
            return illidan->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 1.0f : 0.0f;
        }
        case BtBosses::DATA_RELIQUARY_OF_SOULS:
        {
            if (!GetBtBoss(bot, BtBosses::DATA_RELIQUARY_OF_SOULS))
                return 1.0f;

            Unit* essence = nullptr;
            for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
            {
                Unit* unit = ref->GetOwner();
                // After being defeated, an Essence stays on threat lists while it returns to the
                // Reliquary with 1 HP. This check prevents cooldowns from being released during
                // such period.
                if (unit && IsReliquaryEssence(unit->GetEntry()) &&
                    !unit->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
                {
                    essence = unit;
                    break;
                }
            }

            return !essence || essence->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }
        case BtBosses::DATA_SUPREMUS:
        {
            Unit* supremus = GetBtBoss(bot, BtBosses::DATA_SUPREMUS);
            if (!supremus)
                return 1.0f;

            return IsSupremusKitePhase(supremus) ||
                supremus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }
        case BtBosses::DATA_HIGH_WARLORD_NAJENTUS:
        case BtBosses::DATA_TERON_GOREFIEND:
        case BtBosses::DATA_GURTOGG_BLOODBOIL:
        case BtBosses::DATA_MOTHER_SHAHRAZ:
        {
            Unit* boss = GetBtBoss(bot, static_cast<BtBosses>(*bossId));
            return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }
        case BtBosses::DATA_ILLIDARI_COUNCIL:
        {
            // Empyreal Balance shares health, so Gathios's percentage is the Council's.
            Unit* gathios = GetBtCreature(bot, BtCreatureData::DATA_GATHIOS_THE_SHATTERER);
            return gathios && gathios->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT ? 0.0f : 1.0f;
        }
        default:
            return 1.0f;
    }
}

// Supremus's bounding radius is 80 yd, far beyond his combat reach, and Killing Spree places the
// rogue by bounding radius, so it takes the rogue well out of the fight.
// Essence of Suffering targets the closest player, and Killing Spree makes the rogue the closest.
float BlackTempleDisableKillingSpreeMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!IsKillingSpree(bot, action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ||
        GetBtBoss(bot, BtBosses::DATA_SUPREMUS) ? 0.0f : 1.0f;
}

// Shade of Akama and its Channelers and Sorcerers are not tankable, and Fel Rage fixes Gurtogg on
// its target, so either way, a taunt or Misdirection is useless.
float BlackTempleNoThreatRedirectsMultiplier::GetValueInEncounter(Action* action)
{
    bool const isMisdirect = bot->getClass() == CLASS_HUNTER &&
        (dynamic_cast<CastMisdirectionOnMainTankAction*>(action) ||
         dynamic_cast<BlackTempleMisdirectToMainTankAction*>(action));
    if (!isMisdirect && !IsTauntAction(bot, action))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    uint32 const entry = target->GetEntry();
    if (entry == Id(BtNpcs::NPC_SHADE_OF_AKAMA) ||
        entry == Id(BtNpcs::NPC_ASHTONGUE_CHANNELER) ||
        entry == Id(BtNpcs::NPC_ASHTONGUE_SORCERER))
    {
        return 0.0f;
    }

    if (entry != Id(BtNpcs::NPC_GURTOGG_BLOODBOIL))
        return 1.0f;

    return target->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)) ? 0.0f : 1.0f;
}

// Trash

float ShadowmoonReaverHoldChargeBuildingSpellsMultiplier::GetValue(Action* action)
{
    auto* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    auto const& reavers = context->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    if (reavers.empty())
        return 1.0f;

    // Summons whose own casts build charges must be held for the whole pull.
    if (dynamic_cast<CastMirrorImageAction*>(action) ||
        dynamic_cast<CastSummonGargoyleAction*>(action) ||
        dynamic_cast<CastSearingTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemAction*>(action) ||
        dynamic_cast<CastFireElementalTotemMeleeAction*>(action))
    {
        return 0.0f;
    }

    SpellInfo const* spellInfo =
        sSpellMgr->GetSpellInfo(AI_VALUE2(uint32, "spell id", spellAction->getSpell()));
    ChaoticChargeReach const reach = GetChaoticChargeReach(spellInfo);
    if (reach == ChaoticChargeReach::None)
        return 1.0f;

    if (reach == ChaoticChargeReach::Target)
        return IsShadowmoonReaverUnsafeForMagic(action->GetTarget()) ? 0.0f : 1.0f;

    return IsAnyShadowmoonReaverUnsafeForMagic(botAI) ? 0.0f : 1.0f;
}

// High Warlord Naj'entus

float HighWarlordNajentusDisableCombatFormationMoveMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

// Tidal Shield makes Naj'entus invalid, causing bots to drop combat. Stop them from breaking their
// spread (due to FollowAction) and from alternating attack/drop target repeatedly.
float HighWarlordNajentusTidalShieldHoldPositionMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT)
        return 1.0f;

    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<TankAssistAction*>(action) &&
        !dynamic_cast<FollowAction*>(action))
    {
        return 1.0f;
    }

    Unit* najentus = GetEncounterBoss();
    return najentus && najentus->HasAura(Id(BtSpells::SPELL_TIDAL_SHIELD)) ? 0.0f : 1.0f;
}

// Supremus

float SupremusRunAwayWhenFixatedMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !(bot->getClass() == CLASS_MAGE && dynamic_cast<CastEvocationAction*>(action)))
    {
        return 1.0f;
    }

    // Attack doesn't move the bot, and a fixated bot that dropped its target needs it to pick one.
    if (dynamic_cast<SupremusKiteBossAction*>(action) || dynamic_cast<AttackAction*>(action))
        return 1.0f;

    Unit* supremus = GetEncounterBoss();
    if (!supremus || supremus->GetVictim() != bot)
        return 1.0f;

    return IsSupremusKitePhase(supremus) ? 0.0f : 1.0f;
}

// If reaching a target would mean walking through fire, hold that standard reach. The encounter's
// reach around fire action walks around it instead. A blocked reach the action doesn't route around
// simply doesn't run.
float SupremusDoNotReachThroughFireMultiplier::GetValueInEncounter(Action* action)
{
    bool const isHealReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isSpellReach = dynamic_cast<ReachSpellAction*>(action);
    bool const isMeleeReach = dynamic_cast<ReachMeleeAction*>(action);
    if (!isHealReach && !isSpellReach && !isMeleeReach)
        return 1.0f;

    if (!GetEncounterBoss())
        return 1.0f;

    float range;
    if (isHealReach)
        range = botAI->GetRange("heal");
    else if (isSpellReach)
        range = botAI->GetRange("spell");
    else
        range = sPlayerbotAIConfig.MeleeDistance;

    return IsSupremusReachBlockedByFire(botAI, action->GetTarget(), range) ? 0.0f : 1.0f;
}

float SupremusControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isDisperseOrTankFace = dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action);
    if (!isDisperseOrTankFace && !dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;

    Unit* supremus = GetEncounterBoss();
    if (!supremus)
        return 1.0f;

    if (isDisperseOrTankFace)
        return 0.0f;

    // Disable avoid aoe for all bots in the kite phase and Supremus's victim in the tank phase.
    return IsSupremusKitePhase(supremus) || supremus->GetVictim() == bot ? 0.0f : 1.0f;
}

// Shade of Akama

// The platform where Shade starts is out of LoS of bots next to it, causing melee to drop their
// targets on the way up.
float ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    if (!PlayerbotAI::IsDps(bot) || !PlayerbotAI::IsMelee(bot))
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    return target->GetEntry() == Id(BtNpcs::NPC_ASHTONGUE_CHANNELER) ||
        target->GetEntry() == Id(BtNpcs::NPC_ASHTONGUE_SORCERER) ? 0.0f : 1.0f;
}

// Teron Gorefiend

float TeronGorefiendControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    bool const isRangedReach =
        PlayerbotAI::IsRanged(bot) && dynamic_cast<ReachTargetAction*>(action);

    if (!isRangedReach && !dynamic_cast<CombatFormationMoveAction*>(action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action) &&
        !IsRepositionAction(bot, action))
    {
        return 1.0f;
    }

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

float TeronGorefiendMarkedBotOnlyMoveToDieMultiplier::GetValueInEncounter(Action* action)
{
    Aura* shadow = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    if (!shadow || shadow->GetDuration() > GOREFIEND_SHADOW_OF_DEATH_MOVE_MS)
        return 1.0f;

    if (dynamic_cast<TeronGorefiendMoveToCornerToDieAction*>(action))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

float TeronGorefiendSpiritsAttackShadowyConstructsMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE)))
        return 1.0f;

    if (dynamic_cast<TeronGorefiendDestroyShadowyConstructsAction*>(action))
        return 1.0f;

    return dynamic_cast<WipeAction*>(action) ? 1.0f : 0.0f;
}

// Gurtogg Bloodboil

float GurtoggBloodboilControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (IsGurtoggFelRageTarget(bot))
    {
        // The Fel Rage target moves only to bring the boss to the tank position.
        if (!dynamic_cast<MovementAction*>(action))
            return 1.0f;

        if (dynamic_cast<AttackAction*>(action))
            return 1.0f;
    }
    else
    {
        if (!dynamic_cast<CombatFormationMoveAction*>(action) &&
            !dynamic_cast<FollowAction*>(action) &&
            !dynamic_cast<FleeAction*>(action) && !IsRepositionAction(bot, action))
        {
            return 1.0f;
        }

        if (dynamic_cast<SetBehindTargetAction*>(action))
            return 1.0f;
    }

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

// Gurtogg is fixated on the Fel Rage target, so it should not use threat drop abilities.
float GurtoggBloodboilFelRageTargetHoldsAggroMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsGurtoggFelRageTarget(bot))
        return 1.0f;

    // Feign Death, Fade, Invisibility, Vanish, Cower and Soulshatter are left out: their trigger
    // is "medium threat", which needs two attackers on the bot, and Gurtogg has no adds.
    bool const dropsThreat =
        dynamic_cast<CastIceBlockAction*>(action) ||
        dynamic_cast<CastDivineShieldAction*>(action) ||
        dynamic_cast<CastFeintAction*>(action);

    return dropsThreat ? 0.0f : 1.0f;
}

// Non-tanks other than healers stay under GURTOGG_THREAT_HOLD_RATIO of the second tank's threat,
// except during Fel Rage, when bots cannot gain threat.
float GurtoggBloodboilHoldThreatMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<AttackAction*>(action))
        return 1.0f;

    if (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        bot->HasAura(Id(BtSpells::SPELL_INSIGNIFICANCE)))
    {
        return 1.0f;
    }

    Unit* gurtogg = GetEncounterBoss();
    if (!gurtogg)
        return 1.0f;

    if (AI_VALUE(Unit*, "current target") != gurtogg && action->GetTarget() != gurtogg)
        return 1.0f;

    float const secondTankThreat = AI_VALUE(float, "gurtogg bloodboil second tank threat");
    if (secondTankThreat <= 0.0f)
        return 1.0f;

    if (gurtogg->GetThreatMgr().GetThreat(bot) < secondTankThreat * GURTOGG_THREAT_HOLD_RATIO)
        return 1.0f;

    return dynamic_cast<CastHealingSpellAction*>(action) ? 1.0f : GetDpsHoldValue(bot, action);
}

// Reliquary of Souls

float ReliquaryOfSoulsDoNotWasteHealingMultiplier::GetValueInEncounter(Action* action)
{
    if (!dynamic_cast<CastTreeFormAction*>(action) &&
        !dynamic_cast<CastHealingSpellAction*>(action))
    {
        return 1.0f;
    }

    if (dynamic_cast<CastPowerWordShieldOnAlmostFullHealthBelowAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnNotFullAction*>(action) ||
        dynamic_cast<CastPowerWordShieldAction*>(action) ||
        dynamic_cast<CastPowerWordShieldOnPartyAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ? 0.0f : 1.0f;
}

// Tank face, set behind, and disperse move melee around Suffering, which can mess with the
// tailored actions to keep tanks closest to the boss to ensure they are the fixated targets.
float ReliquaryOfSoulsControlMeleeMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (PlayerbotAI::IsRanged(bot))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "essence of suffering") ? 0.0f : 1.0f;
}

float ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_SHAMAN)
        return 1.0f;

    if (!dynamic_cast<CastPurgeAction*>(action) &&
        !dynamic_cast<CastTranquilizingShotAction*>(action))
    {
        return 1.0f;
    }

    Unit* target = action->GetTarget();
    if (!target || target->GetEntry() != Id(BtNpcs::NPC_ESSENCE_OF_DESIRE))
        return 1.0f;

    Aura* runeShield = target->GetAura(Id(BtSpells::SPELL_RUNE_SHIELD));
    if (!runeShield)
        return 1.0f;

    return GetAuraElapsedMs(runeShield) < RUNE_SHIELD_MAGE_PRIORITY_MS ? 0.0f : 1.0f;
}

// Spell Reflection triggers from any cast at the tank, so it would be blown on Spirit Shock,
// which can't be reflected. Rely on BlackTempleCastSpellReflectionAction for Deaden instead.
float ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_WARRIOR)
        return 1.0f;

    if (!dynamic_cast<CastSpellReflectionAction*>(action))
        return 1.0f;

    return AI_VALUE2(Unit*, "find target", "essence of desire") ? 0.0f : 1.0f;
}

// Mother Shahraz

float MotherShahrazControlMovementMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    bool const isReachAction = dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<CastReachTargetSpellAction*>(action);
    if (isReachAction)
    {
        if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
            return 1.0f;

        return IsShahrazBeingPositioned(GetEncounterBoss()) ? 0.0f : 1.0f;
    }

    if (!dynamic_cast<CombatFormationMoveAction*>(action) && !IsRepositionAction(bot, action))
        return 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    return GetEncounterBoss() ? 0.0f : 1.0f;
}

float MotherShahrazFatalAttractionRunAwayMultiplier::GetValueInEncounter(Action* action)
{
    if (!bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION)))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    return dynamic_cast<MotherShahrazBreakFatalAttractionAction*>(action) ? 1.0f : 0.0f;
}

// Illidari Council

float IllidariCouncilControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    bool const inCombat = botAI->GetState() == BOT_STATE_COMBAT;
    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return inCombat && GetIllidariCouncilMembers(botAI).gathios ? 0.0f : 1.0f;

    if (PlayerbotAI::IsTank(bot))
        return inCombat ? GetTankValue(action) : 1.0f;

    return GetNonTankValue(action);
}

float IllidariCouncilControlMovementAndTargetingMultiplier::GetTankValue(Action* action)
{
    if (bot->getClass() == CLASS_WARRIOR)
    {
        // Save Spell Reflection for Judgement of Command via BlackTempleCastSpellReflectionAction.
        if (dynamic_cast<CastSpellReflectionAction*>(action))
            return GetIllidariCouncilMembers(botAI).gathios ? 0.0f : 1.0f;

        if (dynamic_cast<CastVigilanceAction*>(action))
        {
            if (!GetIllidariCouncilMembers(botAI).gathios)
                return 1.0f;
            // Do not siphon aggro from the mage tank.
            Player* mageTank = GetZerevorMageTank(botAI);
            return mageTank && action->GetTarget() == mageTank ? 0.0f : 1.0f;
        }
    }

    bool const isAoeThreat = IsAoeThreatAction(bot, action);
    bool const isTaunt = IsTauntAction(bot, action);
    bool const isAvoidAoe = dynamic_cast<AvoidAoeAction*>(action);
    if (!isAoeThreat && !isTaunt && !isAvoidAoe &&
        !dynamic_cast<CombatFormationMoveAction*>(action))
    {
        return 1.0f;
    }

    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    Unit* gathios = members.gathios;
    Unit* zerevor = members.zerevor;
    Unit* malande = members.malande;
    if (!gathios || !zerevor || !malande)
        return 1.0f;

    if (isAoeThreat)
    {
        if (zerevor->GetVictim() == bot)
            return 0.0f;

        return IsAnotherCouncilMemberWithin(botAI, COUNCIL_AOE_THREAT_CLEARANCE) ? 0.0f : 1.0f;
    }

    // Malande and Veras can be taunted, so their tanks may taunt back their own member, though
    // I'm not sure if this is Blizzlike.
    if (isTaunt)
    {
        bool const isSingleTargetTaunt = dynamic_cast<CastTauntAction*>(action) ||
            dynamic_cast<CastGrowlAction*>(action) ||
            dynamic_cast<CastHandOfReckoningAction*>(action) ||
            dynamic_cast<CastDarkCommandAction*>(action);
        if (!isSingleTargetTaunt)
            return 0.0f;

        Unit* target = action->GetTarget();
        if (target == malande)
            return PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ? 1.0f : 0.0f;

        return target && target == members.veras &&
            PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ? 1.0f : 0.0f;
    }

    // The council tanks handle hazards via encounter actions.
    if (isAvoidAoe)
    {
        if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
            return 0.0f;

        return members.veras && !IsVerasVanished(members.veras) &&
            PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ? 0.0f : 1.0f;
    }

    // The Malande tank is the only one without a planned position, so it is allowed to tank face.
    // Tank face arcs around Malande, so while she stands in a patch it moves the tank back into it,
    // undoing the Focus Malande action's step out.
    if (dynamic_cast<TankFaceAction*>(action) && PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        float const reach = bot->GetMeleeRange(malande) + COUNCIL_PATCH_MARGIN;
        return IsInCouncilPatch(*malande, gathios, zerevor, reach) ? 0.0f : 1.0f;
    }

    return 0.0f;
}

float IllidariCouncilControlMovementAndTargetingMultiplier::GetNonTankValue(Action* action)
{
    if (dynamic_cast<SetBehindTargetAction*>(action))
        return 1.0f;

    if (!dynamic_cast<CombatFormationMoveAction*>(action) && !IsRepositionAction(bot, action) &&
        !dynamic_cast<FollowAction*>(action) && !dynamic_cast<FleeAction*>(action))
    {
        // Healers are subject to stricter rules.
        return PlayerbotAI::IsHeal(bot) ? GetHealerValue(action) : 1.0f;
    }

    return GetIllidariCouncilMembers(botAI).gathios ? 0.0f : 1.0f;
}

// The mage tank's healer moves only by its own positioning action when Zerevor is on the mage tank,
// and no other healer attempts to reach the mage tank to heal it.
float IllidariCouncilControlMovementAndTargetingMultiplier::GetHealerValue(Action* action)
{
    bool const isHealReach = dynamic_cast<ReachPartyMemberToHealAction*>(action);
    bool const isHealerMove = dynamic_cast<MovementAction*>(action) &&
        !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<IllidariCouncilPositionMageTankHealerAction*>(action);
    if (!isHealReach && !isHealerMove)
        return 1.0f;

    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.zerevor)
        return 1.0f;

    bool const isMageTankHealer = PlayerbotAI::IsAssistHealOfIndex(bot, 0, true);

    if (isHealReach && !isMageTankHealer)
    {
        Player* mageTank = GetZerevorMageTank(botAI);
        if (mageTank && AI_VALUE(Unit*, "party member to heal") == mageTank)
            return 0.0f;
    }

    if (isHealerMove)
        return isMageTankHealer && IsZerevorOnMageTank(botAI, members.zerevor) ? 0.0f : 1.0f;

    return 1.0f;
}

float IllidariCouncilDisableMageTankActionsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_MAGE)
        return 1.0f;

    if (action->getThreatType() != Action::ActionThreatType::Aoe &&
        !dynamic_cast<AvoidAoeAction*>(action) && !dynamic_cast<CastIceBlockAction*>(action) &&
        !dynamic_cast<CastInvisibilityAction*>(action))
    {
        return 1.0f;
    }

    if (!IsZerevorMageTank(botAI))
        return 1.0f;

    return GetIllidariCouncilMembers(botAI).zerevor ? 0.0f : 1.0f;
}

// Dampen Magic is the only buff on the council that can be dispelled, and the mage tank steals it.
float IllidariCouncilHoldMisdirectionAndDispelsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    switch (bot->getClass())
    {
        case CLASS_HUNTER:
        {
            if (!dynamic_cast<CastMisdirectionOnMainTankAction*>(action) &&
                !dynamic_cast<CastTranquilizingShotAction*>(action))
            {
                return 1.0f;
            }
            break;
        }
        case CLASS_SHAMAN:
        {
            if (!dynamic_cast<CastPurgeAction*>(action))
                return 1.0f;
            break;
        }
        default:
            return 1.0f;
    }

    return GetIllidariCouncilMembers(botAI).gathios ? 0.0f : 1.0f;
}

// Ranged interrupts are saved for Circle of Healing. Melee ones are gated by a range check
// because otherwise cast failed due to being out of range still turns the bot toward its target.
float IllidariCouncilManageInterruptsMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    switch (bot->getClass())
    {
        case CLASS_MAGE:
        {
            if (!dynamic_cast<CastCounterspellAction*>(action) &&
                !dynamic_cast<CastCounterspellOnEnemyHealerAction*>(action))
            {
                return 1.0f;
            }
            break;
        }
        case CLASS_SHAMAN:
        {
            if (!dynamic_cast<CastWindShearAction*>(action) &&
                !dynamic_cast<CastWindShearOnEnemyHealerAction*>(action))
            {
                return 1.0f;
            }
            break;
        }
        case CLASS_WARLOCK:
        {
            if (!dynamic_cast<CastSpellLockAction*>(action))
                return 1.0f;
            break;
        }
        case CLASS_DEATH_KNIGHT:
        case CLASS_ROGUE:
        case CLASS_WARRIOR:
        {
            if (!dynamic_cast<CastSpellOnEnemyHealerAction*>(action))
                return 1.0f;

            Unit* malande = GetIllidariCouncilMembers(botAI).malande;
            return malande && !bot->IsWithinMeleeRange(malande) ? 0.0f : 1.0f;
        }
        default:
            return 1.0f;
    }

    return GetIllidariCouncilMembers(botAI).malande ? 0.0f : 1.0f;
}

float IllidariCouncilWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsDpsHoldCandidate(bot, action))
        return 1.0f;

    BtInstanceState const* state = FindBtState(bot->GetInstanceId());
    if (!state || !state->councilDpsWaitTimer ||
        GetMSTimeDiffToNow(*state->councilDpsWaitTimer) >= COUNCIL_DPS_WAIT_MS)
    {
        return 1.0f;
    }

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 1, false) ||
        IsZerevorMageTank(botAI))
    {
        return 1.0f;
    }

    return GetDpsHoldValue(bot, action);
}

// Illidan Stormrage <The Betrayer>

namespace
{

bool IsHeldAtIllidanLandingStack(Action* action)
{
    return dynamic_cast<MovementAction*>(action) && !dynamic_cast<AttackAction*>(action) &&
        !dynamic_cast<IllidanStormragePositionForLandingAction*>(action) &&
        !dynamic_cast<IllidanStormrageIsolateBotWithInfectionAction*>(action);
}

bool IsHeldIllidanReach(PlayerbotAI* botAI, Action* action, IllidanPhase phase)
{
    Player* bot = botAI->GetBot();

    if (!dynamic_cast<ReachTargetAction*>(action) &&
        !dynamic_cast<SetBehindTargetAction*>(action) &&
        !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !IsKillingSpree(bot, action))
    {
        return false;
    }

    if (HasParasiticShadowfiend(bot))
        return true;

    if (phase != IllidanPhase::Demon)
        return false;

    if (!PlayerbotAI::IsMelee(bot))
        return true;

    return !IsTargetingShadowDemon(botAI);
}

bool IsHeldIllidanHeal(PlayerbotAI* botAI, Unit* illidan, Action* action)
{
    if (!dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None)
        return true;

    if (phase == IllidanPhase::Flying || GetIllidanDpsHold(botAI, illidan) == IllidanDpsHold::None)
        return false;

    if (illidan->HasUnitState(UNIT_STATE_CONTROLLED))
        return false;

    return !DoIllidanTanksHaveAggro(botAI, *illidan, phase);
}

bool IsIllidanAoeAction(Player* bot, Action* action)
{
    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    return IsAoeThreatAction(bot, action) ||
        (castSpellAction && castSpellAction->getThreatType() == Action::ActionThreatType::Aoe);
}

bool IsGuardianSummon(Player* bot, Action* action)
{
    switch (bot->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        {
            return dynamic_cast<CastSummonGargoyleAction*>(action) ||
                dynamic_cast<CastArmyOfTheDeadAction*>(action);
        }
        case CLASS_DRUID:
            return dynamic_cast<CastForceOfNatureAction*>(action);
        case CLASS_MAGE:
            return dynamic_cast<CastMirrorImageAction*>(action);
        case CLASS_PRIEST:
            return dynamic_cast<CastShadowfiendAction*>(action);
        case CLASS_SHAMAN:
        {
            return dynamic_cast<CastFeralSpiritAction*>(action) ||
                dynamic_cast<CastFireElementalTotemAction*>(action) ||
                dynamic_cast<CastFireElementalTotemMeleeAction*>(action);
        }
        default:
            return false;
    }
}

bool IsIllidanShadowBlastOnBot(Unit* illidan, Player* bot)
{
    if (!illidan)
        return false;

    Spell const* spell = illidan->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return spell && spell->m_spellInfo->Id == Id(BtSpells::SPELL_SHADOW_BLAST) &&
        spell->m_targets.GetUnitTargetGUID() == bot->GetGUID();
}

} // end anonymous namespace (Illidan Stormrage)

// During Illidan's speech, bots move only to their pull places. Not gated on the encounter, which
// starts as the speech ends.
float IllidanStormrageHoldPullPositionsMultiplier::GetValue(Action* action)
{
    if (botAI->GetState() == BOT_STATE_COMBAT || bot->GetMapId() != BT_MAP_ID)
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        dynamic_cast<IllidanStormragePositionOnGrateDuringSpeechAction*>(action))
    {
        return 1.0f;
    }

    return AI_VALUE(bool, "illidan stormrage speech in progress") ? 0.0f : 1.0f;
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetValueInEncounter(Action* action)
{
    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return 1.0f;

    if (bot->getClass() == CLASS_WARRIOR && dynamic_cast<CastVigilanceAction*>(action))
    {
        Player* warlockTank = GetIllidanWarlockTank(botAI);
        return warlockTank && action->GetTarget() == warlockTank ? 0.0f : 1.0f;
    }

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None)
    {
        if (dynamic_cast<MovementAction*>(action) &&
            !dynamic_cast<IllidanStormrageMainTankRepositionBossAction*>(action))
        {
            return 0.0f;
        }

        return dynamic_cast<DropTargetAction*>(action) && PlayerbotAI::IsTank(bot) &&
            PlayerbotAI::IsMainTank(bot) ? 0.0f : 1.0f;
    }

    // Shadowfiends and infections do not end during the death scene, so infected bots need to
    // continue to stay away from the raid.
    if (phase == IllidanPhase::Dying)
    {
        bool const isHeldMove = dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) ||
            IsHeldIllidanReach(botAI, action, phase);
        return isHeldMove && HasParasiticShadowfiend(bot) ? 0.0f : 1.0f;
    }

    // Transforming from Demon to Humanoid phase does not end a pending Shadow Blast cast. The
    // warlock tank needs to stay in the Demon Form position until the cast finishes, as moving
    // to its phase 3/5 position early would result in Shadow Blast hitting the raid.
    if (phase != IllidanPhase::Demon && dynamic_cast<MovementAction*>(action) &&
        IsIllidanShadowBlastOnBot(illidan, bot))
    {
        return 0.0f;
    }

    // While Illidan is in flight and landing, he is unselectable and an invalid target. Drop target
    // needs to be held to keep bots in their combat engine so they do not revert to non-combat
    // strategies (particularly problematic with respect to healing).
    if (dynamic_cast<DropTargetAction*>(action))
        return phase == IllidanPhase::Flying || phase == IllidanPhase::Landing ? 0.0f : 1.0f;

    bool const isTankAssist = dynamic_cast<TankAssistAction*>(action);
    bool const isDpsAssist = dynamic_cast<DpsAssistAction*>(action);
    if (isTankAssist || isDpsAssist || dynamic_cast<CastDebuffSpellOnAttackerAction*>(action))
    {
        return botAI->GetState() == BOT_STATE_COMBAT ?
            GetTargetingValue(phase, action, isTankAssist, isDpsAssist) : 1.0f;
    }

    if (PlayerbotAI::IsTank(bot))
        return GetTankMovementValue(illidan, phase, action);

    return GetNonTankMovementValue(phase, action);
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetTargetingValue(
    IllidanPhase phase, Action* action, bool isTankAssist, bool isDpsAssist)
{
    if (isTankAssist)
        return phase == IllidanPhase::Landing && !PlayerbotAI::IsMainTank(bot) ? 1.0f : 0.0f;

    if (isDpsAssist)
    {
        if (phase == IllidanPhase::Demon || PlayerbotAI::IsRanged(bot))
            return 0.0f;

        return phase == IllidanPhase::Initial || phase == IllidanPhase::Flying ||
            phase == IllidanPhase::Grounded || phase == IllidanPhase::Maiev ? 0.0f : 1.0f;
    }

    if (action->getThreatType() == Action::ActionThreatType::None)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    uint32 const entry = target ? target->GetEntry() : 0;
    if (entry != Id(BtNpcs::NPC_SHADOW_DEMON) && entry != Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
        return 1.0f;

    // The opt-in threat strategy reads an add no tank has touched as 100% threat.
    context->GetValue<bool>("neglect threat")->Set(true);
    // A dot on an attacker would go on something other than the add, and the add must be focused.
    return 0.0f;
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetTankMovementValue(
    Unit* illidan, IllidanPhase phase, Action* action)
{
    bool const isMove = dynamic_cast<MovementAction*>(action);
    bool const isSpell = dynamic_cast<CastSpellAction*>(action);
    if (!isMove && !isSpell)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    bool const isGapCloser = dynamic_cast<CastReachTargetSpellAction*>(action);

    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Landing ||
        phase == IllidanPhase::Grounded || phase == IllidanPhase::Demon ||
        phase == IllidanPhase::Maiev)
    {
        if (!PlayerbotAI::IsMainTank(bot))
        {
            if (IsHeldIllidanReach(botAI, action, phase))
                return 0.0f;

            if (phase != IllidanPhase::Landing)
                return 1.0f;

            return isGapCloser || IsHeldAtIllidanLandingStack(action) ? 0.0f : 1.0f;
        }

        // If Illidan aggros onto another player during the Humanoid phase, the main tank returns to
        // standard tank assist and movement to try to pick Illidan back up.
        if ((phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
             phase == IllidanPhase::Maiev) && illidan->GetVictim() != bot)
        {
            return 1.0f;
        }

        bool const canChaseDemon = phase == IllidanPhase::Demon &&
            !IsIllidanDemonFormEnding(*illidan, ILLIDAN_MAIN_TANK_WALK_IN_MS) &&
            !IsHeldIllidanReach(botAI, action, phase);

        if (isMove)
        {
            if (dynamic_cast<IllidanStormrageMainTankRepositionBossAction*>(action) ||
                dynamic_cast<IllidanStormrageAssignTargetPriorityAction*>(action))
            {
                return 1.0f;
            }

            bool const isChase = dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<SetBehindTargetAction*>(action);
            return isChase && canChaseDemon ? 1.0f : 0.0f;
        }

        return isGapCloser && !canChaseDemon ? 0.0f : 1.0f;
    }

    if (phase != IllidanPhase::Flying)
        return 1.0f;

    if (IsFlameOfAzzinothTank(bot))
    {
        if (!isMove)
            return 1.0f;

        if (dynamic_cast<IllidanStormrageAssistTanksHandleFlamesAction*>(action) ||
            dynamic_cast<IllidanStormrageIsolateBotWithInfectionAction*>(action))
        {
            return 1.0f;
        }

        if (dynamic_cast<AvoidAoeAction*>(action) || IsHeldIllidanReach(botAI, action, phase))
            return 0.0f;

        // Flame tanks rely on standard tank assist and movement to pick up their Flame if loose.
        Unit* flame = GetAssignedFlameOfAzzinoth(bot);
        return flame && flame->GetVictim() != bot ? 1.0f : 0.0f;
    }

    if (isMove)
    {
        return dynamic_cast<IllidanStormragePositionAboveGrateAction*>(action) ||
            dynamic_cast<IllidanStormrageIsolateBotWithInfectionAction*>(action) ? 1.0f : 0.0f;
    }

    return isSpell ? 0.0f : 1.0f;
}

float IllidanStormrageControlMovementAndTargetingMultiplier::GetNonTankMovementValue(
    IllidanPhase phase, Action* action)
{
    bool const isBlockedMove =
        (dynamic_cast<CombatFormationMoveAction*>(action) &&
         !dynamic_cast<SetBehindTargetAction*>(action)) ||
        dynamic_cast<FollowAction*>(action) || dynamic_cast<FleeAction*>(action) ||
        IsRepositionAction(bot, action);
    bool const isPhaseMove = !isBlockedMove &&
        (dynamic_cast<ReachTargetAction*>(action) ||
         dynamic_cast<SetBehindTargetAction*>(action) ||
         dynamic_cast<AvoidAoeAction*>(action) ||
         dynamic_cast<CastReachTargetSpellAction*>(action) ||
         IsKillingSpree(bot, action));
    bool const isLandingMove = !isBlockedMove && IsHeldAtIllidanLandingStack(action);
    if (!isBlockedMove && !isPhaseMove && !isLandingMove)
        return 1.0f;

    if (isBlockedMove)
        return 0.0f;

    // Everyone waits at the landing stack while he lands.
    if (phase == IllidanPhase::Landing)
        return 0.0f;

    if (!isPhaseMove)
        return 1.0f;

    if (phase == IllidanPhase::Flying)
        return 0.0f;

    return IsHeldIllidanReach(botAI, action, phase) ? 0.0f : 1.0f;
}

// Near a new Shadowfiend, don't attack it, cast AoE, or heal. Switching targets is fine.
float IllidanStormrageHoldNearNewShadowfiendMultiplier::GetValueInEncounter(Action* action)
{
    auto castSpellAction = dynamic_cast<CastSpellAction*>(action);
    bool const isAttackAction = dynamic_cast<AttackAction*>(action);
    if (!castSpellAction && !isAttackAction)
        return 1.0f;

    Unit* target = castSpellAction ? AI_VALUE(Unit*, "current target") : action->GetTarget();
    bool const isOnShadowfiend =
        target && target->GetEntry() == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND);
    bool const isAoeSpell = IsIllidanAoeAction(bot, action);

    if (!isOnShadowfiend && !isAoeSpell && !dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!GetEncounterBoss())
        return 1.0f;

    return IsNearNewParasiticShadowfiend(botAI) ? 0.0f : 1.0f;
}

// Guardians cannot be set passive like pets, so they are summoned only in Phases 3 and 5 (to keep
// them from dying in Phase 4 and to save the cooldown through Phases 1 and 2), and not during any
// dps hold, which otherwise lets Mirror Image and Army of the Dead through as buffs.
float IllidanStormrageHoldGuardianSummonsMultiplier::GetValueInEncounter(Action* action)
{
    if (!IsGuardianSummon(bot, action))
        return 1.0f;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return 1.0f;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase != IllidanPhase::Grounded && phase != IllidanPhase::Maiev)
        return 0.0f;

    return GetIllidanDpsHold(botAI, illidan) == IllidanDpsHold::None ? 1.0f : 0.0f;
}

// Shamans do not set totems during phase changes until the tanks have aggro, so the ones they
// recalled are not set again at once. In phases 1, 3, and 5 they also do not set totems if
// they are closer to the infection position than their nearest totem. Without this multiplier,
// Shamans will drop their totems at the infection position (and then drop them again after they
// return to their normal position).
float IllidanStormrageHoldTotemsMultiplier::GetValueInEncounter(Action* action)
{
    if (bot->getClass() != CLASS_SHAMAN ||
        (!dynamic_cast<CastTotemAction*>(action) &&
         !dynamic_cast<CastCallOfTheElementsAction*>(action)))
    {
        return 1.0f;
    }

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return 1.0f;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (IsIllidanTankAggroPending(*illidan, phase))
        return 0.0f;

    if (phase != IllidanPhase::Initial && phase != IllidanPhase::Grounded &&
        phase != IllidanPhase::Maiev)
    {
        return 1.0f;
    }

    float nearestTotem = std::numeric_limits<float>::max();
    bool hasTotem = false;
    for (uint8 slot = SUMMON_SLOT_TOTEM_FIRE; slot < MAX_TOTEM_SLOT; ++slot)
    {
        Creature* totem = bot->GetMap()->GetCreature(bot->m_SummonSlot[slot]);
        if (totem && totem->IsAlive())
        {
            nearestTotem = std::min(nearestTotem, bot->GetExactDist2d(totem));
            hasTotem = true;
        }
    }

    if (!hasTotem)
        return HasParasiticShadowfiend(bot) ? 0.0f : 1.0f;

    Position const infectionPosition = GetIllidanRecordedInfectionPosition(botAI, *illidan);
    return bot->GetExactDist2d(infectionPosition) < nearestTotem ? 0.0f : 1.0f;
}

float IllidanStormrageWaitForDpsMultiplier::GetValueInEncounter(Action* action)
{
    bool const isHeal = dynamic_cast<CastHealingSpellAction*>(action);
    bool const isItem = dynamic_cast<UseItemAction*>(action); // for mana potions and gems
    if (!isHeal && !isItem &&
        (!IsDpsHoldCandidate(bot, action) ||
         dynamic_cast<IllidanStormrageAssignTargetPriorityAction*>(action)))
    {
        return 1.0f;
    }

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return 1.0f;

    if (IsIllidanPickupPending(botAI, illidan))
    {
        if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
            return 1.0f;

        if (isHeal || isItem)
            return 0.0f;

        // hasOwnTarget addresses that secondary dots are not cast on the current target.
        bool const hasOwnTarget = dynamic_cast<AttackAction*>(action) ||
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
        Unit* target = hasOwnTarget ? action->GetTarget() : AI_VALUE(Unit*, "current target");
        bool const isAoe = action->getThreatType() == Action::ActionThreatType::Aoe;
        return IsIllidanPickupExemptTarget(target, isAoe) ? 1.0f : 0.0f;
    }

    if (isItem)
        return 1.0f;

    // Delegate any totem hold to IllidanStormrageHoldTotemsMultiplier.
    if (bot->getClass() == CLASS_SHAMAN &&
        (dynamic_cast<CastTotemAction*>(action) ||
         dynamic_cast<CastCallOfTheElementsAction*>(action)))
    {
        return 1.0f;
    }

    if (isHeal)
        return IsHeldIllidanHeal(botAI, illidan, action) ? 0.0f : 1.0f;

    if (GetIllidanPhase(illidan) == IllidanPhase::Flying)
    {
        if (!IsFlameOfAzzinothDpsHeld(*illidan) || IsFlameOfAzzinothTank(bot))
            return 1.0f;

        bool const hasOwnTarget = dynamic_cast<AttackAction*>(action) ||
            dynamic_cast<CastDebuffSpellOnAttackerAction*>(action);
        Unit* target = hasOwnTarget ? action->GetTarget() : AI_VALUE(Unit*, "current target");
        bool const isOnFlame = target && target->GetEntry() == Id(BtNpcs::NPC_FLAME_OF_AZZINOTH);
        return isOnFlame ? GetDpsHoldValue(bot, action) : 1.0f;
    }

    if (GetIllidanDpsHold(botAI, illidan) == IllidanDpsHold::None)
        return 1.0f;

    bool const isOnIllidan = AI_VALUE(Unit*, "current target") == illidan ||
        dynamic_cast<CastDebuffSpellOnAttackerAction*>(action) ||
        (dynamic_cast<AttackAction*>(action) && action->GetTarget() == illidan);

    return isOnIllidan ? GetDpsHoldValue(bot, action) : 1.0f;
}

float IllidanStormrageDisableTankSoulshatterMultiplier::GetValueInEncounter(Action* action)
{
    if (botAI->GetState() == BOT_STATE_NON_COMBAT)
        return 1.0f;

    if (bot->getClass() != CLASS_WARLOCK)
        return 1.0f;

    if (!dynamic_cast<CastSoulshatterAction*>(action))
        return 1.0f;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return 1.0f;

    return IsIllidanWarlockTank(botAI) ? 0.0f : 1.0f;
}

// While Shadow Demons are up, the only AoE allowed is Seed of Corruption, which has its own action.
float IllidanStormrageDisableAoeOnShadowDemonsMultiplier::GetValueInEncounter(Action* action)
{
    // Heals report AoE threat too.
    if (dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!IsIllidanAoeAction(bot, action))
        return 1.0f;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return 1.0f;

    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    return shadowDemons.empty() ? 1.0f : 0.0f;
}
