#ifndef PLAYERBOTS_BWLTRIGGERCONTEXT_H
#define PLAYERBOTS_BWLTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "BWLTriggers.h"

class RaidBwlTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidBwlTriggerContext()
    {
        creators["bwl suppression device"] = &RaidBwlTriggerContext::bwl_suppression_device;
        creators["bwl affliction bronze"] = &RaidBwlTriggerContext::bwl_affliction_bronze;
        creators["bwl wild magic"] = &RaidBwlTriggerContext::bwl_wild_magic;
        creators["bwl nefarian fear ward"] = &RaidBwlTriggerContext::bwl_nefarian_fear_ward;
        creators["bwl death talon wyrmguard tank"] = &RaidBwlTriggerContext::bwl_death_talon_wyrmguard_tank;
        creators["bwl death talon wyrmguard ranged"] = &RaidBwlTriggerContext::bwl_death_talon_wyrmguard_ranged;
    }

private:
    static Trigger* bwl_suppression_device(PlayerbotAI* ai) { return new BwlSuppressionDeviceTrigger(ai); }
    static Trigger* bwl_affliction_bronze(PlayerbotAI* ai) { return new BwlAfflictionBronzeTrigger(ai); }
    static Trigger* bwl_wild_magic(PlayerbotAI* ai) { return new BwlWildMagicTrigger(ai); }
    static Trigger* bwl_nefarian_fear_ward(PlayerbotAI* ai) { return new BwlNefarianFearWardTrigger(ai); }
    static Trigger* bwl_death_talon_wyrmguard_tank(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardTankTrigger(ai); }
    static Trigger* bwl_death_talon_wyrmguard_ranged(PlayerbotAI* ai) { return new BwlDeathTalonWyrmguardRangedTrigger(ai); }
};

#endif
