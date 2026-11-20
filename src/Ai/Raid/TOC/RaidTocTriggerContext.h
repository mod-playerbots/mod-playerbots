/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAIDTOCTRIGGERCONTEXT_H
#define PLAYERBOTS_RAIDTOCTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "RaidTocTriggers.h"

class RaidTocTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidTocTriggerContext()
    {
        creators["toc champions chased"] = &RaidTocTriggerContext::toc_champions_chased;
        creators["toc twin valkyr fight"] = &RaidTocTriggerContext::toc_twin_valkyr_fight;
        creators["toc icehowl charge"] = &RaidTocTriggerContext::toc_icehowl_charge;
        creators["toc anub submerge"] = &RaidTocTriggerContext::toc_anub_submerge;
        creators["toc jaraxxus adds"] = &RaidTocTriggerContext::toc_jaraxxus_adds;
        creators["toc jaraxxus nether power"] = &RaidTocTriggerContext::toc_jaraxxus_nether_power;
        creators["toc gormok tank swap"] = &RaidTocTriggerContext::toc_gormok_tank_swap;
        creators["toc burning bile"] = &RaidTocTriggerContext::toc_burning_bile;
    }

private:
    static Trigger* toc_champions_chased(PlayerbotAI* botAI) { return new TocChampionsChasedTrigger(botAI); }
    static Trigger* toc_twin_valkyr_fight(PlayerbotAI* botAI) { return new TocTwinValkyrFightTrigger(botAI); }
    static Trigger* toc_icehowl_charge(PlayerbotAI* botAI) { return new TocIcehowlChargeTrigger(botAI); }
    static Trigger* toc_anub_submerge(PlayerbotAI* botAI) { return new TocAnubSubmergeTrigger(botAI); }
    static Trigger* toc_jaraxxus_adds(PlayerbotAI* botAI) { return new TocJaraxxusAddsTrigger(botAI); }
    static Trigger* toc_jaraxxus_nether_power(PlayerbotAI* botAI) { return new TocJaraxxusNetherPowerTrigger(botAI); }
    static Trigger* toc_gormok_tank_swap(PlayerbotAI* botAI) { return new TocGormokTankSwapTrigger(botAI); }
    static Trigger* toc_burning_bile(PlayerbotAI* botAI) { return new TocBurningBileTrigger(botAI); }
};

#endif
