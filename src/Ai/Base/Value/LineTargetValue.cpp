/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "LineTargetValue.h"

#include "Playerbots.h"

Unit* LineTargetValue::Calculate()
{
    Player* master = GetMaster();
    if (!master)
        return nullptr;

    Group* group = master->GetGroup();
    if (!group)
        return nullptr;

    Player* prev = master;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* player = ObjectAccessor::FindPlayer(itr->guid);
        if (!player || !player->IsAlive() || player == master)
            continue;

        if (player == bot)
            return prev;

        prev = player;
    }

    return master;
}
