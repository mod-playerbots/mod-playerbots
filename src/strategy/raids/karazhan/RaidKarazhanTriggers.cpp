#include "RaidKarazhanTriggers.h"
#include "RaidKarazhanHelpers.h"
#include "RaidKarazhanActions.h"
#include "Playerbots.h"

using namespace KarazhanHelpers;

bool ManaWarpIsAboutToExplodeTrigger::IsActive()
{
    Unit* manaWarp = AI_VALUE2(Unit*, "find target", "mana warp");
    if (!manaWarp)
        return false;

    return manaWarp->GetHealthPct() < 15 && (bot->getClass() == CLASS_ROGUE ||
           bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_PALADIN);
}

bool AttumenTheHuntsmanNeedTargetPriorityTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);

    return midnight || attumen || attumenMounted;
}

bool AttumenTheHuntsmanAttumenSpawnedTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    if (!midnight || !attumen)
        return false;

    return botAI->IsAssistTankOfIndex(bot, 0);
}

bool AttumenTheHuntsmanAttumenIsMountedTrigger::IsActive()
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    return !botAI->IsMainTank(bot) &&
           attumenMounted->GetVictim() != bot;
}

bool AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger::IsActive()
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!midnight && !attumenMounted)
        return false;

    return IsMapIDTimerManager(botAI, bot);
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
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");

    Unit* target = GetFirstAliveUnit({ dorothea, catriona, keira, rafe, robin, crispin });
    if (!target)
        return false;

    return botAI->IsDps(bot);
}

bool MaidenOfVirtueHealersAreStunnedByRepentanceTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    return botAI->IsTank(bot) && maiden->GetVictim() == bot;
}

bool MaidenOfVirtueHolyWrathDealsChainDamageTrigger::IsActive()
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    return botAI->IsRanged(bot);
}

bool BigBadWolfBossEngagedByTankTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    return botAI->IsTank(bot) && wolf->GetVictim() == bot &&
           !bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool BigBadWolfBossIsChasingLittleRedRidingHoodTrigger::IsActive()
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    return bot->HasAura(SPELL_LITTLE_RED_RIDING_HOOD);
}

bool RomuloAndJulianneBothBossesRevivedTrigger::IsActive()
{
    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!julianne && !romulo)
        return false;

    return (julianne->IsAlive() || romulo->IsAlive()) && IsMapIDTimerManager(botAI, bot);
}

bool WizardOfOzNeedTargetPriorityTrigger::IsActive()
{
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
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    if (!strawman)
        return false;

    return strawman->IsAlive() && bot->getClass() == CLASS_MAGE;
}

bool TheCuratorAstralFlareSpawnedTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    Unit* target = AI_VALUE2(Unit*, "find target", "astral flare");
    if (!curator || !target)
        return false;

    return botAI->IsDps(bot);
}

bool TheCuratorBossEngagedByTanksTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    return botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0);
}

bool TheCuratorBossAstralFlaresCastArcingSearTrigger::IsActive()
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    return botAI->IsRanged(bot);
}

bool TerestianIllhoofNeedTargetPriorityTrigger::IsActive()
{
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");
    if (!illhoof)
        return false;

    Unit* target = GetFirstAliveUnit(
    {
        AI_VALUE2(Unit*, "find target", "demon chains"),
        AI_VALUE2(Unit*, "find target", "kil'rek"),
        illhoof
    });

    return target != nullptr && IsMapIDTimerManager(botAI, bot);
}

bool ShadeOfAranArcaneExplosionIsCastingTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    return aran->HasUnitState(UNIT_STATE_CASTING) &&
           aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION) &&
           !IsFlameWreathActive(botAI, bot);
}

bool ShadeOfAranFlameWreathIsActiveTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    return IsFlameWreathActive(botAI, bot);
}

// Exclusion of Banish is so the player may Banish elementals if they wish
bool ShadeOfAranConjuredElementalsSummonedTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    Unit* elemental = AI_VALUE2(Unit*, "find target", "conjured elemental");

    return elemental && elemental->IsAlive() &&
           !elemental->HasAura(SPELL_WARLOCK_BANISH) && IsMapIDTimerManager(botAI, bot);
}

bool ShadeOfAranBossUsesCounterspellAndBlizzardTrigger::IsActive()
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    return !IsFlameWreathActive(botAI, bot) &&
           !(aran->HasUnitState(UNIT_STATE_CASTING) &&
             aran->FindCurrentSpellBySpellId(SPELL_ARCANE_EXPLOSION)) &&
             botAI->IsRanged(bot);
}

bool NetherspiteRedBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);

    return redPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBlueBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);

    return bluePortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteGreenBeamIsActiveTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    return greenPortal && !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED);
}

bool NetherspiteBotIsNotBeamBlockerTrigger::IsActive()
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);

    return !netherspite->HasAura(SPELL_NETHERSPITE_BANISHED) && bot != redBlocker &&
           bot != blueBlocker && bot != greenBlocker;
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
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");

    return netherspite && (botAI->IsTank(bot) || IsMapIDTimerManager(botAI, bot));
}

bool PrinceMalchezaarBotIsEnfeebledTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    return bot->HasAura(SPELL_ENFEEBLE);
}

bool PrinceMalchezaarInfernalsAreSpawnedTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    return !botAI->IsMainTank(bot);
}

bool PrinceMalchezaarBossEngagedByMainTankTrigger::IsActive()
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    return botAI->IsMainTank(bot) && malchezaar->GetVictim() == bot;
}

// Z-axis of 95 yards is used to determine if Nightbane is flying
bool NightbaneBossEngagedByMainTankTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    return nightbane->GetPositionZ() <= 95.0f && botAI->IsMainTank(bot);
}

bool NightbaneRangedBotsAreInCharredEarthTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    return nightbane->GetPositionZ() <= 95.0f && botAI->IsRanged(bot);
}

bool NightbaneMainTankIsSusceptibleToFearTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    Group* group = bot->GetGroup();
    if (!nightbane || !group)
        return false;

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    return bot->getClass() == CLASS_PRIEST && mainTank &&
           !mainTank->HasAura(SPELL_FEAR_WARD) && botAI->CanCastSpell("fear ward", mainTank);
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
    if (!nightbane || nightbane->GetPositionZ() <= 95.0f)
        return false;

    const uint32 mapId = nightbane->GetMapId();
    const time_t now = std::time(nullptr);

    return nightbaneFlightPhaseStartTimer.find(mapId) != nightbaneFlightPhaseStartTimer.end() &&
           (now - nightbaneFlightPhaseStartTimer[mapId] < 35);
}

bool NightbaneNeedToManageTimersAndTrackersTrigger::IsActive()
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");

    return nightbane;
}
