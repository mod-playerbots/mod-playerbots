/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PartyMemberSnaredTargetValue.h"

#include <limits>

#include "MovementHelper.h"
#include "PlayerbotAIAware.h"
#include "Playerbots.h"

class PartyMemberSnaredTargetPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    PartyMemberSnaredTargetPredicate(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) {}

    bool Check(Unit* unit) override
    {
        if (!unit || !unit->IsAlive() || !unit->IsInWorld() || unit == botAI->GetBot())
            return false;

        if (unit->GetMapId() != botAI->GetBot()->GetMapId())
            return false;

        if (!botAI->GetBot()->IsWithinLOSInMap(unit))
            return false;

        if (botAI->GetBot()->GetDistance2d(unit) > botAI->GetRange("spell"))
            return false;

        return ai::movement::IsMovementImpaired(unit);
    }
};

Unit* PartyMemberSnaredTargetValue::Calculate()
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    PartyMemberSnaredTargetPredicate predicate(botAI);
    Player* bestTarget = nullptr;
    float closestDistance = std::numeric_limits<float>::max();

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member)
            continue;

        if (!predicate.Check(member))
            continue;

        float distance = bot->GetDistance2d(member);
        if (distance < closestDistance)
        {
            closestDistance = distance;
            bestTarget = member;
        }
    }

    return bestTarget;
}
