#include "RaidBwlHelpers.h"

namespace BlackwingLairHelpers
{
    bool IsActiveSuppressionDeviceInRange(const GameObject* go, const Player* bot)
    {
        return go &&
               go->GetEntry() == GO_SUPPRESSION_DEVICE &&
               go->GetDistance(bot) < 15.0f &&
               go->GetGoState() == GO_STATE_READY;
    }
}
