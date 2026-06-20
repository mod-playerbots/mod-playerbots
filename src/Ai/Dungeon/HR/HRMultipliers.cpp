#include "HRMultipliers.h"
#include "HRActions.h"
#include "HRTriggers.h"
#include "MovementActions.h"
#include "ReachTargetActions.h"
#include "FollowActions.h"
#include "Playerbots.h"

// Omor the Unscarred

float OmorTreacherousAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return 1.0f;

    if (!bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_TREACHEROUS_AURA)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<OmorTreacherousAuraFleeFromPlayersAction*>(action)))
        return 0.0f;

    return 1.0f;
}

float OmorBaneOfTreacheryAuraFleeFromPlayersMultiplier::GetValue(Action* action)
{
    Unit* omor = AI_VALUE2(Unit*, "find target", "omor the unscarred");
    if (!omor)
        return 1.0f;

    if (!bot->HasAura(static_cast<uint32>(HellfireRampartsIDs::SPELL_BANE_OF_TREACHERY)))
        return 1.0f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) &&
         !dynamic_cast<OmorBaneOfTreacheryAuraFleeFromPlayersAction*>(action)))
        return 0.0f;

    return 1.0f;
}