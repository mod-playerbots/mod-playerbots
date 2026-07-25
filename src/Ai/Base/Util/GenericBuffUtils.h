/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GENERICBUFFUTILS_H
#define PLAYERBOTS_GENERICBUFFUTILS_H

#include <string>
#include <unordered_map>

#include "Common.h"

class Player;
class PlayerbotAI;
class Unit;

namespace ai::buff
{

typedef std::unordered_map<std::string, uint32> MissingBuffReagentNoticeMap;

bool IsGroupVariantEnabled(Player* bot, std::string const& name);

std::string MakeAuraQualifierForBuff(std::string const& name);

std::string GroupVariantFor(std::string const& name);

bool NeedsPostLoginBuffGrace(std::string const& name);

bool ShouldDeferPartyBuffEvaluationForRecentLogin(
    Player* bot,
    Unit* target,
    std::string const& spell);

bool ShouldDeferGreaterBlessingAssignmentForRecentLogin(Player* bot);

bool HasRequiredReagents(Player* bot, uint32 spellId);

void ClearMissingBuffReagentNotice(PlayerbotAI* botAI, std::string const& groupName);

bool TryAnnounceMissingBuffReagents(
    PlayerbotAI* botAI, std::string const& baseName, std::string const& groupName);

std::string UpgradeToGroupIfAppropriate(
    Player* bot,
    PlayerbotAI* botAI,
    std::string const& baseName,
    std::string* outMissingReagentGroupName = nullptr);

}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId);
}

#endif
