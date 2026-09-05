/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GOSSIPHELLOACTION_H
#define PLAYERBOTS_GOSSIPHELLOACTION_H

#include "Action.h"

class PlayerbotAI;

class GossipHelloAction : public Action
{
public:
    GossipHelloAction(PlayerbotAI* botAI) : Action(botAI, "gossip hello") {}

    bool Execute(Event event) override;
    // Overload for direct usage
    bool Execute(ObjectGuid guid, int32 menuToSelect, bool silent = false);

private:
    void TellGossipMenus();
    bool ProcessGossip(int32 menuToSelect, bool silent);
    void TellGossipText(uint32 textId);
};

#endif
