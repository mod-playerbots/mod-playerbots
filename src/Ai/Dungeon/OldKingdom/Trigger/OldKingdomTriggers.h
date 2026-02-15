#ifndef _PLAYERBOT_WOTLKDUNGEONOKTRIGGERS_H
#define _PLAYERBOT_WOTLKDUNGEONOKTRIGGERS_H

#include "Trigger.h"
#include "GenericTriggers.h"

enum OldKingdomIDs
{
    // Elder Nadox
    BUFF_GUARDIAN_AURA                 = 56153,

    // Jedoga Shadowseeker
    NPC_TWILIGHT_VOLUNTEER             = 30385,

    // Forgotten One(s)
    SPELL_SHADOW_CRASH_N               = 60833,
    SPELL_SHADOW_CRASH_H               = 60848,
};

class NadoxGuardianTrigger : public Trigger
{
public:
    NadoxGuardianTrigger(PlayerbotAI* ai) : Trigger(ai, "elder nadox guardian") {}
    bool IsActive() override;
};

class JedogaVolunteerTrigger : public Trigger
{
public:
    JedogaVolunteerTrigger(PlayerbotAI* ai) : Trigger(ai, "jedoga volunteer") {}
    bool IsActive() override;
};

class ShadowCrashTrigger : public Trigger
{
public:
    ShadowCrashTrigger(PlayerbotAI* ai) : Trigger(ai, "shadow crash") {}
    bool IsActive() override;
};

#endif
