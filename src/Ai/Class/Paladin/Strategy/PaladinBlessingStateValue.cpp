/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinBlessingStateValue.h"

#include <algorithm>
#include <vector>

#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace
{
    constexpr char const* BstatsStrategy = "bstats";
    constexpr char const* BmanaStrategy = "bmana";
    constexpr char const* BdpsStrategy = "bdps";

    void FillRoleState(PaladinBlessingRoleState& roleState, std::vector<ObjectGuid> const& candidates)
    {
        roleState.hasWearer = !candidates.empty();
        if (!roleState.hasWearer)
            return;

        roleState.designated = *std::min_element(candidates.begin(), candidates.end());
    }
}  // namespace

PaladinBlessingRoleState const& PaladinBlessingState::GetRoleState(PaladinBlessingRole role) const
{
    switch (role)
    {
        case PaladinBlessingRole::Bstats:
            return bstats;
        case PaladinBlessingRole::Bmana:
            return bmana;
        case PaladinBlessingRole::Bdps:
            return bdps;
        default:
            return bstats;
    }
}

bool PaladinBlessingState::IsDesignated(Player* bot, PaladinBlessingRole role) const
{
    if (!bot)
        return false;

    if (!inGroup)
        return true;

    PaladinBlessingRoleState const& roleState = GetRoleState(role);
    if (!roleState.hasWearer)
        return true;

    return bot->GetGUID() == roleState.designated;
}

PaladinBlessingState PaladinBlessingStateValue::Calculate()
{
    PaladinBlessingState state;
    Player* bot = botAI->GetBot();
    if (!bot)
        return state;

    Group* group = bot->GetGroup();
    if (!group)
    {
        state.inGroup = false;
        state.paladinCount = (bot->getClass() == CLASS_PALADIN) ? 1u : 0u;
        return state;
    }

    state.inGroup = true;

    std::vector<ObjectGuid> bstatsCandidates;
    std::vector<ObjectGuid> bmanaCandidates;
    std::vector<ObjectGuid> bdpsCandidates;

    for (GroupReference* memberRef = group->GetFirstMember(); memberRef; memberRef = memberRef->next())
    {
        Player* member = memberRef->GetSource();
        if (!member || !member->IsInWorld() || member->getClass() != CLASS_PALADIN)
            continue;

        ++state.paladinCount;

        PlayerbotAI* otherAI = GET_PLAYERBOT_AI(member);
        if (!otherAI)
            continue;

        if (otherAI->HasStrategy(BstatsStrategy, BOT_STATE_NON_COMBAT))
            bstatsCandidates.push_back(member->GetGUID());
        if (otherAI->HasStrategy(BmanaStrategy, BOT_STATE_NON_COMBAT))
            bmanaCandidates.push_back(member->GetGUID());
        if (otherAI->HasStrategy(BdpsStrategy, BOT_STATE_NON_COMBAT))
            bdpsCandidates.push_back(member->GetGUID());
    }

    FillRoleState(state.bstats, bstatsCandidates);
    FillRoleState(state.bmana, bmanaCandidates);
    FillRoleState(state.bdps, bdpsCandidates);

    return state;
}