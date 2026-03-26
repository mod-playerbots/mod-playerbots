#include "RaidNaxxActions.h"

bool FourHorsemenAttractAlternativelyAction::Execute(Event)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    this->helper.CalculatePosToGo(bot);

    const std::pair<float, float> position = helper.CurrentAttractPos();
    const bool ableToMove = this->MoveTo(
        this->bot->GetMapId(),
        position.first,
        position.second,
        helper.posZ,
        false,
        false,
        false,
        false,
        MovementPriority::MOVEMENT_COMBAT
    );

    if (ableToMove)
    {
        return true;
    }

    Unit* const attackTarget = this->helper.CurrentAttackTarget();

    Value<Unit*>* currentTargetValue = this->context->GetValue<Unit*>("current target");

    if (currentTargetValue == nullptr)
    {
        return false;
    }

    if (currentTargetValue->Get() == attackTarget)
    {
        return false;
    }

    return false;
}

bool FourHorsemenAttactInOrderAction::Execute(Event)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    Unit* target = nullptr;

    Value<Unit*>* const korthazzValue = this->context->GetValue<Unit*>("find target", "thane korth'azz");
    Value<Unit*>* const blaumeuxValue = this->context->GetValue<Unit*>("find target", "lady blaumeux");
    Value<Unit*>* const zeliekValue = this->context->GetValue<Unit*>("find target", "sir zeliek");
    Value<Unit*>* const rivendareValue = this->context->GetValue<Unit*>("find target", "baron rivendare");

    if (korthazzValue == nullptr || blaumeuxValue == nullptr || zeliekValue == nullptr || rivendareValue == nullptr)
    {
        return false;
    }

    Unit* const korthazz = korthazzValue->Get();
    Unit* const blaumeux = blaumeuxValue->Get();
    Unit* const zeliek = zeliekValue->Get();
    Unit* const rivendare = rivendareValue->Get();
    Unit* fourthHorseman = rivendare;

    if (fourthHorseman == nullptr)
    {
        Value<Unit*>* const mograineValue = this->context->GetValue<Unit*>("find target", "highlord mograine");

        if (mograineValue == nullptr)
        {
            return false;
        }

        fourthHorseman = mograineValue->Get();
    }

    std::array<Unit*, 4> attack_order{ korthazz, fourthHorseman, blaumeux, zeliek };

    if (this->botAI->IsAssistTank(bot))
    {
        attack_order = { fourthHorseman, korthazz, blaumeux, zeliek };
    }

    for (Unit* horseman : attack_order)
    {
        if (horseman == nullptr || !horseman->IsAlive())
        {
            continue;
        }

        target = horseman;
    }

    if (target == nullptr)
    {
        return false;
    }

    Value<Unit*>* const currentTargetValue = this->context->GetValue<Unit*>("current target");

    if (currentTargetValue == nullptr)
    {
        return false;
    }

    if (currentTargetValue->Get() == target && this->botAI->GetState() == BOT_STATE_COMBAT)
    {
        return false;
    }

    if (!this->bot->IsWithinLOSInMap(target))
    {
        return this->MoveNear(target, 22.0f, MovementPriority::MOVEMENT_COMBAT);
    }

    return this->Attack(target);
}
