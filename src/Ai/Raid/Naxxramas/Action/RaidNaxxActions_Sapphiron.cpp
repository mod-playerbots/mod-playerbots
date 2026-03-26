#include "RaidNaxxActions.h"

#include "RaidNaxxBossHelper.h"
#include "RaidNaxxSpellIds.h"

bool SapphironGroundPositionAction::Execute(Event)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    if (this->botAI->IsMainTank(bot))
    {
        Value<bool>* const hasAggroValue = this->context->GetValue<bool>("has aggro", "current target");

        if (hasAggroValue == nullptr)
        {
            return false;
        }

        if (hasAggroValue->Get())
        {
            return this->MoveTo(
                NAXX_MAP_ID,
                this->helper.mainTankPos.first,
                this->helper.mainTankPos.second,
                this->helper.GENERIC_HEIGHT,
                false,
                false,
                false,
                false,
                MovementPriority::MOVEMENT_COMBAT
            );
        }

        return false;
    }

    if (this->helper.JustLanded())
    {
        const uint32_t index = this->botAI->GetGroupSlotIndex(bot);
        const float start_angle = 0.85f * M_PI;
        const float offset_angle = M_PI * 0.02f * index;
        const float angle = start_angle + offset_angle;
        float distance = 5.0f;

        if (this->botAI->IsRanged(bot))
        {
            distance = 35.0f;
        }

        if (botAI->IsHeal(bot))
        {
            distance = 30.0f;
        }

        const float posX = this->helper.center.first + cos(angle) * distance;
        const float posY = this->helper.center.second + sin(angle) * distance;

        const bool movedTo = this->MoveTo(
            NAXX_MAP_ID,
            posX,
            posY,
            this->helper.GENERIC_HEIGHT,
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );

        if (movedTo)
        {
            return true;
        }

        return this->MoveInside(
            NAXX_MAP_ID,
            posX,
            posY,
            this->helper.GENERIC_HEIGHT,
            2.0f,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    std::vector<float> dest{};

    if (this->helper.FindPosToAvoidChill(dest))
    {
        return this->MoveTo(
            NAXX_MAP_ID,
            dest[0],
            dest[1],
            dest[2],
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    return false;
}

bool SapphironFlightPositionAction::Execute(Event)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    if (this->helper.WaitForExplosion())
    {
        return MoveToNearestIcebolt();
    }

    std::vector<float> dest{};

    if (this->helper.FindPosToAvoidChill(dest))
    {
        return this->MoveTo(
            NAXX_MAP_ID,
            dest[0],
            dest[1],
            dest[2],
            false,
            false,
            false,
            false,
            MovementPriority::MOVEMENT_COMBAT
        );
    }

    return false;
}

bool SapphironFlightPositionAction::MoveToNearestIcebolt()
{
    Group* const group = this->bot->GetGroup();

    if (group == nullptr)
    {
        return false;
    }

    Player* playerWithIcebolt = nullptr;
    float minDistance = 0.0f;

    for (const GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* const member = ref->GetSource();

        const bool hasIceboltAuraById = NaxxSpellIds::HasAnyAura(this->botAI, member, {NaxxSpellIds::Icebolt10, NaxxSpellIds::Icebolt25});
        const bool hasIceboltAuraByName = this->botAI->HasAura("icebolt", member, false, false, -1, true);

        if (!hasIceboltAuraById && !hasIceboltAuraByName)
        {
            continue;;
        }


        if (playerWithIcebolt == nullptr || minDistance > this->bot->GetDistance(member))
        {
            playerWithIcebolt = member;
            minDistance = this->bot->GetDistance(member);
        }
    }

    if (playerWithIcebolt == nullptr)
    {
        return false;
    }

    Value<Unit*>* const sapphironValue = this->context->GetValue<Unit*>("find target", "sapphiron");

    const Unit* const boss = sapphironValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    const float angle = boss->GetAngle(playerWithIcebolt);
    const float posX = playerWithIcebolt->GetPositionX() + cos(angle) * 3.0f;
    const float posY = playerWithIcebolt->GetPositionY() + sin(angle) * 3.0f;

    const bool canMoveTo = this->MoveTo(
        NAXX_MAP_ID,
        posX,
        posY,
        this->helper.GENERIC_HEIGHT,
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

    return this->MoveNear(
        playerWithIcebolt,
        3.0f,
        MovementPriority::MOVEMENT_COMBAT
    );

}
