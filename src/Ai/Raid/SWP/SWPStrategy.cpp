/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SWPStrategy.h"
#include "AiObjectContext.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPMultipliers.h"

void RaidSunwellStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("sunwell plateau bot is not in combat", {
        NextAction("sunwell plateau reset encounter states", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("sunwell plateau bot has protective aura", {
        NextAction("sunwell plateau remove protective aura", ACTION_EMERGENCY) }));

    // Trash
    triggers.push_back(new TriggerNode("volatile fiend self destructs when near", {
        NextAction("volatile fiend keep enemy away from group", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("apocalypse guard protected by infernal defense", {
        NextAction("apocalypse guard attack with holy magic", ACTION_RAID) }));

    // Kalecgos
    triggers.push_back(new TriggerNode("kalecgos boss engaged by tank", {
        NextAction("kalecgos tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos spectral rift is open", {
        NextAction("kalecgos enter spectral rift", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kalecgos bots take splash damage", {
        NextAction("kalecgos disperse ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos bot has too many arcane buffet stacks", {
        NextAction("kalecgos remove arcane buffet", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kalecgos humanoid kalec tanks sathrovarr", {
        NextAction("kalecgos sathrovarr tank stand with kalec", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kalecgos bots don't observe gravity", {
        NextAction("kalecgos return to spectral realm ground", ACTION_EMERGENCY + 10) }));

    // Brutallus
    triggers.push_back(new TriggerNode("brutallus pulling boss", {
        NextAction("brutallus misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by tanks", {
        NextAction("brutallus tanks handle boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by melee", {
        NextAction("brutallus position melee", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus boss engaged by ranged", {
        NextAction("brutallus position ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("brutallus bot is burning", {
        NextAction("brutallus handle burn", ACTION_EMERGENCY + 1) }));

    // Felmyst
    triggers.push_back(new TriggerNode("felmyst pulling boss", {
        NextAction("felmyst misdirect boss to main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by main tank on ground", {
        NextAction("felmyst main tank position boss on ground", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by ranged on ground", {
        NextAction("felmyst position ranged on ground", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst boss engaged by melee on ground", {
        NextAction("felmyst position melee on ground", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("felmyst bot is encapsulated", {
        NextAction("felmyst remove encapsulate", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst bot near encapsulated player", {
        NextAction("felmyst run away from encapsulated player", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("felmyst player has gas nova", {
        NextAction("felmyst mass dispel gas nova", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("felmyst demonic vapor trails are active", {
        NextAction("felmyst avoid demonic vapor", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("felmyst bot is demonic vapor target", {
        NextAction("felmyst kite demonic vapor", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst fog of corruption is active", {
        NextAction("felmyst move to safe fog lane", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("felmyst melee cannot reach boss", {
        NextAction("felmyst melee clear target", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("felmyst player is charmed by fog", {
        NextAction("felmyst kill charmed player", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("felmyst manage landing dps timer", {
        NextAction("felmyst should hold dps while landing", ACTION_EMERGENCY + 8) }));

    // Eredar Twins
    triggers.push_back(new TriggerNode("eredar twins melee is at balcony", {
        NextAction("eredar twins melee jump down from balcony", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("eredar twins pulling bosses", {
        NextAction("eredar twins misdirect bosses to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash engaged by two tanks", {
        NextAction("eredar twins main and second assist tanks position sacrolash", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins alythess engaged by first assist tank", {
        NextAction("eredar twins first assist tank move out of blaze", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins bosses engaged by ranged", {
        NextAction("eredar twins position ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("eredar twins determining dps priority", {
        NextAction("eredar twins dps prioritize lady sacrolash", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("eredar twins bot has too many flame touched stacks", {
        NextAction("eredar twins remove flame sear", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("eredar twins only one boss remains", {
        NextAction("eredar twins stack in room center", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("eredar twins bot has conflagration", {
        NextAction("eredar twins conflagrated bot move from group", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("eredar twins sacrolash victim has conflagration", {
        NextAction("eredar twins move from conflag sacrolash victim", ACTION_EMERGENCY + 10) }));

    // M'uru
    triggers.push_back(new TriggerNode("m'uru void sentinel or entropius has appeared", {
        NextAction("m'uru misdirect enemies to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru boss transformed into entropius", {
        NextAction("m'uru main tank pick up entropius", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("m'uru bosses engaged by ranged", {
        NextAction("m'uru position ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru void sentinel pulses shadow", {
        NextAction("m'uru tanks move sentinel to safe position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("m'uru adds spawn at entrance", {
        NextAction("m'uru second assist tank guard ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru determining dps priority", {
        NextAction("m'uru set dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("m'uru dark fiends spawned", {
        NextAction("m'uru kill dark fiends with dispel", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("m'uru entropius spawns darkness pools", {
        NextAction("m'uru don't touch the dark fiend", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("m'uru darkness is coming", {
        NextAction("m'uru flee the darkness", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("m'uru the singularity is near", {
        NextAction("m'uru flee from singularity", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru berserker is buffed with flurry", {
        NextAction("m'uru cast stun on shadowsword berserker", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("m'uru fury mage casting fel fireball", {
        NextAction("m'uru interrupt fel fireball", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("m'uru fury mage is buffed with spell fury", {
        NextAction("m'uru cast spellsteal on spell fury", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("m'uru void spawn available for enslave", {
        NextAction("m'uru warlock enslave void spawn", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("m'uru warlock has enslaved void spawn", {
        NextAction("m'uru enslaved void spawn cast shadow bolt volley", ACTION_RAID + 4) }));

    // Kil'jaeden <The Deceiver>
    triggers.push_back(new TriggerNode("kil'jaeden encounter has begun", {
        NextAction("kil'jaeden announce dragon orb user", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden hands of the deceiver are active", {
        NextAction("kil'jaeden stun hands of the deceiver", ACTION_EMERGENCY),
        NextAction("kil'jaeden mark and prioritize hands of the deceiver", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by tanks", {
        NextAction("kil'jaeden position tanks", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by melee", {
        NextAction("kil'jaeden position melee", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden boss engaged by ranged", {
        NextAction("kil'jaeden position ranged", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot has fire bloom", {
        NextAction("kil'jaeden remove fire bloom", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kil'jaeden says: Chaos! Destruction! Oblivion!", {
        NextAction("kil'jaeden stack for shield of the blue", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden dragon orb is active", {
        NextAction("kil'jaeden use dragon orb", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot has stale root after dragon", {
        NextAction("kil'jaeden release stale root", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kil'jaeden bot controls dragon", {
        NextAction("kil'jaeden control dragon", ACTION_RAID + 3) }));
}

void RaidSunwellStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Kalecgos
    multipliers.push_back(new KalecgosControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KalecgosWaitToDecurseMultiplier(botAI));
    multipliers.push_back(new KalecgosControlMovementMultiplier(botAI));
    multipliers.push_back(new KalecgosRestrictTauntMultiplier(botAI));
    multipliers.push_back(new KalecgosSuppressAssistTankPullThreatMultiplier(botAI));
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
    multipliers.push_back(new FelmystPrioritizeDemonicVaporKiteMultiplier(botAI));
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
    multipliers.push_back(new EredarTwinsNoMovingIntoConflagrationMultiplier(botAI));
    multipliers.push_back(new EredarTwinsDelayCooldownsMultiplier(botAI));

    // M'uru
    multipliers.push_back(new MuruDisableDefaultTargetingMultiplier(botAI));
    multipliers.push_back(new MuruControlMovementMultiplier(botAI));
    multipliers.push_back(new MuruControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new MuruDelayCooldownsMultiplier(botAI));

    // Kil'jaeden <The Deceiver>
    multipliers.push_back(new KiljaedenDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new KiljaedenTanksFocusAssignedHandOnlyMultiplier(botAI));
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
        if (attacker && attacker->GetEntry() == static_cast<uint32>(SwpNpcs::NPC_DARK_FIEND))
            exclusions.insert(guid);
    }
}

void AppendMuruTankExclusions(PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    Unit* muru = AI_VALUE2(Unit*, "find target", "m'uru");
    if (!muru || muru->GetHealth() <= 1)
        return;

    constexpr float maxTankTargetDistanceFromStack = 25.0f;

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker || attacker->GetEntry() == static_cast<uint32>(SwpNpcs::NPC_VOID_SENTINEL))
            continue;

        if (guid == muru->GetGUID())
        {
            exclusions.insert(guid);
            continue;
        }

        Player* bot = botAI->GetBot();
        if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) && TryGetMuruDarknessActiveState(bot, muru))
            continue;

        if (attacker->GetExactDist2d(
                MURU_STACK_POSITION.GetPositionX(), MURU_STACK_POSITION.GetPositionY()) >
            maxTankTargetDistanceFromStack)
        {
            exclusions.insert(guid);
        }
    }
}

void AppendKiljaedenShieldOrbExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsMelee(botAI->GetBot()))
        return;

    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return;

    for (auto const& guid :
         AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (attacker && attacker->GetEntry() == static_cast<uint32>(SwpNpcs::NPC_SHIELD_ORB))
            exclusions.insert(guid);
    }
}

void AppendKiljaedenSinisterReflectionExclusions(
    PlayerbotAI* botAI, AiObjectContext* context, GuidSet& exclusions)
{
    if (!AI_VALUE2(Unit*, "find target", "kil'jaeden"))
        return;

    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* attacker = botAI->GetUnit(guid);
        if (!attacker ||
            attacker->GetEntry() != static_cast<uint32>(SwpNpcs::NPC_SINISTER_REFLECTION))
        {
            continue;
        }

        Unit* victim = attacker->GetVictim();
        if (!victim || !victim->IsPlayer() || !PlayerbotAI::IsTank(victim->ToPlayer()))
            exclusions.insert(guid);
    }
}

} // end anonymous namespace

void RaidSunwellStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    AppendFelmystVaporPhaseMeleeExclusions(botAI, context, exclusions);
    AppendMuruDarkFiendExclusions(botAI, context, exclusions);
    AppendKiljaedenShieldOrbExclusions(botAI, context, exclusions);

    switch (type)
    {
        case TargetValueExclusionType::Tank:
            AppendMuruTankExclusions(botAI, context, exclusions);
            break;
        case TargetValueExclusionType::Dps:
        case TargetValueExclusionType::Attacker:
            AppendKiljaedenSinisterReflectionExclusions(botAI, context, exclusions);
            break;
        case TargetValueExclusionType::None:
            break;
    }
}
