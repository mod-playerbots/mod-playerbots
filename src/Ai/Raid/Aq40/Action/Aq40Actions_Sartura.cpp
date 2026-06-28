#include "Aq40Actions.h"

#include <algorithm>
#include <limits>

#include "../Util/Aq40Helpers_Shared.h"
namespace Aq40BossActions
{
Unit* FindSarturaTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitByAnyName(botAI, attackers, { "battleguard sartura" });
}

std::vector<Unit*> FindSarturaGuards(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitsByAnyName(botAI, attackers, { "sartura's royal guard" });
}
}    // namespace Aq40BossActions

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
