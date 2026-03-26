#include "ObjectGuid.h"
#include "RaidNaxxActions.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

// @TODO: This needs to be completely rewritten because it is currently an unmaintainable bowl of spaghetti.
bool KelthuzadChooseTargetAction::Execute(Event /*event*/)
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
    Unit* target_soldier = nullptr;
    Unit* target_weaver = nullptr;
    Unit* target_abomination = nullptr;
    Unit* target_kelthuzad = nullptr;
    Unit* target_guardian = nullptr;

    for (GuidVector::const_iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* const unit = this->botAI->GetUnit(*i);

        if (unit == nullptr)
        {
            continue;
        }

        if (this->botAI->EqualLowercaseName(unit->GetName(), "guardian of icecrown"))
        {
            if (!target_guardian)
                target_guardian = unit;
            else if (unit->GetVictim() && target_guardian->GetVictim() && unit->GetVictim()->ToPlayer() &&
                     target_guardian->GetVictim()->ToPlayer() && !this->botAI->IsAssistTank(unit->GetVictim()->ToPlayer()) &&
                     this->botAI->IsAssistTank(target_guardian->GetVictim()->ToPlayer()))
            {
                target_guardian = unit;
            }
            else if (unit->GetVictim() && target_guardian->GetVictim() && unit->GetVictim()->ToPlayer() &&
                     target_guardian->GetVictim()->ToPlayer() && !this->botAI->IsAssistTank(unit->GetVictim()->ToPlayer()) &&
                     !this->botAI->IsAssistTank(target_guardian->GetVictim()->ToPlayer()) &&
                     target_guardian->GetDistance2d(helper.center.first, helper.center.second) >
                         bot->GetDistance2d(unit))
            {
                target_guardian = unit;
            }
        }

        if (unit->GetDistance2d(helper.center.first, helper.center.second) > 30.0f)
            continue;

        if (bot->GetDistance2d(unit) > sPlayerbotAIConfig.spellDistance)
            continue;

        if (this->botAI->EqualLowercaseName(unit->GetName(), "unstoppable abomination"))
        {
            if (target_abomination == nullptr ||
                target_abomination->GetDistance2d(helper.center.first, helper.center.second) >
                    unit->GetDistance2d(helper.center.first, helper.center.second))
            {
                target_abomination = unit;
            }
        }
        if (this->botAI->EqualLowercaseName(unit->GetName(), "soldier of the frozen wastes"))
        {
            if (target_soldier == nullptr ||
                target_soldier->GetDistance2d(helper.center.first, helper.center.second) >
                    unit->GetDistance2d(helper.center.first, helper.center.second))
            {
                target_soldier = unit;
            }
        }
        if (this->botAI->EqualLowercaseName(unit->GetName(), "soul weaver"))
        {
            if (target_weaver == nullptr || target_weaver->GetDistance2d(helper.center.first, helper.center.second) >
                                                unit->GetDistance2d(helper.center.first, helper.center.second))
                target_weaver = unit;
        }

        if (this->botAI->EqualLowercaseName(unit->GetName(), "kel'thuzad"))
        {
            target_kelthuzad = unit;
        }
    }

    std::vector<Unit*> targets{ target_abomination, target_kelthuzad };

    if (this->botAI->IsRanged(bot))
    {
        targets = {target_weaver, target_soldier, target_abomination, target_kelthuzad};

        if (this->botAI->GetRangedDpsIndex(bot) <= 1)
        {
            targets = {target_soldier, target_weaver, target_abomination, target_kelthuzad};
        }
    }

    if (this->botAI->IsAssistTank(bot))
    {
        targets = { target_abomination, target_guardian, target_kelthuzad };
    }

    for (Unit* t : targets)
    {
        if (t)
        {
            target = t;

            break;
        }
    }
    if (context->GetValue<Unit*>("current target")->Get() == target)
    {
        return false;
    }

    if (target_kelthuzad && target == target_kelthuzad)
    {
        return this->Attack(target, true);
    }

    return this->Attack(target, false);
}

// @TODO: This needs to be completely rewritten because it is currently an unmaintainable bowl of spaghetti.
bool KelthuzadPositionAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    if (helper.IsPhaseOne())
    {
        if (AI_VALUE(Unit*, "current target") == nullptr)
            return MoveInside(NAXX_MAP_ID, helper.center.first, helper.center.second, bot->GetPositionZ(), 3.0f,
                              MovementPriority::MOVEMENT_COMBAT);
    }
    else if (helper.IsPhaseTwo())
    {
        Unit* shadow_fissure = helper.GetAnyShadowFissure();
        if (!shadow_fissure || !bot->IsWithinDistInMap(shadow_fissure, 10.0f))
        {
            float distance, angle;
            if (botAI->IsMainTank(bot))
            {
                if (AI_VALUE2(bool, "has aggro", "current target"))
                    return MoveTo(NAXX_MAP_ID, helper.tank_pos.first, helper.tank_pos.second, bot->GetPositionZ(), false, false, false,
                                  false, MovementPriority::MOVEMENT_COMBAT);
                else
                    return false;
            }
            else if (botAI->IsRanged(bot))
            {
                uint32 index = botAI->GetRangedIndex(bot);
                if (index < 8)
                {
                    distance = 20.0f;
                    angle = index * M_PI / 4;
                }
                else
                {
                    distance = 32.0f;
                    angle = (index - 8) * M_PI / 4;
                }
                float dx, dy;
                dx = helper.center.first + cos(angle) * distance;
                dy = helper.center.second + sin(angle) * distance;
                return MoveTo(NAXX_MAP_ID, dx, dy, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
            }
            else if (botAI->IsTank(bot))
            {
                Unit* cur_tar = AI_VALUE(Unit*, "current target");
                if (cur_tar && cur_tar->GetVictim() && cur_tar->GetVictim()->ToPlayer() &&
                    botAI->EqualLowercaseName(cur_tar->GetName(), "guardian of icecrown") &&
                    botAI->IsAssistTank(cur_tar->GetVictim()->ToPlayer()))
                {
                    return MoveTo(NAXX_MAP_ID, helper.assist_tank_pos.first, helper.assist_tank_pos.second, bot->GetPositionZ(),
                                  false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
                }
                else
                    return false;
            }
        }
        else
        {
            float dx, dy;
            float angle;
            if (!botAI->IsRanged(bot))
                angle = shadow_fissure->GetAngle(helper.center.first, helper.center.second);
            else
                angle = bot->GetAngle(shadow_fissure) + M_PI;

            dx = shadow_fissure->GetPositionX() + cos(angle) * 10.0f;
            dy = shadow_fissure->GetPositionY() + sin(angle) * 10.0f;
            return MoveTo(NAXX_MAP_ID, dx, dy, bot->GetPositionZ(), false, false, false, false, MovementPriority::MOVEMENT_COMBAT);
        }
    }
    return false;
}
