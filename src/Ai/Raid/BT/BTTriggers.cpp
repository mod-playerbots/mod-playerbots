/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTTriggers.h"
#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "Spell.h"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace BtHelpers;
using namespace EncounterHelpers;

// Shared

bool BlackTempleNoEncounterInProgressTrigger::IsActive()
{
    return !IsEncounterInProgress(bot, BT_MAP_ID);
}

bool BlackTempleClassRolesShouldBeAssignedTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID);
}

// Misdirect immediately after the pull. Used for High Warlord Naj'entus, Supremus, Teron Gorefiend,
// Gurtogg Bloodboil, Mother Shahraz, and the Illidari Council (on Gathios).
bool BlackTempleHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* boss = _target ? GetBtCreature(bot, *_target) : GetEncounterBoss();
    return boss && boss->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

// Trash

bool SisterOfPleasureShouldBeFocusedTrigger::IsActive()
{
    if (IsMechanicTrackerBot(bot, BT_MAP_ID))
        return !GetSkulledSisterOfPain(bot) && FindLinkedSisterOfPleasure(botAI);

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == Id(BtNpcs::NPC_SISTER_OF_PAIN) &&
        GetSkulledSisterOfPain(bot) == target;
}

// Wands keep shooting once started, so a multiplier alone can't stop wanding.
bool ShadowmoonReaverWandBuildsChargesTrigger::IsActive()
{
    Spell* wand = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    if (!wand || GetChaoticChargeReach(wand->m_spellInfo) == ChaoticChargeReach::None)
        return false;

    return IsShadowmoonReaverUnsafeForMagic(wand->m_targets.GetUnitTarget());
}

// A pet keeps its victim when its owner switches (PetAI::OwnerAttacked), so it needs commanding.
bool ShadowmoonReaverShouldControlCasterPetTrigger::IsActive()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!IsChargeBuildingPet(pet))
        return false;

    // Also true while the pet is passive, so the action can restore a stance it set.
    return pet->HasReactState(REACT_PASSIVE) ||
        !context->GetValue<GuidVector>("shadowmoon reavers")->RefGet().empty();
}

// High Warlord Naj'entus

bool HighWarlordNajentusShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool HighWarlordNajentusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!GetEncounterBoss())
        return false;

    return GetNearestPlayerInRadius(bot, NAJENTUS_RANGED_SPREAD_DISTANCE);
}

bool HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!GetEncounterBoss())
        return false;

    return FindNajentusUnassignedImpaledPlayer(bot);
}

bool HighWarlordNajentusImpalingSpineOnGroupMemberTrigger::IsActiveInEncounter()
{
    return GetNajentusImpaledPlayerToFree(bot);
}

bool HighWarlordNajentusNeedsSpineThrowerTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    if (!GetEncounterBoss())
        return false;

    return !GetNajentusSpineThrower(bot);
}

bool HighWarlordNajentusHasTidalShieldTrigger::IsActiveInEncounter()
{
    if (!IsNajentusSpineThrower(bot) || IsNajentusSpineInFlight(bot))
        return false;

    Unit* najentus = GetEncounterBoss();
    if (!najentus || !najentus->HasAura(Id(BtSpells::SPELL_TIDAL_SHIELD)))
        return false;

    return bot->HasItemCount(Id(BtItems::ITEM_NAJENTUS_SPINE));
}

// Supremus

bool SupremusShouldBePulledBackByTankTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* supremus = GetEncounterBoss();
    if (!supremus || IsSupremusKitePhase(supremus))
        return false;

    return supremus->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool SupremusRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* supremus = GetEncounterBoss();
    if (!supremus || IsSupremusKitePhase(supremus))
        return false;

    // The spread step flees from the nearest player without considering hazards, so the trigger
    // returns false if one step (plus 1 yd of slack) could put the bot inside a hazard.
    float const fleeStep =
        std::min(SUPREMUS_RANGED_SPREAD_DISTANCE + 1.0f, sPlayerbotAIConfig.FleeDistance);
    if (IsInSupremusHazard(
            GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
            SupremusHazardZone::Safe, fleeStep))
    {
        return false;
    }

    return GetNearestPlayerInRadius(bot, SUPREMUS_RANGED_SPREAD_DISTANCE);
}

bool SupremusFixatedOnBotTrigger::IsActiveInEncounter()
{
    Unit* supremus = GetEncounterBoss();
    if (!supremus || supremus->GetVictim() != bot || !IsSupremusKitePhase(supremus))
        return false;

    if (CanSupremusCatchStandingBot(bot, supremus))
        return true;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusNearFireTrigger::IsActiveInEncounter()
{
    if (!GetEncounterBoss())
        return false;

    if (bot->isMoving() && CanWalkThroughSupremusFire(bot))
        return false;

    return IsInSupremusHazard(
        GetSupremusHazards(botAI), bot->GetPositionX(), bot->GetPositionY(),
        SupremusHazardZone::Safe);
}

bool SupremusReachBlockedByFireTrigger::IsActiveInEncounter()
{
    if (!GetEncounterBoss())
        return false;

    return GetSupremusFireBlockedReachTarget(botAI);
}

// Shade of Akama

bool ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger::IsActiveInEncounter()
{
    bool const isMeleeDps = PlayerbotAI::IsDps(bot) && PlayerbotAI::IsMelee(bot);
    bool const isShadowPriest =
        bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("shadow", BOT_STATE_COMBAT);
    // The mechanic tracker is included solely to mark, as its role is indeterminate.
    if (!isMeleeDps && !isShadowPriest && !IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    return GetShadeOfAkamaKillTarget(botAI);
}

// Teron Gorefiend

bool TeronGorefiendShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetEncounterBoss();
}

bool TeronGorefiendRangedShouldPositionOnBalconyTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsRanged(bot) && GetEncounterBoss();
}

// Feign Death and Vanish cancel the Shadow of Death cast. It is Blizzlike for Ice Block and Divine
// Shield to block the cast, but Shadow of Death pierces their invulnerability in AC.
bool TeronGorefiendCastsShadowOfDeathTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_ROGUE)
        return false;

    Unit* gorefiend = GetEncounterBoss();
    if (!gorefiend)
        return false;

    // Once the bolt is launched, nothing cancels it.
    Spell* shadow = gorefiend->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->getState() == SPELL_STATE_PREPARING &&
        shadow->m_targets.GetUnitTarget() == bot;
}

bool TeronGorefiendShouldPositionForVengefulSpiritTrigger::IsActiveInEncounter()
{
    Aura* shadow = bot->GetAura(Id(BtSpells::SPELL_SHADOW_OF_DEATH));
    return shadow && shadow->GetDuration() <= GOREFIEND_SHADOW_OF_DEATH_MOVE_MS;
}

bool TeronGorefiendTransformedIntoVengefulSpiritTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE));
}

// Gurtogg Bloodboil

bool GurtoggBloodboilShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* gurtogg = GetEncounterBoss();
    return gurtogg && !gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
}

bool GurtoggBloodboilShouldPositionForBloodboilTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    // Bloodboil is not cast during Fel Rage.
    Unit* gurtogg = GetEncounterBoss();
    if (!gurtogg || gurtogg->HasAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE)))
        return false;

    return !IsGurtoggFelRageTarget(bot);
}

bool GurtoggBloodboilMeleeShouldStayBehindTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* gurtogg = GetEncounterBoss();
    if (!gurtogg)
        return false;

    if (IsGurtoggFelRageTarget(bot))
        return false;

    Player* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget)
        return false;

    return bot->GetExactDist2d(GetGurtoggFelRageMeleePosition(bot, *gurtogg, *felRageTarget)) >
        GURTOGG_FEL_RAGE_MELEE_TOLERANCE;
}

// Melee go behind Gurtogg instead.
bool GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger::IsActiveInEncounter()
{
    if (PlayerbotAI::IsMelee(bot) || IsGurtoggFelRageTarget(bot))
        return false;

    Unit* gurtogg = GetEncounterBoss();
    if (!gurtogg)
        return false;

    Player* felRageTarget = GetGurtoggFelRageTarget(gurtogg);
    if (!felRageTarget || bot->GetExactDist2d(felRageTarget) >= GURTOGG_FEL_RAGE_AVOID_DISTANCE)
        return false;

    // Arcing Smash only hits in front of Gurtogg, but he turns faster than bots can react, so at
    // the start of Fel Rage, everyone near the target moves away.
    Aura* felRage = gurtogg->GetAura(Id(BtSpells::SPELL_BOSS_FEL_RAGE));
    if (felRage && GetAuraElapsedMs(felRage) < GURTOGG_FEL_RAGE_EARLY_AVOID_MS)
        return true;

    return gurtogg->HasInArc(static_cast<float>(M_PI), bot);
}

bool GurtoggBloodboilFelRageOnBotTrigger::IsActiveInEncounter()
{
    return IsGurtoggFelRageTarget(bot) &&
        bot->GetExactDist2d(GURTOGG_TANK_POSITION) > BT_ARRIVAL_DISTANCE;
}

// Reliquary of Souls

bool ReliquaryOfSoulsHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (desire && desire->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT)
        return true;

    Unit* anger = AI_VALUE2(Unit*, "find target", "essence of anger");
    return anger && anger->GetHealthPct() > BOSS_ENGAGED_HEALTH_PCT;
}

bool ReliquaryOfSoulsShouldPositionForSufferingTrigger::IsActiveInEncounter()
{
    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        IsOutOfSufferingPosition(bot, suffering);
}

bool ReliquaryOfSoulsHealersShouldAttackSufferingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    // Aura of Suffering (-100% healing) does not affect Power Word: Shield.
    if (bot->getClass() == CLASS_PRIEST && botAI->HasStrategy("disc", BOT_STATE_COMBAT))
        return false;

    Unit* suffering = AI_VALUE2(Unit*, "find target", "essence of suffering");
    return suffering && !suffering->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
}

bool ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    return desire && desire->HasAura(Id(BtSpells::SPELL_RUNE_SHIELD));
}

bool ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* desire = AI_VALUE2(Unit*, "find target", "essence of desire");
    if (!desire)
        return false;

    Spell* deaden = desire->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_DEADEN));
    return deaden && deaden->getState() == SPELL_STATE_PREPARING &&
        deaden->m_targets.GetUnitTarget() == bot;
}

// Mother Shahraz

bool MotherShahrazShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetEncounterBoss())
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazBeingPositionedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return false;

    return IsShahrazBeingPositioned(GetEncounterBoss());
}

bool MotherShahrazRangedShouldPositionUnderStatueTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!GetEncounterBoss())
        return false;

    return !bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

bool MotherShahrazFatalAttractionTrigger::IsActiveInEncounter()
{
    return bot->HasAura(Id(BtSpells::SPELL_FATAL_ATTRACTION));
}

// Illidari Council

bool IllidariCouncilGathiosShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (!GetIllidariCouncilMembers(botAI).gathios)
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilGathiosCastingJudgementOfCommandTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARRIOR || !PlayerbotAI::IsTank(bot))
        return false;

    Unit* gathios = GetIllidariCouncilMembers(botAI).gathios;
    if (!gathios || !gathios->HasAura(Id(BtSpells::SPELL_SEAL_OF_COMMAND)))
        return false;

    Spell* judgement = gathios->FindCurrentSpellBySpellId(Id(BtSpells::SPELL_JUDGEMENT));
    return judgement && judgement->getState() == SPELL_STATE_PREPARING &&
        judgement->m_targets.GetUnitTarget() == bot && PlayerbotAI::IsMainTank(bot);
}

bool IllidariCouncilMalandeCastingCircleOfHealingTrigger::IsActiveInEncounter()
{
    if (bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_HUNTER ||
        bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_PRIEST)
    {
        return false;
    }

    if (!PlayerbotAI::IsDps(bot) || IsZerevorMageTank(botAI))
        return false;

    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.malande || !members.gathios || !members.zerevor)
        return false;

    if (PlayerbotAI::IsRanged(bot) &&
        !ShouldRangedInterruptCircleOfHealing(members.malande, members.gathios, members.zerevor))
    {
        return false;
    }

    return CanInterruptCircleOfHealing(members.malande);
}

bool IllidariCouncilMalandeShouldBeTankedTrigger::IsActiveInEncounter()
{
    return PlayerbotAI::IsTank(bot) && GetIllidariCouncilMembers(botAI).malande &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false);
}

bool IllidariCouncilVerasShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* veras = GetIllidariCouncilMembers(botAI).veras;
    if (!veras || IsVerasVanished(veras))
        return false;

    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, false);
}

bool IllidariCouncilZerevorShouldBeTankedByMageTrigger::IsActiveInEncounter()
{
    return IsZerevorMageTank(botAI) && GetIllidariCouncilMembers(botAI).zerevor;
}

bool IllidariCouncilMageTankHealerShouldRepositionTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsHeal(bot))
        return false;

    Unit* zerevor = GetIllidariCouncilMembers(botAI).zerevor;
    if (!zerevor)
        return false;

    if (!PlayerbotAI::IsAssistHealOfIndex(bot, 0, true) || !IsZerevorOnMageTank(botAI, zerevor))
        return false;

    Player* mageTank = GetZerevorMageTank(botAI);
    return !IsMageTankHealerPositionSafe(bot->GetPosition(), mageTank, zerevor) ||
        !bot->IsWithinLOSInMap(mageTank);
}

bool IllidariCouncilRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    if (!GetIllidariCouncilMembers(botAI).zerevor)
        return false;

    if (IsZerevorMageTank(botAI) || PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return false;

    return !HasDangerousCouncilAura(bot) &&
        GetNearestPlayerInRadius(bot, COUNCIL_RANGED_SPREAD_DISTANCE);
}

bool IllidariCouncilPetShouldSwitchTargetTrigger::IsActiveInEncounter()
{
    Guardian* pet = bot->GetGuardianPet();
    if (!pet || !pet->IsAlive() || pet->HasReactState(REACT_PASSIVE))
        return false;

    Unit* target = GetCouncilPetTarget(botAI, pet);
    return target && pet->GetVictim() != target;
}

bool IllidariCouncilShouldAssignDpsPriorityTrigger::IsActiveInEncounter()
{
    IllidariCouncilMembers const members = GetIllidariCouncilMembers(botAI);
    if (!members.gathios)
        return false;

    if ((PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        IsZerevorMageTank(botAI))
    {
        return false;
    }

    // The Veras tank is included in the DPS assignment action when Veras is vanished.
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 1, false))
        return true;

    return !members.veras || IsVerasVanished(members.veras);
}

bool IllidariCouncilShouldManageDpsTimerTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) && GetIllidariCouncilMembers(botAI).gathios;
}

// Illidan Stormrage <The Betrayer>

// Not gated on the encounter, which starts as the speech ends, so throttled to once per second.
bool IllidanStormrageShouldPositionForPullTrigger::IsActive()
{
    // You are not prepared!
    return AI_VALUE(bool, "illidan stormrage speech in progress");
}

bool IllidanStormrageHunterShouldMisdirectTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev)
    {
        return IsIllidanPhaseStartWithin(*illidan, phase, ILLIDAN_MAIN_TANK_PICKUP_MS);
    }

    return phase == IllidanPhase::Flying || phase == IllidanPhase::Demon;
}

bool IllidanStormrageShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Flying || phase == IllidanPhase::Dying)
        return false;

    // In Maiev's arrival, only once the tank is out of Shadow Prison.
    if (phase == IllidanPhase::None && bot->HasAura(Id(BtSpells::SPELL_SHADOW_PRISON)))
        return false;

    return PlayerbotAI::IsMainTank(bot);
}

// An infected bot runs to the infection position, and so does a trap hunter on duty, to lay its
// Frost Trap.
bool IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger::IsActiveInEncounter()
{
    if (!HasParasiticShadowfiend(bot) && !IsIllidanTrapHunterOnDuty(botAI))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || illidan->GetVictim() == bot)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None)
        return false;

    if (phase == IllidanPhase::Flying && !IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    return !IsExemptFromIllidanInfectionRun(botAI, illidan, bot);
}

// The mechanic tracker records each new infection while there is an infection position, keeps
// looking for a trap hunter while none is ready, and drops an entry once the infection is gone.
bool IllidanStormrageShouldTrackInfectionsTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    auto const& infected = context->GetValue<GuidVector>(
        "illidan stormrage players with parasitic shadowfiend")->RefGet();
    IllidanPhase const phase = GetIllidanPhase(illidan);
    bool const hasInfectionPosition = phase != IllidanPhase::None && phase != IllidanPhase::Flying;
    auto const* infections = FindIllidanInfections(bot->GetInstanceId());
    if (infections)
    {
        for (auto const& [player, infection] : *infections)
        {
            if (std::find(infected.begin(), infected.end(), player) == infected.end())
                return true;

            if (!hasInfectionPosition)
                continue;

            if (!infection.isTrapAssigned)
                return true;

            if (infection.hunter.IsEmpty())
                continue;

            if (IsExemptFromIllidanInfectionRun(
                    botAI, illidan, ObjectAccessor::GetPlayer(*bot, player)) ||
                !IsIllidanTrapHunterReady(ObjectAccessor::GetPlayer(*bot, infection.hunter)))
            {
                return true;
            }
        }
    }

    if (!hasInfectionPosition)
        return false;

    for (ObjectGuid const& guid : infected)
    {
        if (!infections || !infections->contains(guid))
            return true;
    }

    return false;
}

bool IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    return IsFlameOfAzzinothTank(bot) && !IsIllidanInfectionPositionKeptInFlight(botAI);
}

bool IllidanStormragePetsDieToHazardsTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    return ShouldIllidanPetsBePassive(botAI, illidan) != pet->HasReactState(REACT_PASSIVE);
}

// Pets are ideal for killing Shadowfiends because they cannot be infected.
bool IllidanStormragePetShouldAttackShadowfiendTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    // A pet's hit puts its owner on the Shadowfiend's threat list, so when the owner is next to a
    // newly birthed Shadowfiend, the pet needs to be called off to avoid an immediate infection.
    Unit* victim = pet->GetVictim();
    uint32 const victimEntry = victim && victim->IsAlive() ? victim->GetEntry() : 0;
    bool const isNearNewShadowfiend = IsNearNewParasiticShadowfiend(botAI);

    if (victimEntry == Id(BtNpcs::NPC_PARASITIC_SHADOWFIEND))
        return isNearNewShadowfiend;

    if (victimEntry == Id(BtNpcs::NPC_SHADOW_DEMON) || isNearNewShadowfiend)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::Demon)
    {
        if (GetShadowDemonForPetToAttack(botAI, *pet))
            return false;
    }
    else if (phase == IllidanPhase::Initial || phase == IllidanPhase::Flying ||
        phase == IllidanPhase::Grounded || phase == IllidanPhase::Maiev ||
        phase == IllidanPhase::Dying)
    {
        // During phase 2, pets clean up any residual Shadowfiends but otherwise remain passive to
        // not die to fire.
        if ((bot->getClass() != CLASS_HUNTER && bot->getClass() != CLASS_WARLOCK) ||
            (phase != IllidanPhase::Flying && pet->HasReactState(REACT_PASSIVE)))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return GetNearestParasiticShadowfiend(botAI, *pet);
}

bool IllidanStormragePetShouldAttackShadowDemonTrigger::IsActiveInEncounter()
{
    Pet* pet = bot->GetPet();
    if (!pet || !pet->IsAlive())
        return false;

    Unit* victim = pet->GetVictim();
    if (victim && victim->IsAlive() && victim->GetEntry() == Id(BtNpcs::NPC_SHADOW_DEMON))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return GetShadowDemonForPetToAttack(botAI, *pet);
}

bool IllidanStormrageShouldPositionAboveGrateTrigger::IsActiveInEncounter()
{
    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Flying)
        return false;

    if (IsIllidanInfectionPositionKeptInFlight(botAI))
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    return !IsFlameOfAzzinothTank(bot);
}

// Ice Block, Divine Shield, and Cloak of Shadows each clear Dark Barrage and Agonizing Flames.
// Exclude the main tank and Flame tanks since immunity causes them to lose their tank targets.
bool IllidanStormrageImmunityCanClearDebuffTrigger::IsActiveInEncounter()
{
    bool const isPaladin = bot->getClass() == CLASS_PALADIN;
    if (!isPaladin && bot->getClass() != CLASS_MAGE && bot->getClass() != CLASS_ROGUE)
        return false;

    Aura const* darkBarrage = bot->GetAura(Id(BtSpells::SPELL_DARK_BARRAGE));
    bool const isDarkBarrageDue =
        darkBarrage && GetAuraElapsedMs(darkBarrage) >= ILLIDAN_DARK_BARRAGE_REACTION_MS;
    if (!isDarkBarrageDue && !bot->HasAura(Id(BtSpells::SPELL_AGONIZING_FLAMES)))
        return false;

    if (isPaladin)
    {
        return !(PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot)) &&
            !IsFlameOfAzzinothTank(bot);
    }

    return true;
}

bool IllidanStormrageImmunityNoLongerNeededTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_MAGE &&
        (bot->getClass() != CLASS_PALADIN || PlayerbotAI::IsHeal(bot)))
    {
        return false;
    }

    uint32 const spellId = GetSelfImmunitySpell(bot);
    if (!spellId || !bot->HasAura(spellId))
        return false;

    // Below half health is a proxy for a scenario in which the immunity is keeping the bot from
    // dying, so do not remove it.
    constexpr float keepImmunityHealthPct = 50.0f;
    if (bot->GetHealthPct() <= keepImmunityHealthPct)
        return false;

    return GetEncounterBoss();
}

bool IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_SHAMAN)
        return false;

    bool hasTotem = false;
    for (uint8 slot = SUMMON_SLOT_TOTEM_FIRE; slot < MAX_TOTEM_SLOT; ++slot)
        hasTotem = hasTotem || !bot->m_SummonSlot[slot].IsEmpty();

    if (!hasTotem)
        return false;

    Unit* illidan = GetEncounterBoss();
    return illidan && IsIllidanTankAggroPending(*illidan, GetIllidanPhase(illidan));
}

bool IllidanStormrageShouldPositionForLandingTrigger::IsActiveInEncounter()
{
    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Landing)
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !HasParasiticShadowfiend(bot);
}

// Melee stop attacking during the Demon Form transformation since they need to get away before
// Aura of Dread becomes active.
bool IllidanStormrageMeleeShouldStopAttackingTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || (bot->GetVictim() != illidan && AI_VALUE(Unit*, "current target") != illidan))
        return false;

    IllidanDpsHold const window = GetIllidanHoldWindow(illidan);
    return window == IllidanDpsHold::DemonFormStart || window == IllidanDpsHold::DemonFormNoPush;
}

// Auto Shot, wands, and channels (Drain Soul, Mind Flay) need to be cancelled by an action during
// a hold.
bool IllidanStormrageShouldStopShootingTrigger::IsActiveInEncounter()
{
    Spell* shot = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    Spell* channel = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!shot && !channel)
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    if (channel && IsIllidanPickupChannelHeld(botAI, illidan, *channel))
        return true;

    ObjectGuid const guid = illidan->GetGUID();
    bool const isOnIllidan = (shot && shot->m_targets.GetUnitTargetGUID() == guid) ||
        (channel && channel->m_targets.GetUnitTargetGUID() == guid);
    return isOnIllidan && GetIllidanDpsHold(botAI, illidan) != IllidanDpsHold::None;
}

bool IllidanStormrageRangedShouldSpreadTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan)
        return false;

    IllidanPhase const phase = GetIllidanPhase(illidan);

    if (phase == IllidanPhase::Demon)
    {
        if (IsIllidanWarlockTank(botAI))
            return false;

        if (!PlayerbotAI::IsHeal(bot) && GetShadowDemonMovingToWarlockTank(botAI))
            return false;

        return true;
    }

    return phase == IllidanPhase::Initial || phase == IllidanPhase::Grounded ||
        phase == IllidanPhase::Maiev;
}

// A Demon fixated on the warlock tank is a 5-alarm fire so all ranged dps are freed from their
// assigned positions to move into range to attack it.
bool IllidanStormrageRangedShouldReachShadowDemonTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsRangedDps(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    if (HasParasiticShadowfiend(bot) || IsIllidanWarlockTank(botAI))
        return false;

    Unit* demon = GetShadowDemonMovingToWarlockTank(botAI);
    return demon && !bot->IsWithinCombatRange(demon, botAI->GetRange("spell"));
}

bool IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger::IsActiveInEncounter()
{
    if (!PlayerbotAI::IsMelee(bot))
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot))
        return false;

    return !IsTargetingShadowDemon(botAI);
}

bool IllidanStormrageWarlockShouldTankDemonFormTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return false;

    return IsIllidanWarlockTank(botAI);
}

// Every warlock, the tank included, casts Seed of Corruption on the Shadow Demons while two or more
// are alive. They all move toward the raid, so a Seed going off on one usually hits others.
bool IllidanStormrageWarlockShouldSeedShadowDemonsTrigger::IsActiveInEncounter()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* illidan = GetEncounterBoss();
    if (!illidan || GetIllidanPhase(illidan) != IllidanPhase::Demon || HasParasiticShadowfiend(bot))
        return false;

    if (IsIllidanPhaseStartWithin(*illidan, IllidanPhase::Demon, ILLIDAN_SHADOW_DEMON_SPAWN_MS))
        return false;

    auto const& shadowDemons =
        context->GetValue<std::vector<ShadowDemon>>("illidan stormrage shadow demons")->RefGet();
    if (shadowDemons.size() < 2)
        return false;

    return !IsNearNewParasiticShadowfiend(botAI);
}

bool IllidanStormrageShouldAssignTargetPriorityTrigger::IsActiveInEncounter()
{
    Unit* illidan = GetEncounterBoss();
    IllidanPhase const phase = GetIllidanPhase(illidan);
    if (phase == IllidanPhase::None || phase == IllidanPhase::Dying)
        return false;

    if (!PlayerbotAI::IsTank(bot))
        return true;

    if (phase != IllidanPhase::Initial && phase != IllidanPhase::Grounded &&
        phase != IllidanPhase::Demon && phase != IllidanPhase::Maiev)
    {
        return false;
    }

    if (!PlayerbotAI::IsMainTank(bot))
        return true;

    return phase == IllidanPhase::Demon &&
        !IsIllidanDemonFormEnding(*illidan, ILLIDAN_MAIN_TANK_WALK_IN_MS);
}

bool IllidanStormrageShouldManageDpsTimersTrigger::IsActiveInEncounter()
{
    return IsMechanicTrackerBot(bot, BT_MAP_ID) &&
        GetEncounterBoss();
}

// Destroying hazards after phase 2 is not gated behind CheatMask. The strategy simply cannot work
// without doing this.
bool IllidanStormrageShouldClearHazardsBeforeLandingTrigger::IsActiveInEncounter()
{
    if (!IsMechanicTrackerBot(bot, BT_MAP_ID))
        return false;

    Unit* illidan = GetEncounterBoss();
    return illidan && GetIllidanPhase(illidan) == IllidanPhase::Landing;
}
