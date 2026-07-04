#include "Aq40Actions.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "RtiTargetValue.h"
#include "../Aq40BossHelper.h"
#include "../Util/Aq40Helpers_Shared.h"

namespace
{
float constexpr kPi = 3.14159265f;

uint32 GetHuhuranSpreadOrdinal(Player* bot, PlayerbotAI* botAI, bool forMelee, uint32& outCohortSize)
{
    Group const* group = bot->GetGroup();
    if (!group)
    {
        outCohortSize = forMelee ? 8u : 12u;
        return static_cast<uint32>(bot->GetGUID().GetCounter() % outCohortSize);
    }

    uint32 index = 0;
    uint32 botOrdinal = 0;
    bool found = false;
    for (GroupReference const* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !Aq40BossHelper::IsNearEncounter(bot, member))
            continue;

        if (Aq40BossHelper::IsEncounterTank(member, member))
            continue;

        PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
        bool memberIsMelee;
        if (memberAI)
            memberIsMelee = !memberAI->IsRanged(member) && !memberAI->IsHeal(member);
        else
            memberIsMelee = false;
        if (forMelee != memberIsMelee)
            continue;

        if (member->GetGUID() == bot->GetGUID())
        {
            botOrdinal = index;
            found = true;
        }

        ++index;
    }

    outCohortSize = std::max(index, forMelee ? 8u : 12u);
    if (found)
        return botOrdinal;

    outCohortSize = forMelee ? 8u : 12u;
    return static_cast<uint32>(bot->GetGUID().GetCounter() % outCohortSize);
}
}    // namespace

namespace Aq40BossActions
{
Unit* FindHuhuranTarget(PlayerbotAI* botAI, GuidVector const& attackers)
{
    return FindUnitByAnyName(botAI, attackers, { "princess huhuran" });
}
}    // namespace Aq40BossActions

bool Aq40HuhuranChooseTargetAction::Execute(Event /*event*/)
{
    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* target = Aq40BossActions::FindHuhuranTarget(botAI, encounterUnits);
    if (!target)
        return false;

    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        Aq40Helpers::SetRaidTargetIcon(bot, target, RtiTargetValue::skullIndex, "huhuran", "skull");

    Aq40Helpers::SetRtiTarget(botAI, "skull", target);

    if (Aq40BossHelper::ShouldWaitForEncounterTankAggro(bot, bot, target, true))
        return false;

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "huhuran", "boss", target);
    return Attack(target);
}

bool Aq40HuhuranPoisonSpreadAction::Execute(Event /*event*/)
{
    if (Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    if (!botAI->IsRanged(bot) && !botAI->IsHeal(bot))
        return false;

    GuidVector encounterUnits = Aq40BossHelper::GetEncounterUnits(botAI, context->GetValue<GuidVector>("attackers")->Get());
    Unit* huhuran = Aq40BossActions::FindHuhuranTarget(botAI, encounterUnits);
    if (!huhuran)
        return false;

    float const desiredDistance = 28.0f;
    uint32 totalSlots = 0;
    uint32 const slot = GetHuhuranSpreadOrdinal(bot, botAI, false, totalSlots);
    float const angle = static_cast<float>(slot) * ((2.0f * kPi) / static_cast<float>(totalSlots));

    float const moveX = huhuran->GetPositionX() + std::cos(angle) * desiredDistance;
    float const moveY = huhuran->GetPositionY() + std::sin(angle) * desiredDistance;

    if (bot->GetDistance2d(moveX, moveY) < 3.0f)
        return false;

    Aq40Helpers::LogAq40Info(bot, "spread_position",
        "huhuran:poison:" + std::to_string(slot),
        "boss=huhuran hazard=poison slot=" + std::to_string(slot));
    return MoveTo(bot->GetMapId(), moveX, moveY, huhuran->GetPositionZ(), false, false, false, true,
                  MovementPriority::MOVEMENT_COMBAT, true, false);
}
