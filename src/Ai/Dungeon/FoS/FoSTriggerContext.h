/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSTRIGGERCONTEXT_H
#define PLAYERBOTS_FOSTRIGGERCONTEXT_H

#include "FoSTriggers.h"
#include "NamedObjectContext.h"

class WotlkDungeonFoSTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WotlkDungeonFoSTriggerContext()
    {
        creators["bronjahm position"] = &WotlkDungeonFoSTriggerContext::bronjahm_position;
        creators["move from bronjahm"] = &WotlkDungeonFoSTriggerContext::move_from_bronjahm;
        creators["switch to soul fragment"] = &WotlkDungeonFoSTriggerContext::switch_to_soul_fragment;
        creators["devourer of souls"] = &WotlkDungeonFoSTriggerContext::devourer_of_souls;
    }

private:
    static Trigger* move_from_bronjahm(PlayerbotAI* ai) { return new MoveFromBronjahmTrigger(ai); }
    static Trigger* switch_to_soul_fragment(PlayerbotAI* ai) { return new SwitchToSoulFragment(ai); }
    static Trigger* bronjahm_position(PlayerbotAI* ai) { return new BronjahmPositionTrigger(ai); }
    static Trigger* devourer_of_souls(PlayerbotAI* ai) { return new DevourerOfSoulsTrigger(ai); }
};

#endif  // !_PLAYERBOT_WOTLKDUNGEONFOSTRIGGERCONTEXT_H
