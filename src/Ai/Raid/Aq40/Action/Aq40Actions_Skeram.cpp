#include "Aq40Actions.h"

#include <algorithm>

#include "RtiTargetValue.h"
#include "../Util/Aq40Helpers_Shared.h"
#include "../Util/Aq40Helpers_Skeram.h"

namespace
{
bool IsAttackableSkeramTarget(Player* bot, Unit* target)
{
    return bot && target && target->IsInWorld() && target->IsAlive() && target->GetMapId() == bot->GetMapId() &&
           !target->IsFriendlyTo(bot) && (target->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) != UNIT_FLAG_NOT_SELECTABLE;
}

Unit* FindSkeramSkullTarget(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return nullptr;

    Unit* skullTarget = Aq40Helpers::ResolveRaidTargetIcon(bot, botAI, RtiTargetValue::skullIndex);
    if (!IsAttackableSkeramTarget(bot, skullTarget))
        return nullptr;

    if (!botAI->EqualLowercaseName(skullTarget->GetName(), "the prophet skeram"))
        return nullptr;

    return skullTarget;
}
}

namespace Aq40BossActions
{
Unit* FindSkeramTarget(PlayerbotAI* botAI, GuidVector const& attackers, bool preferLowestHealth)
{
    if (!botAI)
        return nullptr;

    Player* bot = botAI->GetBot();
    if (!bot)
    {
        std::vector<Unit*> skerams = FindUnitsByAnyName(botAI, attackers, { "the prophet skeram" });
        return skerams.empty() ? nullptr : skerams.front();
    }

    Unit* skullTarget = FindSkeramSkullTarget(bot, botAI);
    if (skullTarget)
        return skullTarget;

    std::vector<Unit*> skerams = FindUnitsByAnyName(botAI, attackers, { "the prophet skeram" });

    skerams.erase(std::remove_if(skerams.begin(), skerams.end(), [bot](Unit* skeram)
    {
        return !IsAttackableSkeramTarget(bot, skeram);
    }), skerams.end());

    if (skerams.empty())
        return nullptr;

    std::sort(skerams.begin(), skerams.end(), [bot, preferLowestHealth](Unit* left, Unit* right)
    {
        bool const leftPrimaryHeld = Aq40BossHelper::IsUnitHeldByEncounterTank(bot, left, true);
        bool const rightPrimaryHeld = Aq40BossHelper::IsUnitHeldByEncounterTank(bot, right, true);
        if (leftPrimaryHeld != rightPrimaryHeld)
            return leftPrimaryHeld > rightPrimaryHeld;

        bool const leftHeld = Aq40BossHelper::IsUnitHeldByEncounterTank(bot, left);
        bool const rightHeld = Aq40BossHelper::IsUnitHeldByEncounterTank(bot, right);
        if (leftHeld != rightHeld)
            return leftHeld > rightHeld;

        if (preferLowestHealth && left->GetHealthPct() != right->GetHealthPct())
            return left->GetHealthPct() < right->GetHealthPct();

        bool const leftLos = bot->IsWithinLOSInMap(left);
        bool const rightLos = bot->IsWithinLOSInMap(right);
        if (leftLos != rightLos)
            return leftLos > rightLos;

        float const leftDistance = bot->GetDistance2d(left);
        float const rightDistance = bot->GetDistance2d(right);
        if (leftDistance != rightDistance)
            return leftDistance < rightDistance;

        if (!preferLowestHealth && left->GetHealthPct() != right->GetHealthPct())
            return left->GetHealthPct() < right->GetHealthPct();

        return left->GetGUID().GetRawValue() < right->GetGUID().GetRawValue();
    });

    return skerams.front();
}

bool HasSkeramSkullTarget(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    return FindSkeramSkullTarget(bot, botAI) != nullptr;
}
}    // namespace Aq40BossActions

bool Aq40SkeramAcquirePlatformTargetAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, encounterUnits);
    if (!target)
        return false;

    if (!Aq40BossHelper::IsEncounterTank(bot, bot) && !Aq40BossActions::HasSkeramSkullTarget(botAI))
    {
        if (Aq40Helpers::IsSkeramPostBlinkHoldActive(bot, botAI, attackers))
            return false;

        if (!Aq40BossHelper::HasAnyNamedUnitHeldByEncounterTank(botAI, bot, encounterUnits, { "the prophet skeram" }, true))
            return false;
    }

    Aq40Helpers::SetRtiTarget(botAI, "skull", target);

    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        Aq40Helpers::SetRaidTargetIcon(bot, target, RtiTargetValue::skullIndex, "skeram", "skull");

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "skeram", "platform", target);
    return Attack(target);
}

bool Aq40SkeramInterruptAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    std::vector<Unit*> skerams =
        Aq40BossActions::FindUnitsByAnyName(botAI, encounterUnits, { "the prophet skeram" });

    if (skerams.empty())
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (currentTarget)
    {
        for (Unit* skeram : skerams)
        {
            if (skeram == currentTarget && skeram->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                Aq40Helpers::LogAq40Info(bot, "interrupt", "skeram:" + Aq40Helpers::GetAq40LogUnit(skeram),
                    "boss=skeram target=" + Aq40Helpers::GetAq40LogUnit(skeram));
                return botAI->DoSpecificAction("interrupt spell", Event(), true);
            }
        }
    }

    Unit* target = nullptr;
    for (Unit* skeram : skerams)
    {
        if (!skeram)
            continue;

        if (skeram->GetCurrentSpell(CURRENT_GENERIC_SPELL))
        {
            target = skeram;
            break;
        }
    }

    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > 22.0f)
        return MoveNear(target, 18.0f, MovementPriority::MOVEMENT_COMBAT);

    Aq40Helpers::LogAq40Target(bot, "skeram", "interrupt", target);
    return Attack(target);
}

bool Aq40SkeramFocusRealBossAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, encounterUnits, true);

    if (!target)
        return false;

    bool const hasSkullTarget = Aq40BossActions::HasSkeramSkullTarget(botAI);
    if (!Aq40BossHelper::IsEncounterTank(bot, bot) && !hasSkullTarget)
    {
        if (Aq40Helpers::IsSkeramPostBlinkHoldActive(bot, botAI, attackers))
            return false;

        if (!Aq40BossHelper::HasAnyNamedUnitHeldByEncounterTank(botAI, bot, encounterUnits, { "the prophet skeram" }, true))
            return false;
    }

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    Aq40Helpers::SetRtiTarget(botAI, "skull", target);

    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "skeram", "execute", target);
    return Attack(target);
}

bool Aq40SkeramControlMindControlAction::Execute(Event /*event*/)
{
    GuidVector const attackers = context->GetValue<GuidVector>("attackers")->Get();
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, attackers);

    if (Aq40BossHelper::TryCrowdControlCharmedPlayer(bot, botAI, encounterUnits))
    {
        Aq40Helpers::LogAq40Info(bot, "mind_control", "skeram:cc", "boss=skeram action=cc");
        return true;
    }

    GuidVector skeramUnits = Aq40Helpers::GetObservedSkeramEncounterUnits(bot, botAI, attackers);
    Unit* target = Aq40BossActions::FindSkeramTarget(botAI, skeramUnits);
    if (!target || (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target))
        return false;

    float const desiredRange = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 24.0f : 4.0f;
    float const engageSlack = (botAI->IsRanged(bot) || botAI->IsHeal(bot)) ? 4.0f : 2.0f;
    if (!bot->IsWithinLOSInMap(target) || bot->GetDistance2d(target) > (desiredRange + engageSlack))
        return MoveNear(target, desiredRange, MovementPriority::MOVEMENT_COMBAT);

    Aq40Helpers::LogAq40Target(bot, "skeram", "mc_fallback", target);
    return Attack(target);
}
