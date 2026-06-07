/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>
#include "Common.h"

class Player;
class PlayerbotAI;
class Unit;

namespace ai::buff
{

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

std::string UpgradeToGroupIfAppropriate(
    Player* bot,
    PlayerbotAI* botAI,
    std::string const& baseName);

}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId);
}
