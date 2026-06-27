#include "RaidAq40Actions.h"

#include <algorithm>

#include "../../RaidBossHelpers.h"

namespace
{
uint32 constexpr kSkullIndex = 7;

bool IsAttackableSkeramTarget(Player* bot, Unit* target)
{
    return bot && target && target->IsInWorld() && target->IsAlive() && target->GetMapId() == bot->GetMapId() &&
           !target->IsFriendlyTo(bot) && (target->GetUnitFlags() & UNIT_FLAG_NOT_SELECTABLE) != UNIT_FLAG_NOT_SELECTABLE;
}

Unit* FindSkeramSkullTarget(Player* bot, PlayerbotAI* botAI)
{
    if (!bot || !botAI)
        return nullptr;

    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // Skull is the authoritative "real boss" marker — the base AI system
    // already auto-focuses skull via DpsTargetValue and FindTargetStrategy.
    // We only need this lookup for the fallback sort logic below.
    ObjectGuid const skullGuid = group->GetTargetIcon(kSkullIndex);
    if (!skullGuid)
        return nullptr;

    Unit* skullTarget = botAI->GetUnit(skullGuid);
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

    // Skull-marked Skeram takes absolute priority — this is the authoritative
    // "real boss" signal set by the encounter tank, immune to blink/aggro
    // confusion.
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
