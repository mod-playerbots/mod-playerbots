#include "RaidNaxxActions.h"

uint32 RotateAroundTheCenterPointAction::FindNearestWaypoint()
{
    float minDistance = 0;
    int ret = -1;
    for (int i = 0; i < intervals; i++)
    {
        float w_x = waypoints[i].first, w_y = waypoints[i].second;
        float dis = bot->GetDistance2d(w_x, w_y);
        if (ret == -1 || dis < minDistance)
        {
            ret = i;
            minDistance = dis;
        }
    }
    return ret;
}
