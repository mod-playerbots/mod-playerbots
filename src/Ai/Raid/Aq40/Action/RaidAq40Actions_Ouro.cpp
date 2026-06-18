#include "RaidAq40Actions.h"

#include <cmath>

#include "SharedDefines.h"
#include "../RaidAq40BossHelper.h"
#include "../Util/RaidAq40Helpers_Shared.h"

namespace
{
float constexpr kPi = 3.14159265f;
}    // namespace

namespace Aq40BossActions
{
Unit* FindOuroTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitByAnyName(botAI, attackers, { "ouro" });
}
}    // namespace Aq40BossActions

namespace
{
// FindLowestHealthUnit now lives in Aq40BossHelper.

Unit* FindOuroScarabs(PlayerbotAI* botAI, GuidVector const& attackers)
{
    std::vector<Unit*> scarabs =
        Aq40BossActions::FindUnitsByAnyName(botAI, attackers, { "qiraji scarab", "scarab" });
    return Aq40BossHelper::FindLowestHealthUnit(scarabs);
}

Unit* FindNearestDirtMound(Player* bot, PlayerbotAI* botAI, GuidVector const& attackers)
{
    std::vector<Unit*> mounds = Aq40BossActions::FindUnitsByAnyName(botAI, attackers, { "dirt mound" });
    Unit* closest = nullptr;
    float closestDistance = 9999.0f;
    for (Unit* mound : mounds)
    {
        if (!mound)
            continue;

        float d = bot->GetDistance2d(mound);
        if (d < closestDistance)
        {
            closestDistance = d;
            closest = mound;
        }
    }

    return closest;
}

// FindBurrowedOuro now lives in Aq40BossHelper.
}    // namespace

bool Aq40OuroChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    if (encounterUnits.empty())
        return false;

    bool const isPrimaryTank = Aq40BossHelper::IsEncounterPrimaryTank(bot, bot);
    bool const isBackupTank0 = Aq40BossHelper::IsEncounterBackupTank(bot, bot, 0);
    bool const isBackupTank1 = Aq40BossHelper::IsEncounterBackupTank(bot, bot, 1);
    Unit* target = nullptr;

    std::vector<Unit*> scarabs = Aq40BossActions::FindUnitsByAnyName(botAI, encounterUnits, { "qiraji scarab", "scarab" });
    std::sort(scarabs.begin(), scarabs.end(), [](Unit* left, Unit* right)
    {
        if (!left || !right)
            return left != nullptr;
        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });

    if (isPrimaryTank)
        target = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);

    if (!target && (isBackupTank0 || isBackupTank1))
    {
        uint32 assignedIndex = isBackupTank0 ? 0u : 1u;
        if (assignedIndex < scarabs.size())
            target = scarabs[assignedIndex];
    }

    if (!target && Aq40BossHelper::IsEncounterTank(bot, bot))
    {
        if (!scarabs.empty())
            target = FindOuroScarabs(botAI, encounterUnits);
        if (!target)
            target = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);
    }

    if (!target)
    {
        for (Unit* scarab : scarabs)
        {
            if (Aq40BossHelper::IsUnitHeldByEncounterTank(bot, scarab))
            {
                target = scarab;
                break;
            }
        }
    }

    if (!target)
    {
        Unit* ouro = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);
        if (ouro && Aq40BossHelper::IsUnitHeldByEncounterTank(bot, ouro, true))
            target = ouro;
    }

    bool const targetIsBoss = target && botAI->EqualLowercaseName(target->GetName(), "ouro");
    if (Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target, targetIsBoss))
        return false;

    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    Aq40Helpers::LogAq40Target(bot, "ouro", targetIsBoss ? "boss" : "scarab", target);
    return Attack(target);
}

bool Aq40OuroHoldMeleeContactAction::Execute(Event /*event*/)
{
    if (!Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* ouro = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);
    if (!ouro)
        return false;

    float d = bot->GetDistance2d(ouro);
    if (d <= 8.0f)
        return false;

    float dx = bot->GetPositionX() - ouro->GetPositionX();
    float dy = bot->GetPositionY() - ouro->GetPositionY();
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        len = 1.0f;
    }

    float desired = 4.0f;
    float moveX = ouro->GetPositionX() + (dx / len) * desired;
    float moveY = ouro->GetPositionY() + (dy / len) * desired;
    Aq40Helpers::LogAq40Info(bot, "tank_position",
        "ouro:melee_contact:" + Aq40Helpers::GetAq40LogUnit(ouro),
        "boss=ouro reason=melee_contact target=" + Aq40Helpers::GetAq40LogUnit(ouro));
    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40OuroAvoidSweepAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* ouro = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);
    if (!ouro)
        return false;

    if (bot->GetDistance2d(ouro) > 10.0f)
        return false;

    float dx = bot->GetPositionX() - ouro->GetPositionX();
    float dy = bot->GetPositionY() - ouro->GetPositionY();
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        len = 1.0f;
    }

    float desired = 16.0f;
    float moveX = ouro->GetPositionX() + (dx / len) * desired;
    float moveY = ouro->GetPositionY() + (dy / len) * desired;
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "ouro:sweep:" + Aq40Helpers::GetAq40LogUnit(ouro),
        "boss=ouro hazard=sweep source=" + Aq40Helpers::GetAq40LogUnit(ouro));
    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40OuroAvoidSandBlastAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* ouro = Aq40BossActions::FindOuroTarget(botAI, encounterUnits);
    if (!ouro)
        return false;

    // Move behind Ouro using orientation + PI (Grobbulus behind-boss pattern from Naxxramas).
    // Melee stay at melee range, ranged/healers at their normal backline distance.
    float behindAngle = ouro->GetOrientation() + kPi;
    float distance = (botAI->IsRanged(bot) || botAI->IsHeal(bot))
        ? std::max(bot->GetDistance2d(ouro), 20.0f) : 6.0f;
    float moveX = ouro->GetPositionX() + std::cos(behindAngle) * distance;
    float moveY = ouro->GetPositionY() + std::sin(behindAngle) * distance;

    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "ouro:sand_blast:" + Aq40Helpers::GetAq40LogUnit(ouro),
        "boss=ouro hazard=sand_blast source=" + Aq40Helpers::GetAq40LogUnit(ouro));
    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool Aq40OuroAvoidSubmergeAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* hazard = FindNearestDirtMound(bot, botAI, encounterUnits);
    if (!hazard)
        hazard = Aq40BossHelper::FindBurrowedOuro(botAI, encounterUnits);
    if (!hazard)
        return false;

    float d = bot->GetDistance2d(hazard);
    if (d > 16.0f)
        return false;

    float dx = bot->GetPositionX() - hazard->GetPositionX();
    float dy = bot->GetPositionY() - hazard->GetPositionY();
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.1f)
    {
        dx = std::cos(bot->GetOrientation());
        dy = std::sin(bot->GetOrientation());
        len = 1.0f;
    }

    float desired = 26.0f;
    float moveX = hazard->GetPositionX() + (dx / len) * desired;
    float moveY = hazard->GetPositionY() + (dy / len) * desired;
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "ouro:submerge:" + Aq40Helpers::GetAq40LogUnit(hazard),
        "boss=ouro hazard=submerge source=" + Aq40Helpers::GetAq40LogUnit(hazard));
    return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}
