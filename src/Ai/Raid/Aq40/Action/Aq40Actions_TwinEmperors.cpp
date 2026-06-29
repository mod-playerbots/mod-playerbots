#include "Aq40Actions.h"

#include <algorithm>
#include <array>
#include <limits>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Pet.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "../Aq40SpellIds.h"
#include "../Util/Aq40Helpers_Shared.h"
#include "../Aq40Scripts.h"

namespace
{
float constexpr kTwinExplodeBugDangerRadius = 17.0f;
float constexpr kTwinArcaneBurstDangerRadius = 18.0f;
float constexpr kTwinArcaneBurstLooseRadius = 24.0f;
float constexpr kTwinWarlockMinRange = 19.0f;
float constexpr kTwinWarlockMaxRange = 30.0f;
float constexpr kTwinWarlockPreferredRange = 24.0f;
float constexpr kTwinMeleeContactRange = 5.0f;
float constexpr kTwinBossSeparationDistance = 62.0f;

enum class TwinMarkerAssignment : uint8
{
    None,
    Skull,
    Cross
};

struct TwinMarkerSwapState
{
    Aq40BossHelper::Twin::TankPairAssignments assignments;
    bool assignmentsComplete = false;
    bool crossParity = false;
    bool inTeleportWindow = false;
    bool initialized = false;
};

std::mutex sTwinMarkerSwapMutex;
std::unordered_map<uint32, TwinMarkerSwapState> sTwinMarkerSwapStateByInstance;

uint32 GetTwinInstanceKey(Player* bot)
{
    if (!bot)
        return 0;

    return bot->GetMap() ? bot->GetMap()->GetInstanceId() : bot->GetMapId();
}

uint8 CountAssigned(std::array<ObjectGuid, 2> const& guids)
{
    uint8 count = 0;
    for (ObjectGuid const& guid : guids)
    {
        if (!guid.IsEmpty())
            ++count;
    }

    return count;
}

bool SameAssignments(Aq40BossHelper::Twin::TankPairAssignments const& left,
                     Aq40BossHelper::Twin::TankPairAssignments const& right)
{
    for (uint8 index = 0; index < 2; ++index)
    {
        if (left.warlockTanks[index] != right.warlockTanks[index] ||
            left.meleeTanks[index] != right.meleeTanks[index])
        {
            return false;
        }
    }

    return true;
}

TwinMarkerSwapState RefreshTwinMarkerSwapState(Player* bot)
{
    TwinMarkerSwapState result;
    if (!bot)
        return result;

    uint32 const instanceKey = GetTwinInstanceKey(bot);
    if (!instanceKey)
        return result;

    Aq40BossHelper::Twin::TankPairAssignments const assignments =
        Aq40BossHelper::Twin::GetTankPairAssignments(bot);
    bool const assignmentsComplete = assignments.IsComplete();
    bool const teleportWindow = Aq40Scripts::IsTwinTeleportPickupWindow(bot);

    {
        std::lock_guard<std::mutex> guard(sTwinMarkerSwapMutex);
        TwinMarkerSwapState& stored = sTwinMarkerSwapStateByInstance[instanceKey];
        bool const assignmentsChanged = !stored.initialized ||
                                        !SameAssignments(stored.assignments, assignments) ||
                                        stored.assignmentsComplete != assignmentsComplete;

        if (assignmentsChanged)
        {
            stored.assignments = assignments;
            stored.assignmentsComplete = assignmentsComplete;
            stored.crossParity = false;
            stored.inTeleportWindow = false;
            stored.initialized = true;
        }

        if (assignmentsComplete && teleportWindow && !stored.inTeleportWindow)
            stored.crossParity = !stored.crossParity;

        stored.inTeleportWindow = teleportWindow;
        result = stored;
    }

    if (!assignmentsComplete)
    {
        std::ostringstream fields;
        fields << "boss=twin reason=missing_marker_swap_pairs"
               << " warlocks=" << static_cast<uint32>(CountAssigned(assignments.warlockTanks))
               << " melee_tanks=" << static_cast<uint32>(CountAssigned(assignments.meleeTanks));
        Aq40Helpers::LogAq40Warn(bot, "tank_assignment", "twin:marker_swap:incomplete",
            fields.str(), 5000);
    }

    return result;
}

TwinMarkerAssignment GetAssignedMarkerForPair(TwinMarkerSwapState const& state, uint8 pairIndex)
{
    if (!state.assignmentsComplete || pairIndex > 1)
        return TwinMarkerAssignment::None;

    bool const pairZeroOnCross = state.crossParity;
    if (pairIndex == 0)
        return pairZeroOnCross ? TwinMarkerAssignment::Cross : TwinMarkerAssignment::Skull;

    return pairZeroOnCross ? TwinMarkerAssignment::Skull : TwinMarkerAssignment::Cross;
}

TwinMarkerAssignment GetAssignedMarkerForBot(TwinMarkerSwapState const& state, Player* bot)
{
    int8 const pairIndex = state.assignments.GetPairIndex(bot);
    if (pairIndex < 0)
        return TwinMarkerAssignment::None;

    return GetAssignedMarkerForPair(state, static_cast<uint8>(pairIndex));
}

Player* GetPlayerByGuid(PlayerbotAI* botAI, ObjectGuid guid)
{
    if (!botAI || guid.IsEmpty())
        return nullptr;

    Unit* unit = botAI->GetUnit(guid);
    return unit ? unit->ToPlayer() : nullptr;
}

Player* GetAssignedWarlockForMarker(PlayerbotAI* botAI, TwinMarkerSwapState const& state,
                                    TwinMarkerAssignment marker)
{
    if (!state.assignmentsComplete || marker == TwinMarkerAssignment::None)
        return nullptr;

    for (uint8 index = 0; index < 2; ++index)
    {
        if (GetAssignedMarkerForPair(state, index) == marker)
            return GetPlayerByGuid(botAI, state.assignments.warlockTanks[index]);
    }

    return nullptr;
}

Player* GetAssignedMeleeTankForMarker(PlayerbotAI* botAI, TwinMarkerSwapState const& state,
                                      TwinMarkerAssignment marker)
{
    if (!state.assignmentsComplete || marker == TwinMarkerAssignment::None)
        return nullptr;

    for (uint8 index = 0; index < 2; ++index)
    {
        if (GetAssignedMarkerForPair(state, index) == marker)
            return GetPlayerByGuid(botAI, state.assignments.meleeTanks[index]);
    }

    return nullptr;
}

bool IsTwinBoss(Unit* unit)
{
    return unit && Aq40SpellIds::IsTwinEmperorEntry(unit->GetEntry());
}

bool StopTwinBossPressure(Player* bot)
{
    if (!bot)
        return false;

    bool stopped = false;
    if (IsTwinBoss(bot->GetVictim()))
    {
        bot->AttackStop();
        stopped = true;
    }

    if (Pet* pet = bot->GetPet())
    {
        if (IsTwinBoss(pet->GetVictim()))
        {
            pet->AttackStop();
            stopped = true;
        }
    }

    return stopped;
}

bool HoldTwinStandby(Player* bot, PlayerbotAI* botAI)
{
    bool stopped = StopTwinBossPressure(bot);

    if (botAI && botAI->GetAiObjectContext())
    {
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (IsTwinBoss(currentTarget))
        {
            botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Set(nullptr);
            if (bot)
                bot->SetTarget(ObjectGuid::Empty);
            stopped = true;
        }
    }

    return stopped;
}

std::vector<Player*> GetFallbackMeleeTankCandidates(Player* bot)
{
    std::vector<Player*> tanks;
    auto append = [&tanks, bot](Player* candidate)
    {
        if (!candidate || !candidate->IsAlive() || candidate->getClass() == CLASS_WARLOCK ||
            !Aq40BossHelper::IsSameInstance(bot, candidate))
        {
            return;
        }

        Aq40BossHelper::Twin::AppendUniquePlayer(tanks, candidate);
    };

    append(Aq40BossHelper::GetEncounterPrimaryTank(bot));
    append(Aq40BossHelper::GetEncounterBackupTank(bot, 0));
    append(Aq40BossHelper::GetEncounterBackupTank(bot, 1));
    return tanks;
}

Player* GetNearestPlayerToUnit(std::vector<Player*> const& players, Unit* target)
{
    if (!target)
        return nullptr;

    Player* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (Player* player : players)
    {
        if (!player || !player->IsAlive())
            continue;

        float const distance = player->GetDistance2d(target);
        if (distance >= nearestDistance)
            continue;

        nearest = player;
        nearestDistance = distance;
    }

    return nearest;
}

bool IsFallbackVeknilashTank(Player* bot, Unit* veknilash)
{
    if (!bot || !veknilash || !Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40BossHelper::IsUnitFocusedOnPlayer(veknilash, bot))
        return true;

    return GetNearestPlayerToUnit(GetFallbackMeleeTankCandidates(bot), veknilash) == bot;
}

std::vector<Player*> GetFallbackWarlockCandidates(Player* bot)
{
    std::vector<Player*> warlocks;
    if (!bot)
        return warlocks;

    Group const* group = bot->GetGroup();
    for (Player* member : Aq40BossHelper::GetSameInstanceGroupMembers(bot))
    {
        if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
            continue;

        if ((group && group->IsAssistant(member->GetGUID())) || GET_PLAYERBOT_AI(member))
            Aq40BossHelper::Twin::AppendUniquePlayer(warlocks, member);
    }

    return warlocks;
}

bool IsFallbackVeklorWarlock(Player* bot, PlayerbotAI* botAI, Unit* veklor)
{
    if (!bot || !botAI || !veklor || bot->getClass() != CLASS_WARLOCK || botAI->IsHeal(bot))
        return false;

    if (Aq40BossHelper::IsUnitFocusedOnPlayer(veklor, bot))
        return true;

    return GetNearestPlayerToUnit(GetFallbackWarlockCandidates(bot), veklor) == bot;
}

bool HasVeklorWarlockPickup(Player* bot, PlayerbotAI* botAI, TwinMarkerSwapState const& state, Unit* veklor)
{
    if (!veklor)
        return false;

    if (state.assignmentsComplete)
        return Aq40BossHelper::IsUnitFocusedOnPlayer(
            veklor, GetAssignedWarlockForMarker(botAI, state, TwinMarkerAssignment::Cross));

    for (Player* warlock : GetFallbackWarlockCandidates(bot))
    {
        if (Aq40BossHelper::IsUnitFocusedOnPlayer(veklor, warlock))
            return true;
    }

    return false;
}

bool HasVeknilashMeleePickup(Player* bot, PlayerbotAI* botAI, TwinMarkerSwapState const& state, Unit* veknilash)
{
    if (!veknilash)
        return false;

    if (state.assignmentsComplete)
        return Aq40BossHelper::IsUnitFocusedOnPlayer(
            veknilash, GetAssignedMeleeTankForMarker(botAI, state, TwinMarkerAssignment::Skull));

    return Aq40BossHelper::IsUnitHeldByEncounterTank(bot, veknilash);
}

bool ShouldAttackTwinBug(Player* bot, PlayerbotAI* botAI)
{
    return bot && botAI && !botAI->IsHeal(bot) && !Aq40BossHelper::Twin::IsTankPairMember(bot) &&
           (bot->getClass() == CLASS_HUNTER || PlayerbotAI::IsRanged(bot));
}

void ApplyTwinBossMarkers(Player* bot, Unit* veknilash, Unit* veklor)
{
    if (!bot)
        return;

    if (veknilash)
        Aq40Helpers::SetRaidTargetIcon(bot, veknilash, RtiTargetValue::skullIndex, "twin", "skull");
    if (veklor)
        Aq40Helpers::SetRaidTargetIcon(bot, veklor, RtiTargetValue::crossIndex, "twin", "cross");
}

void ApplyTwinTargetMarker(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!target)
        return;

    switch (target->GetEntry())
    {
        case Aq40SpellIds::TwinVeknilashNpcEntry:
            Aq40Helpers::SetRtiTarget(botAI, "skull", target);
            break;
        case Aq40SpellIds::TwinVeklorNpcEntry:
            Aq40Helpers::SetRtiTarget(botAI, "cross", target);
            break;
        default:
            if (Aq40BossHelper::Twin::IsTwinKillBug(botAI, target))
            {
                Aq40Helpers::SetRaidTargetIcon(bot, target, RtiTargetValue::starIndex, "twin", "star");
                Aq40Helpers::SetRtiTarget(botAI, "star", target);
            }
            break;
    }
}

Unit* ResolveTwinTarget(Player* bot, PlayerbotAI* botAI, GuidVector const& encounterUnits, char const*& reason)
{
    reason = "fallback";
    if (!bot || !botAI)
        return nullptr;

    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    TwinMarkerSwapState const markerSwap = RefreshTwinMarkerSwapState(bot);

    if (Aq40BossHelper::Twin::IsTankPairMember(bot))
    {
        reason = "tank_pair";
        return nullptr;
    }

    Unit* currentStar = Aq40Helpers::ResolveRaidTargetIcon(bot, botAI, RtiTargetValue::starIndex);
    if (ShouldAttackTwinBug(bot, botAI))
    {
        if (Aq40BossHelper::Twin::IsTwinKillBug(botAI, currentStar))
        {
            reason = "bug";
            return currentStar;
        }

        float const bugRange = bot->getClass() == CLASS_HUNTER ? 30.0f : 26.0f;
        Unit* nearbyBug = Aq40BossHelper::Twin::FindNearestKillBug(bot, botAI, encounterUnits, bugRange);
        if (nearbyBug)
        {
            reason = "bug";
            return nearbyBug;
        }
    }

    if (currentStar && Aq40SpellIds::IsTwinBugEntry(currentStar->GetEntry()) &&
        !Aq40BossHelper::Twin::IsTwinKillBug(botAI, currentStar))
    {
        Aq40Helpers::ClearRaidTargetIcon(bot, RtiTargetValue::starIndex, "twin", "star");
    }

    if (Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI) ||
        Aq40BossHelper::Twin::IsTrueCasterProfile(bot, botAI))
    {
        if (veklor && !HasVeklorWarlockPickup(bot, botAI, markerSwap, veklor))
        {
            reason = "wait_veklor_tank";
            return nullptr;
        }

        reason = "veklor";
        return veklor ? veklor : veknilash;
    }

    if (Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI))
    {
        if (veknilash && !HasVeknilashMeleePickup(bot, botAI, markerSwap, veknilash))
        {
            reason = "wait_veknilash_tank";
            return nullptr;
        }

        reason = "veknilash";
        return veknilash ? veknilash : veklor;
    }

    return veklor ? veklor : veknilash;
}

bool CastFirstAvailable(PlayerbotAI* botAI, Unit* target, std::initializer_list<char const*> spells)
{
    if (!botAI || !target)
        return false;

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target))
            return true;
    }

    return false;
}

bool CastFirstAvailableSelf(PlayerbotAI* botAI, Player* bot, std::initializer_list<char const*> spells)
{
    if (!botAI || !bot)
        return false;

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

bool CastTwinMeleeThreat(PlayerbotAI* botAI, Unit* target)
{
    return CastFirstAvailable(botAI, target,
        { "shield slam", "revenge", "devastate", "sunder armor", "heroic strike",
          "hammer of the righteous", "shield of righteousness", "judgement", "avenger's shield",
          "mangle (bear)", "lacerate", "maul", "swipe (bear)",
          "icy touch", "rune strike", "heart strike", "death strike", "plague strike" });
}

void StopPetFromVeklor(Player* bot, Unit* veklor)
{
    if (!bot || !veklor)
        return;

    Pet* pet = bot->GetPet();
    if (pet && pet->GetVictim() == veklor)
        pet->AttackStop();
}

bool HasTrackedExplodeBugHazard(Player* bot, PlayerbotAI* botAI, Unit*& outBug, Position& outPosition)
{
    outBug = nullptr;
    if (!bot || !botAI)
        return false;

    ObjectGuid sourceGuid;
    if (!Aq40Scripts::GetTwinExplodeBugSource(bot, sourceGuid, outPosition))
        return false;

    if (!sourceGuid.IsEmpty())
    {
        Unit* source = botAI->GetUnit(sourceGuid);
        if (source && source->IsAlive() && source->IsInWorld() && Aq40SpellIds::IsTwinBugEntry(source->GetEntry()))
        {
            outBug = source;
            outPosition = source->GetPosition();
        }
    }

    return bot->GetExactDist2d(outPosition.GetPositionX(), outPosition.GetPositionY()) <= kTwinExplodeBugDangerRadius;
}
}    // namespace

bool Aq40TwinChooseTargetAction::Execute(Event /*event*/)
{
    if (!bot || botAI->IsHeal(bot))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    ApplyTwinBossMarkers(bot, Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits),
                         Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits));

    char const* reason = "none";
    Unit* target = ResolveTwinTarget(bot, botAI, encounterUnits, reason);
    if (!target)
        return false;

    ApplyTwinTargetMarker(bot, botAI, target);

    if (target->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry &&
        Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI) &&
        !Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI))
    {
        return false;
    }

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "twin", reason, target, 1000);
    return Attack(target);
}

bool Aq40TwinTankAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    if (!veknilash)
        return false;

    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    ApplyTwinBossMarkers(bot, veknilash, veklor);

    TwinMarkerSwapState const markerSwap = RefreshTwinMarkerSwapState(bot);
    if (markerSwap.assignmentsComplete)
    {
        if (!markerSwap.assignments.IsMeleeTank(bot))
            return false;

        if (GetAssignedMarkerForBot(markerSwap, bot) != TwinMarkerAssignment::Skull)
        {
            bool const stopped = HoldTwinStandby(bot, botAI);
            if (!veklor)
                return stopped;

            float const distance = bot->GetDistance2d(veklor);
            if (distance < kTwinWarlockMinRange)
                return MoveAway(veklor, kTwinWarlockPreferredRange - distance) || stopped;

            if (distance > kTwinWarlockMaxRange)
                return MoveNear(veklor, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT) || stopped;

            return stopped;
        }
    }
    else if (!IsFallbackVeknilashTank(bot, veknilash))
        return false;

    Aq40Helpers::SetRtiTarget(botAI, "skull", veknilash);

    if (bot->GetDistance2d(veknilash) > 8.0f)
        return MoveNear(veknilash, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);

    if (veknilash->GetVictim() != bot)
    {
        if (CastFirstAvailable(botAI, veknilash, { "hand of reckoning", "dark command", "growl", "taunt" }))
            return true;
    }

    if (veklor && Aq40BossHelper::IsUnitFocusedOnPlayer(veknilash, bot) &&
        veknilash->GetDistance2d(veklor) < kTwinBossSeparationDistance)
    {
        float const separationGap = kTwinBossSeparationDistance - veknilash->GetDistance2d(veklor);
        return MoveAway(veklor, std::min(12.0f, separationGap + 2.0f));
    }

    if (CastTwinMeleeThreat(botAI, veknilash))
        return true;

    if (AI_VALUE(Unit*, "current target") == veknilash && bot->GetVictim() == veknilash)
        return false;

    Aq40Helpers::LogAq40Target(bot, "twin", "tank_veknilash", veknilash, 1000);
    return Attack(veknilash);
}

bool Aq40TwinWarlockTankAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    if (!veklor)
        return false;

    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    ApplyTwinBossMarkers(bot, veknilash, veklor);

    TwinMarkerSwapState const markerSwap = RefreshTwinMarkerSwapState(bot);
    if (markerSwap.assignmentsComplete)
    {
        if (!markerSwap.assignments.IsWarlockTank(bot))
            return false;

        if (GetAssignedMarkerForBot(markerSwap, bot) != TwinMarkerAssignment::Cross)
        {
            bool const stopped = HoldTwinStandby(bot, botAI);
            if (!veknilash)
                return stopped;

            float const distance = bot->GetDistance2d(veknilash);
            if (distance < kTwinWarlockMinRange)
                return MoveAway(veknilash, kTwinWarlockPreferredRange - distance) || stopped;

            if (distance > kTwinWarlockMaxRange)
                return MoveNear(veknilash, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT) || stopped;

            return stopped;
        }
    }
    else if (!IsFallbackVeklorWarlock(bot, botAI, veklor))
        return false;

    Aq40Helpers::SetRtiTarget(botAI, "cross", veklor);

    if (bot->GetTarget() != veklor->GetGUID() || AI_VALUE(Unit*, "current target") != veklor)
    {
        Aq40Helpers::LogAq40Target(bot, "twin", "warlock_veklor", veklor, 1000);
        Attack(veklor);
    }

    if (!botAI->HasAura("shadow ward", bot))
        CastFirstAvailableSelf(botAI, bot, { "shadow ward" });

    if (CastFirstAvailable(botAI, veklor, { "searing pain", "shadow bolt" }))
        return true;

    float const distance = bot->GetDistance2d(veklor);
    if (distance < kTwinWarlockMinRange)
        return MoveAway(veklor, kTwinWarlockPreferredRange - distance);

    if (distance > kTwinWarlockMaxRange)
        return MoveNear(veklor, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT);

    return bot->GetVictim() != veklor ? Attack(veklor) : false;
}

bool Aq40TwinAvoidHazardAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* explodeBug = Aq40BossHelper::Twin::FindNearestBug(
        bot, botAI, encounterUnits, kTwinExplodeBugDangerRadius, true);
    Position explodePosition;
    if (!explodeBug && HasTrackedExplodeBugHazard(bot, botAI, explodeBug, explodePosition))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:explode_bug:tracked",
            "boss=twin hazard=explode_bug source=tracked_script_source", 1000);
        return FleePosition(explodePosition, kTwinExplodeBugDangerRadius, 250U);
    }

    if (explodeBug)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "twin:explode_bug:" + Aq40Helpers::GetAq40LogUnit(explodeBug),
            "boss=twin hazard=explode_bug source=" + Aq40Helpers::GetAq40LogUnit(explodeBug), 1000);
        return FleePosition(explodeBug->GetPosition(), kTwinExplodeBugDangerRadius, 250U) ||
               MoveAway(explodeBug, kTwinExplodeBugDangerRadius - bot->GetDistance2d(explodeBug));
    }

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::TwinBlizzard }) ||
        Aq40Scripts::IsTwinBlizzardWindow(bot))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        if (botAI->DoSpecificAction("avoid aoe", Event(), true))
        {
            Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard",
                "boss=twin hazard=blizzard action=avoid_aoe", 1000);
            return true;
        }
    }

    return false;
}

bool Aq40TwinAvoidVeklorAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    if (!veklor)
        return false;

    TwinMarkerSwapState const markerSwap = RefreshTwinMarkerSwapState(bot);
    if (markerSwap.assignmentsComplete)
    {
        if (markerSwap.assignments.IsWarlockTank(bot) &&
            GetAssignedMarkerForBot(markerSwap, bot) == TwinMarkerAssignment::Cross)
        {
            return false;
        }
    }
    else if (IsFallbackVeklorWarlock(bot, botAI, veklor))
        return false;

    StopPetFromVeklor(bot, veklor);

    float const distance = bot->GetDistance2d(veklor);
    bool const arcaneWindow = Aq40Scripts::IsTwinArcaneBurstWindow(bot);
    float const safeRadius = arcaneWindow ? kTwinArcaneBurstLooseRadius : kTwinArcaneBurstDangerRadius;
    if (distance > safeRadius)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "twin:veklor_range:" + Aq40Helpers::GetAq40LogUnit(veklor),
        "boss=twin hazard=arcane_burst source=" + Aq40Helpers::GetAq40LogUnit(veklor), 1000);
    return MoveAway(veklor, safeRadius - distance + 2.0f);
}
