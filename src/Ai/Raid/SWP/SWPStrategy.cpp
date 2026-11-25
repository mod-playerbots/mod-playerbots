/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPStrategy.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPMultipliers.h"
#include "SWPShared.h"

using namespace SwpHelpers;
using namespace EncounterHelpers;

void RaidSwpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shared
    triggers.push_back(new TriggerNode("sunwell no encounter in progress",
        { NextAction("sunwell reset encounter states", ACTION_EMERGENCY + 10),
          NextAction("eredar twins announce class roles", ACTION_EMERGENCY + 8),
          NextAction("kil'jaeden announce dragon orb user", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("sunwell class roles should be assigned",
        { NextAction("sunwell assign class roles", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("sunwell aura to remove",
        { NextAction("sunwell remove aura", ACTION_EMERGENCY) }));

    // Trash
    triggers.push_back(new TriggerNode("volatile fiend self destructs when near",
        { NextAction("volatile fiend intercept", ACTION_EMERGENCY + 1),
          NextAction("volatile fiend move away", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("apocalypse guard protected by infernal defense",
        { NextAction("apocalypse guard attack with holy magic", ACTION_RAID) }));

    // Kalecgos
    triggers.push_back(new TriggerNode("kalecgos should communicate boss health",
        { NextAction("kalecgos announce boss health", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("kalecgos hunter should misdirect",
        { NextAction("kalecgos misdirect to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos requires tank rotation",
        { NextAction("kalecgos surface tank position dragon", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos spectral rift is open",
        { NextAction("kalecgos enter spectral rift", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kalecgos ranged should spread",
        { NextAction("kalecgos spread ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos too many arcane buffet stacks",
        { NextAction("kalecgos remove arcane buffet", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kalecgos humanoid kalec tanks sathrovarr",
        { NextAction("kalecgos sathrovarr tank stand with kalec", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos bots don't observe gravity",
        { NextAction("kalecgos return to spectral realm ground", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kalecgos should manage raid assignments",
        { NextAction("kalecgos update raid assignments", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("kalecgos dps hold is active",
        { NextAction("kalecgos stop attacking for hold", ACTION_EMERGENCY + 2),
          NextAction("kalecgos select target for hold", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos should manage pet aggression",
        { NextAction("kalecgos set pet stance", ACTION_RAID + 4) }));

    // Brutallus
    triggers.push_back(new TriggerNode("brutallus hunter should misdirect",
        { NextAction("brutallus misdirect to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus requires two tanks",
        { NextAction("brutallus tanks position and swap", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus melee should stand in place",
        { NextAction("brutallus position melee at rear center", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus ranged should soak meteor slash",
        { NextAction("brutallus position ranged in two groups", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus burn on non-tank",
        { NextAction("brutallus isolate burn", ACTION_EMERGENCY + 1) }));

    // Felmyst
    triggers.push_back(new TriggerNode("felmyst hunter should misdirect",
        { NextAction("felmyst misdirect to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst ground phase should be tanked",
        { NextAction("felmyst main tank position boss on ground", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst ranged should position to dispel and flee",
        { NextAction("felmyst ranged stack in three groups", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst melee should stay together",
        { NextAction("felmyst melee stack behind boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst encapsulate on mage or paladin",
        { NextAction("felmyst remove encapsulate", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst near encapsulated player",
        { NextAction("felmyst run away from encapsulated player", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst player has gas nova",
        { NextAction("felmyst mass dispel gas nova", ACTION_EMERGENCY + 6),
          NextAction("felmyst move into mass dispel reach", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("felmyst takeoff is near",
        { NextAction("felmyst move to air group position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst air phase has started",
        { NextAction("felmyst move to air group position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst should avoid demonic vapor trails",
        { NextAction("felmyst avoid demonic vapor", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("felmyst targeted by demonic vapor",
        { NextAction("felmyst kite demonic vapor", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst fog of corruption is active",
        { NextAction("felmyst move to safe fog lane", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst melee cannot reach flying boss",
        { NextAction("felmyst melee clear target", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst player is charmed by fog",
        { NextAction("felmyst kill charmed player", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst should hold dps while landing",
        { NextAction("felmyst manage landing dps timer", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("felmyst landing dps hold is active",
        { NextAction("felmyst stop attacking for landing", ACTION_EMERGENCY + 2),
          NextAction("felmyst select target for hold", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst should manage pet aggression",
        { NextAction("felmyst set pet stance", ACTION_RAID + 4) }));

    // Eredar Twins
    triggers.push_back(new TriggerNode("eredar twins should leave balcony",
        { NextAction("eredar twins jump from balcony", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("eredar twins hunter should misdirect",
        { NextAction("eredar twins misdirect to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash should be tanked",
        { NextAction("eredar twins position sacrolash tanks", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins alythess should be tanked",
        { NextAction("eredar twins alythess tank move out of blaze", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins ranged should stack on balcony",
        { NextAction("eredar twins ranged stack at balcony edge", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins should focus dps",
        { NextAction("eredar twins dps prioritize sacrolash", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins should manage pet aggression",
        { NextAction("eredar twins set pet stance", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("eredar twins too many flame touched stacks",
        { NextAction("eredar twins remove flame sear", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("eredar twins only alythess remains",
        { NextAction("eredar twins stack behind alythess", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("eredar twins alythess targets non-tank",
        { NextAction("eredar twins step aside from stack", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("eredar twins active conflagration target",
        { NextAction(
            "eredar twins conflagration target move from group", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash victim has conflagration",
        { NextAction("eredar twins move away from sacrolash victim", ACTION_EMERGENCY + 10) }));

    // M'uru
    triggers.push_back(new TriggerNode("m'uru hunter should misdirect",
        { NextAction("m'uru misdirect enemy to tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru entropius should be tanked",
        { NextAction("m'uru main tank pick up entropius", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru ranged should stack or spread",
        { NextAction("m'uru position ranged by phase", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru void sentinel should be tanked",
        { NextAction("m'uru tanks move sentinel to safe position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru entrance should be guarded",
        { NextAction("m'uru second assist tank guard ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru should assign dps priority",
        { NextAction("m'uru assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru dark fiends spawned",
        { NextAction("m'uru kill dark fiends with dispel", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("m'uru darkness is active",
        { NextAction("m'uru melee flee the darkness", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("m'uru berserker has flurry",
        { NextAction("m'uru cast stun on berserker", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("m'uru fury mage casting fel fireball",
        { NextAction("m'uru interrupt fel fireball", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("m'uru fury mage has spell fury",
        { NextAction("m'uru cast spellsteal on spell fury", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru void spawn available for enslave",
        { NextAction("m'uru warlock enslave void spawn", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("m'uru warlock has enslaved void spawn",
        { NextAction("m'uru void spawn cast shadow bolt volley", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("m'uru void zone is near",
        { NextAction("m'uru move clear of void zones and fiends", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("m'uru dark fiend is near",
        { NextAction("m'uru move clear of void zones and fiends", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("m'uru the singularity is near",
        { NextAction("m'uru escape the singularity", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru should manage pet aggression",
        { NextAction("m'uru set pet stance", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("m'uru pet should switch target",
        { NextAction("m'uru command pet target", ACTION_RAID + 3) }));

    // Kil'jaeden <The Deceiver>
    triggers.push_back(new TriggerNode("kil'jaeden hands of the deceiver are active",
        { NextAction("kil'jaeden control hands of the deceiver", ACTION_EMERGENCY),
          NextAction("kil'jaeden mark hand of the deceiver", ACTION_RAID + 1),
          NextAction("kil'jaeden move holy paladin into stun range", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden should hold dps at start",
        { NextAction("kil'jaeden start dps hold timer", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("kil'jaeden dps hold is active",
        { NextAction("kil'jaeden select target for hold", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden should be tanked",
        { NextAction("kil'jaeden position and move tanks", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden melee should split into two groups",
        { NextAction("kil'jaeden position melee and avoid armageddons", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden ranged should keep formation",
        { NextAction("kil'jaeden position ranged and avoid armageddons", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden fire bloom on immunity class",
        { NextAction("kil'jaeden remove fire bloom", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("kil'jaeden casts darkness of a thousand souls",
        { NextAction("kil'jaeden stack for shield of the blue", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden dragon orb is active",
        { NextAction("kil'jaeden use dragon orb", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot controls dragon",
        { NextAction("kil'jaeden dragon buff and protect raid", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("kil'jaeden stale root after dragon",
        { NextAction("kil'jaeden release stale root", ACTION_EMERGENCY + 10) }));

    // Above the stack for Shield of the Blue, so pets are held before their owners move.
    triggers.push_back(new TriggerNode("kil'jaeden should manage pet aggression",
        { NextAction("kil'jaeden set pet stance", ACTION_EMERGENCY + 12) }));
}

void RaidSwpStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Shared
    multipliers.push_back(new SunwellNoEncounterDrinkingMultiplier(botAI));

    // Trash
    multipliers.push_back(new VolatileFiendRestrictApproachMultiplier(botAI));

    // Shared Boss
    multipliers.push_back(new SunwellControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new SunwellDelayDpsCooldownsMultiplier(botAI));

    // Kalecgos
    multipliers.push_back(new KalecgosWaitToDecurseMultiplier(botAI));
    multipliers.push_back(new KalecgosControlMovementMultiplier(botAI));
    multipliers.push_back(new KalecgosRestrictTauntMultiplier(botAI));
    multipliers.push_back(new KalecgosSuppressAssistTankPullThreatMultiplier(botAI));
    multipliers.push_back(new KalecgosEnterSpectralRiftMultiplier(botAI));
    multipliers.push_back(new KalecgosHoldDpsMultiplier(botAI));

    // Brutallus
    multipliers.push_back(new BrutallusControlMovementMultiplier(botAI));
    multipliers.push_back(new BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(botAI));
    multipliers.push_back(new BrutallusRestrictTauntMultiplier(botAI));
    multipliers.push_back(new BrutallusNoRebirthWhileBurningMultiplier(botAI));

    // Felmyst
    multipliers.push_back(new FelmystControlMovementMultiplier(botAI));
    multipliers.push_back(new FelmystWaitForLandingDpsMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeEncapsulateAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystKeepHealersInSideStacksMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeDemonicVaporAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeFogAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystFocusAttacksOnCharmedPlayerMultiplier(botAI));
    multipliers.push_back(new FelmystDisableDotsOnAddsMultiplier(botAI));

    // Eredar Twins
    multipliers.push_back(new EredarTwinsDisableAutoTargetingMultiplier(botAI));
    multipliers.push_back(new EredarTwinsHoldDpsAtStartMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlThreatMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMovementMultiplier(botAI));
    multipliers.push_back(new EredarTwinsIsolateConflagrationMultiplier(botAI));

    // M'uru
    multipliers.push_back(new MuruDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new MuruControlMovementMultiplier(botAI));
    multipliers.push_back(new MuruHoldDpsOnEntropiusMultiplier(botAI));
    multipliers.push_back(new MuruLetMagesStealSpellFuryMultiplier(botAI));

    // Kil'jaeden <The Deceiver>
    multipliers.push_back(new KiljaedenSingleTargetHandsMultiplier(botAI));
    multipliers.push_back(new KiljaedenHoldDpsAtStartMultiplier(botAI));
    multipliers.push_back(new KiljaedenControlMovementAndTargetingMultiplier(botAI));
    multipliers.push_back(new KiljaedenPrioritizeDarknessProtectionMultiplier(botAI));
    multipliers.push_back(new KiljaedenControlDragonMultiplier(botAI));
}

namespace
{

void AppendVolatileFiendMeleeDpsExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsRanged(bot) || PlayerbotAI::IsTank(bot))
        return;

    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == Id(SwpNpcs::NPC_VOLATILE_FIEND))
            exclusions.insert(guid);
    }
}

void AppendApocalypseGuardInfernalDefenseExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsTank(bot) || bot->getClass() == CLASS_PALADIN ||
        bot->getClass() == CLASS_PRIEST)
    {
        return;
    }

    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == Id(SwpNpcs::NPC_APOCALYPSE_GUARD) &&
            attacker->HasAura(Id(SwpSpells::SPELL_INFERNAL_DEFENSE)))
        {
            exclusions.insert(guid);
        }
    }
}

void AppendFelmystVaporPhaseMeleeExclusions(Player* bot, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsMelee(bot))
        return;

    Unit* felmyst = GetSwpBoss(bot, SwpBosses::DATA_FELMYST);
    if (felmyst && IsFelmystAirPhaseTargetSuppressed(felmyst))
        exclusions.insert(felmyst->GetGUID());
}

void AppendMuruDarkFiendExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == Id(SwpNpcs::NPC_DARK_FIEND))
            exclusions.insert(guid);
    }
}

void AppendMuruTankExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsTank(bot))
        return;

    Unit* muru = GetSwpBoss(bot, SwpBosses::DATA_MURU);
    if (!muru || !IsMuruPhaseActive(muru))
        return;

    bool const darknessActive = IsMuruDarknessActive(muru);
    // Even during Darkness, the Sentinel Tank has full freedom to pick up Sentinels.
    bool const distanceUnrestricted = darknessActive &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);

    ObjectGuid const muruGuid = muru->GetGUID();

    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || attacker->GetEntry() == Id(SwpNpcs::NPC_VOID_SENTINEL))
            continue;

        if (darknessActive && guid == muruGuid)
        {
            exclusions.insert(guid);
            continue;
        }

        if (distanceUnrestricted)
            continue;

        if (attacker->GetExactDist2d(MURU_STACK_POSITION) > MURU_MAX_TARGET_DIST_FROM_STACK)
            exclusions.insert(guid);
    }
}

// Shield Orbs fly out of melee reach. Ranged skip the ones they cannot hit from where they stand,
// since they cannot leave their positions to reach them.
void AppendKiljaedenShieldOrbExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    bool const isMelee = PlayerbotAI::IsMelee(bot);
    // The same test as the standard "enemy out of spell range" trigger
    float const spellRange = botAI->GetRange("spell") + CONTACT_DISTANCE;

    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || attacker->GetEntry() != Id(SwpNpcs::NPC_SHIELD_ORB))
            continue;

        if (isMelee || !bot->IsWithinCombatRange(attacker, spellRange))
            exclusions.insert(guid);
    }
}

// Non-tanks do not attack Reflections they cannot hit from where they stand, since their position
// actions would pull them straight back. Melee and Demo Locks in Metamorphosis (who may leave their
// positions while Reflections are up) also skip Reflections inside Kil'jaeden's Knockback radius,
// or if reaching them would require crossing it.
void AppendKiljaedenReflectionExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (PlayerbotAI::IsTank(bot))
        return;

    bool const isMetamorphosis = bot->getClass() == CLASS_WARLOCK &&
        bot->HasAura(Id(SwpSpells::SPELL_METAMORPHOSIS));
    bool const isMelee = PlayerbotAI::IsMelee(bot);
    // The same test as the standard "enemy out of spell range" trigger
    float const spellRange = botAI->GetRange("spell") + CONTACT_DISTANCE;

    for (auto const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || attacker->GetEntry() != Id(SwpNpcs::NPC_SINISTER_REFLECTION))
            continue;

        bool isExcluded = false;
        if (isMetamorphosis)
            isExcluded = IsMeleeBlockedByKiljaedenKnockback(*bot, *attacker);
        else if (isMelee)
        {
            isExcluded = IsMeleeBlockedByKiljaedenKnockback(*bot, *attacker) ||
                !bot->IsWithinMeleeRange(attacker);
        }
        else
            isExcluded = !bot->IsWithinCombatRange(attacker, spellRange);

        if (isExcluded)
            exclusions.insert(guid);
    }
}

} // end anonymous namespace

void RaidSwpStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SWP_MAP_ID)
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    AppendVolatileFiendMeleeDpsExclusions(botAI, context, exclusions);
    AppendApocalypseGuardInfernalDefenseExclusions(botAI, context, exclusions);

    if (!IsEncounterInProgress(bot, SWP_MAP_ID))
        return;

    AppendFelmystVaporPhaseMeleeExclusions(bot, exclusions);
    AppendMuruDarkFiendExclusions(botAI, context, exclusions);
    AppendMuruTankExclusions(botAI, context, exclusions);
    AppendKiljaedenShieldOrbExclusions(botAI, context, exclusions);
    AppendKiljaedenReflectionExclusions(botAI, context, exclusions);
}
