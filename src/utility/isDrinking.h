#ifndef IS_DRINKING_H
#define IS_DRINKING_H

#include <vector>
#include <cstdint>

#include "Unit.h"
#include "Player.h"

inline bool isDrinking(Player* player)
{
    if (!player->IsSitState())
        return false;

    std::vector<uint32_t> drinkAuras = {
        430,   431,   432,   438,   833,  1133,
        1135,  1137,  2639, 18071, 22734, 23692,
        24384, 25690, 25691, 25990, 27089, 33772,
        34291, 41031, 42309, 43182, 43183, 43523,
        43706, 44166, 45020, 52911, 57070, 57085,
        57096, 57098, 57101, 57106, 57301, 57335,
        57341, 57343, 57344, 57354, 57359, 57364,
        57366, 57370, 57426, 58067, 58465, 58474,
        61828, 64056, 64354, 65418, 65419, 65420,
        65421, 65422, 66476
    };

    bool hasAura = player->HasAuras(SearchMethod::MatchAny, drinkAuras);

    return hasAura;
}

#endif
