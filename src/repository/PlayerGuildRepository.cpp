#include <unordered_set>
#include <cstdint>
#include "PlayerGuildRepository.h"
#include "PlayerbotAIConfig.h"
#include "QueryResult.h"
#include "UInt32VectorToString.h"
#include "DatabaseEnv.h"
#include "Log.h"

std::unordered_set<uint32_t> PlayerGuildRepository::GetPlayerGuildsIds()
{
	std::string randomBotsAccountsIdsString = UInt32VectorToString(sPlayerbotAIConfig->randomBotAccounts);

	std::unordered_set<uint32_t> guildIds;

	QueryResult result = CharacterDatabase.Query(
		"SELECT DISTINCT(guild.guildid) "
		"FROM guild "
		"INNER JOIN guild_member ON guild_member.guildid = guild.guildid "
		"INNER JOIN characters ON guild_member.guid = characters.guid "
		"WHERE characters.account NOT IN ({})",
		randomBotsAccountsIdsString
	);

	if (!result)
	{
		LOG_INFO("playerbots", "[PlayerGuildRepository] No guild found containing real players. PlayerGuildRepository is empty.");

		return guildIds;
	}

	do
	{
		Field* fields = result->Fetch();
		uint32_t id = fields[0].Get<uint32_t>();

		LOG_INFO("playerbots", "[PlayerGuildRepository] Adding guild with id '{}' to registry because it contains at least one non random bot account character.", id);

		guildIds.insert(id);
	} while (result->NextRow());

	return guildIds;
}

std::unordered_set<uint64_t> PlayerGuildRepository::GetGuildMembersIds(uint32_t guildId)
{
	std::string randomBotsAccountsIdsString = UInt32VectorToString(sPlayerbotAIConfig->randomBotAccounts);

	std::unordered_set<uint64_t> membersIds;

	QueryResult result = CharacterDatabase.Query(
		"SELECT characters.guid "
		"INNER JOIN guild_member ON guild_member.guid = characters.guid "
		"WHERE guild_member.guildid = {} "
		"AND characters.account NOT IN ({})",
		guildId,
		randomBotsAccountsIdsString
	);

	if (!result)
	{
		LOG_DEBUG("playerbots", "[PlayerGuildRepository] No non random bot member for guild with id {}", guildId);

		return membersIds;
	}

	do
	{
		Field* fields = result->Fetch();
		uint64_t id = fields[0].Get<uint64_t>();

		membersIds.insert(id);
	} while (result->NextRow());

	return membersIds;
}
