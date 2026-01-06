#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "Playerbots.h"

using namespace KarazhanHelpers;

bool ManaWarpIsAboutToExplodeTrigger::IsActive()
{
    Unit* manaWarp = AI_VALUE2(Unit*, "find target", "mana warp");
    return manaWarp && manaWarp->GetHealthPct() < 15;
}

bool AttumenTheHuntsmanNeedTargetPriorityTrigger::IsActive()
{
    if (botAI->IsHeal(bot))
        return false;

    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    return midnight != nullptr;
}

bool AttumenTheHuntsmanAttumenSpawnedTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    return attumen != nullptr;
}

bool AttumenTheHuntsmanAttumenIsMountedTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    return attumenMounted && attumenMounted->GetVictim() != bot;
}

bool AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    return midnight != nullptr;
}

bool MoroesBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    return moroes != nullptr;
}

bool MoroesNeedTargetPriorityTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");

    Unit* target = GetFirstAliveUnit({ dorothea, catriona, keira, rafe, robin, crispin });
    return target != nullptr;
}

bool MaidenOfVirtueHealersAreStunnedByRepentanceTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    return maiden && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueHolyWrathDealsChainDamageTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    return maiden != nullptr;
}

bool BigBadWolfBossEngagedByTankTrigger::IsActive()
{
    if (!botAI->IsTank(bot) || bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD))
        return false;

    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    return wolf != nullptr;
}

bool BigBadWolfBossIsChasingLittleRedRidingHoodTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD))
        return false;

    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    return wolf != nullptr;
}

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!romulo)
        return false;

    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!julianne)
        return false;

    return true;
}

bool WizardOfOzNeedTargetPriorityTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");

    Unit* target = GetFirstAliveUnit({ dorothee, tito, roar, strawman, tinhead, crone });
    return target != nullptr;
}

bool WizardOfOzStrawmanIsVulnerableToFireTrigger::IsActive()
{
    if (bot->getClass() != CLASS_MAGE)
        return false;

    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    return strawman && strawman->IsAlive();
}

bool TheCuratorAstralFlareSpawnedTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* flare = AI_VALUE2(Unit*, "find target", "astral flare");
    return flare != nullptr;
}

bool TheCuratorBossEngagedByTanksTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0))
        return false;

    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    return curator != nullptr;
}

bool TheCuratorBossAstralFlaresCastArcingSearTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    return curator != nullptr;
}

bool TerestianIllhoofNeedTargetPriorityTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    return illhoof != nullptr;
}

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && aran->HasUnitState(UNIT_STATE_CASTING) &&
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION) &&
           !IsFlameWreathActive(botAI, bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && IsFlameWreathActive(botAI, bot);
}

// Exclusion of Banish is so the player may Banish elementals if they wish
bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    if (!IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* elemental = AI_VALUE2(Unit*, "find target", "conjured elemental");
    return elemental && elemental->IsAlive() &&
           !elemental->HasAura(SPELL_WARLOCK_BANISH);
}

bool ShadeOfAranBossUsesCounterspellAndBlizzardTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    return aran && !(aran->HasUnitState(UNIT_STATE_CASTING) &&
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION)) &&
           !IsFlameWreathActive(botAI, bot);
}

bool NetherspiteRedBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    return redPortal != nullptr;
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    return bluePortal != nullptr;
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
    return greenPortal != nullptr;
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    return bot != redBlocker && bot != blueBlocker && bot != greenBlocker;
}

bool NetherspiteBossIsBanishedTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite || !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);
    for (Unit* vz : voidZones)
    {
        if (bot->GetExactDist2d(vz) < 4.0f)
            return true;
    }

    return false;
}

bool NetherspiteNeedToManageTimersAndTrackersTrigger::IsActive()
{
    if (!botAI->IsTank(bot) && !IsInstanceTimerManager(botAI, bot))
        return false;

    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    return netherspite != nullptr;
}

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    return bot->HasAura(SPELL_ENFEEBLE);
}

bool PrinceMalchezaarInfernalsAreSpawnedTrigger::IsActive()
{
    if (botAI->IsMainTank(bot) || bot->HasAura(SPELL_ENFEEBLE))
        return false;

    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    return malchezaar != nullptr;
}

bool PrinceMalchezaarBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    return malchezaar != nullptr;
}

bool NightbaneBossEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneRangedBotsAreInCharredEarthTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    return nightbane && nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
}

bool NightbaneMainTankIsSusceptibleToFearTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    return mainTank && !mainTank->HasAura(SPELL_FEAR_WARD) &&
           botAI->CanCastSpell("fear ward", mainTank);
}

bool NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool NightbaneBossIsFlyingTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const time_t now = std::time(nullptr);
    const uint8 flightPhaseDurationSeconds = 35;

    return nightbaneFlightPhaseStartTimer.find(instanceId) != nightbaneFlightPhaseStartTimer.end() &&
           (now - nightbaneFlightPhaseStartTimer[instanceId] < flightPhaseDurationSeconds);
}

bool NightbaneNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    return nightbane != nullptr;
}
