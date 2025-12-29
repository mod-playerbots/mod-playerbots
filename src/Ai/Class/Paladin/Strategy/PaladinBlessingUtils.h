/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include <string>

class Player;
class PlayerbotAI;
class Unit;

namespace ai::paladin
{
    std::string GetActualBlessingOfMight(Unit* target);
    std::string GetActualBlessingOfWisdom(Unit* target);
    std::string GetActualBlessingOfSanctuary(Unit* target, Player* bot, PlayerbotAI* botAI);
}  // namespace ai::paladin