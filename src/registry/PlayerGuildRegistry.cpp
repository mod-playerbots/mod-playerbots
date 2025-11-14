#include <unordered_set>
#include <vector>
#include <cstdint>
#include <mutex>
#include "PlayerGuildRegistry.h"
#include "PlayerGuildRepository.h"
#include "PlayerbotAIConfig.h"
#include "UInt32VectorToString.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"
#include "Log.h"

void PlayerGuildRegistry::Initialize()
{
	std::unordered_set<uint32_t> playerGuildsIds = sPlayerGuildRepository.GetPlayerGuildsIds();

	ids_.clear();
	ids_.merge(playerGuildsIds);

	LOG_INFO("server.loading", "[PlayerGuildRegistry] Initialized PlayerGuildRegistry with {} guilds containing non random bot account characters.", Size());
}

// Add an ID if not already present
void PlayerGuildRegistry::Add(uint32_t id)
{
	std::lock_guard<std::mutex> lock(mutex_);

	ids_.insert(id);
}

// Remove an ID if present
void PlayerGuildRegistry::Remove(uint32_t id)
{
	std::lock_guard<std::mutex> lock(mutex_);

	ids_.erase(id);
}

// Check if ID is present
bool PlayerGuildRegistry::Contains(uint32_t id) const
{
	std::lock_guard<std::mutex> lock(mutex_);

	return ids_.find(id) != ids_.end();
}

// Retrieve a copy (thread safe)
std::vector<uint32_t> PlayerGuildRegistry::Snapshot() const
{
	std::lock_guard<std::mutex> lock(mutex_);

	return std::vector<uint32_t>(ids_.begin(), ids_.end());
}

std::size_t PlayerGuildRegistry::Size() const
{
	std::lock_guard<std::mutex> lock(mutex_);

	return ids_.size();
}

bool PlayerGuildRegistry::Empty() const
{
	std::lock_guard<std::mutex> lock(mutex_);

	return ids_.empty();
}
