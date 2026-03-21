#include "RaidNaxxActions.h"

#include "Playerbots.h"

bool LoathebPositionAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    if (botAI->IsTank(bot))
    {
        if (AI_VALUE2(bool, "has aggro", "boss target"))
            return MoveTo(533, helper.mainTankPos.first, helper.mainTankPos.second, bot->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    else if (botAI->IsRanged(bot))
        return MoveInside(533, helper.rangePos.first, helper.rangePos.second, bot->GetPositionZ(), 1.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    return false;
}

bool LoathebChooseTargetAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    Unit* target_spore = nullptr;
    for (auto i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (!unit)
            continue;

        if (!unit->IsAlive())
            continue;

        if (botAI->EqualLowercaseName(unit->GetName(), "spore"))
            target_spore = unit;

        if (botAI->EqualLowercaseName(unit->GetName(), "loatheb"))
            target_boss = unit;
    }
    if (target_spore && bot->GetDistance2d(target_spore) <= 1.0f)
        target = target_spore;
    else
        target = target_boss;

    if (!target || context->GetValue<Unit*>("current target")->Get() == target)
        return false;

    return Attack(target);
}
