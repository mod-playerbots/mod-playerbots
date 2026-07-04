#include "Aq40Actions.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "ObjectGuid.h"
#include "../Aq40BossHelper.h"
#include "../Aq40SpellIds.h"
#include "../Util/Aq40Helpers_Shared.h"

namespace Aq40BossActions
{

Unit* FindTrashTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return Aq40BossHelper::FindLowestHealthUnitByAnyName(botAI, attackers, { "anubisath defender" });
}
}    // namespace Aq40BossActions

namespace
{
Unit* FindClosestAq40PlagueSeparationRisk(Player* bot, PlayerbotAI* botAI, float& distanceToCreate)
{
    distanceToCreate = 0.0f;
    if (!bot || !botAI)
        return nullptr;

    Group const* group = bot->GetGroup();
    if (!group)
        return nullptr;

    Unit* riskiestMember = nullptr;
    float largestDeficit = 0.0f;

    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !Aq40BossHelper::IsSameInstance(bot, member))
            continue;

        float const currentDistance = bot->GetDistance2d(member);
        float const requiredDistance =
            Aq40SpellIds::HasAnyAura(botAI, member, { Aq40SpellIds::Aq40DefenderPlague }) ? 28.0f : 20.0f;
        float const deficit = requiredDistance - currentDistance;
        if (deficit <= 0.0f || deficit <= largestDeficit)
            continue;

        largestDeficit = deficit;
        riskiestMember = member;
    }

    distanceToCreate = largestDeficit;
    return riskiestMember;
}

struct Aq40TankRetreatResult
{
    bool valid = false;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

Aq40TankRetreatResult ComputeTankRetreatPosition(Player* bot, Unit* danger, float clearDistance)
{
    Aq40TankRetreatResult result;
    if (!bot || !danger)
        return result;

    Player* tank = Aq40BossHelper::GetEncounterPrimaryTank(bot);
    if (!tank)
        tank = Aq40BossHelper::GetEncounterBackupTank(bot, 0);
    if (!tank)
        return result;

    float dx = tank->GetPositionX() - danger->GetPositionX();
    float dy = tank->GetPositionY() - danger->GetPositionY();
    float mag = std::sqrt(dx * dx + dy * dy);
    if (mag < 0.001f)
        return result;

    dx /= mag;
    dy /= mag;

    float const step = std::min(clearDistance, 8.0f);
    float candidateX = bot->GetPositionX() + dx * step;
    float candidateY = bot->GetPositionY() + dy * step;
    float candidateZ = bot->GetPositionZ();
    float const currentDistToTank = bot->GetDistance2d(tank);
    float const candidateDistToTank = std::sqrt(
        (candidateX - tank->GetPositionX()) * (candidateX - tank->GetPositionX()) +
        (candidateY - tank->GetPositionY()) * (candidateY - tank->GetPositionY()));
    if (candidateDistToTank > currentDistToTank + 1.0f)
        return result;

    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), candidateX, candidateY, candidateZ))
        return result;

    result.valid = true;
    result.x = candidateX;
    result.y = candidateY;
    result.z = candidateZ;
    return result;
}
}    // namespace

bool Aq40TrashChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector const& attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    if (activeUnits.empty())
        return false;

    Unit* target = Aq40BossActions::FindTrashTarget(botAI, activeUnits);
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "trash", "priority", target);
    return Attack(target);
}

bool Aq40TrashChooseTargetAction::isUseful()
{
    GuidVector const& attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector activeUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, attackers);
    if (activeUnits.empty())
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget || !currentTarget->IsAlive())
        return true;

    if (!Aq40BossHelper::IsUnitNamedAny(botAI, currentTarget, { "anubisath defender" }))
        return true;

    for (ObjectGuid const guid : activeUnits)
    {
        if (guid == currentTarget->GetGUID())
            return false;
    }

    return true;
}

bool Aq40TrashAvoidDangerousAoeAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        float separationNeeded = 0.0f;
        Unit* separationRisk = FindClosestAq40PlagueSeparationRisk(bot, botAI, separationNeeded);
        if (!separationRisk || separationNeeded <= 0.0f)
            return false;

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetSelection(ObjectGuid());

        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "trash:plague:" + Aq40Helpers::GetAq40LogUnit(separationRisk),
            "boss=trash hazard=plague source=" + Aq40Helpers::GetAq40LogUnit(separationRisk));
        return MoveAway(separationRisk, separationNeeded);
    }

    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* danger = nullptr;
    std::string dangerKind;
    float highestThreatGap = 0.0f;

    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }))
        {
            float const gap = 24.0f - bot->GetDistance2d(unit);
            if (gap > highestThreatGap)
            {
                highestThreatGap = gap;
                danger = unit;
                dangerKind = "thunderclap";
            }
        }
    }

    if (!danger || highestThreatGap <= 0.0f)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    Aq40TankRetreatResult retreat = ComputeTankRetreatPosition(bot, danger, highestThreatGap + 2.0f);
    if (retreat.valid)
    {
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "trash:" + dangerKind + ":" + Aq40Helpers::GetAq40LogUnit(danger),
            "boss=trash hazard=" + dangerKind + " source=" + Aq40Helpers::GetAq40LogUnit(danger));
        return MoveTo(bot->GetMapId(), retreat.x, retreat.y, retreat.z,
                      false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
    }

    Aq40Helpers::LogAq40Warn(bot, "movement_failure",
        "trash:" + dangerKind + ":" + Aq40Helpers::GetAq40LogUnit(danger),
        "boss=trash hazard=" + dangerKind + " reason=no_safe_retreat source=" + Aq40Helpers::GetAq40LogUnit(danger));
    return false;
}

bool Aq40TrashAvoidDangerousAoeAction::isUseful()
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        float separationNeeded = 0.0f;
        return FindClosestAq40PlagueSeparationRisk(bot, botAI, separationNeeded) != nullptr &&
               separationNeeded > 0.0f;
    }

    if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetActiveCombatUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }) &&
            bot->GetDistance2d(unit) < 24.0f)
            return true;
    }

    return false;
}
