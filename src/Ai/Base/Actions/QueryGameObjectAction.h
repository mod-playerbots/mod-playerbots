/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_QUERYGAMEOBJECTACTION_H
#define PLAYERBOTS_QUERYGAMEOBJECTACTION_H

#include "Action.h"

class PlayerbotAI;
class GameObject;

// Dumps every dynamic detail the server holds about a gameobject (template, spawn,
// update fields, lock, loot, display data, and the bot's own perception of it).
class QueryGameObjectAction : public Action
{
public:
    QueryGameObjectAction(PlayerbotAI* botAI, std::string const name = "query game object")
        : Action(botAI, name)
    {
    }

    bool Execute(Event event) override;

private:
    GameObject* FindTarget(std::string const& param);
    void DumpGameObject(GameObject* go);
};

#endif
