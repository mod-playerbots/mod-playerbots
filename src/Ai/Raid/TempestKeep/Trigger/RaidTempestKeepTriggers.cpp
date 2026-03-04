#include "RaidTempestKeepTriggers.h"
#include "RaidTempestKeepHelpers.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepKaelthasBossAI.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace TempestKeepHelpers;

// Trash

bool CrimsonHandCenturionCastsArcaneVolleyTrigger::IsActive()
{
    return bot->getClass() == CLASS_MAGE &&
           AI_VALUE2(Unit*, "find target", "crimson hand centurion");
}

// Al'ar <Phoenix God>

bool AlarPullingBossTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && alar->GetHealthPct() > 98.0f;
}

bool AlarBossIsFlyingBetweenPlatformsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || !alar->IsVisible())
        return false;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
    {
        Position dest;
        locationIndex = GetAlarDestinationLocationIndex(alar, dest);
    }

    return locationIndex != POINT_QUILL_OR_DIVE_IDX &&
           locationIndex != POINT_MIDDLE_IDX;
}

bool AlarEmbersOfAlarExplodeUponDeathTrigger::IsActive()
{
    return botAI->IsTank(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarKillingEmbersOfAlarDamagesBossTrigger::IsActive()
{
    return botAI->IsRangedDps(bot) &&
           AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    Position dest;
    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
           GetAlarDestinationLocationIndex(alar, dest) == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar)
        return false;

    if (isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    Creature* alarCreature = alar->ToCreature();
    return alarCreature && alarCreature->GetReactState() == REACT_PASSIVE;
}

bool AlarEverythingIsOnFireInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && isAlarInPhase2[alar->GetMap()->GetInstanceId()];
}

bool AlarPhase2EncounterIsAtRoomCenterTrigger::IsActive()
{
    if (bot->GetVictim())
        return false;

    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || !isAlarInPhase2[alar->GetMap()->GetInstanceId()])
        return false;

    Creature* alarCreature = alar->ToCreature();
    if (alarCreature && alarCreature->GetReactState() == REACT_PASSIVE)
        return false;

    Position dest;
    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
           GetAlarDestinationLocationIndex(alar, dest) != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarStrategyChangesBetweenPhasesTrigger::IsActive()
{
    return botAI->IsDps(bot) && IsMechanicTrackerBot(botAI, bot, TEMPEST_KEEP_MAP_ID) &&
           AI_VALUE2(Unit*, "find target", "al'ar");
}

// Void Reaver

bool VoidReaverBossCastsPoundingTrigger::IsActive()
{
    if (!botAI->IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverKnockAwayReducesTankAggroTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    if (bot->getClass() == CLASS_DEATH_KNIGHT ||
        bot->getClass() == CLASS_DRUID ||
        bot->getClass() == CLASS_SHAMAN ||
        bot->getClass() == CLASS_WARRIOR)
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverBossLaunchesArcaneOrbsTrigger::IsActive()
{
    if (!botAI->IsRanged(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() != bot;
}

bool VoidReaverArcaneOrbIsIncomingTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    auto it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 currentTime = getMSTime();
    constexpr uint32 orbDuration = 7000;
    constexpr float safeDistance = 22.0f;

    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, currentTime) <= orbDuration &&
            bot->GetExactDist2d(orb.destination.GetPositionX(),
                                orb.destination.GetPositionY()) < safeDistance)
            return true;
    }

    return false;
}

bool VoidReaverBotIsNotInCombatTrigger::IsActive()
{
    return !bot->IsInCombat();
}

// High Astromancer Solarian

bool HighAstromancerSolarianBossCastsWrathOfTheAstromancerTrigger::IsActive()
{
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
        return false;

    if (!botAI->IsRanged(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    return astromancer && !astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    return bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER);
}

bool HighAstromancerSolarianBossHasVanishedTrigger::IsActive()
{
    if (bot->HasAura(SPELL_WRATH_OF_THE_ASTROMANCER))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Creature* astromancerCreature = astromancer->ToCreature();
    return astromancerCreature &&
           astromancerCreature->GetReactState() == REACT_PASSIVE;
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    return botAI->IsMelee(bot) && AI_VALUE2(Unit*, "find target", "solarium priest");
}

bool HighAstromancerSolarianBossCastsPsychicScreamTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    return astromancer && astromancer->HasAura(SPELL_SOLARIAN_TRANSFORM);
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    return !(botAI->IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS);
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
           kaelAI->GetPhase() == PHASE_ALL_ADVISORS);
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    if (!botAI->IsMainTank(bot))
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    return sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !sanguinar->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR &&
        kaelAI->GetPhase() != PHASE_TRANSITION &&
        kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank || mainTank->HasAura(SPELL_FEAR_WARD))
        return false;

    return botAI->CanCastSpell("fear ward", mainTank);
}

bool KaelthasSunstriderCapernianShouldBeTankedByAWarlockTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return false;

    return GetCapernianTank(bot) == bot;
}

bool KaelthasSunstriderCapernianCastsArcaneBurstAndConflagrationTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        capernian->HasAura(SPELL_PERMANENT_FEIGN_DEATH))
        return false;

    return GetCapernianTank(bot) != bot;
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!botAI->IsAssistTankOfIndex(bot, 0, false))
        return false;

    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    return telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
           !telonicus->HasAura(SPELL_PERMANENT_FEIGN_DEATH);
}

bool KaelthasSunstriderBotsHaveSpecificRolesInPhase3Trigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (!AI_VALUE2(Unit*, "find target", "master engineer telonicus") &&
        !AI_VALUE2(Unit*, "find target", "lord sanguinar"))
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return false;

    return botAI->IsAssistHealOfIndex(bot, 0, true) ||
           botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0, true) ||
           (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (botAI->IsHeal(bot) ||
        botAI->IsMainTank(bot) ||
        botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && (kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
           kaelAI->GetPhase() == PHASE_ALL_ADVISORS);
}

bool KaelthasSunstriderWaitingForTanksToGetAggroOnAdvisorsTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_SINGLE_ADVISOR)
        return false;

    return IsMechanicTrackerBot(botAI, bot, TEMPEST_KEEP_MAP_ID, GetCapernianTank(bot));
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    if (botAI->IsMainTank(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_WEAPONS;
}

bool KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger::IsActive()
{
    return botAI->IsMainTank(bot) &&
           AI_VALUE2(Unit*, "find target", "devastation");
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI ||
        (kaelAI->GetPhase() != PHASE_WEAPONS && kaelAI->GetPhase() != PHASE_ALL_ADVISORS))
        return false;

    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (axe && axe->GetVictim() == bot)
        return false;

    return IsAnyLegendaryWeaponDead(bot);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return false;

    return bot->HasItemCount(ITEM_STAFF_OF_DISINTEGRATION, 1, false) ||
           bot->HasItemCount(ITEM_NETHERSTRAND_LONGBOW, 1, false) ||
           bot->HasItemCount(ITEM_PHASESHIFT_BULWARK, 1, false);
}

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TEMPEST_KEEP_MAP_ID)
        return false;

    Map* map = bot->GetMap();
    if (!map)
        return false;

    constexpr uint32 KAELTHAS_DB_GUID = 158218;
    auto const& creatureStore = map->GetCreatureBySpawnIdStore();
    auto it = creatureStore.find(KAELTHAS_DB_GUID);
    if (it == creatureStore.end())
        return false;

    Creature* kaelthas = it->second;
    if (!kaelthas || bot->GetExactDist2d(kaelthas) > 150.0f)
        return false;

    const std::array<uint8, 3> weaponSlots =
    {
        EQUIPMENT_SLOT_MAINHAND,
        EQUIPMENT_SLOT_OFFHAND,
        EQUIPMENT_SLOT_RANGED
    };

    for (uint8 slot : weaponSlots)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot) &&
            HasEquippableItemForSlot(bot, slot))
            return true;
    }

    return false;
}

bool KaelthasSunstriderBossHasEnteredTheFightTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_FINAL;
}

bool KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (botAI->IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    return AI_VALUE2(Unit*, "find target", "phoenix") ||
           AI_VALUE2(Unit*, "find target", "phoenix egg");
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (botAI->IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    if (!bot->HasItemCount(ITEM_INFINITY_BLADE, 1, true))
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (member->HasAura(SPELL_KAELTHAS_MIND_CONTROL))
                return true;
        }
    }

    return false;
}

bool KaelthasSunstriderBossIsCastingPyroblastTrigger::IsActive()
{
    if (!botAI->IsDps(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && kaelthas->HasAura(SPELL_SHOCK_BARRIER);
}

bool KaelthasSunstriderBossIsManipulatingGravityTrigger::IsActive()
{
    return bot->HasAura(SPELL_GRAVITY_LAPSE);
}
