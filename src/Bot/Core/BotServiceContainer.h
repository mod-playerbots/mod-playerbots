/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_BOT_SERVICE_CONTAINER_H
#define _PLAYERBOT_BOT_SERVICE_CONTAINER_H

#include <memory>

#include "IBotContext.h"
#include "IChatService.h"
#include "IConfigProvider.h"
#include "IItemService.h"
#include "IRoleService.h"
#include "ISpellService.h"

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
        return *_context;
    }

    ISpellService& GetSpellService()
    {
        return *_spellService;
    }

    IChatService& GetChatService()
    {
        return *_chatService;
    }

    IRoleService& GetRoleService()
    {
        return *_roleService;
    }

    IItemService& GetItemService()
    {
        return *_itemService;
    }

    IConfigProvider& GetConfig()
    {
        return *_config;
    }

    // Const accessors
    IBotContext const& GetContext() const
    {
        return *_context;
    }

    ISpellService const& GetSpellService() const
    {
        return *_spellService;
    }

    IChatService const& GetChatService() const
    {
        return *_chatService;
    }

    IRoleService const& GetRoleService() const
    {
        return *_roleService;
    }

    IItemService const& GetItemService() const
    {
        return *_itemService;
    }

    IConfigProvider const& GetConfig() const
    {
        return *_config;
    }

    // Service setters for dependency injection
    void SetContext(std::unique_ptr<IBotContext> context)
    {
        _context = std::move(context);
    }

    void SetSpellService(std::unique_ptr<ISpellService> service)
    {
        _spellService = std::move(service);
    }

    void SetChatService(std::unique_ptr<IChatService> service)
    {
        _chatService = std::move(service);
    }

    void SetRoleService(std::unique_ptr<IRoleService> service)
    {
        _roleService = std::move(service);
    }

    void SetItemService(std::unique_ptr<IItemService> service)
    {
        _itemService = std::move(service);
    }

    void SetConfig(std::unique_ptr<IConfigProvider> config)
    {
        _config = std::move(config);
    }

    // Check if all services are initialized
    bool IsInitialized() const
    {
        return _context && _spellService && _chatService && _roleService && _itemService && _config;
    }

private:
    std::unique_ptr<IBotContext> _context;
    std::unique_ptr<ISpellService> _spellService;
    std::unique_ptr<IChatService> _chatService;
    std::unique_ptr<IRoleService> _roleService;
    std::unique_ptr<IItemService> _itemService;
    std::unique_ptr<IConfigProvider> _config;
};

#endif
