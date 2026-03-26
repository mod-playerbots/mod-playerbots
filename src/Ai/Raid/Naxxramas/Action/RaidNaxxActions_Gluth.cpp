#include "ObjectGuid.h"
#include "RaidNaxxActions.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"

// @TODO: This needs to be completely rewritten
bool GluthChooseTargetAction::Execute(Event /*event*/)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    Value<GuidVector>* const attackersValue = this->context->GetValue<GuidVector>("possible targets");

    if (attackersValue == nullptr)
    {
        return false;
    }

    const GuidVector attackers = attackersValue->Get();
    Unit* target = nullptr;
    Unit* target_boss = nullptr;
    std::vector<Unit*> target_zombies{};

    for (GuidVector::const_iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* const unit = this->botAI->GetUnit(*i);

        if (unit == nullptr)
        {
            continue;
        }

        if (!unit->IsAlive())
        {
            continue;
        }

        const std::string& unitName = unit->GetName();

        if (this->botAI->EqualLowercaseName(unitName, "zombie chow"))
        {
            target_zombies.push_back(unit);
        }

        if (this->botAI->EqualLowercaseName(unitName, "gluth"))
        {
            target_boss = unit;
        }
    }

    if (this->botAI->IsMainTank(this->bot) || this->botAI->IsAssistTankOfIndex(this->bot, 0))
    {
        target = target_boss;
    }
    else if (this->botAI->IsAssistTankOfIndex(this->bot, 1))
    {
        for (Unit* const zombie : target_zombies)
        {
            if (zombie->GetHealthPct() > this->helper.decimatedZombiePct && zombie->GetVictim() != this->bot && zombie->GetDistance2d(this->bot) <= 10.0f)
            {
                if (!target || zombie->GetDistance2d(this->bot) < target->GetDistance2d(this->bot))
                    target = zombie;
            }
        }
    }
    else if (this->botAI->GetClassIndex(this->bot, CLASS_HUNTER) == 0 || this->botAI->GetClassIndex(bot, CLASS_HUNTER) == 1)
    {
        // prevent zombie go straight to gluth
        for (Unit* const zombie : target_zombies)
        {
            if (zombie->GetHealthPct() > this->helper.decimatedZombiePct && zombie->GetVictim() == target_boss &&
                zombie->GetDistance2d(this->bot) <= PlayerbotAIConfig::instance().spellDistance)
            {
                if (!target || zombie->GetDistance2d(this->bot) < target->GetDistance2d(bot))
                {
                    target = zombie;
                }
            }
        }

        if (!target)
        {
            target = target_boss;
        }
    }
    else
    {
        for (Unit* const zombie : target_zombies)
        {
            if (zombie->GetHealthPct() <= this->helper.decimatedZombiePct)
            {
                if (target == nullptr ||
                    target->GetDistance2d(this->helper.mainTankPos25.first, this->helper.mainTankPos25.second) >
                        zombie->GetDistance2d(this->helper.mainTankPos25.first, this->helper.mainTankPos25.second))
                {

                    target = zombie;
                }
            }
        }
        if (target == nullptr)
            target = target_boss;
    }
    if (!target || context->GetValue<Unit*>("current target")->Get() == target)
        return false;

    if (target_boss && target == target_boss)
        return Attack(target, true);

    return Attack(target, false);
    // return Attack(target);
}

// @TODO: This needs to be completely rewritten
bool GluthPositionAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    bool raid25 = bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;
    if (botAI->IsMainTank(bot) || botAI->IsAssistTankOfIndex(bot, 0))
    {
        if (AI_VALUE2(bool, "has aggro", "boss target"))
        {
            if (raid25)
            {
                if (MoveTo(NAXX_MAP_ID, helper.mainTankPos25.first, helper.mainTankPos25.second, bot->GetPositionZ(), false, false, false,
                           false, MovementPriority::MOVEMENT_COMBAT))
                    return true;

                return MoveInside(NAXX_MAP_ID, helper.mainTankPos25.first, helper.mainTankPos25.second, bot->GetPositionZ(), 2.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
            }
            else
            {
                if (MoveTo(NAXX_MAP_ID, helper.mainTankPos10.first, helper.mainTankPos10.second, bot->GetPositionZ(), false, false, false,
                           false, MovementPriority::MOVEMENT_COMBAT))
                    return true;

                return MoveInside(NAXX_MAP_ID, helper.mainTankPos10.first, helper.mainTankPos10.second, bot->GetPositionZ(), 2.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    else if (botAI->IsAssistTankOfIndex(bot, 1))
    {
        if (helper.BeforeDecimate())
        {
            if (MoveTo(bot->GetMapId(), helper.beforeDecimatePos.first, helper.beforeDecimatePos.second, bot->GetPositionZ(), false, false,
                       false, false, MovementPriority::MOVEMENT_COMBAT))
                return true;

            return MoveInside(bot->GetMapId(), helper.beforeDecimatePos.first, helper.beforeDecimatePos.second, bot->GetPositionZ(), 2.0f,
                              MovementPriority::MOVEMENT_COMBAT);
        }
        else
        {
            if (AI_VALUE2(bool, "has aggro", "current target"))
            {
                uint32 nearest = FindNearestWaypoint();
                uint32 next_point = (nearest + 1) % intervals;
                return MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ(),
                              false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
        }
    }
    else if (botAI->IsRangedDps(bot))
    {
        if (raid25)
        {
            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 0)
                return MoveInside(NAXX_MAP_ID, helper.leftSlowDownPos.first, helper.leftSlowDownPos.second, bot->GetPositionZ(), 0.0f,
                                  MovementPriority::MOVEMENT_COMBAT);

            if (botAI->GetClassIndex(bot, CLASS_HUNTER) == 1)
                return MoveInside(NAXX_MAP_ID, helper.rightSlowDownPos.first, helper.rightSlowDownPos.second, bot->GetPositionZ(), 0.0f,
                                  MovementPriority::MOVEMENT_COMBAT);
        }
        return MoveInside(NAXX_MAP_ID, helper.rangedPos.first, helper.rangedPos.second, bot->GetPositionZ(), 3.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    }
    else if (botAI->IsHeal(bot))
        return MoveInside(NAXX_MAP_ID, helper.healPos.first, helper.healPos.second, bot->GetPositionZ(), 0.0f,
                          MovementPriority::MOVEMENT_COMBAT);
    return false;
}

bool GluthSlowdownAction::Execute(Event)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    const bool raid25 = this->bot->GetRaidDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;

    if (!raid25)
    {
        return false;
    }

    if (this->helper.JustStartCombat())
    {
        return false;
    }

    const uint8_t botClass = this->bot->getClass();

    if (botClass == CLASS_HUNTER)
    {
        return this->botAI->CastSpell("frost trap", bot);
    }

    return false;
}
