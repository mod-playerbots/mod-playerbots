#include "RaidAq40Triggers.h"

#include <initializer_list>
#include <limits>

#include "ObjectGuid.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "../Action/RaidAq40Actions.h"
#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Cthun.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "../Util/RaidAq40Helpers_Skeram.h"
#include "../Util/RaidAq40TwinEncounter.h"

namespace
{
bool Aq40EncounterEngaged(PlayerbotAI* botAI, Player* bot)
{
    if (!Aq40BossHelper::IsInAq40(bot))
        return false;

    // Primary check: the bot's own attackers list.
    if (!botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get().empty())
        return true;

    // Fallback: a cluster of nearby group members is in combat, so the
    // encounter is active even though this bot briefly dropped threat.
    // Uses the same cluster check as shared state cleanup so both agree.
    return Aq40BossHelper::IsEncounterCombatActive(bot);
}

// FindBurrowedOuro now lives in Aq40BossHelper.

Unit* FindSelectableCthunBody(PlayerbotAI* botAI, GuidVector const& attackers)
{
    Unit* cthun = Aq40BossHelper::FindUnitByAnyName(botAI, attackers, { "c'thun" });
    if (!cthun || (cthun->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == UNIT_FLAG_NOT_SELECTABLE)
        return nullptr;

    return cthun;
}

// IsSarturaMob / IsSarturaSpinning now live in Aq40BossHelper.

Aq40TwinEncounter::TwinEncounterState const* GetTwinEncounterState(Player* bot)
{
    return bot ? Aq40TwinEncounter::GetEncounterState(bot) : nullptr;
}

Unit* FindTwinUnitByEntry(PlayerbotAI* botAI, GuidVector const& units, uint32 entry)
{
    if (!botAI)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
        return nullptr;

    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->IsInWorld() || unit->IsFriendlyTo(bot) ||
            unit->GetMapId() != bot->GetMapId())
        {
            continue;
        }

        if (unit->GetEntry() == entry)
            return unit;
    }

    return nullptr;
}

bool IsTwinExplodeBugCast(Spell const* spell)
{
    return spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::TwinExplodeBug });
}

Unit* FindClosestTwinExplodeBug(Player* bot, PlayerbotAI* botAI, GuidVector const& units, float maxDistance)
{
    if (!bot || !botAI)
        return nullptr;

    Unit* nearestBug = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();
    for (ObjectGuid const guid : units)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !Aq40SpellIds::IsTwinBugEntry(unit->GetEntry()))
            continue;

        if (!IsTwinExplodeBugCast(unit->GetCurrentSpell(CURRENT_GENERIC_SPELL)))
            continue;

        float const distance = bot->GetDistance2d(unit);
        if (distance > maxDistance || distance >= nearestDistance)
            continue;

        nearestBug = unit;
        nearestDistance = distance;
    }

    return nearestBug;
}

bool HasTwinExplodeBugHazard(Player* bot, PlayerbotAI* botAI, Aq40TwinEncounter::TwinEncounterState const& state,
                             GuidVector const& units, float maxDistance, bool allowTrackedSource)
{
    if (!bot || !botAI)
        return false;

    if (allowTrackedSource)
    {
        ObjectGuid const trackedSourceGuid = Aq40TwinEncounter::GetExplodeBugSourceGuid(state);
        if (!trackedSourceGuid.IsEmpty())
        {
            if (Unit* trackedBug = botAI->GetUnit(trackedSourceGuid);
                trackedBug && trackedBug->IsAlive() && trackedBug->IsInWorld() &&
                Aq40SpellIds::IsTwinBugEntry(trackedBug->GetEntry()) && bot->GetDistance2d(trackedBug) <= maxDistance)
            {
                return true;
            }

            Position const& trackedPosition = Aq40TwinEncounter::GetExplodeBugSourcePosition(state);
            if (bot->GetExactDist2d(trackedPosition.GetPositionX(), trackedPosition.GetPositionY()) <= maxDistance)
                return true;
        }
    }

    return FindClosestTwinExplodeBug(bot, botAI, units, maxDistance) != nullptr;
}

bool IsTwinPrimaryTankController(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state)
{
    if (!bot)
        return false;

    Aq40TwinEncounter::TwinRoleAssignment const* assignment =
        Aq40TwinEncounter::GetAssignmentForMember(state, bot->GetGUID());
    if (!assignment)
        return false;

    if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::WarlockTank)
    {
        return Aq40TwinEncounter::IsPrimaryController(
            state, Aq40TwinEncounter::TwinBoss::Veklor, assignment->memberGuid);
    }

    if (assignment->cohort == Aq40TwinEncounter::TwinRoleCohort::MeleeTank)
    {
        return Aq40TwinEncounter::IsPrimaryController(
            state, Aq40TwinEncounter::TwinBoss::Veknilash, assignment->memberGuid);
    }

    return false;
}

bool IsTwinAssignedOrLockedParticipant(Player* bot, Aq40TwinEncounter::TwinEncounterState const& state)
{
    return Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot) ||
           Aq40TwinEncounter::IsTwinAssignedParticipant(state, bot);
}
}    // namespace

bool Aq40BotIsNotInCombatTrigger::IsActive()
{
    if (!bot || !bot->IsAlive() || bot->IsInCombat() || Aq40BossHelper::IsEncounterCombatActive(bot))
        return false;

    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    if (!Aq40BossHelper::GetActiveCombatUnits(botAI, attackers).empty())
        return false;

    if (Aq40Helpers::IsSkeramEncounterLive(bot, botAI, attackers))
        return false;

    return Aq40Helpers::ShouldRunOutOfCombatMaintenance(bot, botAI);
}

bool Aq40ResistanceStrategyTrigger::IsActive()
{
    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    return Aq40Helpers::IsResistanceManagementNeeded(bot, botAI, attackers);
}

bool Aq40SkeramActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    return Aq40Helpers::IsSkeramEncounterLive(bot, botAI, AI_VALUE(GuidVector, "attackers"));
}

bool Aq40SkeramBlinkTrigger::IsActive()
{
    if (!Aq40SkeramActiveTrigger(botAI).IsActive())
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || !botAI->EqualLowercaseName(currentTarget->GetName(), "the prophet skeram"))
        return false;

    return !AI_VALUE2(bool, "has aggro", "current target");
}

bool Aq40SkeramArcaneExplosionTrigger::IsActive()
{
    if (!Aq40SkeramActiveTrigger(botAI).IsActive())
        return false;

    GuidVector skeramUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : skeramUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "the prophet skeram"))
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::SkeramArcaneExplosion }))
            return true;
    }

    return false;
}

bool Aq40SkeramMindControlTrigger::IsActive()
{
    if (!Aq40SkeramActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

    // "True Fulfillment" can force players/bots into hostile behavior.
        if (unit->IsPlayer() &&
            (unit->IsCharmed() ||
             Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::SkeramTrueFulfillment }) ||
             botAI->HasAura("true fulfillment", unit)))
            return true;
    }

    return false;
}

bool Aq40SkeramExecutePhaseTrigger::IsActive()
{
    if (!Aq40SkeramActiveTrigger(botAI).IsActive())
        return false;

    GuidVector skeramUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : skeramUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (Aq40BossHelper::IsUnitNamedAny(botAI, unit, { "the prophet skeram" }) && unit->GetHealthPct() <= 25.0f)
            return true;
    }

    return false;
}

bool Aq40SarturaActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        if (botAI->EqualLowercaseName(unit->GetName(), "battleguard sartura") ||
            botAI->EqualLowercaseName(unit->GetName(), "sartura's royal guard"))
            return true;
    }

    return false;
}

bool Aq40SarturaWhirlwindTrigger::IsActive()
{
    if (!Aq40SarturaActiveTrigger(botAI).IsActive() || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    bool const isBackline = botAI->IsRanged(bot) || botAI->IsHeal(bot);
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!Aq40BossHelper::IsSarturaSpinning(botAI, unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        bool const isClosingOnBot = unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID();
        if (distance <= 18.0f || (isBackline && isClosingOnBot && distance <= 24.0f))
            return true;
    }

    return false;
}

bool Aq40BugTrioActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits, { "princess yauj", "vem", "lord kri", "yauj brood" });
}

bool Aq40BugTrioHealCastTrigger::IsActive()
{
    if (!Aq40BugTrioActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* yauj = Aq40BossHelper::FindUnitByAnyName(botAI, encounterUnits, { "princess yauj" });
    if (!yauj)
        return false;

    Spell* spell = yauj->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    return spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::BugTrioYaujHeal });
}

bool Aq40BugTrioFearTrigger::IsActive()
{
    if (!Aq40BugTrioActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* yauj = Aq40BossHelper::FindUnitByAnyName(botAI, encounterUnits, { "princess yauj" });
    if (!yauj)
        return false;

    Spell* spell = yauj->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::BugTrioYaujFear }))
        return true;

    Group const* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        if (bot->GetDistance2d(member) > 30.0f)
            continue;

        if (Aq40SpellIds::HasAnyAura(botAI, member, { Aq40SpellIds::BugTrioYaujFear }) || member->HasFearAura())
            return true;
    }

    return false;
}

bool Aq40BugTrioPoisonCloudTrigger::IsActive()
{
    if (!Aq40BugTrioActiveTrigger(botAI).IsActive() || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* kri = Aq40BossHelper::FindUnitByAnyName(botAI, encounterUnits, { "lord kri" });
    if (!kri)
        return false;

    bool poisonCloudWindow = kri->GetHealthPct() <= 5.0f ||
                             Aq40SpellIds::HasAnyAura(botAI, kri, { Aq40SpellIds::BugTrioPoisonCloud });
    return poisonCloudWindow && bot->GetDistance2d(kri) <= 12.0f;
}

bool Aq40FankrissActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && botAI->EqualLowercaseName(unit->GetName(), "fankriss the unyielding"))
            return true;
    }

    return false;
}

bool Aq40FankrissSpawnedTrigger::IsActive()
{
    if (!Aq40FankrissActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && botAI->EqualLowercaseName(unit->GetName(), "spawn of fankriss"))
            return true;
    }

    return false;
}

bool Aq40FankrissMortalWoundTrigger::IsActive()
{
    if (!Aq40FankrissActiveTrigger(botAI).IsActive())
        return false;

    // Only fire for tanks that are currently tanking Fankriss with dangerous Mortal Wound stacks.
    if (!Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* fankriss = Aq40BossActions::FindFankrissTarget(botAI, encounterUnits);
    if (!fankriss || !Aq40BossHelper::IsUnitFocusedOnPlayer(fankriss, bot))
        return false;

    Aura* mortalWound = bot->GetAura(Aq40SpellIds::FankrissMortalWound);
    return mortalWound && mortalWound->GetStackAmount() >= 5;
}

bool Aq40TrashActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    if (Aq40Helpers::IsSkeramEncounterLive(bot, botAI, attackers))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    if (encounterUnits.empty() || Aq40BossHelper::IsBossEncounterActive(botAI, encounterUnits))
        return false;

    return Aq40BossHelper::IsTrashEncounterActive(botAI, encounterUnits);
}

bool Aq40TrashDangerousAoeTrigger::IsActive()
{
    if (!Aq40TrashActiveTrigger(botAI).IsActive() || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
        return true;

    // Only ranged and healers reposition for trash AoE; melee stay on target.
    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        // Defender Thunderclap: reposition ranged/healers within 24y
        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }) &&
            bot->GetDistance2d(unit) <= 24.0f)
            return true;
    }

    return false;
}

bool Aq40HuhuranActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && botAI->EqualLowercaseName(unit->GetName(), "princess huhuran"))
            return true;
    }

    return false;
}

bool Aq40HuhuranPoisonPhaseTrigger::IsActive()
{
    if (!Aq40HuhuranActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "princess huhuran"))
            continue;

    // Phase transition baseline:
    // spread ranged during the dangerous poison volley/enrage window.
        if (unit->GetHealthPct() <= 32.0f)
            return true;

        if (Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::HuhuranFrenzy }))
            return true;
    }

    return false;
}

bool Aq40TwinPrePullReadyTrigger::IsActive()
{
    return !bot->IsInCombat() && Aq40TwinEncounter::IsTwinPrePullStageWindow(bot);
}

bool Aq40TwinApproachTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    return !bot->IsInCombat() && state && Aq40TwinEncounter::IsTwinApproachWindow(*state, bot);
}

bool Aq40TwinDualPullTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    return state && state->phase == Aq40TwinEncounter::TwinEncounterPhase::DualPullWindow &&
           Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
}

bool Aq40TwinSwapPrepTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    return state && Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot) &&
           Aq40TwinEncounter::IsActivePhase(state->phase) && !Aq40TwinEncounter::IsTerminalPhase(state->phase) &&
           Aq40TwinEncounter::IsSwapPrepActive(*state);
}

bool Aq40TwinActiveTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    return state && Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot) &&
           Aq40TwinEncounter::IsActivePhase(state->phase) &&
           !Aq40TwinEncounter::IsTerminalPhase(state->phase);
}

bool Aq40TwinBlizzardTrigger::IsActive()
{
    if (!Aq40TwinActiveTrigger(botAI).IsActive())
        return false;

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::TwinBlizzard }) || botAI->HasAura("blizzard", bot))
        return true;

    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    return state && Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::Blizzard, 5000);
}

bool Aq40TwinExplodeBugTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    if (!Aq40TwinActiveTrigger(botAI).IsActive() || !state || IsTwinPrimaryTankController(bot, *state))
        return false;

    bool const scriptedWindow = Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::ExplodeBug, 2500);
    GuidVector const encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (!HasTwinExplodeBugHazard(bot, botAI, *state, encounterUnits, 17.0f, scriptedWindow))
        return false;

    if (scriptedWindow)
        return true;

    return FindClosestTwinExplodeBug(bot, botAI, encounterUnits, 17.0f) != nullptr;
}

bool Aq40TwinArcaneBurstRiskTrigger::IsActive()
{
    if (!Aq40TwinActiveTrigger(botAI).IsActive())
        return false;

    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    if (state && Aq40TwinEncounter::IsTwinDesignatedWarlockTank(bot) &&
        Aq40TwinEncounter::IsPrimaryController(*state, Aq40TwinEncounter::TwinBoss::Veklor, bot->GetGUID()))
    {
        return false;
    }

    GuidVector const encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* veklor = FindTwinUnitByEntry(botAI, encounterUnits, Aq40SpellIds::TwinVeklorNpcEntry);
    if (!veklor)
        return false;

    float const distance = bot->GetDistance2d(veklor);
    if (distance <= 18.0f)
        return true;

    return state && distance <= 24.0f && Aq40TwinEncounter::IsScriptedEventActive(
        *state, Aq40TwinEncounter::TwinScriptedEvent::ArcaneBurst, 2500);
}

bool Aq40TwinSplitRiskTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    if (!state || Aq40TwinEncounter::IsTerminalPhase(state->phase) ||
        !IsTwinAssignedOrLockedParticipant(bot, *state))
        return false;

    if (state->phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
        state->phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery ||
        state->phase == Aq40TwinEncounter::TwinEncounterPhase::Degraded)
    {
        return false;
    }

    return state->recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Warning ||
           state->recovery.splitBand == Aq40TwinEncounter::TwinSplitBand::Urgent;
}

bool Aq40TwinPostSwapHoldTrigger::IsActive()
{
    Aq40TwinEncounter::TwinEncounterState const* state = GetTwinEncounterState(bot);
    if (!state || Aq40TwinEncounter::IsTerminalPhase(state->phase))
        return false;

    bool const hasLockedPickupAnchor = Aq40TwinEncounter::HasActiveLockedPickupAnchor(bot);
    bool const assignedParticipant = Aq40TwinEncounter::IsTwinAssignedParticipant(*state, bot);
    if (!hasLockedPickupAnchor && !assignedParticipant)
        return false;

    bool const postSwapPhase = state->phase == Aq40TwinEncounter::TwinEncounterPhase::TeleportWindow ||
                               state->phase == Aq40TwinEncounter::TwinEncounterPhase::PickupRecovery;
    bool const threatHoldWindow = assignedParticipant && Aq40TwinEncounter::IsAnyThreatHoldWindowActive(*state);
    return (postSwapPhase || threatHoldWindow) && (hasLockedPickupAnchor || threatHoldWindow);
}

bool Aq40OuroActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits, { "ouro" });
}

bool Aq40OuroScarabsTrigger::IsActive()
{
    if (!Aq40OuroActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits, { "qiraji scarab", "scarab" });
}

bool Aq40OuroSweepTrigger::IsActive()
{
    if (!Aq40OuroActiveTrigger(botAI).IsActive() || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "ouro"))
            continue;

    // Detect actual Sweep cast or aura, matching the pattern used by
    // Sartura whirlwind detection (spell + aura + name fallback).
        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        bool const sweeping =
            (spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::OuroSweep })) ||
            Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::OuroSweep }) ||
            botAI->HasAura("sweep", unit);
        if (!sweeping)
            continue;

        if (bot->GetDistance2d(unit) <= 10.0f)
            return true;
    }

    return false;
}

bool Aq40OuroSandBlastRiskTrigger::IsActive()
{
    if (!Aq40OuroActiveTrigger(botAI).IsActive() || Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "ouro"))
            continue;

    // Non-tanks in Ouro's frontal arc are at Sand Blast risk
    // (pattern from ICC Marrowgar: boss->isInFront(bot)).
        if (unit->isInFront(bot, 10.0f) && bot->GetDistance2d(unit) <= 15.0f)
            return true;
    }

    return false;
}

bool Aq40OuroSubmergeTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (Unit* ouro = Aq40BossHelper::FindBurrowedOuro(botAI, encounterUnits))
        return bot->GetDistance2d(ouro) <= 16.0f;

    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "dirt mound"))
            continue;

        if (bot->GetDistance2d(unit) <= 16.0f)
            return true;
    }

    return false;
}

bool Aq40ViscidusActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits, { "viscidus", "glob of viscidus", "toxic slime" });
}

bool Aq40ViscidusFrozenTrigger::IsActive()
{
    if (!Aq40ViscidusActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !botAI->EqualLowercaseName(unit->GetName(), "viscidus"))
            continue;

        if (Aq40SpellIds::HasAnyAura(botAI, unit,
                { Aq40SpellIds::ViscidusFreeze }))
            return true;
    }

    return false;
}

bool Aq40ViscidusGlobTrigger::IsActive()
{
    if (!Aq40ViscidusActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits, { "glob of viscidus" });
}

bool Aq40CthunActiveTrigger::IsActive()
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits,
                                           { "c'thun", "eye of c'thun", "eye tentacle", "claw tentacle",
                                             "giant eye tentacle", "giant claw tentacle", "flesh tentacle" });
}

bool Aq40CthunPhase2Trigger::IsActive()
{
    if (!Aq40CthunActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (FindSelectableCthunBody(botAI, encounterUnits))
        return true;

    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits,
                                           { "giant eye tentacle", "giant claw tentacle", "flesh tentacle" });
}

bool Aq40CthunAddsPresentTrigger::IsActive()
{
    if (!Aq40CthunActiveTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::HasAnyNamedUnit(botAI, encounterUnits,
                                           { "eye tentacle", "claw tentacle", "giant eye tentacle", "giant claw tentacle",
                                             "flesh tentacle" });
}

bool Aq40CthunDarkGlareTrigger::IsActive()
{
    if (!Aq40CthunActiveTrigger(botAI).IsActive())
        return false;

    if (Aq40CthunInStomachTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        if (!botAI->EqualLowercaseName(unit->GetName(), "eye of c'thun"))
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        bool castingDarkGlare =
            spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::CthunDarkGlare });
        bool hasDarkGlare = Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::CthunDarkGlare }) ||
                            botAI->HasAura("dark glare", unit);
        if (castingDarkGlare || hasDarkGlare)
            return true;
    }

    return false;
}

bool Aq40CthunInStomachTrigger::IsActive()
{
    return Aq40Helpers::IsCthunInStomach(bot, botAI);
}

bool Aq40CthunVulnerableTrigger::IsActive()
{
    if (!Aq40CthunPhase2Trigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40Helpers::IsCthunVulnerableNow(botAI, encounterUnits);
}

bool Aq40CthunEyeCastTrigger::IsActive()
{
    if (!Aq40CthunActiveTrigger(botAI).IsActive() || Aq40CthunInStomachTrigger(botAI).IsActive())
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        bool isEyeTentacle = botAI->EqualLowercaseName(unit->GetName(), "eye tentacle") ||
                             botAI->EqualLowercaseName(unit->GetName(), "giant eye tentacle");
        Spell* spell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        bool eyeCast = spell && Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::CthunMindFlay });
        if (isEyeTentacle && eyeCast)
            return true;
    }

    return false;
}
