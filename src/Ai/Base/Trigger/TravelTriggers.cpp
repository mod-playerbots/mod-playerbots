/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TravelTriggers.h"

#include "Playerbots.h"
#include "ServerFacade.h"

bool NoTravelTargetTrigger::IsActive() { return !context->GetValue<TravelTarget*>("travel target")->Get()->isActive(); }

bool FarFromTravelTargetTrigger::IsActive()
{
    return context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}

bool NearDarkPortalTrigger::IsActive() { return bot->GetAreaId() == 72; }

bool AtDarkPortalAzerothTrigger::IsActive()
{
    if (bot->GetAreaId() == 72)
    {
        if (ServerFacade::instance().GetDistance2d(bot, -11906.9f, -3208.53f) < 20.0f)
        {
            return true;
        }
    }

    return false;
}

bool AtDarkPortalOutlandTrigger::IsActive()
{
    if (bot->GetAreaId() == 3539)
    {
        if (ServerFacade::instance().GetDistance2d(bot, -248.1939f, 921.919f) < 10.0f)
        {
            return true;
        }
    }

    return false;
}
