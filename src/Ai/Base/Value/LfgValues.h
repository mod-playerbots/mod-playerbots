/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_LFGVALUES_H
#define PLAYERBOTS_LFGVALUES_H

#include "Value.h"

class PlayerbotAI;

class LfgProposalValue : public ManualSetValue<uint32>
{
public:
    LfgProposalValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "lfg proposal") {}
};

#endif
