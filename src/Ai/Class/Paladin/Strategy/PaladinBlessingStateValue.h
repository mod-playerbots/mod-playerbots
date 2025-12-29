/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "Common.h"
#include "ObjectGuid.h"
#include "Value.h"

class Player;
class PlayerbotAI;

enum class PaladinBlessingRole : uint8
{
    Bstats,
    Bmana,
    Bdps
};

struct PaladinBlessingRoleState
{
    ObjectGuid designated;
    bool hasWearer = false;
};

struct PaladinBlessingState
{
    uint32 paladinCount = 0;
    bool inGroup = false;
    PaladinBlessingRoleState bstats;
    PaladinBlessingRoleState bmana;
    PaladinBlessingRoleState bdps;

    bool IsSolo() const { return paladinCount <= 1u; }
    bool IsDesignated(Player* bot, PaladinBlessingRole role) const;

private:
    PaladinBlessingRoleState const& GetRoleState(PaladinBlessingRole role) const;
};

class PaladinBlessingStateValue : public CalculatedValue<PaladinBlessingState>
{
public:
    PaladinBlessingStateValue(PlayerbotAI* botAI)
        : CalculatedValue<PaladinBlessingState>(botAI, "paladin blessing state", 2 * 1000)
    {
    }

    PaladinBlessingState Calculate() override;
};