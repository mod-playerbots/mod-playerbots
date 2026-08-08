/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKTriggers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"
#include "TKActions.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"

using namespace TkHelpers;

// General
bool TempestKeepBotIsNotInCombatTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, TK_MAP_ID) && !AI_VALUE2(bool, "combat", "self target");
}

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
    return alar && alar->GetHealthPct() > 95.0f;
}

bool AlarBossIsFlyingBetweenPlatformsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    return locationIndex != POINT_QUILL_OR_DIVE_IDX && locationIndex != POINT_MIDDLE_IDX;
}

bool AlarEmbersOfAlarExplodeUponDeathTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarKillingEmbersOfAlarDamagesBossTrigger::IsActive()
{
    return PlayerbotAI::IsRangedDps(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
        GetAlarDestinationLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetHealthPct() > 5.0f)
        return false;

    if (IsAlarInPhase2(alar->GetMap()->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarEverythingIsOnFireInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && IsAlarInPhase2(alar->GetMap()->GetInstanceId());
}

bool AlarShouldManagePhaseTrackerTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, TK_MAP_ID) && AI_VALUE2(Unit*, "find target", "al'ar");
}

// Void Reaver

bool VoidReaverBossCastsPoundingTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "void reaver");
}

bool VoidReaverKnockAwayReducesTankAggroTrigger::IsActive()
{
    if (bot->getClass() == CLASS_DEATH_KNIGHT || bot->getClass() == CLASS_DRUID ||
        bot->getClass() == CLASS_SHAMAN || bot->getClass() == CLASS_WARRIOR)
    {
        return false;
    }

    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    return voidReaver && voidReaver->GetVictim() == bot;
}

bool VoidReaverRangedShouldStandBackTrigger::IsActive()
{
    if (!PlayerbotAI::IsRanged(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    auto const it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return true;

    uint32 const now = getMSTime();

    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, now) <= ARCANE_ORB_DURATION_MS &&
            bot->GetExactDist2d(
                orb.destination.GetPositionX(),
                orb.destination.GetPositionY()) < ARCANE_ORB_BUFFER_DISTANCE)
        {
            return false;
        }
    }

    return true;
}

bool VoidReaverArcaneOrbIsIncomingTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    auto const it = voidReaverArcaneOrbs.find(bot->GetMap()->GetInstanceId());
    if (it == voidReaverArcaneOrbs.end() || it->second.empty())
        return false;

    uint32 const now = getMSTime();

    for (auto const& orb : it->second)
    {
        if (getMSTimeDiff(orb.castTime, now) <= ARCANE_ORB_DURATION_MS &&
            bot->GetExactDist2d(
                orb.destination.GetPositionX(),
                orb.destination.GetPositionY()) < ARCANE_ORB_SAFE_DISTANCE)
        {
            return true;
        }
    }

    return false;
}

// High Astromancer Solarian

bool HighAstromancerSolarianEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    if (!astromancer)
        return false;

    Creature* astromancerCreature = astromancer->ToCreature();
    return astromancerCreature && astromancerCreature->GetReactState() != REACT_PASSIVE;
}

bool HighAstromancerSolarianBotHasWrathOfTheAstromancerTrigger::IsActive()
{
    return HasWrathOfTheAstromancer(bot);
}

bool HighAstromancerSolarianSolariumPriestsSpawnedTrigger::IsActive()
{
    if (!PlayerbotAI::IsMelee(bot) || PlayerbotAI::IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "solarium priest");
}

bool HighAstromancerSolarianBossCastsPsychicScreamTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* astromancer = AI_VALUE2(Unit*, "find target", "high astromancer solarian");
    return astromancer && astromancer->HasAura(Id(TkSpells::SPELL_SOLARIAN_TRANSFORM));
}

// Kael'thas Sunstrider <Lord of the Blood Elves>

bool KaelthasSunstriderThaladredIsFixatedOnBotTrigger::IsActive()
{
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || thaladred->GetVictim() != bot)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (PlayerbotAI::IsTank(bot) && kaelAI->GetPhase() == PHASE_ALL_ADVISORS)
        return false;

    return true;
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    return kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR || kaelAI->GetPhase() == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderSanguinarEngagedByMainTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsMainTank(bot))
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    return sanguinar && !sanguinar->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(sanguinar);
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

    return kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR ||
        kaelAI->GetPhase() == PHASE_TRANSITION ||
        kaelAI->GetPhase() == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderCapernianShouldBeTankedByAWarlockTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK || GetCapernianTank(bot) != bot)
        return false;

    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    return capernian && !capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(capernian);
}

bool KaelthasSunstriderCapernianCastsArcaneBurstAndConflagrationTrigger::IsActive()
{
    Unit* capernian = AI_VALUE2(Unit*, "find target", "grand astromancer capernian");
    if (!capernian || capernian->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) ||
        IsFeigningDeath(capernian))
    {
        return false;
    }

    if (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot)
        return false;

    return true;
}

bool KaelthasSunstriderTelonicusEngagedByFirstAssistTankTrigger::IsActive()
{
    if (!PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
        return false;

    Unit* telonicus = AI_VALUE2(Unit*, "find target", "master engineer telonicus");
    return telonicus && !telonicus->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
        !IsFeigningDeath(telonicus);
}

bool KaelthasSunstriderBotsHaveSpecificRolesInPhase3Trigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_ALL_ADVISORS)
        return false;

    // Proxy for revival/Kael talk phase (can pick any advisor here)
    Unit* thaladred = AI_VALUE2(Unit*, "find target", "thaladred the darkener");
    if (!thaladred || !thaladred->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    return PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false) ||
        PlayerbotAI::IsAssistHealOfIndex(bot, 0, false) ||
        (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot) || PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, false))
    {
        return false;
    }

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    return kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR || kaelAI->GetPhase() == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    return kaelAI && kaelAI->GetPhase() == PHASE_SINGLE_ADVISOR;
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI || kaelAI->GetPhase() != PHASE_WEAPONS)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return true;
}

bool KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "devastation");
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadAndLootableTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (!kaelAI)
        return false;

    if (kaelAI->GetPhase() < PHASE_WEAPONS || kaelAI->GetPhase() > PHASE_ALL_ADVISORS)
        return false;

    Unit* axe = AI_VALUE2(Unit*, "find target", "devastation");
    if (axe && axe->GetVictim() == bot)
        return false;

    return IsAnyLegendaryWeaponDead(bot);
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return false;

    bool HasUsableLegendaryWeapon =
        bot->HasItemCount(Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION), 1, false) ||
        bot->HasItemCount(Id(TkItems::ITEM_NETHERSTRAND_LONGBOW), 1, false) ||
        bot->HasItemCount(Id(TkItems::ITEM_PHASESHIFT_BULWARK), 1, false);

    return HasUsableLegendaryWeapon;
}

bool KaelthasSunstriderLegendaryWeaponsWereLostTrigger::IsActive()
{
    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    if (AI_VALUE2(bool, "combat", "self target"))
        return false;

    constexpr uint32 kaelthasDbGuid = 158218;
    auto const& creatureStore = bot->GetMap()->GetCreatureBySpawnIdStore();
    auto it = creatureStore.find(kaelthasDbGuid);
    if (it == creatureStore.end())
        return false;

    Creature* kaelthas = it->second;
    if (!kaelthas || bot->GetExactDist2d(kaelthas) > 125.0f)
        return false;

    static constexpr std::array weaponSlots = {
        EQUIPMENT_SLOT_MAINHAND, EQUIPMENT_SLOT_OFFHAND, EQUIPMENT_SLOT_RANGED, };

    for (uint8 slot : weaponSlots)
    {
        if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot) && HasEquippableItemForSlot(bot, slot))
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
    if (!kaelthas || kaelthas->GetVictim() == bot)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    return AI_VALUE2(Unit*, "find target", "phoenix") ||
        AI_VALUE2(Unit*, "find target", "phoenix egg");
}

bool KaelthasSunstriderRaidMemberIsMindControlledTrigger::IsActive()
{
    if (PlayerbotAI::IsCaster(bot))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (PlayerbotAI::IsTank(bot) && kaelthas->GetVictim() == bot)
        return false;

    if (!bot->HasItemCount(Id(TkItems::ITEM_INFINITY_BLADE), 1, true))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->HasAura(Id(TkSpells::SPELL_KAELTHAS_MIND_CONTROL)))
            return true;
    }

    return false;
}

bool KaelthasSunstriderBossIsManipulatingGravityTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && kaelthas->GetHealthPct() <= 50.0f;
}
