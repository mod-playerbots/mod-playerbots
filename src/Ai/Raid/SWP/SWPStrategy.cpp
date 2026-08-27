/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPStrategy.h"
#include "Playerbots.h"
#include "SWPSharedConstants.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPMultipliers.h"

void RaidSunwellStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("sunwell plateau no encounter in progress", {
        NextAction("sunwell plateau reset encounter states", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("sunwell plateau bot has aura to remove", {
        NextAction("sunwell plateau remove aura", ACTION_EMERGENCY) }));

    // Trash
    triggers.push_back(new TriggerNode("volatile fiend self destructs when near", {
        NextAction("volatile fiend keep enemy away from group", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("apocalypse guard protected by infernal defense", {
        NextAction("apocalypse guard attack with holy magic", ACTION_RAID) }));

    // Kalecgos
    triggers.push_back(new TriggerNode("kalecgos should communicate boss health", {
        NextAction("kalecgos announce boss health", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos pulling boss", {
        NextAction("kalecgos misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos boss requires tank rotation", {
        NextAction("kalecgos surface tank position dragon", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos spectral rift is open", {
        NextAction("kalecgos enter spectral rift", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kalecgos bots take splash damage", {
        NextAction("kalecgos disperse ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos too many arcane buffet stacks", {
        NextAction("kalecgos remove arcane buffet", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kalecgos humanoid kalec tanks sathrovarr", {
        NextAction("kalecgos sathrovarr tank stand with kalec", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos bots don't observe gravity", {
        NextAction("kalecgos return to spectral realm ground", ACTION_EMERGENCY + 10) }));

    // Brutallus
    triggers.push_back(new TriggerNode("brutallus pulling boss", {
        NextAction("brutallus misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus requires two tanks", {
        NextAction("brutallus tanks position and swap", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus melee should stand in place", {
        NextAction("brutallus position melee at rear center", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus ranged should soak meteor slash", {
        NextAction("brutallus position ranged in two groups", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus bot is burning", {
        NextAction("brutallus isolate burn", ACTION_EMERGENCY + 1) }));

    // Felmyst
    triggers.push_back(new TriggerNode("felmyst pulling boss", {
        NextAction("felmyst misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst ground phase should be tanked", {
        NextAction("felmyst main tank position boss on ground", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst ranged should position to dispel and flee", {
        NextAction("felmyst ranged stack in three groups", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst melee should stay together", {
        NextAction("felmyst melee stack behind boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst bot is encapsulated", {
        NextAction("felmyst remove encapsulate", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst bot near encapsulated player", {
        NextAction("felmyst run away from encapsulated player", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst player has gas nova", {
        NextAction("felmyst mass dispel gas nova", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("felmyst should avoid demonic vapor trails", {
        NextAction("felmyst avoid demonic vapor", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst bot is demonic vapor target", {
        NextAction("felmyst kite demonic vapor", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst fog of corruption is active", {
        NextAction("felmyst move to safe fog lane", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst melee cannot reach flying boss", {
        NextAction("felmyst melee clear target", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst player is charmed by fog", {
        NextAction("felmyst kill charmed player", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst should hold dps while landing", {
        NextAction("felmyst manage landing dps timer", ACTION_EMERGENCY + 8) }));

    // Eredar Twins
    triggers.push_back(new TriggerNode("eredar twins melee is at balcony", {
        NextAction("eredar twins melee jump from balcony", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("eredar twins pulling bosses", {
        NextAction("eredar twins misdirect bosses to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash requires two tanks", {
        NextAction("eredar twins position sacrolash tanks", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins alythess casts blaze on tank", {
        NextAction("eredar twins alythess tank move out of blaze", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins ranged needs los", {
        NextAction("eredar twins ranged stack at balcony edge", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins should focus dps", {
        NextAction("eredar twins dps prioritize sacrolash", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins too many flame touched stacks", {
        NextAction("eredar twins remove flame sear", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("eredar twins only alythess remains", {
        NextAction("eredar twins stack in room center", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("eredar twins active conflagration target", {
        NextAction("eredar twins conflagration target move from group", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash victim has conflagration", {
        NextAction("eredar twins move away from sacrolash victim", ACTION_EMERGENCY + 10) }));

    // M'uru
    triggers.push_back(new TriggerNode("m'uru void sentinel or entropius has appeared", {
        NextAction("m'uru misdirect enemies to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru boss transformed into entropius", {
        NextAction("m'uru main tank pick up entropius", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru ranged should stack or spread", {
        NextAction("m'uru position ranged by phase", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru void sentinel pulses shadow", {
        NextAction("m'uru tanks move sentinel to safe position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru adds spawn at entrance", {
        NextAction("m'uru second assist tank guard ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru determining dps priority", {
        NextAction("m'uru assign dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru dark fiends spawned", {
        NextAction("m'uru kill dark fiends with dispel", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("m'uru darkness is coming", {
        NextAction("m'uru melee flee the darkness", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("m'uru berserker is buffed with flurry", {
        NextAction("m'uru cast stun on berserker", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("m'uru fury mage casting fel fireball", {
        NextAction("m'uru interrupt fel fireball", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("m'uru fury mage is buffed with spell fury", {
        NextAction("m'uru cast spellsteal on spell fury", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru void spawn available for enslave", {
        NextAction("m'uru warlock enslave void spawn", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("m'uru warlock has enslaved void spawn", {
        NextAction("m'uru void spawn cast shadow bolt volley", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("m'uru entropius darkness pools spawn dark fiends", {
        NextAction("m'uru keep distance from dark fiends", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("m'uru the singularity is near", {
        NextAction("m'uru escape the singularity", ACTION_EMERGENCY + 7) }));

    // Kil'jaeden <The Deceiver>
    triggers.push_back(new TriggerNode("kil'jaeden encounter has begun", {
        NextAction("kil'jaeden announce dragon orb user", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden hands of the deceiver are active", {
        NextAction("kil'jaeden stun hands of the deceiver", ACTION_EMERGENCY),
        NextAction("kil'jaeden assign hands of the deceiver", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden tanks should hold boss and reflections", {
        NextAction("kil'jaeden position and move tanks", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by melee", {
        NextAction("kil'jaeden position melee", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by ranged", {
        NextAction("kil'jaeden position ranged and avoid armageddons", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot has fire bloom", {
        NextAction("kil'jaeden remove fire bloom", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kil'jaeden says: Chaos! Destruction! Oblivion!", {
        NextAction("kil'jaeden stack for shield of the blue", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden dragon orb is active", {
        NextAction("kil'jaeden use dragon orb", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot has stale root after dragon", {
        NextAction("kil'jaeden release stale root", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot controls dragon", {
        NextAction("kil'jaeden dragon buff and protect raid", ACTION_RAID + 3) }));
}

void RaidSunwellStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Kalecgos
    multipliers.push_back(new KalecgosControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KalecgosWaitToDecurseMultiplier(botAI));
    multipliers.push_back(new KalecgosControlMovementMultiplier(botAI));
    multipliers.push_back(new KalecgosRestrictTauntMultiplier(botAI));
    multipliers.push_back(new KalecgosSuppressAssistTankPullThreatMultiplier(botAI));
    multipliers.push_back(new KalecgosEnterSpectralRiftMultiplier(botAI));
    multipliers.push_back(new KalecgosDelayCooldownsForSathrovarrMultiplier(botAI));

    // Brutallus
    multipliers.push_back(new BrutallusControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new BrutallusControlMovementMultiplier(botAI));
    multipliers.push_back(new BrutallusNoKillingSpreeWhenNearbyBurnMultiplier(botAI));
    multipliers.push_back(new BrutallusRestrictTauntMultiplier(botAI));
    multipliers.push_back(new BrutallusDelayCooldownsMultiplier(botAI));

    // Felmyst
    multipliers.push_back(new FelmystControlMovementMultiplier(botAI));
    multipliers.push_back(new FelmystWaitForLandingDpsMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeEncapsulateAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeDemonicVaporAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystPrioritizeFogAvoidanceMultiplier(botAI));
    multipliers.push_back(new FelmystFocusAttacksOnCharmedPlayerMultiplier(botAI));
    multipliers.push_back(new FelmystDontDotAddsMultiplier(botAI));
    multipliers.push_back(new FelmystDelayCooldownsMultiplier(botAI));

    // Eredar Twins
    multipliers.push_back(new EredarTwinsDisableAutomaticTargetingMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new EredarTwinsHoldDpsAtStartMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlThreatMultiplier(botAI));
    multipliers.push_back(new EredarTwinsControlMovementMultiplier(botAI));
    multipliers.push_back(new EredarTwinsIsolateConflagrationMultiplier(botAI));
    multipliers.push_back(new EredarTwinsDelayCooldownsMultiplier(botAI));

    // M'uru
    multipliers.push_back(new MuruDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new MuruControlMovementMultiplier(botAI));
    multipliers.push_back(new MuruControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new MuruDelayCooldownsMultiplier(botAI));

    // Kil'jaeden <The Deceiver>
    multipliers.push_back(new KiljaedenDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new KiljaedenTanksFocusAssignedHandOnlyMultiplier(botAI));
    multipliers.push_back(new KiljaedenDpsFocusAssignedHandOnlyMultiplier(botAI));
    multipliers.push_back(new KiljaedenControlMovementAndTargetingMultiplier(botAI));
    multipliers.push_back(new KiljaedenPrioritizeDarknessProtectionMultiplier(botAI));
    multipliers.push_back(new KiljaedenControlDragonMultiplier(botAI));
}

namespace
{

using namespace SwpHelpers;

void AppendFelmystVaporPhaseMeleeExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsMelee(botAI->GetBot()))
        return;

    Unit* felmyst = AI_VALUE2(Unit*, "find target", "felmyst");
    if (IsFelmystAirPhaseTargetSuppressed(felmyst))
        exclusions.insert(felmyst->GetGUID());
}

void AppendMuruDarkFiendExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (!AI_VALUE2(Unit*, "find target", "m'uru"))
        return;

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == Id(SwpNpcs::NPC_DARK_FIEND))
            exclusions.insert(guid);
    }
}

void AppendMuruTankExclusions(PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsTank(bot))
        return;

    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!IsMuruPhaseActive(muru))
        return;

    bool const darknessActive = TryGetMuruDarknessActiveState(bot, muru);
    // Even during Darkness, the Sentinel Tank has full freedom to pick up Sentinels
    bool const distanceUnrestricted = darknessActive &&
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);

    ObjectGuid const muruGuid = muru->GetGUID();

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
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

void AppendKiljaedenShieldOrbExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsMelee(botAI->GetBot()))
        return;

    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return;

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == Id(SwpNpcs::NPC_SHIELD_ORB))
            exclusions.insert(guid);
    }
}

// This activates only after the Reflections become aggressive (after 3s or when attacked,
// whichever is earlier); up until then, they are not on the attackers list anyway
/* void AppendKiljaedenSinisterReflectionExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (PlayerbotAI::IsTank(botAI->GetBot()))
        return;

    Unit* kiljaeden = AI_VALUE2(Unit*, "find target", "kil'jaeden");
    if (!kiljaeden || kiljaeden->GetHealthPct() > 85.0f)
        return;

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || attacker->GetEntry() != Id(SwpNpcs::NPC_SINISTER_REFLECTION))
            continue;

        Unit* victim = attacker->GetVictim();
        if (!victim || !victim->IsPlayer() || !PlayerbotAI::IsTank(victim->ToPlayer()))
            exclusions.insert(guid);
    }
} */

} // end anonymous namespace

void RaidSunwellStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    AppendFelmystVaporPhaseMeleeExclusions(botAI, context, exclusions);
    AppendMuruTankExclusions(botAI, context, exclusions);
    AppendMuruDarkFiendExclusions(botAI, context, exclusions);
    AppendKiljaedenShieldOrbExclusions(botAI, context, exclusions);
    // AppendKiljaedenSinisterReflectionExclusions(botAI, context, exclusions);
}
