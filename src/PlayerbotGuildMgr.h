#ifndef _PLAYERBOT_PLAYERBOTGUILDMGR_H
#define _PLAYERBOT_PLAYERBOTGUILDMGR_H

#include "Guild.h"
#include "Player.h"
#include "PlayerbotAI.h"

class PlayerbotAI;

class PlayerbotGuildMgr
{
public:
    static PlayerbotGuildMgr* instance()
    {
        static PlayerbotGuildMgr instance;
        return &instance;
    }

    void Init();
    std::string AssignToGuild(Player* player);
    void LoadGuildNames();
    void ValidateGuildCache();
    void ResetGuildCache();
    bool CreateGuild(Player* player, std::string guildName);
    void OnGuildUpdate  (Guild* guild);
    bool SetGuildEmblem(uint32 guildId);
    void DeleteBotGuilds();
    bool IsRealGuild(uint32 guildId);
    bool IsRealGuild(Player* bot);

private:
    PlayerbotGuildMgr();
    std::unordered_map<std::string, bool> _guildNames;

    struct GuildCache
    {
        std::string name;
        uint8 status;
        uint32 maxMembers = 0;
        uint32 memberCount = 0;
        uint8 faction = 0;
        bool hasRealPlayer = false;
    };
    std::unordered_map<uint32 , GuildCache> _guildCache;
    std::vector<std::string> _shuffled_guild_keys;
};

void PlayerBotsGuildValidationScript();

#define sPlayerbotGuildMgr PlayerbotGuildMgr::instance()

#endif