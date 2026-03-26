#include "RaidNaxxActions.h"

#include "PlayerbotAIConfig.h"
#include "RaidNaxxSpellIds.h"

bool ThaddiusAttackNearestPetAction::isUseful()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    if (!this->helper.IsPhasePet())
    {
        return false;
    }

    const Unit* const target = helper.GetNearestPet();

    if (target == nullptr)
    {
        return false;
    }

    if (!this->bot->IsWithinDistInMap(target, 50.0f))
    {
        return false;
    }

    return true;
}

bool ThaddiusAttackNearestPetAction::Execute(Event)
{
    Unit* const target = this->helper.GetNearestPet();

    if (!this->bot->IsWithinLOSInMap(target))
    {
        return this->MoveTo(target, 0, MovementPriority::MOVEMENT_COMBAT);
    }

    Value<Unit*>* const currentTargetValue = this->context->GetValue<Unit*>("current target");

    if (currentTargetValue == nullptr)
    {
        return false;
    }

    if (currentTargetValue->Get() == nullptr)
    {
        return false;
    }

    if (currentTargetValue->Get() != target)
    {
        return this->Attack(target);
    }

    Value<bool>* const hasAggroValue = this->context->GetValue<bool>("has aggro", "current target");

    if (hasAggroValue == nullptr)
    {
        return false;
    }

    if (this->botAI->IsTank(this->bot) && hasAggroValue->Get())
    {
        const std::pair<float, float> posForTank = this->helper.PetPhaseGetPosForTank();

        return this->MoveTo(
            533,
            posForTank.first,
            posForTank.second,
            this->helper.tankPosZ,
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    if (this->botAI->IsRanged(this->bot))
    {
        const std::pair<float, float> posForRanged = this->helper.PetPhaseGetPosForRanged();

        return this->MoveTo(
            533,
            posForRanged.first,
            posForRanged.second,
            this->helper.tankPosZ,
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    return false;
}

bool ThaddiusMoveToPlatformAction::isUseful() { return true; }

bool ThaddiusMoveToPlatformAction::Execute(Event)
{
    const std::vector<std::pair<float, float>> position = {
        // high left
        {3462.99f, -2918.90f},
        // high right
        {3520.65f, -2976.51f},
        // low left
        {3471.36f, -2910.65f},
        // low right
        {3528.80f, -2967.04f},
        // center
        {3512.19f, -2928.58f},
    };
    const float high_z = 312.00f;
    const float low_z = 304.02f;
    const bool is_left = this->bot->GetDistance2d(position[0].first, position[0].second) < this->bot->GetDistance2d(position[1].first, position[1].second);

    if (this->bot->GetPositionZ() < (high_z - 3.0f))
    {
        return this->MoveTo(
            this->bot->GetMapId(),
            position[4].first,
            position[4].second,
            low_z,
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    if (is_left)
    {
        const bool canMoveTo = this->MoveTo(
            this->bot->GetMapId(),
            position[0].first,
            position[0].second,
            high_z,
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );

        if (canMoveTo)
        {
            return true;
        }

        const float distance = bot->GetExactDist2d(position[0].first, position[0].second);

        if (distance < PlayerbotAIConfig::instance().contactDistance)
        {
            this->JumpTo(
                this->bot->GetMapId(),
                position[2].first,
                position[2].second,
                low_z,
                MovementPriority::MOVEMENT_COMBAT
            );
        }

        return true;
    }

    const bool canMoveTo = this->MoveTo(
        this->bot->GetMapId(),
        position[1].first,
        position[1].second,
        high_z,
        false,
        false,
        false,
        false,
        MovementPriority::MOVEMENT_COMBAT
    );

    if (canMoveTo)
    {
        return true;
    }

    const float distance = this->bot->GetExactDist2d(position[1].first, position[1].second);

    if (distance < PlayerbotAIConfig::instance().contactDistance)
    {
        this->JumpTo(
            this->bot->GetMapId(),
            position[3].first,
            position[3].second,
            low_z,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    return true;
}

bool ThaddiusMovePolarityAction::isUseful()
{
    Value<bool>* const hasAggroValue = this->context->GetValue<bool>("has aggro", "current target");

    if (hasAggroValue == nullptr)
    {
        return false;
    }

    return !this->botAI->IsMainTank(this->bot) || hasAggroValue->Get();
}

bool ThaddiusMovePolarityAction::Execute(Event)
{
    const std::vector<std::pair<float, float>> position = {
        // left melee
        {3508.29f, -2920.12f},
        // left ranged
        {3501.72f, -2913.36f},
        // right melee
        {3519.74f, -2931.69f},
        // right ranged
        {3524.32f, -2936.26f},
        // center melee
        {3512.19f, -2928.58f},
        // center ranged
        {3504.68f, -2936.68f},
    };

    const bool hasNegativeChargeById = NaxxSpellIds::HasAnyAura(
        this->botAI,
        this->bot,
        { NaxxSpellIds::NegativeCharge10, NaxxSpellIds::NegativeCharge25, NaxxSpellIds::NegativeChargeStack }
    );

    const bool hasNegativeChargeByName = this->botAI->HasAura("negative charge", bot, false, false, -1, true);
    const bool hasNegativeCharge = hasNegativeChargeById || hasNegativeChargeByName;

    if (hasNegativeCharge)
    {
        const uint8_t positionIndex = uint8_t(this->botAI->IsRanged(this->bot));

        return this->MoveTo(
            this->bot->GetMapId(),
            position[positionIndex].first,
            position[positionIndex].second,
            this->bot->GetPositionZ(),
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    const bool hasPositiveChargeById = NaxxSpellIds::HasAnyAura(
        this->botAI,
        this->bot,
        { NaxxSpellIds::PositiveCharge10, NaxxSpellIds::PositiveCharge25, NaxxSpellIds::PositiveChargeStack }
    );
    const bool hasPositiveChargeByName = this->botAI->HasAura("positive charge", bot, false, false, -1, true);
    const bool hasPositiveCharge = hasPositiveChargeById || hasPositiveChargeByName;

    if (hasPositiveCharge)
    {
        const uint8_t positionIndex = 2 + uint8_t(this->botAI->IsRanged(this->bot));

        return this->MoveTo(
            this->bot->GetMapId(),
            position[positionIndex].first,
            position[positionIndex].second,
            this->bot->GetPositionZ(),
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );

    }

    const uint8_t positionIndex = 4 + uint8_t(this->botAI->IsRanged(bot));

    return this->MoveTo(
        this->bot->GetMapId(),
        position[positionIndex].first,
        position[positionIndex].second,
        this->bot->GetPositionZ(),
        false,
        false,
        false,
        false,
        MovementPriority::MOVEMENT_COMBAT
    );
}
