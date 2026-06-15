/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_AQ20UTILS_H
#define _PLAYERBOT_AQ20UTILS_H

#include "GameObject.h"
#include "Unit.h"

class RaidAq20Utils
{
public:
    static bool IsOssirianBuffActive(Unit* ossirian);
    static int32 GetOssirianDebuffTimeRemaining(Unit* ossirian);
    static GameObject* GetNearestCrystal(Unit* ossirian);
};

#endif
