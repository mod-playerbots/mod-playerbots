/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_NEWRPGOUTDOORPVP_H
#define _PLAYERBOT_NEWRPGOUTDOORPVP_H

#include "NewRpgBaseAction.h"
#include "OutdoorPvP.h"

class NewRpgOutdoorPvpAction : public NewRpgBaseAction
{
public:
    NewRpgOutdoorPvpAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg outdoor pvp") {}

    virtual bool Execute(Event event) override;
    OPvPCapturePoint* SelectNewObjective(OutdoorPvP::OPvPCapturePointMap const& capturePointMap);

private:
    bool PatrolCapturePoint(GameObject* objectiveGO, float radius);
};

#endif
