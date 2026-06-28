#include "Aq40Triggers.h"

#include "ObjectGuid.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "Action/Aq40Actions.h"
#include "Aq40SpellIds.h"
#include "Util/Aq40Helpers_Cthun.h"
#include "Util/Aq40Helpers_Shared.h"
#include "Util/Aq40Helpers_Skeram.h"
#include "Aq40Scripts.h"

namespace
{
bool Aq40EncounterEngaged(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !botAI->GetAiObjectContext() || !Aq40BossHelper::IsInAq40(bot))
        return false;

    if (!botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get().empty())
        return true;

    return Aq40BossHelper::IsEncounterCombatActive(bot);
}

Unit* FindSelectableCthunBody(PlayerbotAI* botAI, GuidVector const& attackers)
{
    Unit* cthun = Aq40BossHelper::FindUnitByAnyName(botAI, attackers, { "c'thun" });
    if (!cthun || (cthun->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) == UNIT_FLAG_NOT_SELECTABLE)
        return nullptr;

    return cthun;
}

bool IsTwinActive(PlayerbotAI* botAI, Player* bot)
{
    if (!Aq40EncounterEngaged(botAI, bot))
        return false;

    GuidVector encounterUnits =
        Aq40BossHelper::GetActiveCombatUnits(botAI, botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
    return Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits) ||
           Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
}

bool HasTrackedTwinExplodeBugHazard(Player* bot, PlayerbotAI* botAI, float maxDistance)
{
    if (!bot || !botAI)
        return false;

    ObjectGuid sourceGuid;
    Position sourcePosition;
    if (!Aq40Scripts::GetTwinExplodeBugSource(bot, sourceGuid, sourcePosition))
        return false;

    if (!sourceGuid.IsEmpty())
    {
        if (Unit* trackedBug = botAI->GetUnit(sourceGuid);
            trackedBug && trackedBug->IsAlive() && trackedBug->IsInWorld() &&
            Aq40SpellIds::IsTwinBugEntry(trackedBug->GetEntry()) && bot->GetDistance2d(trackedBug) <= maxDistance)
        {
            return true;
        }
    }

    return bot->GetExactDist2d(sourcePosition.GetPositionX(), sourcePosition.GetPositionY()) <= maxDistance;
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

    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

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

        if (unit->GetHealthPct() <= 32.0f)
            return true;

        if (Aq40SpellIds::HasAnyAura(botAI, unit, { Aq40SpellIds::HuhuranFrenzy }))
            return true;
    }

    return false;
}

bool Aq40TwinActiveTrigger::IsActive()
{
    return IsTwinActive(botAI, bot);
}

bool Aq40TwinTeleportTrigger::IsActive()
{
    return IsTwinActive(botAI, bot) && Aq40Scripts::IsTwinTeleportPickupWindow(bot);
}

bool Aq40TwinHazardTrigger::IsActive()
{
    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::TwinBlizzard }))
        return true;

    if (!IsTwinActive(botAI, bot))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    if (Aq40BossHelper::Twin::FindNearestBug(bot, botAI, encounterUnits, 17.0f, true))
        return true;

    if (HasTrackedTwinExplodeBugHazard(bot, botAI, 17.0f))
        return true;

    return Aq40Scripts::IsTwinBlizzardWindow(bot);
}

bool Aq40TwinVeklorRangeTrigger::IsActive()
{
    if (!IsTwinActive(botAI, bot) || Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    if (!veklor)
        return false;

    float const distance = bot->GetDistance2d(veklor);
    if (distance <= 18.0f)
        return true;

    return distance <= 24.0f && Aq40Scripts::IsTwinArcaneBurstWindow(bot);
}

bool Aq40TwinBugTrigger::IsActive()
{
    if (!IsTwinActive(botAI, bot) || botAI->IsHeal(bot))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, AI_VALUE(GuidVector, "attackers"));
    return Aq40BossHelper::Twin::FindNearestBug(
        bot, botAI, encounterUnits, bot->getClass() == CLASS_HUNTER ? 30.0f : 24.0f) != nullptr;
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
