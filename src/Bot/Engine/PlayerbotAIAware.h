/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERbotAIAWARE_H
#define _PLAYERBOT_PLAYERbotAIAWARE_H

class PlayerbotAI;
class BotServiceContainer;

/**
 * @brief Base class for components that need access to bot AI context
 *
 * This class provides two construction modes:
 * 1. Legacy mode: Direct PlayerbotAI pointer (for existing code)
 * 2. DI mode: BotServiceContainer for testable, decoupled code
 *
 * During the refactoring transition, both modes are supported.
 * New code should prefer using the BotServiceContainer constructor.
 */
class PlayerbotAIAware
{
public:
    /**
     * @brief Legacy constructor for direct PlayerbotAI access
     * @param botAI Pointer to the bot's AI instance
     * @deprecated Prefer using the BotServiceContainer constructor for new code
     */
    PlayerbotAIAware(PlayerbotAI* botAI) : botAI(botAI), services_(nullptr) {}

    /**
     * @brief DI constructor for testable, decoupled access
     * @param services Reference to the bot's service container
     *
     * When using this constructor, botAI will be nullptr.
     * Use GetServices() to access bot functionality.
     */
    explicit PlayerbotAIAware(BotServiceContainer& services) : botAI(nullptr), services_(&services) {}

    virtual ~PlayerbotAIAware() = default;

    /**
     * @brief Check if services are available (DI mode)
     * @return true if using service container
     */
    bool HasServices() const { return services_ != nullptr; }

    /**
     * @brief Get the service container
     * @return Pointer to service container, or nullptr if in legacy mode
     */
    BotServiceContainer* GetServices() { return services_; }
    BotServiceContainer const* GetServices() const { return services_; }

protected:
    PlayerbotAI* botAI;              // Legacy: direct AI pointer
    BotServiceContainer* services_;  // DI: service container
};

#endif
