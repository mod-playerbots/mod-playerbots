/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEWRPGOUTDOORPVP_H
#define PLAYERBOTS_NEWRPGOUTDOORPVP_H

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
