/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_NORTHRENDBEASTS_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_NORTHRENDBEASTS_H

#include <string>
#include <vector>

#include "Define.h"
#include "NamedObjectContext.h"
#include "Trigger.h"

enum class NorthrendBeastsStem : uint8
{
    Gormok,  // snobolds too
    Worms,
    Icehowl
};

// One per stem, named with its prefix since the gate keys on that. Fires only for the stem's own
// beasts, so the three never act on one pick.
class NorthrendBeastsDpsTargetTrigger : public Trigger
{
public:
    NorthrendBeastsDpsTargetTrigger(PlayerbotAI* botAI, std::string const name, NorthrendBeastsStem ownStem)
        : Trigger(botAI, name), _stem(ownStem) {}
    bool IsActive() override;

private:
    NorthrendBeastsStem const _stem;
};

// Triggers shared by Gormok, the Jormungars and Icehowl.
class ToCNorthrendBeastsTriggerContext : public NamedObjectContext<Trigger>
{
};

inline void AddToCNorthrendBeastsTriggerNodes(std::vector<TriggerNode*>& /*triggers*/) {}

#endif
