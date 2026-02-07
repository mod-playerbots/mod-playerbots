/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "ChatCommandTrigger.h"
#include "Player.h"

ChatCommandTrigger::ChatCommandTrigger(PlayerbotAI* botAI, std::string const command)
    : Trigger(botAI, command), triggered(false), owner(nullptr)
{
}

void ChatCommandTrigger::ExternalEvent(std::string const paramName, Player* eventPlayer)
{
    param = paramName;
    owner = eventPlayer;
    triggered = true;

    if (eventPlayer)
        LOG_ERROR("playerbots", "{} chat command event {} {}", this->getName(),paramName, eventPlayer->GetName());
    else
        LOG_ERROR("playerbots", "{} chat command event {}",this->getName(), paramName);

}

Event ChatCommandTrigger::Check()
{
    if (!triggered)
        return Event();

    LOG_ERROR("playerbots", "returning event {} {} {}",getName(),  param, owner->GetName());

    return Event(getName(), param, owner);
}

void ChatCommandTrigger::Reset() { triggered = false; }
