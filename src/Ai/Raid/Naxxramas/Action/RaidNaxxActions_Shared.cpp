#include "MovementActions.h"
#include "Player.h"

uint32_t RotateAroundTheCenterPointAction::FindNearestWaypoint()
{
    float minDistance = 0.0f;
    int ret = -1;

    for (uint32_t i = 0; i < this->intervals; ++i)
    {
        const float w_x = this->waypoints[i].first, w_y = this->waypoints[i].second;
        const float dis = this->bot->GetDistance2d(w_x, w_y);

        if (ret == -1 || dis < minDistance)
        {
            ret = i;
            minDistance = dis;
        }
    }

    return ret;
}
