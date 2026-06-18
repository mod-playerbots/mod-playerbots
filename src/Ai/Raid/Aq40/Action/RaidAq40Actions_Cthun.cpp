#include "RaidAq40Actions.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <unordered_map>

#include "GameObject.h"
#include "ObjectGuid.h"
#include "../RaidAq40BossHelper.h"
#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Cthun.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "Timer.h"

namespace Aq40BossActions
{
Unit* FindCthunTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitByAnyName(botAI, attackers, { "c'thun", "eye of c'thun", "claw tentacle", "giant eye tentacle" });
}
}    // namespace Aq40BossActions

namespace
{
float constexpr kPi = 3.14159265f;
uint32 constexpr kCthunGiantWavePeriodMs = 20000;
float constexpr kCthunSafeZTolerance = 6.0f;

struct CthunSpreadState
{
    std::unordered_map<uint64, Position> positions;
    uint64 anchorGuid = 0;
};
std::unordered_map<uint32, CthunSpreadState> sCthunSpreadByInstance;

struct CthunSpreadSlot
{
    bool innerRing = false;
    uint32 ordinal = 0;
    uint32 ringSize = 1;
};

enum class CthunExpectedGiantType : uint8
{
    Claw = 0,
    Eye = 1
};

Unit* FindCthunBody(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "c'thun" });
}

Unit* FindCthunEye(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "eye of c'thun" });
}

bool IsCthunBossUnit(PlayerbotAI* botAI, Unit* unit)
{
    return unit && Aq40BossHelper::IsUnitNamedAny(botAI, unit, { "c'thun", "eye of c'thun" });
}

bool ResolveSafeCthunFloorPosition(Player* bot, float targetX, float targetY, float& outX, float& outY, float& outZ)
{
    if (!bot || !bot->GetMap())
        return false;

    outX = targetX;
    outY = targetY;
    outZ = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), outX, outY, outZ))
        return false;

    return std::fabs(outZ - bot->GetPositionZ()) <= kCthunSafeZTolerance;
}

bool ShouldBlockUnsafeCthunBossPursuit(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!bot || !target || !IsCthunBossUnit(botAI, target))
        return false;

    return std::fabs(target->GetPositionZ() - bot->GetPositionZ()) > kCthunSafeZTolerance ||
           !bot->IsWithinLOSInMap(target);
}

void PinCthunTarget(PlayerbotAI* botAI, AiObjectContext* context, Unit* target)
{
    if (!botAI || !context || !target)
        return;

    ObjectGuid const guid = target->GetGUID();
    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({ guid });
    context->GetValue<ObjectGuid>("pull target")->Set(guid);
}

Unit* FindHighestPriorityCthunAdd(PlayerbotAI* botAI, GuidVector const& attackers)
{
    Unit* add = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "flesh tentacle" });
    if (add)
        return add;

    add = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "eye tentacle" });
    if (add)
        return add;

    add = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant eye tentacle" });
    if (add)
        return add;

    add = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant claw tentacle" });
    if (add)
        return add;

    return Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "claw tentacle" });
}

CthunExpectedGiantType GetExpectedGiantType(PlayerbotAI* botAI, GuidVector const& attackers)
{
    uint32 elapsed = Aq40Helpers::GetCthunPhase2ElapsedMs(botAI, attackers);
    uint32 window = (elapsed / kCthunGiantWavePeriodMs) % 2;
    return window == 0 ? CthunExpectedGiantType::Claw : CthunExpectedGiantType::Eye;
}

Unit* FindTankPriorityCthunAdd(PlayerbotAI* botAI, GuidVector const& attackers)
{
    CthunExpectedGiantType expected = GetExpectedGiantType(botAI, attackers);
    if (expected == CthunExpectedGiantType::Eye)
    {
        Unit* giantEye = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant eye tentacle" });
        if (giantEye)
            return giantEye;
    }

    Unit* add = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant claw tentacle" });
    if (add)
        return add;

    if (expected == CthunExpectedGiantType::Claw)
    {
        Unit* giantEye = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant eye tentacle" });
        if (giantEye)
            return giantEye;
    }

    return FindHighestPriorityCthunAdd(botAI, attackers);
}

bool ShouldWaitForCthunPhase2Pickup(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    if (!bot || !botAI || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    uint32 const elapsedMs = Aq40Helpers::GetCthunPhase2ElapsedMs(botAI, attackers);
    if (elapsedMs > 2000)
        return false;

    Unit* giantClaw = Aq40BossActions::FindUnitByAnyName(botAI, attackers, { "giant claw tentacle" });
    return giantClaw && !Aq40BossHelper::IsUnitHeldByEncounterTank(bot, giantClaw);
}

Unit* FindCthunBeamSpacingRisk(Player* bot, PlayerbotAI* botAI, Unit* eye)
{
    if (!bot || !botAI || !eye)
        return nullptr;

    ObjectGuid const eyeTargetGuid = eye->GetTarget();
    if (!eyeTargetGuid || eyeTargetGuid == bot->GetGUID())
        return nullptr;

    Unit* eyeTarget = botAI->GetUnit(eyeTargetGuid);
    if (!eyeTarget || !eyeTarget->IsAlive())
        return nullptr;

    return bot->GetDistance2d(eyeTarget) < 12.0f ? eyeTarget : nullptr;
}

Creature* FindLikelyStomachExitTrigger(Player* bot)
{
    static constexpr uint32 kCthunExitTriggerEntry = 15800;
    return bot ? bot->FindNearestCreature(kCthunExitTriggerEntry, 150.0f, true) : nullptr;
}

uint32 GetSpreadOrdinal(Player* bot, PlayerbotAI* botAI, bool forMelee, uint32& outCohortSize)
{
    Group const* group = bot->GetGroup();
    if (!group)
    {
        outCohortSize = forMelee ? 8u : 12u;
        return static_cast<uint32>(bot->GetGUID().GetCounter() % outCohortSize);
    }

    uint32 index = 0;
    uint32 botOrdinal = 0;
    bool found = false;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !Aq40BossHelper::IsNearEncounter(bot, member))
            continue;

        if (Aq40BossHelper::IsEncounterTank(member, member))
            continue;

    // Human non-tanks count in the ranged cohort so bots spread around
    // them rather than stacking on top of them.  We cannot reliably
    // classify human hybrids, but counting them as ranged is the safest
    // default (worst case: a melee human occupies a ranged slot, giving
    // bots slightly wider ranged spread, which is harmless).
        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        bool memberIsMelee;
        if (memberAI)
            memberIsMelee = !memberAI->IsRanged(member) && !memberAI->IsHeal(member);
        else
            memberIsMelee = false;  // default humans to ranged
        if (forMelee != memberIsMelee)
            continue;

        if (member->GetGUID() == bot->GetGUID())
        {
            botOrdinal = index;
            found = true;
        }

        ++index;
    }

    outCohortSize = std::max(index, forMelee ? 8u : 12u);
    if (found)
        return botOrdinal;

    outCohortSize = forMelee ? 8u : 12u;
    return static_cast<uint32>(bot->GetGUID().GetCounter() % outCohortSize);
}

CthunSpreadSlot GetSpreadSlot(Player* bot, PlayerbotAI* botAI, bool forMelee)
{
    uint32 cohortSize = 0;
    uint32 ordinal = GetSpreadOrdinal(bot, botAI, forMelee, cohortSize);
    if (forMelee)
        return { false, ordinal, std::max<uint32>(cohortSize, 8u) };

    uint32 innerRingSize = std::max<uint32>(1u, cohortSize / 2);
    uint32 outerRingSize = std::max<uint32>(1u, cohortSize - innerRingSize);
    bool const innerRing = (ordinal % 2u) != 0u;
    uint32 ringOrdinal = ordinal / 2u;
    if (innerRing)
        return { true, ringOrdinal % innerRingSize, innerRingSize };

    return { false, ringOrdinal % outerRingSize, outerRingSize };
}

Position GetAssignedCthunSpreadPosition(Player* bot, PlayerbotAI* botAI, Unit* boss, bool forMelee)
{
    uint32 const instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    CthunSpreadState& state = sCthunSpreadByInstance[instanceId];

    // Clear stale positions on wipe/re-pull: only when no encounter-local
    // cluster of combatants exists in the instance.
    if (!Aq40BossHelper::IsEncounterCombatActive(bot))
    {
        state.positions.clear();
        state.anchorGuid = 0;
    }

    // Invalidate cached positions when the reference boss changes
    // (e.g. Eye of C'Thun → C'Thun body on phase transition).
    uint64 const bossGuid = boss->GetGUID().GetRawValue();
    if (state.anchorGuid != bossGuid)
    {
        state.positions.clear();
        state.anchorGuid = bossGuid;
    }

    uint64 const botGuid = bot->GetGUID().GetRawValue();
    auto itr = state.positions.find(botGuid);
    if (itr != state.positions.end())
        return itr->second;

    CthunSpreadSlot const slot = GetSpreadSlot(bot, botAI, forMelee);
    float const angle = static_cast<float>(slot.ordinal) * ((2.0f * kPi) / static_cast<float>(slot.ringSize));
    float const radius = forMelee ? 8.0f : (slot.innerRing ? 18.0f : 28.0f);

    Position assigned(boss->GetPositionX() + std::cos(angle) * radius,
                      boss->GetPositionY() + std::sin(angle) * radius,
                      boss->GetPositionZ());
    state.positions[botGuid] = assigned;
    return assigned;
}

}    // namespace

bool Aq40CthunChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (encounterUnits.empty())
        return false;

    if (ShouldWaitForCthunPhase2Pickup(bot, botAI, encounterUnits))
    {
        Aq40Helpers::LogAq40Info(bot, "tank_pickup_wait", "cthun:phase2_adds",
            "boss=cthun reason=phase2_pickup");
        return false;
    }

    bool const isPrimaryTank = Aq40BossHelper::IsEncounterPrimaryTank(bot, bot);
    Unit* target = nullptr;
    std::string reason;

    if (Aq40Helpers::IsCthunInStomach(bot, botAI))
    {
        target = Aq40BossActions::FindUnitByAnyName(botAI, encounterUnits, { "flesh tentacle" });
        reason = "stomach_flesh";
    }

    if (!target)
    {
        target = isPrimaryTank ? FindTankPriorityCthunAdd(botAI, encounterUnits) : FindHighestPriorityCthunAdd(botAI, encounterUnits);
        if (target)
            reason = isPrimaryTank ? "tank_add" : "add_priority";
    }

    if (!target)
    {
        Unit* body = FindCthunBody(botAI, encounterUnits);
        Unit* eye = FindCthunEye(botAI, encounterUnits);
        if (body && Aq40Helpers::IsCthunVulnerableNow(botAI, encounterUnits))
        {
            target = body;
            reason = "vulnerable";
        }
        else
        {
            target = eye ? eye : body;
            reason = eye ? "eye" : "body";
        }
    }

    bool const isTankControlledAdd =
        target && (botAI->EqualLowercaseName(target->GetName(), "giant claw tentacle") ||
                   botAI->EqualLowercaseName(target->GetName(), "claw tentacle"));
    if (isTankControlledAdd && Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target))
        return false;

    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    PinCthunTarget(botAI, context, target);
    if (!Aq40Helpers::IsCthunInStomach(bot, botAI) && !Aq40BossHelper::IsEncounterTank(bot, bot) &&
        ShouldBlockUnsafeCthunBossPursuit(bot, botAI, target))
    {
        Aq40Helpers::LogAq40Warn(bot, "movement_failure",
            "cthun:unsafe_boss_pursuit:" + Aq40Helpers::GetAq40LogUnit(target),
            "boss=cthun reason=unsafe_boss_pursuit target=" + Aq40Helpers::GetAq40LogUnit(target));
        return false;
    }

    Aq40Helpers::LogAq40Target(bot, "cthun", reason, target);
    return Attack(target);
}

bool Aq40CthunMaintainSpreadAction::Execute(Event /*event*/)
{
    // Tanks hold position; stomach bots are handled by stomach DPS/exit;
    // only outside-room non-tanks should spread.
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40Helpers::IsCthunInStomach(bot, botAI))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* boss = FindCthunEye(botAI, encounterUnits);
    if (!boss)
        boss = FindCthunBody(botAI, encounterUnits);
    if (!boss)
        return false;

    if (Unit* spacingRisk = FindCthunBeamSpacingRisk(bot, botAI, FindCthunEye(botAI, encounterUnits)))
    {
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "cthun:eye_beam_spacing:" + Aq40Helpers::GetAq40LogUnit(spacingRisk),
            "boss=cthun hazard=eye_beam_spacing source=" + Aq40Helpers::GetAq40LogUnit(spacingRisk));
        return MoveAway(spacingRisk, 12.0f - bot->GetDistance2d(spacingRisk));
    }

    bool const isMelee = !botAI->IsRanged(bot) && !botAI->IsHeal(bot);
    Position assigned = GetAssignedCthunSpreadPosition(bot, botAI, boss, isMelee);
    float moveX = assigned.GetPositionX();
    float moveY = assigned.GetPositionY();
    float moveZ = bot->GetPositionZ();

    float const tolerance = isMelee ? 3.0f : 5.0f;
    if (bot->GetDistance2d(moveX, moveY) < tolerance)
        return false;

    if (!ResolveSafeCthunFloorPosition(bot, moveX, moveY, moveX, moveY, moveZ))
    {
        Aq40Helpers::LogAq40Warn(bot, "movement_failure", "cthun:spread:no_safe_floor",
            "boss=cthun reason=no_safe_floor target=" + Aq40Helpers::GetAq40LogUnit(boss));
        return false;
    }

    Aq40Helpers::LogAq40Info(bot, "spread_position",
        "cthun:" + Aq40Helpers::GetAq40LogUnit(boss),
        "boss=cthun target=" + Aq40Helpers::GetAq40LogUnit(boss) +
        " role=" + std::string(isMelee ? "melee" : "ranged"));
    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool Aq40CthunAvoidDarkGlareAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* boss = FindCthunEye(botAI, encounterUnits);
    if (!boss)
        boss = FindCthunBody(botAI, encounterUnits);
    if (!boss)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    float dx = bot->GetPositionX() - boss->GetPositionX();
    float dy = bot->GetPositionY() - boss->GetPositionY();
    float radius = std::sqrt(dx * dx + dy * dy);
    if (radius < 10.0f)
        return false;

    float currentAngle = std::atan2(dy, dx);

    // Determine beam direction from boss orientation (Dark Glare fires
    // in the direction the Eye is facing and rotates).
    float beamAngle = boss->GetOrientation();
    // Vector from boss along beam direction
    float beamDx = std::cos(beamAngle);
    float beamDy = std::sin(beamAngle);
    // Normalize bot vector from boss
    float botDx = dx / radius;
    float botDy = dy / radius;
    // Cross product determines which side of the beam the bot is on
    // (pattern used by ICC Putricideoid directional avoidance).
    float cross = beamDx * botDy - beamDy * botDx;
    // Move in the direction that takes us away from the beam sweep.
    // If cross > 0, bot is counter-clockwise from beam → rotate further CCW (+angle).
    // If cross < 0, bot is clockwise from beam → rotate further CW (-angle).
    // If cross ~0, bot is right in the beam path → pick either direction.
    float step = kPi / 5.0f;  // 36 degrees
    float nextAngle = currentAngle + (cross >= 0.0f ? step : -step);
    float moveX = boss->GetPositionX() + std::cos(nextAngle) * radius;
    float moveY = boss->GetPositionY() + std::sin(nextAngle) * radius;
    float moveZ = bot->GetPositionZ();

    if (!ResolveSafeCthunFloorPosition(bot, moveX, moveY, moveX, moveY, moveZ))
    {
        Aq40Helpers::LogAq40Warn(bot, "movement_failure",
            "cthun:dark_glare:no_safe_floor:" + Aq40Helpers::GetAq40LogUnit(boss),
            "boss=cthun hazard=dark_glare reason=no_safe_floor source=" + Aq40Helpers::GetAq40LogUnit(boss));
        return false;
    }

    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "cthun:dark_glare:" + Aq40Helpers::GetAq40LogUnit(boss),
        "boss=cthun hazard=dark_glare source=" + Aq40Helpers::GetAq40LogUnit(boss));
    return MoveTo(bot->GetMapId(), moveX, moveY, moveZ, false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40CthunStomachDpsAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* fleshTentacle = Aq40BossActions::FindUnitByAnyName(botAI, encounterUnits, { "flesh tentacle" });
    if (!fleshTentacle)
        return false;

    if (AI_VALUE(Unit*, "current target") == fleshTentacle && bot->GetVictim() == fleshTentacle)
        return false;

    PinCthunTarget(botAI, context, fleshTentacle);
    Aq40Helpers::LogAq40Target(bot, "cthun", "stomach_flesh", fleshTentacle);
    return Attack(fleshTentacle);
}

bool Aq40CthunStomachExitAction::Execute(Event /*event*/)
{
    Aura* acid = Aq40SpellIds::GetAnyAura(bot, { Aq40SpellIds::CthunDigestiveAcid });
    if (!acid)
        acid = botAI->GetAura("digestive acid", bot, false, true);
    if (!acid)
        return false;

    uint32 exitStacks = 10;
    if (Aq40BossHelper::IsEncounterPrimaryTank(bot, bot))
        exitStacks = 1;
    else if (botAI->IsHeal(bot))
        exitStacks = 5;

    if (acid->GetStackAmount() < exitStacks)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    GameObject* portal = Aq40Helpers::FindLikelyStomachExitPortal(bot, botAI);
    if (portal)
    {
        Aq40Helpers::LogAq40Info(bot, "stomach_exit",
            "cthun:portal:stacks:" + std::to_string(acid->GetStackAmount()),
            "boss=cthun method=portal stacks=" + std::to_string(acid->GetStackAmount()));
        return MoveTo(bot->GetMapId(), portal->GetPositionX(), portal->GetPositionY(), portal->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    if (Creature* exitTrigger = FindLikelyStomachExitTrigger(bot))
    {
        Aq40Helpers::LogAq40Info(bot, "stomach_exit",
            "cthun:trigger:stacks:" + std::to_string(acid->GetStackAmount()),
            "boss=cthun method=trigger stacks=" + std::to_string(acid->GetStackAmount()));
        return MoveTo(bot->GetMapId(), exitTrigger->GetPositionX(), exitTrigger->GetPositionY(), exitTrigger->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }

    // Fallback: deterministic radial sweep to avoid straight-line dead ends.
    uint32 direction = (getMSTime() / 2000) % 8;
    float angle = (2.0f * kPi) * (static_cast<float>(direction) / 8.0f);
    float moveDistance = 14.0f;
    float moveX = bot->GetPositionX() + std::cos(angle) * moveDistance;
    float moveY = bot->GetPositionY() + std::sin(angle) * moveDistance;

    Aq40Helpers::LogAq40Warn(bot, "stomach_exit",
        "cthun:fallback:stacks:" + std::to_string(acid->GetStackAmount()),
        "boss=cthun method=fallback stacks=" + std::to_string(acid->GetStackAmount()));
    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40CthunPhase2AddPriorityAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (ShouldWaitForCthunPhase2Pickup(bot, botAI, encounterUnits))
    {
        Aq40Helpers::LogAq40Info(bot, "tank_pickup_wait", "cthun:phase2_priority",
            "boss=cthun reason=phase2_pickup");
        return false;
    }

    Unit* target = Aq40BossHelper::IsEncounterPrimaryTank(bot, bot) ?
        FindTankPriorityCthunAdd(botAI, encounterUnits) : FindHighestPriorityCthunAdd(botAI, encounterUnits);
    bool const isTankControlledAdd =
        target && (botAI->EqualLowercaseName(target->GetName(), "giant claw tentacle") ||
                   botAI->EqualLowercaseName(target->GetName(), "claw tentacle"));
    if (isTankControlledAdd && Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target))
        return false;
    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    PinCthunTarget(botAI, context, target);
    Aq40Helpers::LogAq40Target(bot, "cthun", "phase2_add", target);
    return Attack(target);
}

bool Aq40CthunVulnerableBurstAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (!Aq40Helpers::IsCthunVulnerableNow(botAI, encounterUnits))
        return false;

    // Keep tentacle cleanup priority even inside weakened windows.
    if (Aq40BossActions::FindUnitByAnyName(botAI, encounterUnits,
        { "flesh tentacle", "eye tentacle", "giant eye tentacle", "claw tentacle", "giant claw tentacle" }))
        return false;

    Unit* body = FindCthunBody(botAI, encounterUnits);
    if (!body || (AI_VALUE(Unit*, "current target") == body && bot->GetVictim() == body))
        return false;

    PinCthunTarget(botAI, context, body);
    Aq40Helpers::LogAq40Info(bot, "vulnerable", "cthun:body",
        "boss=cthun phase=vulnerable target=" + Aq40Helpers::GetAq40LogUnit(body));
    Aq40Helpers::LogAq40Target(bot, "cthun", "vulnerable", body);
    return Attack(body);
}

bool Aq40CthunInterruptEyeAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    std::vector<Unit*> eyes = Aq40BossActions::FindUnitsByAnyName(botAI, encounterUnits, { "eye tentacle", "giant eye tentacle" });
    for (Unit* eye : eyes)
    {
        if (!eye || !eye->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            continue;

    // Switch target this tick; interrupt will fire on the next cycle
    // once the bot is facing/in range. Avoids failing interrupts from
    // same-tick target switch (Naxxramas uses separate ticks too).
        if (AI_VALUE(Unit*, "current target") != eye || bot->GetVictim() != eye)
        {
            PinCthunTarget(botAI, context, eye);
            Aq40Helpers::LogAq40Target(bot, "cthun", "interrupt_setup", eye);
            return Attack(eye);
        }

        Aq40Helpers::LogAq40Info(bot, "interrupt",
            "cthun:" + Aq40Helpers::GetAq40LogUnit(eye),
            "boss=cthun target=" + Aq40Helpers::GetAq40LogUnit(eye));
        return botAI->DoSpecificAction("interrupt spell", Event(), true);
    }

    return false;
}
