#include "ObjectGuid.h"
#include "RaidNaxxActions.h"

bool LoathebPositionAction::Execute(Event /*event*/)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    if (this->botAI->IsTank(bot))
    {
        Value<bool>* const hasAggroValue = this->context->GetValue<bool>("has aggro", "boss target");

        if (hasAggroValue == nullptr)
        {
            return false;
        }

        if (!hasAggroValue->Get())
        {
            return false;
        }

        return this->MoveTo(
            533,
            this->helper.mainTankPos.first,
            this->helper.mainTankPos.second,
            this->bot->GetPositionZ(),
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    if (this->botAI->IsRanged(bot))
    {
        return this->MoveInside(
            533,
            this->helper.rangePos.first,
            this->helper.rangePos.second,
            this->bot->GetPositionZ(),
            1.0f,
            MovementPriority::MOVEMENT_COMBAT
        );
    }
    return false;
}

bool LoathebChooseTargetAction::Execute(Event /*event*/)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    Value<GuidVector>* const attackersValue = this->context->GetValue<GuidVector>("attackers");

    if (attackersValue == nullptr)
    {
        return false;
    }

    const GuidVector attackers = attackersValue->Get();

    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    Unit* target_spore = nullptr;

    for (GuidVector::const_iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = this->botAI->GetUnit(*i);

        if (unit == nullptr)
        {
            continue;
        }

        if (!unit->IsAlive())
        {
            continue;
        }

        if (this->botAI->EqualLowercaseName(unit->GetName(), "spore"))
        {
            target_spore = unit;
        }

        if (this->botAI->EqualLowercaseName(unit->GetName(), "loatheb"))
        {
            target_boss = unit;
        }
    }

    target = target_boss;

    if (target_spore != nullptr && this->bot->GetDistance2d(target_spore) <= 1.0f)
    {
        target = target_spore;
    }

    if (target == nullptr || this->context->GetValue<Unit*>("current target")->Get() == target)
    {
        return false;
    }

    return this->Attack(target);
}
