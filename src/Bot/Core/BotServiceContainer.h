/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_SERVICE_CONTAINER_H
#define _PLAYERBOT_BOT_SERVICE_CONTAINER_H

#include <memory>

class IBotContext;
class ISpellService;
class IChatService;
class IRoleService;
class IItemService;
class IConfigProvider;

/**
 * @brief Container for all bot services
 *
 * This class aggregates all service interfaces needed by bot components,
 * providing a single point of access for dependency injection.
 *
 * Usage in production:
 * @code
 * BotServiceContainer services;
 * services.Initialize(playerbotAI);  // Wraps existing PlayerbotAI
 * @endcode
 *
 * Usage in tests:
 * @code
 * BotServiceContainer services;
 * services.SetContext(std::make_unique<MockBotContext>());
 * services.SetSpellService(std::make_unique<MockSpellService>());
 * // etc.
 * @endcode
 */
class BotServiceContainer
{
public:
    BotServiceContainer() = default;
    ~BotServiceContainer() = default;

    // Prevent copying
    BotServiceContainer(BotServiceContainer const&) = delete;
    BotServiceContainer& operator=(BotServiceContainer const&) = delete;

    // Allow moving
    BotServiceContainer(BotServiceContainer&&) = default;
    BotServiceContainer& operator=(BotServiceContainer&&) = default;

    // Service accessors
    IBotContext& GetContext()
    {
        return *context_;
    }

    ISpellService& GetSpellService()
    {
        return *spellService_;
    }

    IChatService& GetChatService()
    {
        return *chatService_;
    }

    IRoleService& GetRoleService()
    {
        return *roleService_;
    }

    IItemService& GetItemService()
    {
        return *itemService_;
    }

    IConfigProvider& GetConfig()
    {
        return *config_;
    }

    // Const accessors
    IBotContext const& GetContext() const
    {
        return *context_;
    }

    ISpellService const& GetSpellService() const
    {
        return *spellService_;
    }

    IChatService const& GetChatService() const
    {
        return *chatService_;
    }

    IRoleService const& GetRoleService() const
    {
        return *roleService_;
    }

    IItemService const& GetItemService() const
    {
        return *itemService_;
    }

    IConfigProvider const& GetConfig() const
    {
        return *config_;
    }

    // Service setters for dependency injection
    void SetContext(std::unique_ptr<IBotContext> context)
    {
        context_ = std::move(context);
    }

    void SetSpellService(std::unique_ptr<ISpellService> service)
    {
        spellService_ = std::move(service);
    }

    void SetChatService(std::unique_ptr<IChatService> service)
    {
        chatService_ = std::move(service);
    }

    void SetRoleService(std::unique_ptr<IRoleService> service)
    {
        roleService_ = std::move(service);
    }

    void SetItemService(std::unique_ptr<IItemService> service)
    {
        itemService_ = std::move(service);
    }

    void SetConfig(std::unique_ptr<IConfigProvider> config)
    {
        config_ = std::move(config);
    }

    // Check if all services are initialized
    bool IsInitialized() const
    {
        return context_ && spellService_ && chatService_ && roleService_ && itemService_ && config_;
    }

private:
    std::unique_ptr<IBotContext> context_;
    std::unique_ptr<ISpellService> spellService_;
    std::unique_ptr<IChatService> chatService_;
    std::unique_ptr<IRoleService> roleService_;
    std::unique_ptr<IItemService> itemService_;
    std::unique_ptr<IConfigProvider> config_;
};

#endif
