#include "RaidAq40Actions.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>

#include "ObjectGuid.h"
#include "../RaidAq40BossHelper.h"
#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "../Util/RaidAq40Helpers_Skeram.h"
#include "../Util/RaidAq40TwinEncounter.h"
#include "../../RaidBossHelpers.h"

namespace
{
struct Aq40ManagedResistanceState
{
    bool natureCombatEnabled = false;
    bool natureNonCombatEnabled = false;
    bool shamanNatureCombatEnabled = false;
    bool priestShadowNonCombatEnabled = false;
    bool paladinShadowCombatEnabled = false;
};

enum class Aq40TwinShadowResistanceMode : uint8
{
    None = 0,
    PriestBuff,
    SidePaladinAuras,
};

struct Aq40TwinShadowResistanceContext
{
    bool required = false;
    Aq40TwinShadowResistanceMode mode = Aq40TwinShadowResistanceMode::None;
    Player* priestProvider = nullptr;
    std::array<Player*, 2> paladinProviderBySide = { nullptr, nullptr };
    std::array<Player*, 2> warlockTankBySide = { nullptr, nullptr };
};

std::unordered_map<uint64, Aq40ManagedResistanceState> sManagedResistanceStateByBot;
std::unordered_map<uint64, bool> sAq40CleanupReportedDirtyByBot;

bool HasAnyManagedResistanceFlags(Aq40ManagedResistanceState const& state)
{
    return state.natureCombatEnabled || state.natureNonCombatEnabled || state.shamanNatureCombatEnabled ||
           state.priestShadowNonCombatEnabled || state.paladinShadowCombatEnabled;
}

bool IsTwinShadowResistanceWindow(Aq40TwinEncounter::TwinEncounterState const& state)
{
    if (Aq40TwinEncounter::IsTerminalPhase(state.phase) ||
        !Aq40TwinEncounter::HasDeterministicAssignments(state))
    {
        return false;
    }

    return ((state.mode == Aq40TwinEncounter::TwinStrategyMode::StandardCompReady) &&
            state.phase == Aq40TwinEncounter::TwinEncounterPhase::PrePull) ||
           Aq40TwinEncounter::IsActivePhase(state.phase) ||
           state.phase == Aq40TwinEncounter::TwinEncounterPhase::Degraded;
}

Aq40TwinShadowResistanceContext BuildTwinShadowResistanceContext(Player* bot)
{
    Aq40TwinShadowResistanceContext context;
    if (!bot)
        return context;

    Aq40TwinEncounter::TwinEncounterState const* state = Aq40TwinEncounter::GetEncounterState(bot);
    if (!state || !IsTwinShadowResistanceWindow(*state))
        return context;

    context.required = true;

    Group* group = bot->GetGroup();
    if (!group)
        return context;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !member->IsInWorld() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        Aq40TwinEncounter::TwinRoleAssignment const* assignment =
            Aq40TwinEncounter::GetAssignmentForMember(*state, member->GetGUID());
        if (!assignment)
            continue;

        if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank &&
            Aq40TwinEncounter::IsKnownSide(assignment->stableSide))
        {
            context.warlockTankBySide[assignment->stableSide == Aq40TwinEncounter::TwinSide::Side1 ? 1u : 0u] = member;
        }

        if (member->getClass() == CLASS_PRIEST)
        {
            if (!context.priestProvider || member->GetGUID().GetRawValue() < context.priestProvider->GetGUID().GetRawValue())
                context.priestProvider = member;

            continue;
        }

        if (member->getClass() != CLASS_PALADIN || !Aq40TwinEncounter::IsKnownSide(assignment->stableSide))
            continue;

        size_t const sideIndex = assignment->stableSide == Aq40TwinEncounter::TwinSide::Side1 ? 1u : 0u;
        Player*& sideProvider = context.paladinProviderBySide[sideIndex];
        if (!sideProvider || member->GetGUID().GetRawValue() < sideProvider->GetGUID().GetRawValue())
            sideProvider = member;
    }

    if (context.priestProvider)
        context.mode = Aq40TwinShadowResistanceMode::PriestBuff;
    else if (context.paladinProviderBySide[0] && context.paladinProviderBySide[1])
        context.mode = Aq40TwinShadowResistanceMode::SidePaladinAuras;

    return context;
}

bool IsTwinSidePaladinProvider(Player* bot, Aq40TwinShadowResistanceContext const& context,
                               Aq40TwinEncounter::TwinSide* outSide = nullptr)
{
    if (!bot || context.mode != Aq40TwinShadowResistanceMode::SidePaladinAuras)
        return false;

    for (size_t sideIndex = 0; sideIndex < context.paladinProviderBySide.size(); ++sideIndex)
    {
        if (context.paladinProviderBySide[sideIndex] != bot)
            continue;

        if (outSide)
            *outSide = sideIndex == 1u ? Aq40TwinEncounter::TwinSide::Side1 : Aq40TwinEncounter::TwinSide::Side0;

        return true;
    }

    return false;
}

bool HasTwinShadowProtectionAura(PlayerbotAI* botAI, Player* target)
{
    return botAI && target &&
           (botAI->HasAura("shadow protection", target) || botAI->HasAura("prayer of shadow protection", target));
}

bool HasTwinShadowProtectionMissing(PlayerbotAI* botAI, Aq40TwinShadowResistanceContext const& context)
{
    if (!botAI || context.mode != Aq40TwinShadowResistanceMode::PriestBuff)
        return false;

    for (Player* warlockTank : context.warlockTankBySide)
    {
        if (warlockTank && !HasTwinShadowProtectionAura(botAI, warlockTank))
            return true;
    }

    return false;
}

char const* ToString(Aq40TwinShadowResistanceMode mode)
{
    switch (mode)
    {
        case Aq40TwinShadowResistanceMode::None: return "none";
        case Aq40TwinShadowResistanceMode::PriestBuff: return "priest_buff";
        case Aq40TwinShadowResistanceMode::SidePaladinAuras: return "side_paladin_auras";
    }

    return "none";
}

}    // namespace

namespace Aq40Helpers
{

bool HasManagedResistanceState(Player* bot)
{
    if (!bot)
        return false;

    auto const itr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    return itr != sManagedResistanceStateByBot.end() && HasAnyManagedResistanceFlags(itr->second);
}

bool ClearManagedResistanceStrategies(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return false;

    auto itr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    if (itr == sManagedResistanceStateByBot.end())
        return false;

    Aq40ManagedResistanceState& managedState = itr->second;
    bool cleaned = false;

    if (managedState.natureCombatEnabled && botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_COMBAT);
        managedState.natureCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.natureNonCombatEnabled && botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_NON_COMBAT);
        managedState.natureNonCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.shamanNatureCombatEnabled && botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-nature resistance", BotState::BOT_STATE_COMBAT);
        managedState.shamanNatureCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.paladinShadowCombatEnabled && botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT))
    {
        botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_COMBAT);
        managedState.paladinShadowCombatEnabled = false;
        cleaned = true;
    }

    if (managedState.priestShadowNonCombatEnabled && botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT))
    {
        botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_NON_COMBAT);
        managedState.priestShadowNonCombatEnabled = false;
        cleaned = true;
    }

    if (!HasAnyManagedResistanceFlags(managedState))
    {
        sManagedResistanceStateByBot.erase(itr);
        cleaned = true;
    }

    return cleaned;
}

}    // namespace Aq40Helpers

namespace
{

void LogAq40CleanupTransition(Player* bot, bool wasDirty)
{
    if (!bot)
        return;

    uint64 const botGuid = bot->GetGUID().GetRawValue();
    auto itr = sAq40CleanupReportedDirtyByBot.find(botGuid);
    bool const previousDirty = itr != sAq40CleanupReportedDirtyByBot.end() && itr->second;

    if (wasDirty)
    {
        if (!previousDirty)
            LOG_INFO("playerbots", "AQ40 cleanup: bot={} cleaned stale recovery state and can resume follow", bot->GetName());

        sAq40CleanupReportedDirtyByBot[botGuid] = true;
        return;
    }

    if (previousDirty)
        LOG_INFO("playerbots", "AQ40 cleanup: bot={} recovery state already clean", bot->GetName());

    sAq40CleanupReportedDirtyByBot[botGuid] = false;
}

// IsSarturaMob / IsSarturaSpinning now live in Aq40BossHelper.

}    // namespace

namespace Aq40BossActions
{
Unit* FindUnitByAnyName(PlayerbotAI* botAI, GuidVector const& attackers, std::initializer_list<char const*> names)
{
    return Aq40BossHelper::FindUnitByAnyName(botAI, attackers, names);
}

std::vector<Unit*> FindUnitsByAnyName(PlayerbotAI* botAI, GuidVector const& attackers,
                                      std::initializer_list<char const*> names)
{
    return Aq40BossHelper::FindUnitsByAnyName(botAI, attackers, names);
}

Unit* FindTrashTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    // Simplified: only Anubisath Defenders trigger the trash strategy now.
    // Pick the lowest-health defender, falling back to closest attacker.
    return Aq40BossHelper::FindLowestHealthUnitByAnyName(botAI, attackers, { "anubisath defender" });
}
}    // namespace Aq40BossActions

namespace
{
Unit* FindClosestAq40PlagueSeparationRisk(Player* bot, PlayerbotAI* botAI, float& distanceToCreate)
{
    distanceToCreate = 0.0f;
    if (!bot || !botAI)
        return nullptr;

    Group const* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* riskiestMember = nullptr;
    float largestDeficit = 0.0f;

    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        float const currentDistance = bot->GetDistance2d(member);
        float const requiredDistance =
            Aq40SpellIds::HasAnyAura(botAI, member, { Aq40SpellIds::Aq40DefenderPlague }) ? 28.0f : 20.0f;
        float const deficit = requiredDistance - currentDistance;
        if (deficit <= 0.0f || deficit <= largestDeficit)
            continue;

        largestDeficit = deficit;
        riskiestMember = member;
    }

    distanceToCreate = largestDeficit;
    return riskiestMember;
}

struct Aq40TankRetreatResult
{
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Compute a short retreat position toward the encounter tank, away from a danger source.
// Returns invalid if no tank is available or the candidate would push the bot farther from
// the tank (deeper into the room).  Callers should hold position when invalid.
Aq40TankRetreatResult ComputeTankRetreatPosition(Player* bot, Unit* danger, float clearDistance)
{
    Aq40TankRetreatResult result;
    if (!bot || !danger)
        return result;

    Player* tank = Aq40BossHelper::GetEncounterPrimaryTank(bot);
    if (!tank)
        tank = Aq40BossHelper::GetEncounterBackupTank(bot, 0);
    if (!tank)
        return result;

    // Direction from danger toward the tank (retreat direction).
    float dx = tank->GetPositionX() - danger->GetPositionX();
    float dy = tank->GetPositionY() - danger->GetPositionY();
    float mag = std::sqrt(dx * dx + dy * dy);
    if (mag < 0.001f)
        return result;

    dx /= mag;
    dy /= mag;

    // Short corrective step, capped at 8y to avoid large scatter.
    float const step = std::min(clearDistance, 8.0f);
    float candidateX = bot->GetPositionX() + dx * step;
    float candidateY = bot->GetPositionY() + dy * step;
    float candidateZ = bot->GetPositionZ();

    // Safety: reject if the candidate is farther from the tank than we currently are.
    float const currentDistToTank = bot->GetDistance2d(tank);
    float const candidateDistToTank = std::sqrt(
        (candidateX - tank->GetPositionX()) * (candidateX - tank->GetPositionX()) +
        (candidateY - tank->GetPositionY()) * (candidateY - tank->GetPositionY()));
    if (candidateDistToTank > currentDistToTank + 1.0f)
        return result;

    // Validate collision.
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), candidateX, candidateY, candidateZ))
        return result;

    result.valid = true;
    result.x = candidateX;
    result.y = candidateY;
    result.z = candidateZ;
    return result;
}
}    // namespace

bool Aq40ManageResistanceStrategiesAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    Aq40ManagedResistanceState& managedState = sManagedResistanceStateByBot[bot->GetGUID().GetRawValue()];

    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    bool const inAq40 = Aq40BossHelper::IsInAq40(bot);
    bool const needNatureResistance =
        inAq40 && Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
            { "princess huhuran", "viscidus", "glob of viscidus", "toxic slime" });
    Aq40TwinShadowResistanceContext const twinShadowContext = BuildTwinShadowResistanceContext(bot);
    bool const needTwinShadowResistance = twinShadowContext.required &&
                                          twinShadowContext.mode != Aq40TwinShadowResistanceMode::None;

    bool acted = false;

    if (bot->getClass() == CLASS_HUNTER)
    {
        bool const hasNatureStrategyCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT);
        bool const hasNatureStrategyNonCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT);

        if (needNatureResistance)
        {
            if (!hasNatureStrategyCombat)
            {
                botAI->ChangeStrategy("+rnature", BotState::BOT_STATE_COMBAT);
                managedState.natureCombatEnabled = true;
                acted = true;
            }
            if (!hasNatureStrategyNonCombat)
            {
                botAI->ChangeStrategy("+rnature", BotState::BOT_STATE_NON_COMBAT);
                managedState.natureNonCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("aspect of the wild", bot))
                acted = botAI->DoSpecificAction("aspect of the wild", Event(), true) || acted;
        }
        else if (managedState.natureCombatEnabled || managedState.natureNonCombatEnabled)
        {
            if (managedState.natureCombatEnabled && hasNatureStrategyCombat)
            {
                botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_COMBAT);
                managedState.natureCombatEnabled = false;
                acted = true;
            }
            if (managedState.natureNonCombatEnabled && hasNatureStrategyNonCombat)
            {
                botAI->ChangeStrategy("-rnature", BotState::BOT_STATE_NON_COMBAT);
                managedState.natureNonCombatEnabled = false;
                acted = true;
            }
        }
    }

    if (bot->getClass() == CLASS_SHAMAN)
    {
        bool const hasNatureTotemStrategyCombat = botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT);

        if (needNatureResistance)
        {
            if (!hasNatureTotemStrategyCombat)
            {
                botAI->ChangeStrategy("+nature resistance", BotState::BOT_STATE_COMBAT);
                managedState.shamanNatureCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("nature resistance totem", bot))
                acted = botAI->DoSpecificAction("nature resistance totem", Event(), true) || acted;
        }
        else if (managedState.shamanNatureCombatEnabled)
        {
            if (hasNatureTotemStrategyCombat)
            {
                botAI->ChangeStrategy("-nature resistance", BotState::BOT_STATE_COMBAT);
                acted = true;
            }
            managedState.shamanNatureCombatEnabled = false;
        }
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        bool const shouldProvideTwinShadow = twinShadowContext.mode == Aq40TwinShadowResistanceMode::PriestBuff &&
                                             twinShadowContext.priestProvider == bot;
        bool const hasShadowStrategyNonCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT);

        if (shouldProvideTwinShadow)
        {
            if (!hasShadowStrategyNonCombat)
            {
                botAI->ChangeStrategy("+rshadow", BotState::BOT_STATE_NON_COMBAT);
                managedState.priestShadowNonCombatEnabled = true;
                acted = true;
            }

            if (HasTwinShadowProtectionMissing(botAI, twinShadowContext))
            {
                for (Player* warlockTank : twinShadowContext.warlockTankBySide)
                {
                    if (!warlockTank || HasTwinShadowProtectionAura(botAI, warlockTank))
                        continue;

                    if (botAI->CastSpell("shadow protection", warlockTank))
                    {
                        Aq40Helpers::LogAq40Info(bot, "resistance_strategy",
                            std::string("shadow:priest:") + Aq40Helpers::GetAq40LogUnit(warlockTank),
                            std::string("boss=twin shadow=1 mode=priest_buff action=shadow_protection target=") +
                                Aq40Helpers::GetAq40LogUnit(warlockTank),
                            1000);
                        acted = true;
                        break;
                    }
                }
            }
        }
        else if (managedState.priestShadowNonCombatEnabled)
        {
            if (hasShadowStrategyNonCombat)
            {
                botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_NON_COMBAT);
                acted = true;
            }

            managedState.priestShadowNonCombatEnabled = false;
        }
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        bool const shouldProvideTwinShadow = needTwinShadowResistance &&
                                             IsTwinSidePaladinProvider(bot, twinShadowContext);
        bool const hasShadowStrategyCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT);

        if (shouldProvideTwinShadow)
        {
            if (!hasShadowStrategyCombat)
            {
                botAI->ChangeStrategy("+rshadow", BotState::BOT_STATE_COMBAT);
                managedState.paladinShadowCombatEnabled = true;
                acted = true;
            }

            if (!botAI->HasAura("shadow resistance aura", bot))
                acted = botAI->DoSpecificAction("shadow resistance aura", Event(), true) || acted;
        }
        else if (managedState.paladinShadowCombatEnabled)
        {
            if (hasShadowStrategyCombat)
            {
                botAI->ChangeStrategy("-rshadow", BotState::BOT_STATE_COMBAT);
                acted = true;
            }

            managedState.paladinShadowCombatEnabled = false;
        }
    }

    if (!HasAnyManagedResistanceFlags(managedState))
        sManagedResistanceStateByBot.erase(bot->GetGUID().GetRawValue());

    if (acted)
    {
        std::ostringstream fields;
        fields << "boss=resistance nature=" << (needNatureResistance ? 1 : 0)
               << " shadow=" << (needTwinShadowResistance ? 1 : 0)
               << " shadow_mode=" << ToString(twinShadowContext.mode);
        Aq40Helpers::LogAq40Info(bot, "resistance_strategy",
            std::string("nature:") + (needNatureResistance ? "1" : "0") +
                ":shadow:" + (needTwinShadowResistance ? "1" : "0") +
                ":mode:" + ToString(twinShadowContext.mode),
            fields.str());
    }

    return acted;
}

bool Aq40ManageResistanceStrategiesAction::isUseful()
{
    if (!bot)
        return false;

    auto const managedStateItr = sManagedResistanceStateByBot.find(bot->GetGUID().GetRawValue());
    Aq40ManagedResistanceState const* managedState =
        managedStateItr != sManagedResistanceStateByBot.end() ? &managedStateItr->second : nullptr;

    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector const activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    bool const inAq40 = Aq40BossHelper::IsInAq40(bot);
    bool const needNatureResistance =
        inAq40 && Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits,
            { "princess huhuran", "viscidus", "glob of viscidus", "toxic slime" });
    Aq40TwinShadowResistanceContext const twinShadowContext = BuildTwinShadowResistanceContext(bot);

    if (bot->getClass() == CLASS_HUNTER)
    {
        bool const hasNatureStrategyCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_COMBAT);
        bool const hasNatureStrategyNonCombat = botAI->HasStrategy("rnature", BotState::BOT_STATE_NON_COMBAT);
        return (needNatureResistance &&
                (!hasNatureStrategyCombat || !hasNatureStrategyNonCombat || !botAI->HasAura("aspect of the wild", bot))) ||
               (!needNatureResistance && managedState &&
                (managedState->natureCombatEnabled || managedState->natureNonCombatEnabled));
    }

    if (bot->getClass() == CLASS_SHAMAN)
    {
        bool const hasNatureTotemStrategyCombat = botAI->HasStrategy("nature resistance", BotState::BOT_STATE_COMBAT);
        return (needNatureResistance &&
                (!hasNatureTotemStrategyCombat || !botAI->HasAura("nature resistance totem", bot))) ||
               (!needNatureResistance && managedState && managedState->shamanNatureCombatEnabled);
    }

    if (bot->getClass() == CLASS_PRIEST)
    {
        bool const isPriestProvider = twinShadowContext.mode == Aq40TwinShadowResistanceMode::PriestBuff &&
                                      twinShadowContext.priestProvider == bot;
        bool const hasShadowStrategyNonCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_NON_COMBAT);
        return (isPriestProvider && (!hasShadowStrategyNonCombat || HasTwinShadowProtectionMissing(botAI, twinShadowContext))) ||
               (!isPriestProvider && managedState && managedState->priestShadowNonCombatEnabled);
    }

    if (bot->getClass() == CLASS_PALADIN)
    {
        bool const isPaladinProvider = IsTwinSidePaladinProvider(bot, twinShadowContext);
        bool const hasShadowStrategyCombat = botAI->HasStrategy("rshadow", BotState::BOT_STATE_COMBAT);
        return (isPaladinProvider && (!hasShadowStrategyCombat || !botAI->HasAura("shadow resistance aura", bot))) ||
               (!isPaladinProvider && managedState && managedState->paladinShadowCombatEnabled);
    }

    return false;
}

bool Aq40EraseTimersAndTrackersAction::isUseful()
{
    return bot && bot->IsAlive() && Aq40BossHelper::IsInAq40(bot) &&
           Aq40Helpers::ShouldRunOutOfCombatMaintenance(bot, botAI);
}

bool Aq40EraseTimersAndTrackersAction::Execute(Event /*event*/)
{
    if (!bot || !Aq40BossHelper::IsInAq40(bot))
        return false;

    if (!Aq40Helpers::ShouldRunOutOfCombatMaintenance(bot, botAI))
        return false;

    bool const hadManagedResistance = Aq40Helpers::ClearManagedResistanceStrategies(bot, botAI);
    bool const hadTwinLocalCleanup = Aq40TwinEncounter::ClearTwinLocalCombatState(bot, botAI);
    bool const hadTwinWarlockTankOverlay = Aq40TwinEncounter::ClearTwinWarlockTankStrategy(bot);
    bool const hadPersistentEncounterState = Aq40Helpers::ResetEncounterState(bot);
    bool const recoveredDirtyState =
        hadManagedResistance || hadTwinLocalCleanup || hadTwinWarlockTankOverlay || hadPersistentEncounterState;

    if (hadTwinWarlockTankOverlay)
    {
        Aq40Helpers::LogAq40Info(bot, "twin_strategy",
            "twin:warlock_tank_overlay:cleanup",
            "boss=twin strategy=tank action=cleanup", 1000);
    }

    LogAq40CleanupTransition(bot, recoveredDirtyState);
    return true;
}

bool Aq40SkeramAcquirePlatformTargetAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, encounterUnits);
    if (!target)
        return false;

    if (!Aq40BossHelper::IsEncounterTank(bot, bot) && !Aq40BossActions::HasSkeramSkullTarget(botAI))
    {
        if (Aq40Helpers::IsSkeramPostBlinkHoldActive(bot, botAI, attackers))
            return false;

        if (!Aq40BossHelper::HasAnyNamedUnitHeldByEncounterTank(botAI, bot, encounterUnits, { "the prophet skeram" }, true))
            return false;
    }

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    // Encounter tank marks the real Skeram with skull so the raid can follow
    // through blinks without relying solely on tank aggro detection.
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        MarkTargetWithSkull(bot, target);
        Aq40Helpers::LogAq40Info(bot, "raid_marker", "skeram:skull:" + Aq40Helpers::GetAq40LogUnit(target),
            "boss=skeram marker=skull target=" + Aq40Helpers::GetAq40LogUnit(target));
    }

    Aq40Helpers::LogAq40Target(bot, "skeram", "platform", target);
    return Attack(target);
}

bool Aq40SkeramInterruptAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    std::vector<Unit*> skerams =
        Aq40BossActions::FindUnitsByAnyName(botAI, encounterUnits, { "the prophet skeram" });

    if (skerams.empty())
        return false;

    // If we are already targeting a casting Skeram, fire the interrupt directly.
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
    {
        for (Unit* skeram : skerams)
        {
            if (skeram == currentTarget && skeram->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                Aq40Helpers::LogAq40Info(bot, "interrupt", "skeram:" + Aq40Helpers::GetAq40LogUnit(skeram),
                    "boss=skeram target=" + Aq40Helpers::GetAq40LogUnit(skeram));
                return botAI->DoSpecificAction("interrupt spell", Event(), true);
            }
        }
    }

    // Otherwise switch to a casting Skeram; the interrupt fires next tick.
    Unit* target = nullptr;
    for (Unit* skeram : skerams)
    {
        if (!skeram)
            continue;

        if (skeram->GetCurrentSpell(CURRENT_GENERIC_SPELL))
        {
            target = skeram;
            break;
        }
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > 22.0f)
        return MoveNear(target, 18.0f, MovementPriority::MOVEMENT_COMBAT);

    Aq40Helpers::LogAq40Target(bot, "skeram", "interrupt", target);
    return Attack(target);
}

bool Aq40SkeramFocusRealBossAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, encounterUnits, true);

    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    // When a skull marker is present the target has already been validated as
    // the real boss — skip the tank-aggro prerequisite that frequently fails
    // after blinks.  Only apply the old guards when no skull exists.
    bool const hasSkullTarget = Aq40BossActions::HasSkeramSkullTarget(botAI);
    if (!Aq40BossHelper::IsEncounterTank(bot, bot) && !hasSkullTarget)
    {
        if (Aq40Helpers::IsSkeramPostBlinkHoldActive(bot, botAI, attackers))
            return false;

        if (!Aq40BossHelper::HasAnyNamedUnitHeldByEncounterTank(botAI, bot, encounterUnits, { "the prophet skeram" }, true))
            return false;
    }

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    Aq40Helpers::LogAq40Target(bot, "skeram", "execute", target);
    return Attack(target);
}

bool Aq40SkeramControlMindControlAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, attackers);

    if (Aq40BossHelper::TryCrowdControlCharmedPlayer(bot, botAI, encounterUnits))
    {
        Aq40Helpers::LogAq40Info(bot, "mind_control", "skeram:cc", "boss=skeram action=cc");
        return true;
    }

    // Fallback: force target back to Skeram.
    GuidVector skeramUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, skeramUnits);
    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    Aq40Helpers::LogAq40Target(bot, "skeram", "mc_fallback", target);
    return Attack(target);
}

bool Aq40SarturaChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (encounterUnits.empty())
        return false;

    Unit* sartura = Aq40BossActions::FindSarturaTarget(botAI, encounterUnits);
    std::vector<Unit*> guards = Aq40BossActions::FindSarturaGuards(botAI, encounterUnits);
    std::sort(guards.begin(), guards.end(), [](Unit* left, Unit* right)
    {
        if (!left || !right)
            return left != nullptr;
        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });

    Unit* target = nullptr;
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        if (Aq40BossHelper::IsEncounterPrimaryTank(bot, bot))
            target = sartura;
        else if (Aq40BossHelper::IsEncounterBackupTank(bot, bot, 0) && !guards.empty())
            target = guards[0];
        else if (Aq40BossHelper::IsEncounterBackupTank(bot, bot, 1) && guards.size() >= 2)
            target = guards[1];

        if (!target && !guards.empty())
        {
            target = guards.front();
            for (Unit* guard : guards)
            {
                if (guard && target && guard->GetHealthPct() < target->GetHealthPct())
                    target = guard;
            }
        }

        if (!target)
            target = sartura;
    }
    else
    {
        for (Unit* guard : guards)
        {
            if (Aq40BossHelper::IsUnitHeldByEncounterTank(bot, guard))
            {
                target = guard;
                break;
            }
        }

        if (!target && guards.empty() && sartura &&
            Aq40BossHelper::IsUnitHeldByEncounterTank(bot, sartura, true))
            target = sartura;
    }

    bool const targetIsGuard = target && botAI->EqualLowercaseName(target->GetName(), "sartura's royal guard");
    if (Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target, !targetIsGuard))
        return false;

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "sartura", targetIsGuard ? "guard" : "boss", target);
    return Attack(target);
}

bool Aq40SarturaAvoidWhirlwindAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* threat = nullptr;
    float closestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!Aq40BossHelper::IsSarturaSpinning(botAI, unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        bool const isCloser = distance < closestDistance;
        bool const isChasingBot = unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID();
        bool const currentThreatIsChasing = threat && (threat->GetVictim() == bot || threat->GetTarget() == bot->GetGUID());
        if (!threat || (isChasingBot && !currentThreatIsChasing) || (isChasingBot == currentThreatIsChasing && isCloser))
        {
            threat = unit;
            closestDistance = distance;
        }
    }
    if (!threat)
        return false;

    bool const isBackline = botAI->IsRanged(bot) || botAI->IsHeal(bot);
    bool const isChasingBot = threat->GetVictim() == bot || threat->GetTarget() == bot->GetGUID();
    float currentDistance = bot->GetDistance2d(threat);
    float desiredDistance = (isBackline && isChasingBot) ? 24.0f : 18.0f;
    if (currentDistance >= desiredDistance)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "sartura:whirlwind:" + Aq40Helpers::GetAq40LogUnit(threat),
        "boss=sartura hazard=whirlwind source=" + Aq40Helpers::GetAq40LogUnit(threat));
    return MoveAway(threat, desiredDistance - currentDistance);
}

bool Aq40FankrissChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (encounterUnits.empty())
        return false;

    Unit* target = nullptr;
    Unit* fankriss = Aq40BossActions::FindFankrissTarget(botAI, encounterUnits);
    std::vector<Unit*> spawns = Aq40BossActions::FindFankrissSpawns(botAI, encounterUnits);
    if (!spawns.empty())
    {
        std::sort(spawns.begin(), spawns.end(), [](Unit* left, Unit* right)
        {
            if (!left || !right)
                return left != nullptr;
            return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
        });

        if (Aq40BossHelper::IsEncounterTank(bot, bot))
        {
            bool const hasBossAggro = fankriss && Aq40BossHelper::IsUnitFocusedOnPlayer(fankriss, bot);
            if (hasBossAggro)
                target = fankriss;
            else
            {
                uint32 assignedIndex = 0;
                if (Aq40BossHelper::IsEncounterBackupTank(bot, bot, 0))
                    assignedIndex = 1;
                else if (Aq40BossHelper::IsEncounterBackupTank(bot, bot, 1))
                    assignedIndex = 2;

                if (assignedIndex < spawns.size())
                    target = spawns[assignedIndex];
                else if (!spawns.empty())
                    target = spawns.back();
            }
        }
        else if (!botAI->IsRanged(bot) && !botAI->IsHeal(bot))
        {
            target = fankriss;
        }
        else
        {
            // Ranged/healers prefer a tank-held spawn, but fall back to any spawn
            // to avoid the deadlock where nobody attacks spawns because no tank
            // has picked one up yet.
            for (Unit* spawn : spawns)
            {
                if (Aq40BossHelper::IsUnitHeldByEncounterTank(bot, spawn))
                {
                    target = spawn;
                    break;
                }
            }

            if (!target)
                target = spawns.front();
        }
    }
    else
    {
        target = fankriss;
    }

    // Fall back to Fankriss if no target was resolved (e.g. tank index overshoot).
    if (!target)
        target = fankriss;

    bool const targetIsSpawn = target && botAI->EqualLowercaseName(target->GetName(), "spawn of fankriss");
    if (Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target, !targetIsSpawn))
        return false;

    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "fankriss", targetIsSpawn ? "spawn" : "boss", target);
    return Attack(target);
}

bool Aq40TrashChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector const& attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    if (activeUnits.empty())
        return false;

    Unit* target = Aq40BossActions::FindTrashTarget(botAI, activeUnits);
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "trash", "priority", target);
    return Attack(target);
}

bool Aq40TrashChooseTargetAction::isUseful()
{
    GuidVector const& attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    if (activeUnits.empty())
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || !currentTarget->IsAlive())
        return true;

    if (!Aq40BossHelper::IsUnitNamedAny(botAI, currentTarget, { "anubisath defender" }))
        return true;

    for (ObjectGuid const guid : activeUnits)
    {
        if (guid == currentTarget->GetGUID())
            return false;
    }

    return true;
}

bool Aq40TrashAvoidDangerousAoeAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    // Plague separation path — applies to all non-tank roles.
    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        float separationNeeded = 0.0f;
        Unit* separationRisk = FindClosestAq40PlagueSeparationRisk(bot, botAI, separationNeeded);
        if (!separationRisk || separationNeeded <= 0.0f)
            return false;

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());

        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "trash:plague:" + Aq40Helpers::GetAq40LogUnit(separationRisk),
            "boss=trash hazard=plague source=" + Aq40Helpers::GetAq40LogUnit(separationRisk));
        return MoveAway(separationRisk, separationNeeded);
    }

    // Only ranged and healers reposition for Defender Thunderclap; melee stay on target.
    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* danger = nullptr;
    std::string dangerKind;
    float highestThreatGap = 0.0f;

    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }))
        {
            float const gap = 24.0f - bot->GetDistance2d(unit);
            if (gap > highestThreatGap)
            {
                highestThreatGap = gap;
                danger = unit;
                dangerKind = "thunderclap";
            }
        }
    }

    if (!danger || highestThreatGap <= 0.0f)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    Aq40TankRetreatResult retreat = ComputeTankRetreatPosition(bot, danger, highestThreatGap + 2.0f);
    if (retreat.valid)
    {
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "trash:" + dangerKind + ":" + Aq40Helpers::GetAq40LogUnit(danger),
            "boss=trash hazard=" + dangerKind + " source=" + Aq40Helpers::GetAq40LogUnit(danger));
        return MoveTo(bot->GetMapId(), retreat.x, retreat.y, retreat.z,
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    Aq40Helpers::LogAq40Warn(bot, "movement_failure",
        "trash:" + dangerKind + ":" + Aq40Helpers::GetAq40LogUnit(danger),
        "boss=trash hazard=" + dangerKind + " reason=no_safe_retreat source=" + Aq40Helpers::GetAq40LogUnit(danger));
    return false;
}

bool Aq40TrashAvoidDangerousAoeAction::isUseful()
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        float separationNeeded = 0.0f;
        return FindClosestAq40PlagueSeparationRisk(bot, botAI, separationNeeded) != nullptr &&
               separationNeeded > 0.0f;
    }

    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }) &&
            bot->GetDistance2d(unit) < 24.0f)
            return true;
    }

    return false;
}
