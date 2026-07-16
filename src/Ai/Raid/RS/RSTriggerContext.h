/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSTRIGGERCONTEXT_H
#define PLAYERBOTS_RSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "RSTriggers.h"

class RaidRsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RaidRsTriggerContext()
    {
        creators["rs baltharus brand"] = &RaidRsTriggerContext::rs_baltharus_brand;
        creators["rs baltharus tank position"] = &RaidRsTriggerContext::rs_baltharus_tank_position;
        creators["rs baltharus avoid front"] = &RaidRsTriggerContext::rs_baltharus_avoid_front;
        creators["rs baltharus healer position"] = &RaidRsTriggerContext::rs_baltharus_healer_position;
        creators["rs saviana conflagration"] = &RaidRsTriggerContext::rs_saviana_conflagration;
        creators["rs saviana avoid front"] = &RaidRsTriggerContext::rs_saviana_avoid_front;
        creators["rs saviana tank position"] = &RaidRsTriggerContext::rs_saviana_tank_position;
        creators["rs saviana melee spread"] = &RaidRsTriggerContext::rs_saviana_melee_spread;
        creators["rs zarithrian adds"] = &RaidRsTriggerContext::rs_zarithrian_adds;
        creators["rs zarithrian tank"] = &RaidRsTriggerContext::rs_zarithrian_tank;
        creators["rs halion tank position"] = &RaidRsTriggerContext::rs_halion_tank_position;
        creators["rs halion avoid cones"] = &RaidRsTriggerContext::rs_halion_avoid_cones;
        creators["rs halion combustion"] = &RaidRsTriggerContext::rs_halion_combustion;
        creators["rs halion meteor"] = &RaidRsTriggerContext::rs_halion_meteor;
        creators["rs halion adds"] = &RaidRsTriggerContext::rs_halion_adds;
        creators["rs halion add tank"] = &RaidRsTriggerContext::rs_halion_add_tank;
        creators["rs halion start position"] = &RaidRsTriggerContext::rs_halion_start_position;
        creators["rs halion enter portal"] = &RaidRsTriggerContext::rs_halion_enter_portal;
        creators["rs halion p2 tank position"] = &RaidRsTriggerContext::rs_halion_p2_tank_position;
        creators["rs halion p2 avoid cones"] = &RaidRsTriggerContext::rs_halion_p2_avoid_cones;
        creators["rs halion consumption"] = &RaidRsTriggerContext::rs_halion_consumption;
        creators["rs halion cutter"] = &RaidRsTriggerContext::rs_halion_cutter;
        creators["rs halion heal consumption"] = &RaidRsTriggerContext::rs_halion_heal_consumption;
        creators["rs trash adds"] = &RaidRsTriggerContext::rs_trash_adds;
        creators["rs trash main tank"] = &RaidRsTriggerContext::rs_trash_main_tank;
        creators["rs trash assist tank"] = &RaidRsTriggerContext::rs_trash_assist_tank;
        creators["rs trash ranged"] = &RaidRsTriggerContext::rs_trash_ranged;
        creators["rs trash melee flank"] = &RaidRsTriggerContext::rs_trash_melee_flank;
    }

private:
    static Trigger* rs_baltharus_brand(PlayerbotAI* ai) { return new RsBaltharusBrandTrigger(ai); }
    static Trigger* rs_baltharus_tank_position(PlayerbotAI* ai) { return new RsBaltharusTankPositionTrigger(ai); }
    static Trigger* rs_baltharus_avoid_front(PlayerbotAI* ai) { return new RsBaltharusAvoidFrontTrigger(ai); }
    static Trigger* rs_baltharus_healer_position(PlayerbotAI* ai) { return new RsBaltharusHealerPositionTrigger(ai); }
    static Trigger* rs_saviana_conflagration(PlayerbotAI* ai) { return new RsSavianaConflagrationTrigger(ai); }
    static Trigger* rs_saviana_avoid_front(PlayerbotAI* ai) { return new RsSavianaAvoidFrontTrigger(ai); }
    static Trigger* rs_saviana_tank_position(PlayerbotAI* ai) { return new RsSavianaTankPositionTrigger(ai); }
    static Trigger* rs_saviana_melee_spread(PlayerbotAI* ai) { return new RsSavianaMeleeSpreadTrigger(ai); }
    static Trigger* rs_zarithrian_adds(PlayerbotAI* ai) { return new RsZarithrianAddsTrigger(ai); }
    static Trigger* rs_zarithrian_tank(PlayerbotAI* ai) { return new RsZarithrianTankTrigger(ai); }
    static Trigger* rs_halion_tank_position(PlayerbotAI* ai) { return new RsHalionTankPositionTrigger(ai); }
    static Trigger* rs_halion_avoid_cones(PlayerbotAI* ai) { return new RsHalionAvoidConesTrigger(ai); }
    static Trigger* rs_halion_combustion(PlayerbotAI* ai) { return new RsHalionCombustionTrigger(ai); }
    static Trigger* rs_halion_meteor(PlayerbotAI* ai) { return new RsHalionMeteorTrigger(ai); }
    static Trigger* rs_halion_adds(PlayerbotAI* ai) { return new RsHalionAddsTrigger(ai); }
    static Trigger* rs_halion_add_tank(PlayerbotAI* ai) { return new RsHalionAddTankTrigger(ai); }
    static Trigger* rs_halion_start_position(PlayerbotAI* ai) { return new RsHalionStartPositionTrigger(ai); }
    static Trigger* rs_halion_enter_portal(PlayerbotAI* ai) { return new RsHalionEnterPortalTrigger(ai); }
    static Trigger* rs_halion_p2_tank_position(PlayerbotAI* ai) { return new RsHalionP2TankPositionTrigger(ai); }
    static Trigger* rs_halion_p2_avoid_cones(PlayerbotAI* ai) { return new RsHalionP2AvoidConesTrigger(ai); }
    static Trigger* rs_halion_consumption(PlayerbotAI* ai) { return new RsHalionConsumptionTrigger(ai); }
    static Trigger* rs_halion_cutter(PlayerbotAI* ai) { return new RsHalionCutterTrigger(ai); }
    static Trigger* rs_halion_heal_consumption(PlayerbotAI* ai) { return new RsHalionHealConsumptionTrigger(ai); }
    static Trigger* rs_trash_adds(PlayerbotAI* ai) { return new RsTrashAddsTrigger(ai); }
    static Trigger* rs_trash_main_tank(PlayerbotAI* ai) { return new RsTrashMainTankTrigger(ai); }
    static Trigger* rs_trash_assist_tank(PlayerbotAI* ai) { return new RsTrashAssistTankTrigger(ai); }
    static Trigger* rs_trash_ranged(PlayerbotAI* ai) { return new RsTrashRangedTrigger(ai); }
    static Trigger* rs_trash_melee_flank(PlayerbotAI* ai) { return new RsTrashMeleeFlankTrigger(ai); }
};

#endif
