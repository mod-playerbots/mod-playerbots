#include "RaidNaxxActions.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RaidNaxxSpellIds.h"

bool ThaddiusAttackNearestPetAction::isUseful()
{
    if (!helper.UpdateBossAI())
        return false;

    if (!helper.IsPhasePet())
        return false;

    Unit* target = helper.GetNearestPet();
    if (!target || !bot->IsWithinDistInMap(target, 50.0f))
        return false;

    return true;
}

bool ThaddiusAttackNearestPetAction::Execute(Event /*event*/)
{
    Unit* target = helper.GetNearestPet();
    if (!target || !bot->IsWithinLOSInMap(target))
        return MoveTo(target, 0, MovementPriority::MOVEMENT_COMBAT);

    if (AI_VALUE(Unit*, "current target") != target)
        return Attack(target);

    if (botAI->IsTank(bot) && AI_VALUE2(bool, "has aggro", "current target"))
    {
        std::pair<float, float> posForTank = helper.PetPhaseGetPosForTank();
        return MoveTo(533, posForTank.first, posForTank.second, helper.tankPosZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }
    if (botAI->IsRanged(bot))
    {
        std::pair<float, float> posForRanged = helper.PetPhaseGetPosForRanged();
        return MoveTo(533, posForRanged.first, posForRanged.second, helper.tankPosZ, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
    }
    return false;
}

bool ThaddiusMoveToPlatformAction::isUseful() { return true; }

bool ThaddiusMoveToPlatformAction::Execute(Event /*event*/)
{
    std::vector<std::pair<float, float>> position = {
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
    float high_z = 312.00f, low_z = 304.02f;
    bool is_left = bot->GetDistance2d(position[0].first, position[0].second) <
                   bot->GetDistance2d(position[1].first, position[1].second);
    if (bot->GetPositionZ() >= (high_z - 3.0f))
    {
        if (is_left)
        {
            if (!MoveTo(bot->GetMapId(), position[0].first, position[0].second, high_z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT))
            {
                float distance = bot->GetExactDist2d(position[0].first, position[0].second);
                if (distance < sPlayerbotAIConfig.contactDistance)
                    JumpTo(bot->GetMapId(), position[2].first, position[2].second, low_z, MovementPriority::MOVEMENT_COMBAT);
                    // bot->TeleportTo(bot->GetMapId(), position[2].first, position[2].second, low_z, bot->GetOrientation());
            }
        }
        else
        {
            if (!MoveTo(bot->GetMapId(), position[1].first, position[1].second, high_z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT))
            {
                float distance = bot->GetExactDist2d(position[1].first, position[1].second);
                if (distance < sPlayerbotAIConfig.contactDistance)
                    JumpTo(bot->GetMapId(), position[3].first, position[3].second, low_z, MovementPriority::MOVEMENT_COMBAT);
                    // bot->TeleportTo(bot->GetMapId(), position[3].first, position[3].second, low_z, bot->GetOrientation());
            }
        }
    }
    else
        return MoveTo(bot->GetMapId(), position[4].first, position[4].second, low_z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT);

    return true;
}

bool ThaddiusMovePolarityAction::isUseful()
{
    return !botAI->IsMainTank(bot) || AI_VALUE2(bool, "has aggro", "current target");
}

bool ThaddiusMovePolarityAction::Execute(Event /*event*/)
{
    std::vector<std::pair<float, float>> position = {
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
    uint32 idx;
    if (NaxxSpellIds::HasAnyAura(
            botAI, bot,
            {NaxxSpellIds::NegativeCharge10, NaxxSpellIds::NegativeCharge25, NaxxSpellIds::NegativeChargeStack}) ||
        botAI->HasAura("negative charge", bot, false, false, -1, true))
    {
        idx = 0;
    }
    else if (NaxxSpellIds::HasAnyAura(
                 botAI, bot,
                 {NaxxSpellIds::PositiveCharge10, NaxxSpellIds::PositiveCharge25, NaxxSpellIds::PositiveChargeStack}) ||
             botAI->HasAura("positive charge", bot, false, false, -1, true))
    {
        idx = 1;
    }
    else
    {
        idx = 2;
    }
    idx = idx * 2 + botAI->IsRanged(bot);
    return MoveTo(bot->GetMapId(), position[idx].first, position[idx].second, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
}
