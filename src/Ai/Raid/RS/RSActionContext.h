/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RSACTIONCONTEXT_H
#define PLAYERBOTS_RSACTIONCONTEXT_H

#include "Action.h"
#include "NamedObjectContext.h"
#include "RSActions.h"

class RaidRsActionContext : public NamedObjectContext<Action>
{
public:
    RaidRsActionContext()
    {
        creators["rs baltharus brand"] = &RaidRsActionContext::rs_baltharus_brand;
        creators["rs baltharus tank position"] = &RaidRsActionContext::rs_baltharus_tank_position;
        creators["rs baltharus avoid front"] = &RaidRsActionContext::rs_baltharus_avoid_front;
        creators["rs baltharus healer position"] = &RaidRsActionContext::rs_baltharus_healer_position;
        creators["rs saviana conflagration"] = &RaidRsActionContext::rs_saviana_conflagration;
        creators["rs saviana avoid front"] = &RaidRsActionContext::rs_saviana_avoid_front;
        creators["rs saviana tank position"] = &RaidRsActionContext::rs_saviana_tank_position;
        creators["rs saviana melee spread"] = &RaidRsActionContext::rs_saviana_melee_spread;
        creators["rs zarithrian adds"] = &RaidRsActionContext::rs_zarithrian_adds;
        creators["rs zarithrian tank"] = &RaidRsActionContext::rs_zarithrian_tank;
        creators["rs halion tank position"] = &RaidRsActionContext::rs_halion_tank_position;
        creators["rs halion avoid cones"] = &RaidRsActionContext::rs_halion_avoid_cones;
        creators["rs halion combustion"] = &RaidRsActionContext::rs_halion_combustion;
        creators["rs halion meteor"] = &RaidRsActionContext::rs_halion_meteor;
        creators["rs halion adds"] = &RaidRsActionContext::rs_halion_adds;
        creators["rs halion add tank"] = &RaidRsActionContext::rs_halion_add_tank;
        creators["rs halion start position"] = &RaidRsActionContext::rs_halion_start_position;
        creators["rs halion enter portal"] = &RaidRsActionContext::rs_halion_enter_portal;
        creators["rs halion p2 tank position"] = &RaidRsActionContext::rs_halion_p2_tank_position;
        creators["rs halion p2 avoid cones"] = &RaidRsActionContext::rs_halion_p2_avoid_cones;
        creators["rs halion consumption"] = &RaidRsActionContext::rs_halion_consumption;
        creators["rs halion cutter"] = &RaidRsActionContext::rs_halion_cutter;
        creators["rs halion heal consumption"] = &RaidRsActionContext::rs_halion_heal_consumption;
        creators["rs trash adds"] = &RaidRsActionContext::rs_trash_adds;
        creators["rs trash main tank"] = &RaidRsActionContext::rs_trash_main_tank;
        creators["rs trash assist tank"] = &RaidRsActionContext::rs_trash_assist_tank;
        creators["rs trash ranged"] = &RaidRsActionContext::rs_trash_ranged;
    }

private:
    static Action* rs_baltharus_brand(PlayerbotAI* ai) { return new RsBaltharusBrandAction(ai); }
    static Action* rs_baltharus_tank_position(PlayerbotAI* ai) { return new RsBaltharusTankPositionAction(ai); }
    static Action* rs_baltharus_avoid_front(PlayerbotAI* ai) { return new RsBaltharusAvoidFrontAction(ai); }
    static Action* rs_baltharus_healer_position(PlayerbotAI* ai) { return new RsBaltharusHealerPositionAction(ai); }
    static Action* rs_saviana_conflagration(PlayerbotAI* ai) { return new RsSavianaConflagrationAction(ai); }
    static Action* rs_saviana_avoid_front(PlayerbotAI* ai) { return new RsSavianaAvoidFrontAction(ai); }
    static Action* rs_saviana_tank_position(PlayerbotAI* ai) { return new RsSavianaTankPositionAction(ai); }
    static Action* rs_saviana_melee_spread(PlayerbotAI* ai) { return new RsSavianaMeleeSpreadAction(ai); }
    static Action* rs_zarithrian_adds(PlayerbotAI* ai) { return new RsZarithrianAddsAction(ai); }
    static Action* rs_zarithrian_tank(PlayerbotAI* ai) { return new RsZarithrianTankAction(ai); }
    static Action* rs_halion_tank_position(PlayerbotAI* ai) { return new RsHalionTankPositionAction(ai); }
    static Action* rs_halion_avoid_cones(PlayerbotAI* ai) { return new RsHalionAvoidConesAction(ai); }
    static Action* rs_halion_combustion(PlayerbotAI* ai) { return new RsHalionCombustionAction(ai); }
    static Action* rs_halion_meteor(PlayerbotAI* ai) { return new RsHalionMeteorAction(ai); }
    static Action* rs_halion_adds(PlayerbotAI* ai) { return new RsHalionAddsAction(ai); }
    static Action* rs_halion_add_tank(PlayerbotAI* ai) { return new RsHalionAddTankAction(ai); }
    static Action* rs_halion_start_position(PlayerbotAI* ai) { return new RsHalionStartPositionAction(ai); }
    static Action* rs_halion_enter_portal(PlayerbotAI* ai) { return new RsHalionEnterPortalAction(ai); }
    static Action* rs_halion_p2_tank_position(PlayerbotAI* ai) { return new RsHalionP2TankPositionAction(ai); }
    static Action* rs_halion_p2_avoid_cones(PlayerbotAI* ai) { return new RsHalionP2AvoidConesAction(ai); }
    static Action* rs_halion_consumption(PlayerbotAI* ai) { return new RsHalionConsumptionAction(ai); }
    static Action* rs_halion_cutter(PlayerbotAI* ai) { return new RsHalionCutterAction(ai); }
    static Action* rs_halion_heal_consumption(PlayerbotAI* ai) { return new RsHalionHealConsumptionAction(ai); }
    static Action* rs_trash_adds(PlayerbotAI* ai) { return new RsTrashAddsAction(ai); }
    static Action* rs_trash_main_tank(PlayerbotAI* ai) { return new RsTrashMainTankAction(ai); }
    static Action* rs_trash_assist_tank(PlayerbotAI* ai) { return new RsTrashAssistTankAction(ai); }
    static Action* rs_trash_ranged(PlayerbotAI* ai) { return new RsTrashRangedAction(ai); }
};

#endif
