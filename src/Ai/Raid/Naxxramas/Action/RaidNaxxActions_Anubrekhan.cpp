#include "RaidNaxxActions.h"

#include "ObjectGuid.h"

bool AnubrekhanChooseTargetAction::Execute(Event)
{
    Value<GuidVector>* const attackersValue = this->context->GetValue<GuidVector>("attackers");

    if (attackersValue == nullptr)
    {
        return false;
    }

    const GuidVector attackers = attackersValue->Get();
    Value<Unit*>* const currentTargetValue = this->context->GetValue<Unit*>("current target");

    if (currentTargetValue == nullptr)
    {
        return false;
    }

    const Unit* currentTarget = currentTargetValue->Get();

    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    std::vector<Unit*> target_guards;

    for (const ObjectGuid guid : attackers)
    {
        Unit* const unit = this->botAI->GetUnit(guid);

        if (unit == nullptr)
        {
            continue;
        }

        const std::string& unitName = unit->GetName();

        if (this->botAI->EqualLowercaseName(unitName, "crypt guard"))
        {
            target_guards.push_back(unit);
        }

        if (this->botAI->EqualLowercaseName(unitName, "anub'rekhan"))
        {
            target_boss = unit;
        }
    }

    if (this->botAI->IsMainTank(this->bot))
    {
        target = target_boss;

        if (currentTarget == target)
        {
            return false;
        }

        return this->Attack(target);
    }

    if (target_guards.size() == 0)
    {
        target = target_boss;

        if (currentTarget == target)
        {
            return false;
        }

        return this->Attack(target);
    }

    if (this->botAI->IsAssistTank(this->bot))
    {
        for (Unit* t : target_guards)
        {
            if (target == nullptr)
            {
                target = t;

                continue;
            }

            Unit* targetVictim = target->GetVictim();

            if (targetVictim == nullptr)
            {
                target = t;
            }

            const Player* playerTargetVictim = dynamic_cast<Player*>(targetVictim);

            if (playerTargetVictim == nullptr)
            {
                target = t;
            }
        }

        if (currentTarget == target)
        {
            return false;
        }

        return this->Attack(target);
    }

    for (Unit* t : target_guards)
    {
        if (target == nullptr || target->GetHealthPct() > t->GetHealthPct())
        {
            target = t;
        }
    }

    if (currentTarget == target)
    {
        return false;
    }

    return this->Attack(target);
}

bool AnubrekhanPositionAction::Execute(Event /*event*/)
{
    Value<Unit*>* const findTargetValue = this->context->GetValue<Unit*>("find target", "anub'rekhan");

    if (findTargetValue == nullptr)
    {
        return false;
    }

    Unit* boss = findTargetValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    const bool inPhase = this->botAI->HasAura("locust swarm", boss)
                        || boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

    if (!inPhase)
    {
        return false;
    }

    if (this->botAI->IsMainTank(bot))
    {
        const uint32 nearest = this->FindNearestWaypoint();
        const uint32 next_point = (nearest + 1) % intervals;

        return this->MoveTo(
            this->bot->GetMapId(),
            waypoints[next_point].first,
            waypoints[next_point].second,
            this->bot->GetPositionZ(),
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    return this->MoveInside(
        533,
        3272.49f,
        -3476.27f,
        this->bot->GetPositionZ(),
        3.0f,
        MovementPriority::MOVEMENT_COMBAT
    );
}
