/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDRESISTAURA_H
#define PLAYERBOTS_RAIDRESISTAURA_H

#include <string>

#include "Action.h"
#include "Define.h"
#include "Trigger.h"

class PlayerbotAI;

// Who raises which resistance aura: one bot paladin per wanted school (fire, frost, shadow) and the
// generic nature hunter for Aspect of the Wild. The caller says which schools it wants, and with none
// the paladins go back to their own aura. Nothing resolves off the config's map.

enum class RaidResistSchool : uint8
{
    None,
    Fire,
    Frost,
    Shadow,
    Nature
};

// A set of schools is one bit per school
constexpr uint8 RaidResistBit(RaidResistSchool school)
{
    return static_cast<uint8>(1u << static_cast<uint8>(school));
}

inline constexpr uint8 RAID_RESIST_FIRE = RaidResistBit(RaidResistSchool::Fire);
inline constexpr uint8 RAID_RESIST_FROST = RaidResistBit(RaidResistSchool::Frost);
inline constexpr uint8 RAID_RESIST_SHADOW = RaidResistBit(RaidResistSchool::Shadow);
inline constexpr uint8 RAID_RESIST_NATURE = RaidResistBit(RaidResistSchool::Nature);

struct RaidResistAuraConfig
{
    uint32 mapId;
    // the schools wanted now, given the set the last read wanted
    uint8 (*wantedSchools)(PlayerbotAI* botAI, uint8 previous);
    // fire goes to a paladin with Aura Mastery when it can: the cooldown doubles only its caster's own
    // aura. Null for never.
    bool (*fireWantsMastery)(PlayerbotAI* botAI);
};

// The aura this bot should hold right now
RaidResistSchool GetRaidResistDuty(PlayerbotAI* botAI, RaidResistAuraConfig const& config);

// The class action name, null for None.
char const* RaidResistAuraName(RaidResistSchool school);

// Has a duty but not the aura. A paladin goes by its own application: a weaker duplicate keeps its
// application with the effects stripped, and HasAura by name walks effects.
bool RaidResistAuraMissing(PlayerbotAI* botAI, RaidResistAuraConfig const& config);

bool IsRaidResistHunterHolding(PlayerbotAI* botAI, RaidResistAuraConfig const& config);

// A paladin or hunter whose resistance duty isn't up yet. Register it under a name no encounter gate
// wraps: the wanted schools carry their own window.
class RaidResistAuraTrigger : public Trigger
{
public:
    RaidResistAuraTrigger(PlayerbotAI* botAI, std::string const name, RaidResistAuraConfig const& config)
        : Trigger(botAI, name), _config(config) {}
    bool IsActive() override;

private:
    RaidResistAuraConfig const& _config;
};

// Casts the duty's aura directly, no ChangeStrategy: an added strategy is never taken off again and
// outlives the raid. The hold rules keep the bot's own aura nodes off the slot meanwhile.
class RaidRaiseResistAuraAction : public Action
{
public:
    RaidRaiseResistAuraAction(PlayerbotAI* botAI, std::string const name, RaidResistAuraConfig const& config)
        : Action(botAI, name), _config(config) {}
    bool Execute(Event event) override;
    bool isUseful() override;

private:
    RaidResistAuraConfig const& _config;
};

#endif
