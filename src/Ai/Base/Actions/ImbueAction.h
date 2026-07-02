/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_IMBUEACTION_H
#define PLAYERBOTS_IMBUEACTION_H

#include "Action.h"

class PlayerbotAI;

class ImbueWithPoisonAction : public Action
{
public:
    ImbueWithPoisonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class ImbueWithStoneAction : public Action
{
public:
    ImbueWithStoneAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class ImbueWithOilAction : public Action
{
public:
    ImbueWithOilAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

class TryEmergencyAction : public Action
{
public:
    TryEmergencyAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
};

#endif
