#include "RaidNaxxActions.h"

bool GrobbulusGoBehindAction::Execute(Event)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("boss target");

    if (bossValue == nullptr)
    {
        return false;
    }

    Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    // Position* pos = boss->GetPosition();
    const float orientation = boss->GetOrientation() + M_PI + delta_angle;
    const float x = boss->GetPositionX();
    const float y = boss->GetPositionY();
    const float z = boss->GetPositionZ();
    const float rx = x + cos(orientation) * distance;
    const float ry = y + sin(orientation) * distance;

    return this->MoveTo(
        this->bot->GetMapId(),
        rx,
        ry,
        z,
        false,
        false,
        false,
        false,
        MovementPriority::MOVEMENT_COMBAT
    );
}

bool GrobbulusMoveAwayAction::Execute(Event)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("boss target");

    if (bossValue == nullptr)
    {
        return false;
    }

    Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    const float currentDistance = this->bot->GetExactDist2d(boss);

    if (currentDistance >= distance)
    {
        return false;
    }

    const float angle = boss->GetAngle(this->bot);
    const float x = boss->GetPositionX() + cos(angle) * distance;
    const float y = boss->GetPositionY() + sin(angle) * distance;
    const float z = this->bot->GetPositionZ();

    return this->MoveTo(
        this->bot->GetMapId(),
        x,
        y,
        z,
        false,
        false,
        false,
        false,
        MovementPriority::MOVEMENT_COMBAT
    );
}

uint32_t GrobbulusRotateAction::GetCurrWaypoint()
{
    const uint32_t current = this->FindNearestWaypoint();

    if (this->clockwise)
    {
        return (current + 1) % this->intervals;
    }

    return (current + this->intervals - 1) % this->intervals;
}
