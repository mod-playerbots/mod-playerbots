#include "Aq40Actions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "AiFactory.h"
#include "Pet.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "../Aq40SpellIds.h"
#include "../Util/Aq40Helpers_Shared.h"
#include "../Aq40Scripts.h"

namespace
{
float constexpr kTwinExplodeBugDangerRadius = 17.0f;
float constexpr kTwinArcaneBurstAvoidRadius = 10.0f;
float constexpr kTwinWarlockMinRange = 19.0f;
float constexpr kTwinWarlockMaxRange = 30.0f;
float constexpr kTwinWarlockPreferredRange = 24.0f;
float constexpr kTwinMeleeContactRange = 5.0f;
float constexpr kTwinMeleeControlMaxRange = 8.0f;
float constexpr kTwinBossSeparationTargetDistance = 72.0f;
float constexpr kTwinBossSeparationResumeDistance = 68.0f;

enum class TwinMarkerAssignment : uint8
{
    None,
    Skull,
    Cross
};

struct TwinMarkerSwapState
{
    Aq40BossHelper::Twin::TankPairAssignments assignments;
    Aq40BossHelper::Twin::TankAssignmentMode assignmentMode = Aq40BossHelper::Twin::TankAssignmentMode::Incomplete;
    uint32 lastTeleportSequence = 0;
    bool crossParity = false;
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

std::string FormatAssignmentGuid(ObjectGuid guid)
{
    if (guid.IsEmpty())
        return "none";

    std::ostringstream out;
    out << guid.GetCounter();
    return out.str();
}

std::string BuildAssignmentStateKey(Aq40BossHelper::Twin::TankPairAssignments const& assignments,
                                    Aq40BossHelper::Twin::TankAssignmentMode mode, bool crossParity)
{
    std::ostringstream key;
    key << Aq40BossHelper::Twin::GetTankAssignmentModeToken(mode)
        << ":w0:" << FormatAssignmentGuid(assignments.warlockTanks[0])
        << ":w1:" << FormatAssignmentGuid(assignments.warlockTanks[1])
        << ":m0:" << FormatAssignmentGuid(assignments.meleeTanks[0])
        << ":m1:" << FormatAssignmentGuid(assignments.meleeTanks[1])
        << ":cross:" << (crossParity ? 1 : 0);
    return key.str();
}

void AppendAssignmentFields(std::ostringstream& fields,
                            Aq40BossHelper::Twin::TankPairAssignments const& assignments,
                            Aq40BossHelper::Twin::TankAssignmentMode mode,
                            bool crossParity = false)
{
    fields << " mode=" << Aq40BossHelper::Twin::GetTankAssignmentModeToken(mode)
           << " complete=" << (mode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs ? 1 : 0)
           << " cross_parity=" << (crossParity ? 1 : 0)
           << " warlocks=" << static_cast<uint32>(CountAssigned(assignments.warlockTanks))
           << " melee_tanks=" << static_cast<uint32>(CountAssigned(assignments.meleeTanks))
           << " warlock0_guid=" << FormatAssignmentGuid(assignments.warlockTanks[0])
           << " warlock1_guid=" << FormatAssignmentGuid(assignments.warlockTanks[1])
           << " melee0_guid=" << FormatAssignmentGuid(assignments.meleeTanks[0])
           << " melee1_guid=" << FormatAssignmentGuid(assignments.meleeTanks[1])
           << " active_warlock_guid=" << FormatAssignmentGuid(assignments.GetActiveWarlockTankGuid(crossParity))
           << " active_melee_guid=" << FormatAssignmentGuid(assignments.GetActiveMeleeTankGuid(crossParity));
}

void LogTwinAssignmentState(Player* bot, Aq40BossHelper::Twin::TankPairAssignments const& assignments,
                            Aq40BossHelper::Twin::TankAssignmentMode mode, bool crossParity,
                            uint32 teleportSequence = 0)
{
    std::ostringstream fields;
    fields << "boss=twin reason=";
    switch (mode)
    {
        case Aq40BossHelper::Twin::TankAssignmentMode::FullPairs:
            fields << "full_marker_swap_pairs_ready";
            break;
        case Aq40BossHelper::Twin::TankAssignmentMode::SinglePairFallback:
            fields << "single_pair_fallback";
            break;
        case Aq40BossHelper::Twin::TankAssignmentMode::Incomplete:
        default:
            fields << "missing_tank_pairs";
            break;
    }
    AppendAssignmentFields(fields, assignments, mode, crossParity);
    fields << " teleport_sequence=" << teleportSequence;

    std::string const stateKey = "twin:marker_swap:" + BuildAssignmentStateKey(assignments, mode, crossParity);
    if (mode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs)
        Aq40Helpers::LogAq40Info(bot, "tank_assignment", stateKey, fields.str());
    else
        Aq40Helpers::LogAq40Warn(bot, "tank_assignment", stateKey, fields.str());
}

bool IsTwinTankRelevantClass(Player* player)
{
    if (!player)
        return false;

    switch (player->getClass())
    {
        case CLASS_DEATH_KNIGHT:
        case CLASS_DRUID:
        case CLASS_PALADIN:
        case CLASS_WARRIOR:
        case CLASS_WARLOCK:
            return true;
        default:
            return PlayerbotAI::IsTank(player) || PlayerbotAI::IsExplicitMainTank(player);
    }
}

void LogTwinTankCandidates(Player* bot, Aq40BossHelper::Twin::TankPairAssignments const& assignments,
                           Aq40BossHelper::Twin::TankAssignmentMode mode)
{
    if (!bot || mode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs)
        return;

    Group const* group = bot->GetGroup();
    if (!group)
        return;

    uint32 const botInstanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !IsTwinTankRelevantClass(member))
            continue;

        bool const sameMap = member->GetMapId() == bot->GetMapId();
        bool const sameInstance = sameMap && (!member->GetMap() || member->GetMap()->GetInstanceId() == botInstanceId);
        bool const selectedMelee = assignments.IsMeleeTank(member);
        bool const selectedWarlock = assignments.IsWarlockTank(member);
        bool const runtimeTank = PlayerbotAI::IsTank(member);
        bool const tankRole = Aq40BossHelper::Twin::HasTwinMeleeTankRole(member);

        std::ostringstream fields;
        fields << "boss=twin reason=tank_candidate"
               << " mode=" << Aq40BossHelper::Twin::GetTankAssignmentModeToken(mode)
               << " candidate=" << Aq40Helpers::GetAq40LogToken(member->GetName())
               << " guid=" << FormatAssignmentGuid(member->GetGUID())
               << " class=" << static_cast<uint32>(member->getClass())
               << " spec_tab=" << static_cast<uint32>(AiFactory::GetPlayerSpecTab(member))
               << " form=" << static_cast<uint32>(member->GetShapeshiftForm())
               << " bot=" << (GET_PLAYERBOT_AI(member) ? 1 : 0)
               << " alive=" << (member->IsAlive() ? 1 : 0)
               << " in_world=" << (member->IsInWorld() ? 1 : 0)
               << " same_map=" << (sameMap ? 1 : 0)
               << " same_instance=" << (sameInstance ? 1 : 0)
               << " group_assistant=" << (group->IsAssistant(member->GetGUID()) ? 1 : 0)
               << " explicit_mt=" << (PlayerbotAI::IsExplicitMainTank(member) ? 1 : 0)
               << " runtime_tank=" << (runtimeTank ? 1 : 0)
               << " tank_role=" << (tankRole ? 1 : 0)
               << " twin_candidate=" << (Aq40BossHelper::Twin::IsTwinMeleeTankCandidate(bot, member) ? 1 : 0)
               << " selected_melee=" << (selectedMelee ? 1 : 0)
               << " selected_warlock=" << (selectedWarlock ? 1 : 0);

        Aq40Helpers::LogAq40Info(bot, "tank_candidate",
            "twin:tank_candidate:" + Aq40Helpers::GetAq40LogToken(member->GetName()) + ":" +
                Aq40BossHelper::Twin::GetTankAssignmentModeToken(mode),
            fields.str(), 10000);
    }
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

    return left.mode == right.mode;
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
    Aq40BossHelper::Twin::TankAssignmentMode const assignmentMode = assignments.mode;
    uint32 const teleportSequence = Aq40Scripts::GetTwinTeleportSequence(bot);
    bool logAssignmentState = false;

    {
        std::lock_guard<std::mutex> guard(sTwinMarkerSwapMutex);
        TwinMarkerSwapState& stored = sTwinMarkerSwapStateByInstance[instanceKey];
        bool const assignmentsChanged = !stored.initialized ||
                                        !SameAssignments(stored.assignments, assignments) ||
                                        stored.assignmentMode != assignmentMode;

        if (assignmentsChanged)
        {
            bool const preserveMeleeParity =
                stored.initialized &&
                stored.assignmentMode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs &&
                assignmentMode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs &&
                stored.assignments.meleeTanks == assignments.meleeTanks;

            stored.assignments = assignments;
            stored.assignmentMode = assignmentMode;
            if (!preserveMeleeParity)
                stored.crossParity = false;

            stored.initialized = true;
            logAssignmentState = true;
        }

        if (assignmentMode == Aq40BossHelper::Twin::TankAssignmentMode::FullPairs &&
            teleportSequence && teleportSequence != stored.lastTeleportSequence)
        {
            uint32 const sequenceDelta = teleportSequence - stored.lastTeleportSequence;
            if ((sequenceDelta % 2) != 0)
                stored.crossParity = !stored.crossParity;

            stored.lastTeleportSequence = teleportSequence;
            logAssignmentState = true;
        }
        else if (assignmentMode != Aq40BossHelper::Twin::TankAssignmentMode::FullPairs)
        {
            stored.crossParity = false;
            stored.lastTeleportSequence = teleportSequence;
        }

        result = stored;
    }

    if (logAssignmentState)
    {
        LogTwinAssignmentState(bot, result.assignments, result.assignmentMode, result.crossParity,
                               result.lastTeleportSequence);
        LogTwinTankCandidates(bot, result.assignments, result.assignmentMode);
    }

    return result;
}

TwinMarkerAssignment GetAssignedMarkerForPair(TwinMarkerSwapState const& state, uint8 pairIndex)
{
    if (!state.assignments.HasFullPairs() || pairIndex > 1)
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

char const* ToMarkerToken(TwinMarkerAssignment marker)
{
    switch (marker)
    {
        case TwinMarkerAssignment::Skull:
            return "skull";
        case TwinMarkerAssignment::Cross:
            return "cross";
        case TwinMarkerAssignment::None:
        default:
            return "none";
    }
}

void LogTwinTankRole(Player* bot, std::string const& assignmentState, TwinMarkerAssignment marker, Unit* target)
{
    std::ostringstream fields;
    fields << "boss=twin assignment=" << Aq40Helpers::GetAq40LogToken(assignmentState)
           << " marker=" << ToMarkerToken(marker)
           << " target=" << Aq40Helpers::GetAq40LogUnit(target);
    Aq40Helpers::LogAq40Info(bot, "tank_assignment",
        "twin:role:" + Aq40Helpers::GetAq40LogToken(assignmentState) + ":" + ToMarkerToken(marker),
        fields.str(), 5000);
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
        if (!Aq40BossHelper::Twin::IsTwinMeleeTankCandidate(bot, candidate))
            return;

        Aq40BossHelper::Twin::AppendUniquePlayer(tanks, candidate);
    };

    append(Aq40BossHelper::GetEncounterPrimaryTank(bot));
    append(Aq40BossHelper::GetEncounterBackupTank(bot, 0));
    append(Aq40BossHelper::GetEncounterBackupTank(bot, 1));
    for (Player* member : Aq40BossHelper::GetSameInstanceGroupMembers(bot))
        append(member);
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
    if (!bot || !veknilash || !Aq40BossHelper::Twin::IsTwinMeleeTankCandidate(bot, bot))
        return false;

    if (Aq40BossHelper::IsUnitFocusedOnPlayer(veknilash, bot))
        return true;

    return GetNearestPlayerToUnit(GetFallbackMeleeTankCandidates(bot), veknilash) == bot;
}

bool IsActiveTwinMeleeTank(Player* bot, TwinMarkerSwapState const& markerSwap, Unit* veknilash)
{
    if (!bot)
        return false;

    if (markerSwap.assignments.HasUsableAssignment())
        return markerSwap.assignments.IsActiveMeleeTank(bot, markerSwap.crossParity);

    return IsFallbackVeknilashTank(bot, veknilash);
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

    if (Aq40BossHelper::Twin::IsTankPairMember(bot))
    {
        reason = "tank_pair";
        return nullptr;
    }

    Unit* currentStar = Aq40Helpers::ResolveRaidTargetIcon(bot, botAI, RtiTargetValue::starIndex);
    bool const shouldAttackBug = ShouldAttackTwinBug(bot, botAI);
    if (shouldAttackBug)
    {
        float const bugRange = bot->getClass() == CLASS_HUNTER ? 30.0f : 26.0f;
        if (Aq40BossHelper::Twin::IsTwinKillBug(botAI, currentStar) &&
            bot->GetDistance2d(currentStar) <= bugRange)
        {
            reason = "bug";
            return currentStar;
        }

        Unit* nearbyBug = Aq40BossHelper::Twin::FindNearestKillBug(bot, botAI, encounterUnits, bugRange);
        if (nearbyBug)
        {
            reason = "bug";
            return nearbyBug;
        }

        if (currentStar && Aq40SpellIds::IsTwinBugEntry(currentStar->GetEntry()) &&
            !Aq40BossHelper::Twin::IsTwinKillBug(botAI, currentStar))
        {
            Aq40Helpers::ClearRaidTargetIcon(bot, RtiTargetValue::starIndex, "twin", "star");
        }
    }

    if (Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI) ||
        Aq40BossHelper::Twin::IsTrueCasterProfile(bot, botAI))
    {
        reason = "veklor";
        return veklor ? veklor : veknilash;
    }

    if (Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI))
    {
        reason = "veknilash";
        return veknilash ? veknilash : veklor;
    }

    return veklor ? veklor : veknilash;
}

char const* CastFirstAvailableSpellName(PlayerbotAI* botAI, Unit* target, std::initializer_list<char const*> spells)
{
    if (!botAI || !target)
        return nullptr;

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target))
            return spell;
    }

    return nullptr;
}

bool CastFirstAvailable(PlayerbotAI* botAI, Unit* target, std::initializer_list<char const*> spells)
{
    return CastFirstAvailableSpellName(botAI, target, spells) != nullptr;
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
        { "mangle (bear)", "lacerate", "maul", "swipe (bear)",
          "shield slam", "revenge", "devastate", "sunder armor", "heroic strike",
          "hammer of the righteous", "shield of righteousness", "judgement", "avenger's shield",
          "icy touch", "rune strike", "heart strike", "death strike", "plague strike" });
}

bool IsDruidBearForm(Player* bot)
{
    if (!bot || bot->getClass() != CLASS_DRUID)
        return true;

    ShapeshiftForm const form = bot->GetShapeshiftForm();
    return form == FORM_BEAR || form == FORM_DIREBEAR;
}

bool EnsureTwinMeleeTankForm(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI || bot->getClass() != CLASS_DRUID || IsDruidBearForm(bot))
        return false;

    bool const shifted = CastFirstAvailableSelf(botAI, bot, { "dire bear form", "bear form" });
    if (shifted)
    {
        std::ostringstream fields;
        fields << "boss=twin action=tank_form form=" << static_cast<uint32>(bot->GetShapeshiftForm());
        Aq40Helpers::LogAq40Info(bot, "tank_assignment", "twin:tank_form:bear", fields.str(), 1000);
    }

    return shifted;
}

bool CastTwinWarlockThreat(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    char const* spell = CastFirstAvailableSpellName(botAI, target, { "searing pain", "shadow bolt" });
    if (!spell)
        return false;

    std::ostringstream fields;
    fields << "boss=twin action=warlock_threat spell=" << Aq40Helpers::GetAq40LogToken(spell)
           << " target=" << Aq40Helpers::GetAq40LogUnit(target);
    Aq40Helpers::LogAq40Info(bot, "tank_assignment",
        "twin:warlock_threat:" + Aq40Helpers::GetAq40LogToken(spell), fields.str(), 1000);
    return true;
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

struct TwinTankAnchorMove
{
    Position anchor;
    float bossDistance = 0.0f;
    float tankRange = 0.0f;
    float anchorDistance = 0.0f;
    bool needsSeparation = false;
};

bool BuildTwinFarSideAnchor(Player* bot, Unit* heldBoss, Unit* oppositeBoss, float tankOffset,
                            TwinTankAnchorMove& outMove)
{
    if (!bot || !heldBoss || !oppositeBoss)
        return false;

    float dx = heldBoss->GetPositionX() - oppositeBoss->GetPositionX();
    float dy = heldBoss->GetPositionY() - oppositeBoss->GetPositionY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.1f)
    {
        dx = bot->GetPositionX() - oppositeBoss->GetPositionX();
        dy = bot->GetPositionY() - oppositeBoss->GetPositionY();
        distance = std::sqrt(dx * dx + dy * dy);
    }

    if (distance < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        distance = 1.0f;
    }

    float const nx = dx / distance;
    float const ny = dy / distance;
    float x = oppositeBoss->GetPositionX() + nx * (kTwinBossSeparationTargetDistance + tankOffset);
    float y = oppositeBoss->GetPositionY() + ny * (kTwinBossSeparationTargetDistance + tankOffset);
    float z = heldBoss->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);

    outMove.anchor.Relocate(x, y, z, bot->GetOrientation());
    outMove.bossDistance = heldBoss->GetDistance2d(oppositeBoss);
    outMove.tankRange = bot->GetDistance2d(heldBoss);
    outMove.anchorDistance = bot->GetExactDist2d(x, y);
    outMove.needsSeparation = outMove.bossDistance < kTwinBossSeparationResumeDistance;
    return outMove.needsSeparation;
}

bool BuildTwinMeleeContactAnchor(Player* bot, Unit* heldBoss, Unit* oppositeBoss, TwinTankAnchorMove& outMove)
{
    if (!bot || !heldBoss || !oppositeBoss)
        return false;

    float dx = heldBoss->GetPositionX() - oppositeBoss->GetPositionX();
    float dy = heldBoss->GetPositionY() - oppositeBoss->GetPositionY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.1f)
    {
        dx = bot->GetPositionX() - oppositeBoss->GetPositionX();
        dy = bot->GetPositionY() - oppositeBoss->GetPositionY();
        distance = std::sqrt(dx * dx + dy * dy);
    }

    if (distance < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        distance = 1.0f;
    }

    float const nx = dx / distance;
    float const ny = dy / distance;
    float x = heldBoss->GetPositionX() + nx * kTwinMeleeContactRange;
    float y = heldBoss->GetPositionY() + ny * kTwinMeleeContactRange;
    float z = heldBoss->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);

    outMove.anchor.Relocate(x, y, z, bot->GetOrientation());
    outMove.bossDistance = heldBoss->GetDistance2d(oppositeBoss);
    outMove.tankRange = bot->GetDistance2d(heldBoss);
    outMove.anchorDistance = bot->GetExactDist2d(x, y);
    outMove.needsSeparation = outMove.bossDistance < kTwinBossSeparationResumeDistance;
    return outMove.needsSeparation;
}

TwinTankAnchorMove BuildTwinTankMoveSnapshot(Player* bot, Unit* heldBoss, Unit* oppositeBoss)
{
    TwinTankAnchorMove move;
    if (!bot || !heldBoss)
        return move;

    move.bossDistance = oppositeBoss ? heldBoss->GetDistance2d(oppositeBoss) : 0.0f;
    move.tankRange = bot->GetDistance2d(heldBoss);
    move.anchorDistance = 0.0f;
    move.needsSeparation = oppositeBoss && move.bossDistance < kTwinBossSeparationResumeDistance;
    return move;
}

bool BuildTwinTankHazardContactAnchor(Player* bot, Unit* heldBoss, Position const& hazardPosition,
                                      TwinTankAnchorMove& outMove)
{
    if (!bot || !heldBoss)
        return false;

    float dx = heldBoss->GetPositionX() - hazardPosition.GetPositionX();
    float dy = heldBoss->GetPositionY() - hazardPosition.GetPositionY();
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 0.1f)
    {
        dx = bot->GetPositionX() - hazardPosition.GetPositionX();
        dy = bot->GetPositionY() - hazardPosition.GetPositionY();
        distance = std::sqrt(dx * dx + dy * dy);
    }

    if (distance < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        distance = 1.0f;
    }

    float const nx = dx / distance;
    float const ny = dy / distance;
    float x = heldBoss->GetPositionX() + nx * kTwinMeleeContactRange;
    float y = heldBoss->GetPositionY() + ny * kTwinMeleeContactRange;
    float z = heldBoss->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);

    outMove.anchor.Relocate(x, y, z, bot->GetOrientation());
    outMove.bossDistance = heldBoss->GetExactDist2d(hazardPosition.GetPositionX(), hazardPosition.GetPositionY());
    outMove.tankRange = bot->GetDistance2d(heldBoss);
    outMove.anchorDistance = bot->GetExactDist2d(x, y);
    outMove.needsSeparation = true;
    return heldBoss->GetExactDist2d(x, y) <= kTwinMeleeControlMaxRange;
}

void LogTwinTankMovement(Player* bot, char const* action, TwinMarkerSwapState const& markerSwap,
                         Unit* heldBoss, Unit* oppositeBoss, TwinTankAnchorMove const& move, bool moved,
                         Unit* currentVictim = nullptr, bool hasControl = false)
{
    if (!bot || !action)
        return;

    std::ostringstream fields;
    fields << "boss=twin action=" << action
           << " mode=" << Aq40BossHelper::Twin::GetTankAssignmentModeToken(markerSwap.assignmentMode)
           << " source=" << Aq40Helpers::GetAq40LogUnit(oppositeBoss)
           << " target=" << Aq40Helpers::GetAq40LogUnit(heldBoss)
           << " boss_distance=" << move.bossDistance
           << " tank_range=" << move.tankRange
           << " anchor_distance=" << move.anchorDistance
           << " current_victim=" << Aq40Helpers::GetAq40LogUnit(currentVictim)
           << " has_control=" << (hasControl ? 1 : 0)
           << " form=" << static_cast<uint32>(bot->GetShapeshiftForm())
           << " moved=" << (moved ? 1 : 0)
           << " active_warlock_guid="
           << FormatAssignmentGuid(markerSwap.assignments.GetActiveWarlockTankGuid(markerSwap.crossParity))
           << " active_melee_guid="
           << FormatAssignmentGuid(markerSwap.assignments.GetActiveMeleeTankGuid(markerSwap.crossParity));

    Aq40Helpers::LogAq40Info(bot, "tank_assignment",
        std::string("twin:") + action + ":" + Aq40BossHelper::Twin::GetTankAssignmentModeToken(markerSwap.assignmentMode),
        fields.str(), 1000);
}

void LogTwinActiveTankHazard(Player* bot, char const* hazard, Unit* heldBoss, TwinTankAnchorMove const& move,
                             bool moved)
{
    if (!bot || !hazard)
        return;

    std::ostringstream fields;
    fields << "boss=twin hazard=" << hazard
           << " action=active_tank_contact_reposition"
           << " target=" << Aq40Helpers::GetAq40LogUnit(heldBoss)
           << " tank_range=" << move.tankRange
           << " hazard_distance=" << move.bossDistance
           << " anchor_distance=" << move.anchorDistance
           << " moved=" << (moved ? 1 : 0);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard", std::string("twin:active_tank:") + hazard,
        fields.str(), 1000);
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
    if (markerSwap.assignments.HasUsableAssignment())
    {
        if (!markerSwap.assignments.IsActiveMeleeTank(bot, markerSwap.crossParity))
        {
            if (markerSwap.assignments.HasFullPairs() && markerSwap.assignments.IsMeleeTank(bot))
            {
                TwinMarkerAssignment const assignedMarker = GetAssignedMarkerForBot(markerSwap, bot);
                if (veknilash->GetVictim() == bot)
                {
                    LogTwinTankRole(bot, "standby_melee_holding_skull", assignedMarker, veknilash);
                    return false;
                }

                LogTwinTankRole(bot, "standby_melee_tank", assignedMarker, veklor);
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

            return false;
        }

        LogTwinTankRole(bot,
            markerSwap.assignments.HasSinglePair() ? "active_fallback_melee_tank" : "active_skull_tank",
            GetAssignedMarkerForBot(markerSwap, bot), veknilash);
    }
    else if (!IsFallbackVeknilashTank(bot, veknilash))
        return false;

    Aq40Helpers::SetRtiTarget(botAI, "skull", veknilash);

    bool attacked = false;
    if (bot->GetTarget() != veknilash->GetGUID() || AI_VALUE(Unit*, "current target") != veknilash)
    {
        Aq40Helpers::LogAq40Target(bot, "twin", "tank_veknilash", veknilash, 1000);
        attacked = Attack(veknilash);
    }

    bool const shiftedForm = EnsureTwinMeleeTankForm(bot, botAI);
    Unit* currentVictim = veknilash->GetVictim();
    bool const hasControl = currentVictim == bot;

    bool castTaunt = false;
    if (!hasControl)
    {
        if (bot->getClass() == CLASS_DRUID)
            castTaunt = CastFirstAvailable(botAI, veknilash, { "growl", "taunt" });
        else
            castTaunt = CastFirstAvailable(botAI, veknilash, { "hand of reckoning", "dark command", "taunt", "growl" });
    }

    bool const castThreat = CastTwinMeleeThreat(botAI, veknilash);
    bool moved = false;

    if (!hasControl)
    {
        TwinTankAnchorMove controlMove = BuildTwinTankMoveSnapshot(bot, veknilash, veklor);
        if (controlMove.tankRange > kTwinMeleeContactRange)
            moved = MoveNear(veknilash, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);

        LogTwinTankMovement(bot, "aggro_recover", markerSwap, veknilash, veklor, controlMove, moved,
                            currentVictim, false);
        return moved || castTaunt || castThreat || shiftedForm || attacked;
    }

    float const tankRange = bot->GetDistance2d(veknilash);
    if (tankRange > kTwinMeleeControlMaxRange)
    {
        TwinTankAnchorMove controlMove = BuildTwinTankMoveSnapshot(bot, veknilash, veklor);
        moved = MoveNear(veknilash, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);
        LogTwinTankMovement(bot, "contact_recover", markerSwap, veknilash, veklor, controlMove, moved,
                            currentVictim, true);

        return moved || castThreat || shiftedForm || attacked;
    }

    TwinTankAnchorMove anchorMove;
    if (BuildTwinMeleeContactAnchor(bot, veknilash, veklor, anchorMove))
    {
        if (anchorMove.anchorDistance > 2.0f)
        {
            moved = MoveNear(bot->GetMapId(), anchorMove.anchor.GetPositionX(), anchorMove.anchor.GetPositionY(),
                             anchorMove.anchor.GetPositionZ(), 0.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        LogTwinTankMovement(bot, "melee_separate", markerSwap, veknilash, veklor, anchorMove, moved,
                            currentVictim, true);
    }
    else if (bot->GetDistance2d(veknilash) > kTwinMeleeContactRange)
        moved = MoveNear(veknilash, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);

    if (moved || castTaunt || castThreat || shiftedForm || attacked)
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
    if (markerSwap.assignments.HasUsableAssignment())
    {
        if (!markerSwap.assignments.IsActiveWarlockTank(bot, markerSwap.crossParity))
        {
            if (markerSwap.assignments.HasFullPairs() && markerSwap.assignments.IsWarlockTank(bot))
            {
                TwinMarkerAssignment const assignedMarker = GetAssignedMarkerForBot(markerSwap, bot);
                LogTwinTankRole(bot, "standby_warlock_tank", assignedMarker, veknilash);
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

            return false;
        }

        LogTwinTankRole(bot,
            markerSwap.assignments.HasSinglePair() ? "active_fallback_warlock" : "active_cross_warlock",
            GetAssignedMarkerForBot(markerSwap, bot), veklor);
    }
    else if (!IsFallbackVeklorWarlock(bot, botAI, veklor))
        return false;

    Aq40Helpers::SetRtiTarget(botAI, "cross", veklor);

    bool attacked = false;
    if (bot->GetTarget() != veklor->GetGUID() || AI_VALUE(Unit*, "current target") != veklor)
    {
        Aq40Helpers::LogAq40Target(bot, "twin", "warlock_veklor", veklor, 1000);
        attacked = Attack(veklor);
    }

    bool castWard = false;
    if (!botAI->HasAura("shadow ward", bot))
        castWard = CastFirstAvailableSelf(botAI, bot, { "shadow ward" });

    bool const castThreat = CastTwinWarlockThreat(bot, botAI, veklor);
    bool moved = false;

    TwinTankAnchorMove anchorMove;
    if (BuildTwinFarSideAnchor(bot, veklor, veknilash, kTwinWarlockPreferredRange, anchorMove))
    {
        if (anchorMove.anchorDistance > 2.0f)
        {
            moved = MoveNear(bot->GetMapId(), anchorMove.anchor.GetPositionX(), anchorMove.anchor.GetPositionY(),
                             anchorMove.anchor.GetPositionZ(), 0.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        LogTwinTankMovement(bot, "warlock_separate", markerSwap, veklor, veknilash, anchorMove, moved,
                            veklor ? veklor->GetVictim() : nullptr, veklor && veklor->GetVictim() == bot);
    }
    else
    {
        float const distance = bot->GetDistance2d(veklor);
        if (distance < kTwinWarlockMinRange)
            moved = MoveAway(veklor, kTwinWarlockMinRange - distance + 1.0f);
        else if (distance > kTwinWarlockMaxRange)
            moved = MoveNear(veklor, kTwinWarlockMaxRange, MovementPriority::MOVEMENT_COMBAT);
    }

    if (moved || castThreat || castWard || attacked)
        return true;

    return bot->GetVictim() != veklor ? Attack(veklor) : false;
}

bool Aq40TwinAvoidHazardAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    TwinMarkerSwapState const markerSwap = RefreshTwinMarkerSwapState(bot);
    bool const activeMeleeTank = IsActiveTwinMeleeTank(bot, markerSwap, veknilash);
    auto moveActiveMeleeTankFromHazard = [this, veknilash](char const* hazard, Position const& hazardPosition)
    {
        TwinTankAnchorMove move;
        if (!BuildTwinTankHazardContactAnchor(bot, veknilash, hazardPosition, move))
            return false;

        bool moved = false;
        if (move.anchorDistance > 1.0f)
        {
            moved = MoveNear(bot->GetMapId(), move.anchor.GetPositionX(), move.anchor.GetPositionY(),
                             move.anchor.GetPositionZ(), 0.0f, MovementPriority::MOVEMENT_COMBAT);
        }

        LogTwinActiveTankHazard(bot, hazard, veknilash, move, moved);
        return moved;
    };

    Unit* explodeBug = Aq40BossHelper::Twin::FindNearestBug(
        bot, botAI, encounterUnits, kTwinExplodeBugDangerRadius, true);
    Position explodePosition;
    if (!explodeBug && HasTrackedExplodeBugHazard(bot, botAI, explodeBug, explodePosition))
    {
        if (activeMeleeTank && moveActiveMeleeTankFromHazard("explode_bug", explodePosition))
            return true;

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:explode_bug:tracked",
            "boss=twin hazard=explode_bug source=tracked_script_source", 1000);
        return FleePosition(explodePosition, kTwinExplodeBugDangerRadius, 250U);
    }

    if (explodeBug)
    {
        if (activeMeleeTank && moveActiveMeleeTankFromHazard("explode_bug", explodeBug->GetPosition()))
            return true;

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
        Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
        if (activeMeleeTank)
        {
            if (veklor && bot->GetDistance2d(veklor) <= kTwinArcaneBurstAvoidRadius)
            {
                if (moveActiveMeleeTankFromHazard("arcane_burst", veklor->GetPosition()))
                    return true;

                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard:active_tank_veklor_fallback",
                    "boss=twin hazard=blizzard action=active_tank_move_from_veklor", 1000);
                return MoveAway(veklor, kTwinArcaneBurstAvoidRadius - bot->GetDistance2d(veklor) + 2.0f);
            }

            Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard:active_tank_hold",
                "boss=twin hazard=blizzard action=active_tank_hold_control", 1000);
            return false;
        }

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        if (botAI->DoSpecificAction("avoid aoe", Event(), true))
        {
            Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard",
                "boss=twin hazard=blizzard action=avoid_aoe", 1000);
            return true;
        }

        if (veklor)
        {
            float const distance = bot->GetDistance2d(veklor);
            if (distance <= kTwinArcaneBurstAvoidRadius)
            {
                Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard:veklor_fallback",
                    "boss=twin hazard=blizzard action=move_from_veklor", 1000);
                return MoveAway(veklor, kTwinArcaneBurstAvoidRadius - distance + 2.0f);
            }
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
    if (markerSwap.assignments.HasUsableAssignment())
    {
        if (markerSwap.assignments.IsActiveWarlockTank(bot, markerSwap.crossParity))
            return false;
    }
    else if (IsFallbackVeklorWarlock(bot, botAI, veklor))
        return false;

    StopPetFromVeklor(bot, veklor);

    float const distance = bot->GetDistance2d(veklor);
    float const safeRadius = kTwinArcaneBurstAvoidRadius;
    if (distance > safeRadius)
        return false;

    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    if (IsActiveTwinMeleeTank(bot, markerSwap, veknilash))
    {
        TwinTankAnchorMove move;
        if (BuildTwinTankHazardContactAnchor(bot, veknilash, veklor->GetPosition(), move))
        {
            bool moved = false;
            if (move.anchorDistance > 1.0f)
            {
                moved = MoveNear(bot->GetMapId(), move.anchor.GetPositionX(), move.anchor.GetPositionY(),
                                 move.anchor.GetPositionZ(), 0.0f, MovementPriority::MOVEMENT_COMBAT);
            }

            LogTwinActiveTankHazard(bot, "arcane_burst", veknilash, move, moved);
            if (moved)
                return true;
        }
    }

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "twin:veklor_range:" + Aq40Helpers::GetAq40LogUnit(veklor),
        "boss=twin hazard=arcane_burst source=" + Aq40Helpers::GetAq40LogUnit(veklor), 1000);
    return FleePosition(veklor->GetPosition(), safeRadius + 2.0f, 250U) ||
           MoveAway(veklor, safeRadius - distance + 2.0f);
}
