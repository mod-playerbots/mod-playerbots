#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include "ObjectAccessor.h"
#include "Map.h"
#include "Pet.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"
#include "../RaidAq40BossHelper.h"
#include "../RaidAq40SpellIds.h"
#include "RaidAq40Helpers_Shared.h"
#include "RaidAq40TwinEncounter.h"

namespace
{
float constexpr kTwinRoomBotRadius = 180.0f;
float constexpr kTwinRoomExtendedBotRadius = 220.0f;
float constexpr kTwinExplodeBugInterruptRadius = 15.0f;
float constexpr kTwinReserveAuditAnchorTolerance = 4.0f;
uint32 constexpr kTwinTeleportThreatHoldMs = 8000;
uint32 constexpr kTwinPickupAnchorDurationMs = 6000;
uint32 constexpr kTwinTeleportDebounceMs = 1000;

size_t ToSideIndex(Aq40TwinEncounter::TwinSide side)
{
	return side == Aq40TwinEncounter::TwinSide::Side1 ? 1u : 0u;
}

bool HasActiveAq40CombatStrategy(PlayerbotAI* botAI)
{
	return botAI && botAI->HasStrategy("aq40", BOT_STATE_COMBAT);
}

bool TryGetTwinBoss(Unit const* unit, Aq40TwinEncounter::TwinBoss& outBoss)
{
	if (!unit)
		return false;

	switch (unit->GetEntry())
	{
		case Aq40SpellIds::TwinVeklorNpcEntry:
			outBoss = Aq40TwinEncounter::TwinBoss::Veklor;
			return true;
		case Aq40SpellIds::TwinVeknilashNpcEntry:
			outBoss = Aq40TwinEncounter::TwinBoss::Veknilash;
			return true;
		default:
			return false;
	}
}

bool IsTwinRelevantCaster(Unit* caster)
{
	return caster && caster->GetMap() && caster->GetMapId() == Aq40BossHelper::MAP_ID &&
		   Aq40SpellIds::IsTwinEncounterNpcEntry(caster->GetEntry());
}

float GetDistance2d(float leftX, float leftY, float rightX, float rightY)
{
	float const dx = leftX - rightX;
	float const dy = leftY - rightY;
	return std::sqrt(dx * dx + dy * dy);
}

Aq40TwinEncounter::TwinSide GetTwinSideForPosition(float x, float y)
{
	Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
	float const side0Distance = GetDistance2d(x, y,
		geometry.bossPark[0].position.GetPositionX(), geometry.bossPark[0].position.GetPositionY());
	float const side1Distance = GetDistance2d(x, y,
		geometry.bossPark[1].position.GetPositionX(), geometry.bossPark[1].position.GetPositionY());
	return side0Distance <= side1Distance ? Aq40TwinEncounter::TwinSide::Side0 : Aq40TwinEncounter::TwinSide::Side1;
}

bool IsTwinRelevantBot(Player* player, Unit* source)
{
	if (!player || !source || !player->IsAlive() || !player->IsInWorld() || !player->GetMap())
		return false;

	PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
	if (!HasActiveAq40CombatStrategy(botAI))
		return false;

	if (!Aq40BossHelper::IsInAq40(player) || !source->GetMap() ||
		player->GetMap()->GetInstanceId() != source->GetMap()->GetInstanceId())
	{
		return false;
	}

	Position const& center = Aq40TwinEncounter::GetGeometry().roomCenter.position;
	if (player->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) <= kTwinRoomBotRadius)
		return true;

	if (source->GetDistance2d(player) <= kTwinRoomBotRadius)
		return true;

	return Aq40TwinEncounter::GetLockedPickupAnchor(player) &&
		   player->GetExactDist2d(center.GetPositionX(), center.GetPositionY()) <= kTwinRoomExtendedBotRadius;
}

std::vector<Player*> CollectTwinBots(Unit* source)
{
	std::vector<Player*> bots;
	if (!source || !source->GetMap())
		return bots;

	Map::PlayerList const& players = source->GetMap()->GetPlayers();
	for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
	{
		Player* player = itr->GetSource();
		if (IsTwinRelevantBot(player, source))
			bots.push_back(player);
	}

	return bots;
}

Player* FindTwinInstanceMember(Player* contextBot, ObjectGuid guid)
{
	if (!contextBot || guid.IsEmpty() || !contextBot->GetMap())
		return nullptr;

	Map::PlayerList const& players = contextBot->GetMap()->GetPlayers();
	for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
	{
		Player* member = itr->GetSource();
		if (member && member->GetGUID() == guid)
			return member;
	}

	return nullptr;
}

Unit* GetTwinCurrentTarget(PlayerbotAI* botAI)
{
	if (!botAI || !botAI->GetAiObjectContext())
		return nullptr;

	return botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
}

Unit* GetTwinSelectionTarget(Player* bot, PlayerbotAI* botAI)
{
	if (!bot || !botAI || bot->GetTarget().IsEmpty())
		return nullptr;

	return botAI->GetUnit(bot->GetTarget());
}

Unit* GetTwinPetTarget(Player* bot)
{
	if (!bot)
		return nullptr;

	Pet* pet = bot->GetPet();
	return pet ? pet->GetVictim() : nullptr;
}

Unit* GetTwinObservedTarget(Player* bot, PlayerbotAI* botAI)
{
	if (!bot || !botAI)
		return nullptr;

	if (Unit* currentTarget = GetTwinCurrentTarget(botAI))
		return currentTarget;

	if (Unit* victim = bot->GetVictim())
		return victim;

	return GetTwinSelectionTarget(bot, botAI);
}

bool IsTwinVeklorTarget(Unit const* target)
{
	return target && target->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry;
}

bool IsTwinVeknilashTarget(Unit const* target)
{
	return target && target->GetEntry() == Aq40SpellIds::TwinVeknilashNpcEntry;
}

uint32 GetTwinInitialEngagementElapsedMs(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs)
{
	if (state.modeEnteredAtMs == 0)
		return 0;

	uint32 const now = nowMs ? nowMs : getMSTime();
	return getMSTimeDiff(state.modeEnteredAtMs, now);
}

uint32 GetTwinTeleportElapsedMs(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs)
{
	if (state.lastTeleportAtMs == 0)
		return 0;

	uint32 const now = nowMs ? nowMs : getMSTime();
	return getMSTimeDiff(state.lastTeleportAtMs, now);
}

uint32 GetTwinSwapPrepElapsedMs(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs)
{
	if (state.swapPrepStartAtMs == 0)
		return 0;

	uint32 const now = nowMs ? nowMs : getMSTime();
	return getMSTimeDiff(state.swapPrepStartAtMs, now);
}

void AppendTwinWarlockPoolTelemetryFields(std::ostringstream& fields,
										  Aq40TwinEncounter::TwinEncounterState const& state)
{
	fields << " warlock_pool=full_instance"
		   << " eligible_warlocks=" << static_cast<uint32>(state.eligibleWarlockCount)
		   << " approach_warlocks=" << static_cast<uint32>(state.approachWarlockCount);
}

ObjectGuid GetTwinTelemetryControllerGuid(Aq40TwinEncounter::TwinEncounterState const& state,
									  Aq40TwinEncounter::TwinBoss boss)
{
	Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
	if (!ownership.stableOwner.IsEmpty())
		return ownership.stableOwner;
	if (!ownership.candidateOwner.IsEmpty())
		return ownership.candidateOwner;

	return ownership.expectedOwner;
}

bool IsTwinPostTeleportPickupTelemetryWindow(Aq40TwinEncounter::TwinEncounterState const& state, uint32 nowMs)
{
	return state.lastTeleportAtMs &&
		   (state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
			state.phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
			Aq40TwinEncounter::IsAnyThreatHoldWindowActive(state, nowMs));
}

std::string BuildTwinFirstTeleportIncomingAssignmentStatus(
	Aq40TwinEncounter::TwinBoss boss, Aq40TwinEncounter::TwinRoleAssignment const* assignment)
{
	if (!assignment)
		return "missing_assignment";

	Aq40TwinEncounter::TwinRoleCohort const expectedCohort =
		boss == Aq40TwinEncounter::TwinBoss::Veklor ? Aq40TwinEncounter::TwinRoleCohort::WarlockTank
												 : Aq40TwinEncounter::TwinRoleCohort::MeleeTank;
	Aq40TwinEncounter::TwinSide const expectedSide =
		Aq40TwinEncounter::GetInitialSideForBoss(Aq40TwinEncounter::GetOtherBoss(boss));

	bool const wrongCohort = assignment->cohort != expectedCohort;
	bool const wrongSide = assignment->stableSide != expectedSide;

	if (wrongCohort && wrongSide)
		return "wrong_cohort_side";
	if (wrongCohort)
		return "wrong_cohort";
	if (wrongSide)
		return "wrong_side";

	return "ok";
}

bool GetTwinFirstTeleportIncomingAnchor(Aq40TwinEncounter::TwinBoss boss,
										Aq40TwinEncounter::TwinRoleAssignment const& assignment,
										Aq40TwinEncounter::TwinAnchor& outAnchor, char const*& outLabel)
{
	if (!Aq40TwinEncounter::IsKnownSide(assignment.stableSide))
		return false;

	Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
	size_t const sideIndex = ToSideIndex(assignment.stableSide);
	if (boss == Aq40TwinEncounter::TwinBoss::Veklor)
	{
		outAnchor = geometry.stableVeklorWarlock[sideIndex];
		outLabel = "stable_veklor_warlock";
		return true;
	}

	outAnchor = geometry.reserveMeleeProxy[sideIndex];
	outLabel = "reserve_melee_proxy";
	return true;
}

std::string GetTwinFirstTeleportIncomingHoldState(Aq40TwinEncounter::TwinEncounterState const& state,
												  Aq40TwinEncounter::TwinBoss boss, Player* owner, Unit* target,
												  float anchorError)
{
	if (!owner)
		return "missing";

	if (Aq40TwinEncounter::HasLockedPickupAnchor(owner, boss))
		return "locked_pickup";

	if (Aq40TwinEncounter::IsPrimaryController(state, boss, owner->GetGUID()))
		return "controller";

	if (IsTwinVeklorTarget(target))
		return "targeting_veklor";

	if (IsTwinVeknilashTarget(target))
		return "targeting_veknilash";

	if (target && Aq40SpellIds::IsTwinBugEntry(target->GetEntry()))
		return "targeting_bug";

	return anchorError <= kTwinReserveAuditAnchorTolerance ? "parked" : "off_anchor";
}

void AppendTwinFirstTeleportIncomingOwnerAudit(std::ostringstream& fields, Player* logBot,
											   Aq40TwinEncounter::TwinEncounterState const& state,
											   Aq40TwinEncounter::TwinBoss boss, char const* prefix)
{
	Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
	if (ownership.expectedOwner.IsEmpty())
	{
		fields << " " << prefix << "_incoming_member=none"
			   << " " << prefix << "_incoming_assignment_status=none"
			   << " " << prefix << "_incoming_cohort=none"
			   << " " << prefix << "_incoming_side=unknown"
			   << " " << prefix << "_incoming_slot=0"
			   << " " << prefix << "_incoming_anchor=none"
			   << " " << prefix << "_incoming_anchor_error=0"
			   << " " << prefix << "_incoming_hold=none"
			   << " " << prefix << "_incoming_target=none"
			   << " " << prefix << "_incoming_pet_target=none";
		return;
	}

	Player* incomingOwner = FindTwinInstanceMember(logBot, ownership.expectedOwner);
	Aq40TwinEncounter::TwinRoleAssignment const* assignment =
		Aq40TwinEncounter::GetAssignmentForMember(state, ownership.expectedOwner);
	std::string assignmentStatus = incomingOwner ? BuildTwinFirstTeleportIncomingAssignmentStatus(boss, assignment)
												 : std::string("missing_member");

	fields << " " << prefix << "_incoming_member=" << Aq40Helpers::GetAq40LogUnit(incomingOwner)
		   << " " << prefix << "_incoming_assignment_status=" << assignmentStatus;

	if (!incomingOwner || !assignment)
	{
		fields << " " << prefix << "_incoming_cohort="
			   << (assignment ? Aq40TwinEncounter::ToString(assignment->cohort) : "none")
			   << " " << prefix << "_incoming_side="
			   << (assignment ? Aq40TwinEncounter::ToString(assignment->stableSide) : "unknown")
			   << " " << prefix << "_incoming_slot="
			   << (assignment ? static_cast<uint32>(assignment->slotIndex) : 0u)
			   << " " << prefix << "_incoming_anchor=none"
			   << " " << prefix << "_incoming_anchor_error=0"
			   << " " << prefix << "_incoming_hold="
			   << (incomingOwner ? "unassigned" : "missing")
			   << " " << prefix << "_incoming_target=none"
			   << " " << prefix << "_incoming_pet_target=none";
		return;
	}

	PlayerbotAI* incomingAI = GET_PLAYERBOT_AI(incomingOwner);
	Unit* const incomingTarget =
		incomingAI ? GetTwinObservedTarget(incomingOwner, incomingAI) : incomingOwner->GetVictim();
	Unit* const incomingPetTarget = GetTwinPetTarget(incomingOwner);
	Aq40TwinEncounter::TwinAnchor anchor;
	char const* anchorLabel = "unknown";
	float anchorError = 0.0f;
	if (GetTwinFirstTeleportIncomingAnchor(boss, *assignment, anchor, anchorLabel))
	{
		anchorError = incomingOwner->GetExactDist2d(
			anchor.position.GetPositionX(), anchor.position.GetPositionY());
	}

	fields << " " << prefix << "_incoming_cohort=" << Aq40TwinEncounter::ToString(assignment->cohort)
		   << " " << prefix << "_incoming_side=" << Aq40TwinEncounter::ToString(assignment->stableSide)
		   << " " << prefix << "_incoming_slot=" << static_cast<uint32>(assignment->slotIndex)
		   << " " << prefix << "_incoming_anchor=" << anchorLabel
		   << " " << prefix << "_incoming_anchor_error=" << anchorError
		   << " " << prefix << "_incoming_hold="
		   << GetTwinFirstTeleportIncomingHoldState(state, boss, incomingOwner, incomingTarget, anchorError)
		   << " " << prefix << "_incoming_target=" << Aq40Helpers::GetAq40LogUnit(incomingTarget)
		   << " " << prefix << "_incoming_pet_target=" << Aq40Helpers::GetAq40LogUnit(incomingPetTarget);
}

void LogTwinFirstTeleportReservePromotionAudit(Player* logBot,
											   Aq40TwinEncounter::TwinEncounterState const& preTeleportState,
											   Aq40TwinEncounter::TwinEncounterState const& postTeleportState,
											   Unit* source, uint32 spellId, uint32 nowMs)
{
	if (!logBot)
		return;

	std::ostringstream fields;
	fields << "boss=twin spell=" << spellId
		   << " source=" << Aq40Helpers::GetAq40LogUnit(source)
		   << " phase=" << Aq40TwinEncounter::ToString(postTeleportState.phase)
		   << " mode=" << Aq40TwinEncounter::ToString(postTeleportState.mode)
		   << " teleport_count=" << static_cast<uint32>(postTeleportState.teleportCount)
		   << " engagement_elapsed_ms=" << GetTwinInitialEngagementElapsedMs(postTeleportState, nowMs)
		   << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(postTeleportState, nowMs)
		   << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(postTeleportState, nowMs)
		   << " swap_prep_elapsed_ms=" << GetTwinSwapPrepElapsedMs(preTeleportState, nowMs)
		   << " swap_prep_armed=" << (preTeleportState.swapPrepArmedAtMs ? 1 : 0);

	for (Aq40TwinEncounter::TwinBoss boss : { Aq40TwinEncounter::TwinBoss::Veklor,
											  Aq40TwinEncounter::TwinBoss::Veknilash })
	{
		char const* prefix = boss == Aq40TwinEncounter::TwinBoss::Veklor ? "veklor" : "veknilash";
		Aq40TwinEncounter::TwinStableOwnership const& preOwnership =
			Aq40TwinEncounter::GetOwnership(preTeleportState, boss);
		Aq40TwinEncounter::TwinStableOwnership const& postOwnership =
			Aq40TwinEncounter::GetOwnership(postTeleportState, boss);

		fields << " " << prefix << "_promotion_expected="
			   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, preOwnership.expectedOwner))
			   << " " << prefix << "_promotion_outgoing="
			   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, preOwnership.reserveOwner))
			   << " " << prefix << "_promotion_candidate="
			   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, postOwnership.candidateOwner))
			   << " " << prefix << "_promotion_matches_expected="
			   << (!preOwnership.expectedOwner.IsEmpty() && postOwnership.candidateOwner == preOwnership.expectedOwner);
		AppendTwinFirstTeleportIncomingOwnerAudit(fields, logBot, preTeleportState, boss, prefix);
	}

	Aq40Helpers::LogAq40Info(logBot, "twin_validation",
		"twin:first_swap:reserve_promotion_audit", fields.str(), 1000);
}

void AppendTwinInitialEngagementBossFields(std::ostringstream& fields, Player* logBot,
										   Aq40TwinEncounter::TwinEncounterState const& state,
										   Aq40TwinEncounter::TwinBoss boss, char const* prefix, uint32 nowMs)
{
	Aq40TwinEncounter::TwinStableOwnership const& ownership = Aq40TwinEncounter::GetOwnership(state, boss);
	fields << " " << prefix << "_expected_owner="
		   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, ownership.expectedOwner))
		   << " " << prefix << "_reserve_owner="
		   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, ownership.reserveOwner))
		   << " " << prefix << "_controller="
		   << Aq40Helpers::GetAq40LogUnit(
				  FindTwinInstanceMember(logBot, GetTwinTelemetryControllerGuid(state, boss)))
		   << " " << prefix << "_candidate_owner="
		   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, ownership.candidateOwner))
		   << " " << prefix << "_stable_owner="
		   << Aq40Helpers::GetAq40LogUnit(FindTwinInstanceMember(logBot, ownership.stableOwner))
		   << " " << prefix << "_pickup_owner="
		   << Aq40Helpers::GetAq40LogUnit(
				  FindTwinInstanceMember(logBot, Aq40TwinEncounter::GetPickupOwner(state, boss)))
		   << " " << prefix << "_pickup=" << (Aq40TwinEncounter::IsPickupEstablished(state, boss) ? 1 : 0)
		   << " " << prefix << "_confirm_age_ms="
		   << Aq40TwinEncounter::GetTimeSinceOwnershipConfirmationMs(state, boss, nowMs)
		   << " " << prefix << "_pickup_age_ms="
		   << Aq40TwinEncounter::GetPickupEstablishedAgeMs(state, boss, nowMs)
		   << " " << prefix << "_threat_hold_remaining_ms="
		   << Aq40TwinEncounter::GetThreatHoldRemainingMs(state, boss, nowMs);
}

void LogTwinInitialEngagementPickupStatus(Player* logBot, Aq40TwinEncounter::TwinEncounterState const& state,
										  Aq40TwinEncounter::TwinBoss confirmedBoss, Player* confirmedOwner,
										  Unit* source, uint32 nowMs)
{
	if (!logBot)
		return;

	bool const veklorPickup = Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor);
	bool const veknilashPickup = Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veknilash);
	bool const dualPickupEstablished = veklorPickup && veknilashPickup;
	bool const postTeleportWindow = IsTwinPostTeleportPickupTelemetryWindow(state, nowMs);
	bool const firstTeleportWindow = postTeleportWindow && state.teleportCount == 1;
	uint32 const teleportElapsedMs = GetTwinTeleportElapsedMs(state, nowMs);
	uint32 const swapPrepElapsedMs = GetTwinSwapPrepElapsedMs(state, nowMs);
	bool const singleSidePendingOverrun = postTeleportWindow && !dualPickupEstablished && teleportElapsedMs > 1000;
	bool const veklorPickupEstablishedPostTeleport =
		postTeleportWindow && confirmedBoss == Aq40TwinEncounter::TwinBoss::Veklor &&
		Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor);
	Aq40TwinEncounter::TwinStableOwnership const& confirmedOwnership =
		Aq40TwinEncounter::GetOwnership(state, confirmedBoss);
	bool const confirmedMatchesExpected =
		confirmedOwner && confirmedOwnership.expectedOwner == confirmedOwner->GetGUID();
	bool const confirmedMatchesCandidate =
		confirmedOwner && confirmedOwnership.candidateOwner == confirmedOwner->GetGUID();

	std::ostringstream fields;
	fields << "boss=twin phase=" << Aq40TwinEncounter::ToString(state.phase)
		   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
		   << " teleport_count=" << static_cast<uint32>(state.teleportCount)
		   << " engagement_elapsed_ms=" << GetTwinInitialEngagementElapsedMs(state, nowMs)
		   << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state, nowMs)
		   << " teleport_elapsed_ms=" << teleportElapsedMs
		   << " swap_prep_elapsed_ms=" << swapPrepElapsedMs
		   << " confirmed_boss=" << Aq40TwinEncounter::ToString(confirmedBoss)
		   << " confirmed_owner=" << Aq40Helpers::GetAq40LogUnit(confirmedOwner)
		   << " source=" << Aq40Helpers::GetAq40LogUnit(source)
		   << " pickup_status="
		   << (dualPickupEstablished ? "dual_pickup_established"
				 : (singleSidePendingOverrun ? "single_side_pending_overrun" : "single_side_pending"))
		   << " post_teleport_window=" << (postTeleportWindow ? 1 : 0)
		   << " first_teleport_window=" << (firstTeleportWindow ? 1 : 0)
		   << " confirmed_matches_expected=" << (confirmedMatchesExpected ? 1 : 0)
		   << " confirmed_matches_candidate=" << (confirmedMatchesCandidate ? 1 : 0)
		   << " approach=" << state.approachMemberCount
		   << " staged=" << state.stagedMemberCount
		   << " center_committed=" << state.centerCommittedMemberCount
		   << " strict_ready=" << state.strictReadyMemberCount
		   << " assigned=" << state.assignments.size();
	AppendTwinInitialEngagementBossFields(fields, logBot, state, Aq40TwinEncounter::TwinBoss::Veklor, "veklor",
		nowMs);
	AppendTwinInitialEngagementBossFields(fields, logBot, state, Aq40TwinEncounter::TwinBoss::Veknilash,
		"veknilash", nowMs);

	std::string stateKey;
	if (veklorPickupEstablishedPostTeleport)
	{
		stateKey = "twin:post_swap:veklor_pickup_established";
	}
	else if (dualPickupEstablished)
	{
		stateKey = postTeleportWindow ? std::string("twin:post_swap:dual_pickup_established")
								 : std::string("twin:initial_engagement:dual_pickup_established");
	}
	else if (singleSidePendingOverrun)
	{
		stateKey = std::string("twin:post_swap:single_side_pending_overrun:") +
			Aq40TwinEncounter::ToString(confirmedBoss);
	}
	else if (postTeleportWindow)
	{
		stateKey = std::string("twin:post_swap:single_side_pending:") +
			Aq40TwinEncounter::ToString(confirmedBoss);
	}
	else
	{
		stateKey = std::string("twin:initial_engagement:single_side_pending:") +
			Aq40TwinEncounter::ToString(confirmedBoss);
	}

	if (singleSidePendingOverrun || (firstTeleportWindow && !confirmedMatchesExpected))
		Aq40Helpers::LogAq40Warn(logBot, "twin_validation", stateKey, fields.str(), 1000);
	else
		Aq40Helpers::LogAq40Info(logBot, "twin_validation", stateKey, fields.str(), 1000);
}

bool UpdateHazardTimestamp(uint32& hazardAtMs, uint32 nowMs, uint32 debounceMs = 0)
{
	if (hazardAtMs && debounceMs > 0 && getMSTimeDiff(hazardAtMs, nowMs) < debounceMs)
		return false;

	hazardAtMs = nowMs;
	return true;
}

Player* ResolvePrimarySpellTargetPlayer(Spell* spell, Unit* caster)
{
	if (!spell || !caster)
		return nullptr;

	std::list<TargetInfo> const* targets = spell->GetUniqueTargetInfo();
	if (!targets)
		return nullptr;

	for (TargetInfo const& targetInfo : *targets)
	{
		Unit* target = ObjectAccessor::GetUnit(*caster, targetInfo.targetGUID);
		if (!target)
			continue;

		if (Player* playerTarget = target->ToPlayer())
			return playerTarget;
	}

	return nullptr;
}

Player* ResolveBossOwnerForSpell(Spell* spell, Unit* caster, uint32 spellId)
{
	if (!caster)
		return nullptr;

	if (spellId == Aq40SpellIds::TwinShadowBolt || spellId == Aq40SpellIds::TwinUppercut ||
		spellId == Aq40SpellIds::TwinUnbalancingStrike)
	{
		if (Player* explicitTarget = ResolvePrimarySpellTargetPlayer(spell, caster))
			return explicitTarget;
	}

	if (Unit* victim = caster->GetVictim())
	{
		if (Player* victimPlayer = victim->ToPlayer())
			return victimPlayer;
	}

	if (ObjectGuid const targetGuid = caster->GetTarget())
	{
		if (Unit* target = ObjectAccessor::GetUnit(*caster, targetGuid))
		{
			if (Player* targetPlayer = target->ToPlayer())
				return targetPlayer;
		}
	}

	return ResolvePrimarySpellTargetPlayer(spell, caster);
}

bool MarkEncounterCombat(Aq40TwinEncounter::TwinEncounterState& state, Player* logBot, Unit* caster, uint32 spellId,
						 uint32 nowMs)
{
	if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::PrePull)
	{
		Aq40TwinEncounter::SetMode(state, Aq40TwinEncounter::TwinStrategyMode::Combat, nowMs);
		return true;
	}

	bool const hasDeterministicAssignments = Aq40TwinEncounter::HasDeterministicAssignments(state);
	if (state.mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady && hasDeterministicAssignments)
	{
		Aq40TwinEncounter::SetMode(state, Aq40TwinEncounter::TwinStrategyMode::Combat, nowMs);
		Aq40TwinEncounter::EnterDualPullWindow(state, nowMs);
		return true;
	}

	size_t committedCount = 0u;
	std::string missingCriticalRoles;
	bool const quorumSatisfied = hasDeterministicAssignments &&
		Aq40TwinEncounter::IsTwinCenterCommitQuorumMet(logBot, state, &committedCount, &missingCriticalRoles);
	size_t const quorumRequired = Aq40TwinEncounter::GetTwinPrePullQuorumRequirement(state.assignments.size());

	if (quorumSatisfied)
	{
		if (logBot)
		{
			std::ostringstream fields;
			fields << "boss=twin spell=" << spellId
				   << " source=" << Aq40Helpers::GetAq40LogUnit(caster)
				   << " phase=" << Aq40TwinEncounter::ToString(state.phase)
				   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
				   << " reason=center_commit_quorum_met"
				   << " approach=" << state.approachMemberCount
				   << " staged=" << state.stagedMemberCount
				   << " center_committed=" << committedCount
				   << " strict_ready=" << state.strictReadyMemberCount
				   << " assigned=" << state.assignments.size()
				   << " quorum_required=" << quorumRequired
				   << " missing_critical_roles="
				   << (missingCriticalRoles.empty() ? "none" : missingCriticalRoles);
			AppendTwinWarlockPoolTelemetryFields(fields, state);
			Aq40Helpers::LogAq40Warn(logBot, "twin_script_window", "twin:early_pull_salvage", fields.str(), 1000);
		}

		Aq40TwinEncounter::SetMode(state, Aq40TwinEncounter::TwinStrategyMode::Combat, nowMs);
		Aq40TwinEncounter::EnterDualPullWindow(state, nowMs);
		return true;
	}

	if (logBot)
	{
		std::ostringstream fields;
		fields << "boss=twin spell=" << spellId
			   << " source=" << Aq40Helpers::GetAq40LogUnit(caster)
			   << " phase=" << Aq40TwinEncounter::ToString(state.phase)
			   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
			   << " reason="
			   << (hasDeterministicAssignments
					   ? (missingCriticalRoles.empty() ? "center_commit_quorum_unmet" : "missing_critical_roles")
					   : "no_deterministic_assignments")
			   << " approach=" << state.approachMemberCount
			   << " staged=" << state.stagedMemberCount
			   << " center_committed=" << committedCount
			   << " strict_ready=" << state.strictReadyMemberCount
			   << " assigned=" << state.assignments.size()
			   << " quorum_required=" << quorumRequired
			   << " missing_critical_roles="
			   << (missingCriticalRoles.empty() ? "none" : missingCriticalRoles);
		AppendTwinWarlockPoolTelemetryFields(fields, state);
		Aq40Helpers::LogAq40Warn(logBot, "twin_script_window", "twin:early_pull_invalid", fields.str(), 1000);
	}

	return false;
}

void MaybeLockPickupAnchor(Aq40TwinEncounter::TwinEncounterState const& state, Player* owner,
						   Aq40TwinEncounter::TwinBoss boss, Aq40TwinEncounter::TwinSide side, uint32 nowMs)
{
	if (!owner || !GET_PLAYERBOT_AI(owner) || !Aq40TwinEncounter::IsKnownSide(side))
		return;
	if (state.phase != Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow &&
		state.phase != Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery &&
		!Aq40TwinEncounter::IsThreatHoldWindowActive(state, boss, nowMs))
	{
		return;
	}

	Aq40TwinEncounter::TwinEncounterGeometry const& geometry = Aq40TwinEncounter::GetGeometry();
	Aq40TwinEncounter::TwinAnchor const& anchor =
		boss == Aq40TwinEncounter::TwinBoss::Veklor
			? geometry.stableVeklorWarlock[ToSideIndex(side)]
			: geometry.bossPark[ToSideIndex(side)];

	uint32 const durationMs = std::max(
		Aq40TwinEncounter::GetThreatHoldRemainingMs(state, boss, nowMs), kTwinPickupAnchorDurationMs);
	Aq40TwinEncounter::SetLockedPickupAnchor(owner, boss, side, anchor, durationMs, nowMs);
}

void PromoteStablePhaseIfReady(Aq40TwinEncounter::TwinEncounterState& state, uint32 nowMs)
{
	if (!Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor) ||
		!Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veknilash))
	{
		return;
	}

	Aq40TwinEncounter::SetSplitBand(state, Aq40TwinEncounter::TwinSplitBand::Stable, nowMs);
	Aq40TwinEncounter::EnterStablePhase(state, nowMs);
}

void ConfirmBossOwner(Aq40TwinEncounter::TwinEncounterState& state, Spell* spell, Unit* caster, uint32 spellId,
					  Aq40TwinEncounter::TwinBoss boss, uint32 nowMs, Player* logBot)
{
	if (Aq40TwinEncounter::IsTerminalPhase(state.phase))
		return;

	Player* owner = ResolveBossOwnerForSpell(spell, caster, spellId);
	if (!owner)
		return;

	ObjectGuid const ownerGuid = owner->GetGUID();
	bool changed = false;
	changed |= Aq40TwinEncounter::ConfirmOwner(state, boss, ownerGuid, nowMs);
	changed |= Aq40TwinEncounter::SetStableOwner(state, boss, ownerGuid, nowMs);
	changed |= Aq40TwinEncounter::MarkPickupEstablished(state, boss, ownerGuid, nowMs);

	if (state.phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow)
		Aq40TwinEncounter::EnterPickupRecovery(state, nowMs);

	MaybeLockPickupAnchor(state, owner, boss, GetTwinSideForPosition(caster->GetPositionX(), caster->GetPositionY()),
						  nowMs);
	PromoteStablePhaseIfReady(state, nowMs);

	if (!changed || !logBot)
		return;

	std::ostringstream fields;
	fields << "boss=twin twin_boss=" << Aq40TwinEncounter::ToString(boss)
		   << " owner=" << Aq40Helpers::GetAq40LogUnit(owner)
		   << " source=" << Aq40Helpers::GetAq40LogUnit(caster)
		   << " phase=" << Aq40TwinEncounter::ToString(state.phase);
	Aq40Helpers::LogAq40Info(logBot, "twin_pickup_confirm",
		std::string(Aq40TwinEncounter::ToString(boss)) + ":" + std::to_string(ownerGuid.GetCounter()) + ":" +
			Aq40TwinEncounter::ToString(state.phase),
		fields.str(), 1000);
	LogTwinInitialEngagementPickupStatus(logBot, state, boss, owner, caster, nowMs);
}

void RequestInterruptForTwinBots(std::vector<Player*> const& twinBots, Unit* source, Player* excludedBot = nullptr,
								 float maxDistance = 0.0f,
								 Aq40TwinEncounter::TwinEncounterState const* state = nullptr,
								 bool excludeEncounterTanks = false)
{
	for (Player* bot : twinBots)
	{
		if (!bot || bot == excludedBot)
			continue;
		if (maxDistance > 0.0f && (!source || bot->GetDistance2d(source) > maxDistance))
			continue;
		if (excludeEncounterTanks)
		{
			if (state)
			{
				ObjectGuid const botGuid = bot->GetGUID();
				if (Aq40TwinEncounter::IsPrimaryController(*state, Aq40TwinEncounter::TwinBoss::Veklor, botGuid) ||
					Aq40TwinEncounter::IsPrimaryController(*state, Aq40TwinEncounter::TwinBoss::Veknilash, botGuid))
				{
					continue;
				}
			}
			else if (Aq40BossHelper::IsEncounterTank(bot, bot))
			{
				continue;
			}
		}

		Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);
	}
}

bool ClearTwinTerminalFailureCombatState(Player* bot)
{
	if (!bot)
		return false;

	Aq40TwinEncounter::RequestImmediateMovementInterrupt(bot);

	bool changed = false;
	if (bot->GetVictim())
	{
		bot->AttackStop();
		changed = true;
	}

	if (bot->GetTarget())
	{
		bot->SetTarget();
		bot->SetSelection(ObjectGuid());
		changed = true;
	}

	PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
	if (!botAI || !botAI->GetAiObjectContext())
		return changed;

	auto* context = botAI->GetAiObjectContext();
	if (context->GetValue<Unit*>("old target")->Get())
	{
		context->GetValue<Unit*>("old target")->Set(nullptr);
		changed = true;
	}

	if (context->GetValue<Unit*>("current target")->Get())
	{
		context->GetValue<Unit*>("current target")->Set(nullptr);
		changed = true;
	}

	if (!context->GetValue<GuidVector>("prioritized targets")->Get().empty())
	{
		context->GetValue<GuidVector>("prioritized targets")->Reset();
		changed = true;
	}

	if (!context->GetValue<ObjectGuid>("pull target")->Get().IsEmpty())
	{
		context->GetValue<ObjectGuid>("pull target")->Set(ObjectGuid::Empty);
		changed = true;
	}

	if (!context->GetValue<ObjectGuid>("pull strategy target")->Get().IsEmpty())
	{
		context->GetValue<ObjectGuid>("pull strategy target")->Set(ObjectGuid::Empty);
		changed = true;
	}

	return changed;
}
}    // namespace

class Aq40TwinEmperorsListenerScript : public AllSpellScript
{
public:
	Aq40TwinEmperorsListenerScript() : AllSpellScript("Aq40TwinEmperorsListenerScript") { }

	void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
	{
		if (!spell || !caster || !spellInfo || !Aq40SpellIds::IsTwinEncounterSpell(spellInfo) || !IsTwinRelevantCaster(caster))
			return;

		std::vector<Player*> twinBots = CollectTwinBots(caster);
		if (twinBots.empty())
			return;

		Player* logBot = twinBots.front();
		Aq40TwinEncounter::TwinBoss boss;
		bool const hasBossCaster = TryGetTwinBoss(caster, boss);
		Aq40TwinEncounter::TwinEncounterState* statePtr = Aq40TwinEncounter::GetEncounterState(logBot);
		if (!hasBossCaster && (!statePtr || !Aq40TwinEncounter::IsActivePhase(statePtr->phase)))
			return;

		uint32 const nowMs = getMSTime();
		Aq40TwinEncounter::TwinEncounterState& state =
			statePtr ? *statePtr : Aq40TwinEncounter::EnsureEncounterState(logBot);
		Aq40TwinEncounter::TwinScriptedHazardWindows& hazards = state.scriptedHazards;
		bool const combatArmed =
			!hasBossCaster || MarkEncounterCombat(state, logBot, caster, spellInfo->Id, nowMs);

		if (hasBossCaster && !combatArmed && state.phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull)
			return;

		if (hasBossCaster && combatArmed && spellInfo->Id != Aq40SpellIds::TwinTeleportPrimary &&
			spellInfo->Id != Aq40SpellIds::TwinTeleportSecondary && spellInfo->Id != Aq40SpellIds::TwinHealBrother)
		{
			ConfirmBossOwner(state, spell, caster, spellInfo->Id, boss, nowMs, logBot);
		}

		switch (spellInfo->Id)
		{
			case Aq40SpellIds::TwinTeleportPrimary:
			case Aq40SpellIds::TwinTeleportSecondary:
			{
				if (!UpdateHazardTimestamp(hazards.teleportAtMs, nowMs, kTwinTeleportDebounceMs))
					return;

				bool const firstTeleport = state.teleportCount == 0;
				Aq40TwinEncounter::TwinEncounterState preTeleportState;
				if (firstTeleport)
					preTeleportState = state;

				Aq40TwinEncounter::EnterTeleportWindow(state, kTwinTeleportThreatHoldMs, nowMs);
				RequestInterruptForTwinBots(twinBots, caster);

				std::ostringstream fields;
				fields << "boss=twin spell=" << spellInfo->Id
					   << " source=" << Aq40Helpers::GetAq40LogUnit(caster)
					   << " phase=" << Aq40TwinEncounter::ToString(state.phase)
					   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
					   << " teleport_count=" << static_cast<uint32>(state.teleportCount)
					   << " engagement_elapsed_ms=" << GetTwinInitialEngagementElapsedMs(state, nowMs)
					   << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state, nowMs)
					   << " teleport_elapsed_ms=" << GetTwinTeleportElapsedMs(state, nowMs)
					   << " swap_prep_elapsed_ms=" << GetTwinSwapPrepElapsedMs(state, nowMs)
					   << " max_threat_hold_remaining_ms="
					   << Aq40TwinEncounter::GetMaxThreatHoldRemainingMs(state, nowMs)
					   << " approach=" << state.approachMemberCount
					   << " staged=" << state.stagedMemberCount
					   << " center_committed=" << state.centerCommittedMemberCount
					   << " strict_ready=" << state.strictReadyMemberCount
					   << " assigned=" << state.assignments.size();
				AppendTwinInitialEngagementBossFields(
					fields, logBot, state, Aq40TwinEncounter::TwinBoss::Veklor, "veklor", nowMs);
				AppendTwinInitialEngagementBossFields(
					fields, logBot, state, Aq40TwinEncounter::TwinBoss::Veknilash, "veknilash", nowMs);
				Aq40Helpers::LogAq40Info(logBot, "twin_script_window", "twin:teleport", fields.str(), 1000);
				if (firstTeleport)
					LogTwinFirstTeleportReservePromotionAudit(
						logBot, preTeleportState, state, caster, spellInfo->Id, nowMs);
				return;
			}

			case Aq40SpellIds::TwinBlizzard:
			{
				UpdateHazardTimestamp(hazards.blizzardAtMs, nowMs);
				RequestInterruptForTwinBots(twinBots, caster);

				std::ostringstream fields;
				fields << "boss=twin spell=" << spellInfo->Id
					   << " hazard=blizzard source=" << Aq40Helpers::GetAq40LogUnit(caster)
					   << " phase=" << Aq40TwinEncounter::ToString(state.phase);
				Aq40Helpers::LogAq40Info(logBot, "twin_script_hazard", "twin:blizzard", fields.str(), 1000);
				return;
			}

			case Aq40SpellIds::TwinArcaneBurst:
			{
				UpdateHazardTimestamp(hazards.arcaneBurstAtMs, nowMs);
				Player* activeOwner = hasBossCaster ? ResolveBossOwnerForSpell(spell, caster, spellInfo->Id) : nullptr;
				RequestInterruptForTwinBots(twinBots, caster, activeOwner);

				std::ostringstream fields;
				fields << "boss=twin spell=" << spellInfo->Id
					   << " hazard=arcane_burst source=" << Aq40Helpers::GetAq40LogUnit(caster)
					   << " owner=" << Aq40Helpers::GetAq40LogUnit(activeOwner)
					   << " phase=" << Aq40TwinEncounter::ToString(state.phase);
				Aq40Helpers::LogAq40Info(logBot, "twin_script_hazard", "twin:arcane_burst", fields.str(), 1000);
				return;
			}

			case Aq40SpellIds::TwinHealBrother:
			{
				UpdateHazardTimestamp(hazards.healBrotherAtMs, nowMs);
				Aq40TwinEncounter::EnterTerminalFailure(state, nowMs);
				RequestInterruptForTwinBots(twinBots, caster);

				size_t clearedBotCount = 0u;
				for (Player* twinBot : twinBots)
				{
					if (ClearTwinTerminalFailureCombatState(twinBot))
						++clearedBotCount;
				}

				std::ostringstream fields;
				fields << "boss=twin spell=" << spellInfo->Id
					   << " hazard=heal_brother source=" << Aq40Helpers::GetAq40LogUnit(caster)
					   << " phase=" << Aq40TwinEncounter::ToString(state.phase)
					   << " mode=" << Aq40TwinEncounter::ToString(state.mode)
					   << " split_band=" << Aq40TwinEncounter::ToString(state.recovery.splitBand)
					   << " cleared_bots=" << clearedBotCount
					   << " veklor_pickup_established="
					   << (Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veklor) ? 1 : 0)
					   << " veknilash_pickup_established="
					   << (Aq40TwinEncounter::IsPickupEstablished(state, Aq40TwinEncounter::TwinBoss::Veknilash) ? 1 : 0)
					   << " phase_elapsed_ms=" << Aq40TwinEncounter::GetPhaseElapsedMs(state, nowMs)
					   << " since_last_teleport_ms="
					   << (state.lastTeleportAtMs ? getMSTimeDiff(state.lastTeleportAtMs, nowMs) : 0u);
				Aq40Helpers::LogAq40Warn(logBot, "twin_terminal_failure", "twin:heal_brother", fields.str(), 1000);
				return;
			}

			case Aq40SpellIds::TwinExplodeBug:
			{
				UpdateHazardTimestamp(hazards.explodeBugAtMs, nowMs);
				Aq40TwinEncounter::SetExplodeBugSource(state, caster->GetGUID(), caster->GetPosition());
				RequestInterruptForTwinBots(twinBots, caster, nullptr, kTwinExplodeBugInterruptRadius, &state, true);

				std::ostringstream fields;
				fields << "boss=twin spell=" << spellInfo->Id
					   << " hazard=explode_bug source=" << Aq40Helpers::GetAq40LogUnit(caster)
					   << " phase=" << Aq40TwinEncounter::ToString(state.phase);
				Aq40Helpers::LogAq40Info(logBot, "twin_script_hazard", "twin:explode_bug", fields.str(), 1000);
				return;
			}

			case Aq40SpellIds::TwinMutateBug:
				UpdateHazardTimestamp(hazards.mutateBugAtMs, nowMs);
				return;

			case Aq40SpellIds::TwinUppercut:
				UpdateHazardTimestamp(hazards.uppercutAtMs, nowMs);
				return;

			case Aq40SpellIds::TwinUnbalancingStrike:
				UpdateHazardTimestamp(hazards.unbalancingStrikeAtMs, nowMs);
				return;

			default:
				return;
		}
	}
};

void AddSC_Aq40BotScripts()
{
	new Aq40TwinEmperorsListenerScript();
}
