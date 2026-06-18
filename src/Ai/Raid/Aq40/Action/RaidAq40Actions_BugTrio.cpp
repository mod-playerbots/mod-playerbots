#include "RaidAq40Actions.h"

#include <cmath>

#include "../RaidAq40BossHelper.h"
#include "../RaidAq40SpellIds.h"
#include "../Util/RaidAq40Helpers_Shared.h"
#include "../../RaidBossHelpers.h"

namespace Aq40BossActions
{
Unit* FindBugTrioUnit(PlayerbotAI* botAI, GuidVector const& units, char const* name)
{
    if (!botAI)
        return nullptr;

    return Aq40BossHelper::FindUnitByAnyName(botAI, units, { name });
}

Unit* FindBugTrioTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    Unit* yauj = FindBugTrioUnit(botAI, attackers, "princess yauj");
    if (yauj)
        return yauj;

    Unit* vem = FindBugTrioUnit(botAI, attackers, "vem");
    if (vem)
        return vem;

    return FindBugTrioUnit(botAI, attackers, "lord kri");
}

Unit* FindBugTrioTankOwnedTarget(PlayerbotAI* botAI, Player* bot, GuidVector const& attackers)
{
    Unit* yauj = FindBugTrioUnit(botAI, attackers, "princess yauj");
    if (yauj && Aq40BossHelper::IsUnitHeldByEncounterTank(bot, yauj))
        return yauj;

    Unit* vem = FindBugTrioUnit(botAI, attackers, "vem");
    if (vem && Aq40BossHelper::IsUnitHeldByEncounterTank(bot, vem))
        return vem;

    Unit* kri = FindBugTrioUnit(botAI, attackers, "lord kri");
    if (kri && Aq40BossHelper::IsUnitHeldByEncounterTank(bot, kri))
        return kri;

    return nullptr;
}
}    // namespace Aq40BossActions

namespace
{
GuidVector GetBugTrioCombatUnits(PlayerbotAI* botAI)
{
    if (!botAI)
        return {};

    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    return Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
}

GuidVector GetBugTrioEncounterUnits(PlayerbotAI* botAI)
{
    if (!botAI)
        return {};

    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    return Aq40BossHelper::GetEncounterUnits(botAI, attackers);
}
}    // namespace

bool Aq40BugTrioChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector const activeUnits = GetBugTrioCombatUnits(botAI);
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "princess yauj", "vem", "lord kri", "yauj brood" }))
        return false;

    GuidVector const encounterUnits = GetBugTrioEncounterUnits(botAI);

    Unit* target = nullptr;
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        // Tank picks the highest kill-order target and marks it with skull.
        // The base DpsAssist system will auto-focus skull for all DPS.
        target = Aq40BossActions::FindBugTrioTarget(botAI, encounterUnits);

        if (target)
        {
            MarkTargetWithSkull(bot, target);
            Aq40Helpers::LogAq40Info(bot, "raid_marker",
                "bug_trio:skull:" + Aq40Helpers::GetAq40LogUnit(target),
                "boss=bug_trio marker=skull target=" + Aq40Helpers::GetAq40LogUnit(target));
        }
    }
    else
    {
        // Non-tanks follow a tank-held target in kill order.
        // Skull preference is already handled by the base DpsTargetValue,
        // but we still need explicit targeting here for the wait-for-aggro gate.
        target = Aq40BossActions::FindBugTrioTankOwnedTarget(botAI, bot, encounterUnits);
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "bug_trio",
        Aq40BossHelper::IsEncounterTank(bot, bot) ? "kill_order" : "tank_held", target);
    return Attack(target);
}

bool Aq40BugTrioInterruptHealAction::Execute(Event /*event*/)
{
    GuidVector const activeUnits = GetBugTrioCombatUnits(botAI);
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "princess yauj", "vem", "lord kri", "yauj brood" }))
        return false;

    GuidVector const encounterUnits = GetBugTrioEncounterUnits(botAI);
    Unit* yauj = Aq40BossActions::FindBugTrioUnit(botAI, encounterUnits, "princess yauj");
    if (!yauj)
        return false;

    Spell* spell = yauj->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!spell || !Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::BugTrioYaujHeal }))
        return false;

    if (AI_VALUE(Unit*, "current target") != yauj)
    {
        Aq40Helpers::LogAq40Target(bot, "bug_trio", "interrupt_setup", yauj);
        return Attack(yauj);
    }

    Aq40Helpers::LogAq40Info(bot, "interrupt",
        "bug_trio:" + Aq40Helpers::GetAq40LogUnit(yauj),
        "boss=bug_trio target=" + Aq40Helpers::GetAq40LogUnit(yauj));
    return botAI->DoSpecificAction("interrupt spell", Event(), true);
}

bool Aq40BugTrioAvoidPoisonCloudAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector const activeUnits = GetBugTrioCombatUnits(botAI);
    if (!Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "princess yauj", "vem", "lord kri", "yauj brood" }))
        return false;

    GuidVector const encounterUnits = GetBugTrioEncounterUnits(botAI);
    Unit* kri = Aq40BossActions::FindBugTrioUnit(botAI, encounterUnits, "lord kri");
    if (!kri)
        return false;

    bool poisonCloudWindow = kri->GetHealthPct() <= 5.0f ||
                             Aq40SpellIds::HasAnyAura(botAI, kri, { Aq40SpellIds::BugTrioPoisonCloud });
    if (!poisonCloudWindow)
        return false;

    float currentDistance = bot->GetDistance2d(kri);
    float desiredDistance = 18.0f;
    if (currentDistance >= desiredDistance)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "bug_trio:poison_cloud:" + Aq40Helpers::GetAq40LogUnit(kri),
        "boss=bug_trio hazard=poison_cloud source=" + Aq40Helpers::GetAq40LogUnit(kri));
    return MoveAway(kri, desiredDistance - currentDistance);
}
