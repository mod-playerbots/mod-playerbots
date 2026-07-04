#include "Aq40Actions.h"

#include <algorithm>

#include "../Util/Aq40Helpers_Shared.h"

namespace Aq40BossActions
{
Unit* FindFankrissTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitByAnyName(botAI, attackers, { "fankriss the unyielding" });
}

std::vector<Unit*> FindFankrissSpawns(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitsByAnyName(botAI, attackers, { "spawn of fankriss" });
}
}    // namespace Aq40BossActions

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

bool Aq40FankrissTankSwapAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* fankriss = Aq40BossActions::FindFankrissTarget(botAI, encounterUnits);
    if (!fankriss)
        return false;

    bool const hasBossAggro = Aq40BossHelper::IsUnitFocusedOnPlayer(fankriss, bot);
    if (!hasBossAggro)
        return false;

    std::vector<Unit*> spawns = Aq40BossActions::FindFankrissSpawns(botAI, encounterUnits);
    Unit* spawnTarget = nullptr;
    for (Unit* spawn : spawns)
    {
        if (!Aq40BossHelper::IsUnitHeldByEncounterTank(bot, spawn))
        {
            spawnTarget = spawn;
            break;
        }
    }

    if (!spawnTarget && !spawns.empty())
        spawnTarget = spawns.front();

    if (spawnTarget)
    {
        Aq40Helpers::LogAq40Info(bot, "tank_swap",
            "fankriss:spawn:" + Aq40Helpers::GetAq40LogUnit(spawnTarget),
            "boss=fankriss reason=mortal_wound target=" + Aq40Helpers::GetAq40LogUnit(spawnTarget));
        return Attack(spawnTarget);
    }
    bot->AttackStop();
    Aq40Helpers::LogAq40Warn(bot, "tank_swap", "fankriss:no_spawn",
        "boss=fankriss reason=mortal_wound target=none");
    return true;
}
