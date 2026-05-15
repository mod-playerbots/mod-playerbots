#ifndef PLAYERBOT_NEWRPGOUTDOORPVP_H
#define PLAYERBOT_NEWRPGOUTDOORPVP_H

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
