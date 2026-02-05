/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#pragma once

#include "Action.h"

class PlayerbotAI;
class Unit;

class Trigger : public AiNamedObject
{
public:
    Trigger(
        PlayerbotAI* botAI,
        const std::string name = "trigger",
        int32_t checkInterval = 1
    );

    virtual ~Trigger() {}

    virtual Event Check();
    virtual void ExternalEvent([[maybe_unused]] std::string const param, [[maybe_unused]] Player* owner = nullptr) {}
    virtual void ExternalEvent([[maybe_unused]] WorldPacket& packet, [[maybe_unused]] Player* owner = nullptr) {}
    virtual bool IsActive() { return false; }
    virtual std::vector<NextAction> getHandlers() { return {}; }
    void Update() {}
    virtual void Reset() {}
    virtual Unit* GetTarget();
    virtual Value<Unit*>* GetTargetValue();
    virtual std::string const GetTargetName() { return "self target"; }

    bool needCheck(uint32 now);

protected:
    int32_t checkInterval;
    uint32_t lastCheckTime;
};

class TriggerNode
{
public:
    TriggerNode(
        const std::string& name,
        std::vector<NextAction> handlers = {}
    ) :
    trigger(nullptr),
    handlers(std::move(handlers)),
    name(name)
    {}

    Trigger* getTrigger() { return trigger; }
    void setTrigger(Trigger* trigger) { this->trigger = trigger; }
    const std::string getName() { return name; }

    std::vector<NextAction> getHandlers()
    {
        std::vector<NextAction> result = this->handlers;

        if (trigger != nullptr)
        {
            std::vector<NextAction> extra = trigger->getHandlers();
            result.insert(result.end(), extra.begin(), extra.end());
        }

        return result;
    }

    float getFirstRelevance()
    {
        if (this->handlers.size() > 0)
            return this->handlers[0].getRelevance();

        return -1;
    }

private:
    Trigger* trigger;
    std::vector<NextAction> handlers;
    const std::string name;
};
