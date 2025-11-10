#ifndef PLAYER_GUILD_REGISTRY_H
#define PLAYER_GUILD_REGISTRY_H

#include <unordered_set>
#include <vector>
#include <cstdint>
#include <mutex>

class PlayerGuildRegistry {
public:
    static PlayerGuildRegistry& GetInstance()
    {
        static PlayerGuildRegistry instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton pattern
    PlayerGuildRegistry(const PlayerGuildRegistry&) = delete;
    PlayerGuildRegistry& operator=(const PlayerGuildRegistry&) = delete;

    void Initialize();

    // Add an ID if not already present
    void Add(uint32_t id);

    // Remove an ID if present
    void Remove(uint32_t id);

    // Check if ID is present
    bool Contains(uint32_t id) const;

    // Retrieve a copy (thread safe)
    std::vector<uint32_t> Snapshot() const;

    // Get the number of registered guilds
    std::size_t Size() const;

    // Check if registry is empty
    bool Empty() const;

private:
    PlayerGuildRegistry() = default;
    ~PlayerGuildRegistry() = default;

    mutable std::mutex mutex_;
    std::unordered_set<uint32_t> ids_;
};

#define sPlayerGuildRegistry PlayerGuildRegistry::GetInstance()

#endif
