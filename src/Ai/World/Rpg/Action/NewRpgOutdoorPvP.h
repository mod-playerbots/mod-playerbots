#ifndef PLAYERBOT_NEWRPGOUTDOORPVP_H
#define PLAYERBOT_NEWRPGOUTDOORPVP_H

#include "NewRpgBaseAction.h"
#include "OutdoorPvP.h"

class NewRpgOutdoorPvpAction : public NewRpgBaseAction
{
public:
    NewRpgOutdoorPvpAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg outdoor pvp") {}

    virtual bool Execute(Event event) override;
    OPvPCapturePoint* SelectNewObjective();

protected:
    void GetCapturePoints();
    bool PatrolCapturePoint(GameObject* objectiveGO, float radius);

private:
    OutdoorPvP::OPvPCapturePointMap* capturePointMap = nullptr;
    OutdoorPvP* outdoorPvP = nullptr;
};

#endif
