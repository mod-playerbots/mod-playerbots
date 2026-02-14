#include "RaidBossHelpers.h"
#include "Playerbots.h"
#include "RtiTargetValue.h"

// Functions to mark targets with raid target icons
// Note that these functions do not allow the player to change the icon during the encounter
void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
{
    if (!target)
        return;

    if (Group* group = bot->GetGroup())
    {
        ObjectGuid currentGuid = group->GetTargetIcon(iconId);
        if (currentGuid != target->GetGUID())
            group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
    }
}

void MarkTargetWithSkull(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::skullIndex);
}

void MarkTargetWithSquare(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
}

void MarkTargetWithStar(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
}

void MarkTargetWithCircle(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
}

void MarkTargetWithDiamond(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
}

void MarkTargetWithTriangle(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
}

void MarkTargetWithCross(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
}

void MarkTargetWithMoon(Player* bot, Unit* target)
{
    MarkTargetWithIcon(bot, target, RtiTargetValue::moonIndex);
}

// For bots to set their raid target icon to the specified icon on the specified target
void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
{
    if (!target)
        return;

    std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
    Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

    if (currentRti != rtiName || currentTarget != target)
    {
        botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
        botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
    }
}

// Return the first alive DPS bot in the specified instance map, excluding any specified bot
// Intended for purposes of storing and erasing timers and trackers in associative containers
bool IsMechanicTrackerBot(PlayerbotAI* botAI, Player* bot, uint32 mapId, Player* exclude)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != mapId ||
                !GET_PLAYERBOT_AI(member) || !botAI->IsDps(member))
                continue;

            if (member != exclude)
                return member == bot;
        }
    }

    return false;
}

// Return the first matching alive unit from a cell search of nearby npcs
// More responsive than "find target," but performance cost is much higher
// Re: using the third parameter (false by default), some units are never considered
// to be in combat (e.g., totems)
Unit* GetFirstAliveUnitByEntry(PlayerbotAI* botAI, uint32 entry, bool requireInCombat)
{
    auto const& npcs =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (auto const& npcGuid : npcs)
    {
        Unit* unit = botAI->GetUnit(npcGuid);
        if (unit && unit->IsAlive() && unit->GetEntry() == entry)
        {
            if (!requireInCombat || unit->IsInCombat())
                return unit;
        }
    }

    return nullptr;
}

// Return the nearest alive player (human or bot) within the specified radius
Unit* GetNearestPlayerInRadius(Player* bot, float radius)
{
    Unit* nearestPlayer = nullptr;
    float nearestDistance = radius;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;

            float distance = bot->GetExactDist2d(member);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestPlayer = member;
            }
        }
    }

    return nearestPlayer;
}
