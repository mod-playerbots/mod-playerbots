/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MANAGER_REGISTRY_H
#define _PLAYERBOT_MANAGER_REGISTRY_H

#include <memory>
#include <typeindex>
#include <unordered_map>

class ITravelManager;
class IRandomBotManager;
class IBotRepository;

/**
 * @brief Central registry for all manager interfaces
 *
 * This class provides a single point of access for all manager interfaces,
 * replacing direct singleton access with dependency injection support.
 *
 * Usage in production:
 * @code
 * // Get the global registry
 * auto& registry = ManagerRegistry::Instance();
 *
 * // Access managers through interfaces
 * auto& travelMgr = registry.GetTravelManager();
 * auto& randomBotMgr = registry.GetRandomBotManager();
 * @endcode
 *
 * Usage in tests:
 * @code
 * ManagerRegistry registry;
 * registry.SetTravelManager(std::make_shared<MockTravelManager>());
 * // Use registry in tests...
 * @endcode
 */
class ManagerRegistry
{
public:
    ManagerRegistry() = default;
    ~ManagerRegistry() = default;

    // Singleton access for global registry
    static ManagerRegistry& Instance()
    {
        static ManagerRegistry instance;
        return instance;
    }

    // Prevent copying
    ManagerRegistry(ManagerRegistry const&) = delete;
    ManagerRegistry& operator=(ManagerRegistry const&) = delete;

    // Manager accessors
    ITravelManager& GetTravelManager()
    {
        return *travelManager_;
    }

    IRandomBotManager& GetRandomBotManager()
    {
        return *randomBotManager_;
    }

    IBotRepository& GetBotRepository()
    {
        return *botRepository_;
    }

    // Const accessors
    ITravelManager const& GetTravelManager() const
    {
        return *travelManager_;
    }

    IRandomBotManager const& GetRandomBotManager() const
    {
        return *randomBotManager_;
    }

    IBotRepository const& GetBotRepository() const
    {
        return *botRepository_;
    }

    // Manager setters for dependency injection
    void SetTravelManager(std::shared_ptr<ITravelManager> manager)
    {
        travelManager_ = std::move(manager);
    }

    void SetRandomBotManager(std::shared_ptr<IRandomBotManager> manager)
    {
        randomBotManager_ = std::move(manager);
    }

    void SetBotRepository(std::shared_ptr<IBotRepository> repository)
    {
        botRepository_ = std::move(repository);
    }

    // Check if managers are initialized
    bool HasTravelManager() const { return travelManager_ != nullptr; }
    bool HasRandomBotManager() const { return randomBotManager_ != nullptr; }
    bool HasBotRepository() const { return botRepository_ != nullptr; }

    bool IsInitialized() const
    {
        return travelManager_ && randomBotManager_ && botRepository_;
    }

    // Generic template-based access (for extensibility)
    template<typename T>
    T& Get();

    template<typename T>
    void Register(std::shared_ptr<T> manager);

private:
    std::shared_ptr<ITravelManager> travelManager_;
    std::shared_ptr<IRandomBotManager> randomBotManager_;
    std::shared_ptr<IBotRepository> botRepository_;

    // Generic storage for additional managers
    std::unordered_map<std::type_index, std::shared_ptr<void>> managers_;
};

// Template specializations
template<>
inline ITravelManager& ManagerRegistry::Get<ITravelManager>()
{
    return GetTravelManager();
}

template<>
inline IRandomBotManager& ManagerRegistry::Get<IRandomBotManager>()
{
    return GetRandomBotManager();
}

template<>
inline IBotRepository& ManagerRegistry::Get<IBotRepository>()
{
    return GetBotRepository();
}

template<>
inline void ManagerRegistry::Register<ITravelManager>(std::shared_ptr<ITravelManager> manager)
{
    SetTravelManager(std::move(manager));
}

template<>
inline void ManagerRegistry::Register<IRandomBotManager>(std::shared_ptr<IRandomBotManager> manager)
{
    SetRandomBotManager(std::move(manager));
}

template<>
inline void ManagerRegistry::Register<IBotRepository>(std::shared_ptr<IBotRepository> repository)
{
    SetBotRepository(std::move(repository));
}

// Convenience macro for accessing the global registry
#define sManagerRegistry ManagerRegistry::Instance()

#endif
