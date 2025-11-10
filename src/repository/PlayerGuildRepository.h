#ifndef PLAYER_GUILD_REPOSITORY_H
#define PLAYER_GUILD_REPOSITORY_H

#include <unordered_set>
#include <vector>
#include <cstdint>
#include <mutex>

class PlayerGuildRepository {
public:
    static PlayerGuildRepository& GetInstance()
    {
        static PlayerGuildRepository instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to enforce singleton pattern
    PlayerGuildRepository(const PlayerGuildRepository&) = delete;
    PlayerGuildRepository& operator=(const PlayerGuildRepository&) = delete;

    std::unordered_set<uint32_t> GetPlayerGuildsIds();

	std::unordered_set<uint64_t> GetGuildMembersIds(uint32_t guildId);

private:
    PlayerGuildRepository() = default;
    ~PlayerGuildRepository() = default;
};

#define sPlayerGuildRepository PlayerGuildRepository::GetInstance()

#endif
