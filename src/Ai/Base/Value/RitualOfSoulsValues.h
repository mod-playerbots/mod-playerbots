/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RITUALOFSOULSVALUES_H
#define PLAYERBOTS_RITUALOFSOULSVALUES_H

#include "ObjectGuid.h"
#include "Value.h"

class PlayerbotAI;

constexpr uint32 SPELL_RITUAL_OF_SOULS_R1 = 29893;
constexpr uint32 SPELL_RITUAL_OF_SOULS_R2 = 58887;
constexpr uint32 GO_SOULWELL_R1 = 181621;
constexpr uint32 GO_SOULWELL_R2 = 193169;
constexpr float RITUAL_OF_SOULS_GO_RANGE = 40.0f;

// Soul Portal of another group member in range, never the bot's own.
class RitualOfSoulsPortalValue : public CalculatedValue<ObjectGuid>
{
public:
    RitualOfSoulsPortalValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "ritual of souls portal", 2) {}

    ObjectGuid Calculate() override;
};

// Only searched for a few minutes after a group bot saw a portal, so a soulwell
// whose portal nobody saw stays invisible.
class RitualOfSoulsSoulwellValue : public CalculatedValue<ObjectGuid>
{
public:
    RitualOfSoulsSoulwellValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "ritual of souls soulwell", 2) {}

    ObjectGuid Calculate() override;
};

#endif
