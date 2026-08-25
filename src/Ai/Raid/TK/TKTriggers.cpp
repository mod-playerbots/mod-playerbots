/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKTriggers.h"
#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "TKHelpers.h"
#include <array>

using namespace TkHelpers;
using namespace EncounterHelpers;

// General

// Read the instance's own encounter state rather than the bot's combat state to determine when
// it is safe to erase encounter maps.
bool TempestKeepNoEncounterInProgressTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

bool TempestKeepBotIsStuckFallingTrigger::IsActive()
{
    if (!bot->HasUnitMovementFlag(MOVEMENTFLAG_FALLING) || !bot->movespline->Finalized())
        return false;

    if (bot->GetMapId() != TK_MAP_ID)
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    return instance && !instance->IsEncounterInProgress();
}

// Trash

bool CrimsonHandCenturionCastsArcaneFlurryTrigger::IsActive()
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
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    int8 locationIndex = GetAlarCurrentLocationIndex(alar);
    if (locationIndex == LOCATION_NONE)
        locationIndex = GetAlarDestinationLocationIndex(alar);

    return locationIndex != POINT_QUILL_OR_DIVE_IDX && locationIndex != POINT_MIDDLE_IDX;
}

bool AlarEmbersExplodeUponDeathTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarKillingEmbersDamagesBossTrigger::IsActive()
{
    return PlayerbotAI::IsRangedDps(bot) && AI_VALUE2(Unit*, "find target", "ember of al'ar");
}

bool AlarIncomingFlameQuillsTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX ||
        GetAlarDestinationLocationIndex(alar) == POINT_QUILL_OR_DIVE_IDX;
}

bool AlarRisingFromTheAshesTrigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    if (!alar || alar->GetHealthPct() > 5.0f)
        return false;

    if (IsAlarInPhase2(alar->GetInstanceId()))
        return false;

    return GetAlarCurrentLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX &&
        GetAlarDestinationLocationIndex(alar) != POINT_QUILL_OR_DIVE_IDX;
}

bool AlarIsInPhase2Trigger::IsActive()
{
    Unit* alar = AI_VALUE2(Unit*, "find target", "al'ar");
    return alar && IsAlarInPhase2(alar->GetInstanceId());
}

bool AlarShouldManagePhaseTrackerTrigger::IsActive()
{
    return IsMechanicTrackerBot(bot, TK_MAP_ID) && AI_VALUE2(Unit*, "find target", "al'ar");
}

// Void Reaver

bool VoidReaverShouldBeTankedTrigger::IsActive()
{
    return PlayerbotAI::IsTank(bot) && AI_VALUE2(Unit*, "find target", "void reaver");
}

bool VoidReaverKnockAwayPullsAggroToNonTanksTrigger::IsActive()
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

    return !IsNearActiveArcaneOrb(bot, ARCANE_ORB_BUFFER_DISTANCE);
}

bool VoidReaverArcaneOrbIsIncomingTrigger::IsActive()
{
    if (PlayerbotAI::IsTank(bot))
        return false;

    Unit* voidReaver = AI_VALUE2(Unit*, "find target", "void reaver");
    if (!voidReaver || voidReaver->GetVictim() == bot)
        return false;

    return IsNearActiveArcaneOrb(bot, ARCANE_ORB_SAFE_DISTANCE);
}

// High Astromancer Solarian

bool HighAstromancerSolarianShouldBeTankedTrigger::IsActive()
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

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (PlayerbotAI::IsTank(bot) && phase == PHASE_ALL_ADVISORS)
        return false;

    return phase != PHASE_NONE;
}

bool KaelthasSunstriderPullingTankableAdvisorsTrigger::IsActive()
{
    if (bot->getClass() != CLASS_HUNTER)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderSanguinarOrTelonicusShouldBeTankedTrigger::IsActive()
{
    if (!PlayerbotAI::IsTank(bot))
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "lord sanguinar"));

    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "master engineer telonicus"));

    return false;
}

bool KaelthasSunstriderSanguinarCastsBellowingRoarTrigger::IsActive()
{
    if (bot->getClass() != CLASS_PRIEST)
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase != PHASE_SINGLE_ADVISOR && phase != PHASE_TRANSITION && phase != PHASE_ALL_ADVISORS)
        return false;

    return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "lord sanguinar"));
}

bool KaelthasSunstriderCapernianShouldBeTankedByWarlockTrigger::IsActive()
{
    if (bot->getClass() != CLASS_WARLOCK || GetCapernianTank(bot) != bot)
        return false;

    return IsAdvisorActive(AI_VALUE2(Unit*, "find target", "grand astromancer capernian"));
}

bool KaelthasSunstriderCapernianBlowsUpNearAndFarTrigger::IsActive()
{
    if (!IsAdvisorActive(AI_VALUE2(Unit*, "find target", "grand astromancer capernian")))
        return false;

    if (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot)
        return false;

    return true;
}

bool KaelthasSunstriderBotsShouldHoldPhase3PositionsTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (GetKaelthasPhase(kaelthas) != PHASE_ALL_ADVISORS)
        return false;

    Unit* sanguinar = AI_VALUE2(Unit*, "find target", "lord sanguinar");
    // The healer holds its spot from the start of the revival until Sanguinar dies, since that
    // spot is what keeps both melee tanks in range.
    if (PlayerbotAI::IsAssistHealOfIndex(bot, 0, true))
        return sanguinar && sanguinar->IsAlive();

    // The Sanguinar check is a proxy for the revival/Kael talk phase (any non-selectable advisor
    // would do, since all four revive together, but Sanguinar is already needed for the healer).
    if (!sanguinar || !sanguinar->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        return false;

    return PlayerbotAI::IsMainTank(bot) ||
        PlayerbotAI::IsAssistTankOfIndex(bot, 0, true) ||
        (bot->getClass() == CLASS_WARLOCK && GetCapernianTank(bot) == bot);
}

bool KaelthasSunstriderDeterminingAdvisorKillOrderTrigger::IsActive()
{
    if (PlayerbotAI::IsMainTank(bot) || PlayerbotAI::IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    return phase == PHASE_SINGLE_ADVISOR || phase == PHASE_ALL_ADVISORS;
}

bool KaelthasSunstriderShouldManageAdvisorDpsTimerTrigger::IsActive()
{
    if (!IsMechanicTrackerBot(bot, TK_MAP_ID))
        return false;

    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    return GetKaelthasPhase(kaelthas) == PHASE_SINGLE_ADVISOR;
}

bool KaelthasSunstriderLegendaryWeaponsAreAliveTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    if (GetKaelthasPhase(kaelthas) != PHASE_WEAPONS)
        return false;

    return !PlayerbotAI::IsMainTank(bot);
}

bool KaelthasSunstriderLegendaryAxeCastsWhirlwindTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) &&
        GetLegendaryWeapon(bot, Id(TkNpcs::NPC_DEVASTATION)) != nullptr;
}

bool KaelthasSunstriderLegendaryWeaponsAreDeadTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return false;

    uint32 const phase = GetKaelthasPhase(kaelthas);
    if (phase < PHASE_WEAPONS || phase > PHASE_ALL_ADVISORS)
        return false;

    Unit* axe = GetLegendaryWeapon(bot, Id(TkNpcs::NPC_DEVASTATION));
    if (axe && axe->GetVictim() == bot)
        return false;

    return !GetDeadLegendaryWeaponGuids(botAI).empty();
}

bool KaelthasSunstriderLegendaryWeaponsAreEquippedTrigger::IsActive()
{
    if (PlayerbotAI::IsHeal(bot))
        return false;

    if (PlayerbotAI::IsMelee(bot) && PlayerbotAI::IsDps(bot))
        return false;

    if (!AI_VALUE2(Unit*, "find target", "kael'thas sunstrider"))
        return false;

    return GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_MAINHAND, Id(TkItems::ITEM_STAFF_OF_DISINTEGRATION)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_RANGED, Id(TkItems::ITEM_NETHERSTRAND_LONGBOW)) ||
        GetEquippedItemInSlot(
               bot, EQUIPMENT_SLOT_OFFHAND, Id(TkItems::ITEM_PHASESHIFT_BULWARK));
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
    if (!kaelthas || bot->GetExactDist2d(kaelthas) > KAELTHAS_ROOM_SEARCH_DISTANCE)
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

    return GetKaelthasPhase(kaelthas) == PHASE_FINAL;
}

bool KaelthasSunstriderPhoenixesAndEggsAreSpawningTrigger::IsActive()
{
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas || kaelthas->GetVictim() == bot)
        return false;

    if (GetKaelthasPhase(kaelthas) != PHASE_FINAL)
        return false;

    if (PlayerbotAI::IsMainTank(bot))
        return false;

    if (AI_VALUE2(Unit*, "find target", "phoenix"))
        return true;

    return GetPhoenixEgg(bot);
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
    constexpr float gravityLapseHpThreshold = 50.0f;
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    return kaelthas && kaelthas->GetHealthPct() <= gravityLapseHpThreshold;
}
