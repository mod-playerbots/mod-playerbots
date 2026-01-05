#include "PlayerbotGuildMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "DatabaseEnv.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

PlayerbotGuildMgr::PlayerbotGuildMgr(){}

void PlayerbotGuildMgr::Init()
{
    _guildCache.clear();
    if (sPlayerbotAIConfig->deleteRandomBotGuilds)
        DeleteBotGuilds();

    LoadGuildNames();
    ValidateGuildCache();
}

bool PlayerbotGuildMgr::CreateGuild(Player* player, std::string guildName)
{
    Guild* guild = new Guild();
    if (!guild->Create(player, guildName))
    {
        LOG_ERROR("playerbots", "Error creating guild [ {} ] with leader [ {} ]", guildName,
            player->GetName());
        delete guild;
        return false;
    }
    sGuildMgr->AddGuild(guild);

    LOG_DEBUG("playerbots", "Guild created: id={} name='{}'", guild->GetId(), guildName);
    SetGuildEmblem(guild->GetId());

    GuildCache entry;
    entry.name = guildName;
    entry.memberCount = 1;
    entry.status = 1;
    entry.maxMembers = sPlayerbotAIConfig->randomBotGuildSizeMax;
    entry.faction = player->GetTeamId();

    _guildCache[guild->GetId()] = entry;
    return true;
}

bool PlayerbotGuildMgr::SetGuildEmblem(uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    // create random emblem
    uint32 st, cl, br, bc, bg;
    bg = urand(0, 51);
    bc = urand(0, 17);
    cl = urand(0, 17);
    br = urand(0, 7);
    st = urand(0, 180);

    LOG_DEBUG("playerbots",
        "[TABARD] new guild id={} random -> style={}, color={}, borderStyle={}, borderColor={}, bgColor={}",
        guild->GetId(), st, cl, br, bc, bg);

    // populate guild table with a random tabard design
    CharacterDatabase.Execute(
        "UPDATE guild SET EmblemStyle={}, EmblemColor={}, BorderStyle={}, BorderColor={}, BackgroundColor={} "
        "WHERE guildid={}",
        st, cl, br, bc, bg, guild->GetId());
    LOG_DEBUG("playerbots", "[TABARD] UPDATE done for guild id={}", guild->GetId());

    // Immediate reading for log
    if (QueryResult qr = CharacterDatabase.Query(
            "SELECT EmblemStyle,EmblemColor,BorderStyle,BorderColor,BackgroundColor FROM guild WHERE guildid={}",
            guild->GetId()))
    {
        Field* f = qr->Fetch();
        LOG_DEBUG("playerbots",
            "[TABARD] DB check guild id={} => style={}, color={}, borderStyle={}, borderColor={}, bgColor={}",
            guild->GetId(), f[0].Get<uint8>(), f[1].Get<uint8>(), f[2].Get<uint8>(), f[3].Get<uint8>(), f[4].Get<uint8>());
    }
    return true;
}

std::string PlayerbotGuildMgr::AssignToGuild(Player* player)
{
    if (!player)
        return "";

    uint8_t playerFaction = player->GetTeamId();
    std::vector<GuildCache*> partiallyfilledguilds;
    partiallyfilledguilds.reserve(_guildCache.size());

    for (auto& keyValue : _guildCache)
    {
        GuildCache& cached = keyValue.second;
        if (!cached.hasRealPlayer && cached.status == 1 && cached.faction == playerFaction)
            partiallyfilledguilds.push_back(&cached);
    }

    if (!partiallyfilledguilds.empty())
    {
        size_t idx = static_cast<size_t>(urand(0, static_cast<int>(partiallyfilledguilds.size()) - 1));
        return (partiallyfilledguilds[idx]->name);
    }

    size_t count = std::count_if(
        _guildCache.begin(), _guildCache.end(),
        [](const std::pair<const uint32, GuildCache>& pair)
        {
            return !pair.second.hasRealPlayer;
        }
        );

    if (count < sPlayerbotAIConfig->randomBotGuildCount)
    {
        for (auto& key : _shuffled_guild_keys)
        {
            if (_guildNames[key])
            {
                LOG_INFO("playerbots","Assigning player [{}] to guild [{}]", player->GetName(), key);
                return key;
            }
        }
        LOG_ERROR("playerbots","No available guild names left.");
    }
    return "";
}

void PlayerbotGuildMgr::OnGuildUpdate(Guild* guild)
{
    auto it = _guildCache.find(guild->GetId());
    if (it == _guildCache.end())
        return;

    GuildCache& entry = it->second;
    entry.memberCount = guild->GetMemberCount();
    if (entry.memberCount < entry.maxMembers)
        entry.status = 1;
    else if (entry.memberCount >= entry.maxMembers)
        entry.status = 2; // Full
    std::string guildName = guild->GetName();
    for (auto& it : _guildNames)
    {
        if (it.first == guildName)
        {
            it.second = false;
            break;
        }
    }
}

void PlayerbotGuildMgr::ResetGuildCache()
{
    for (auto it = _guildCache.begin(); it != _guildCache.end();)
    {
        GuildCache& cached = it->second;
        cached.memberCount = 0;
        cached.faction = 2;
        cached.status = 0;
    }
}

void PlayerbotGuildMgr::LoadGuildNames()
{
    LOG_INFO("playerbots", "Loading guild names from playerbots_guild_names...");

    QueryResult result = CharacterDatabase.Query("SELECT name_id, name FROM playerbots_guild_names");

    if (!result)
    {
        LOG_ERROR("playerbots", "No entries found in playerbots_guild_names. List is empty.");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        _guildNames[fields[1].Get<std::string>()] = true;
    } while (result->NextRow());

    for (auto& pair : _guildNames)
        _shuffled_guild_keys.push_back(pair.first);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(_shuffled_guild_keys.begin(), _shuffled_guild_keys.end(), g);
    LOG_INFO("playerbots", "Loaded {} guild entries from playerbots_guild_names table.", _guildNames.size());
}

void PlayerbotGuildMgr::ValidateGuildCache()
{
    QueryResult result = CharacterDatabase.Query("SELECT guildid, name FROM guild");
    if (!result)
    {
        LOG_ERROR("playerbots", "No guilds found in database, resetting guild cache");
        ResetGuildCache();
        return;
    }

    std::unordered_map<uint32, std::string> dbGuilds;
    do
    {
        Field* fields = result->Fetch();
        uint32 guildId = fields[0].Get<uint32>();
        std::string guildName = fields[1].Get<std::string>();
        dbGuilds[guildId] = guildName;
    } while (result->NextRow());

    for (auto it = dbGuilds.begin(); it != dbGuilds.end(); it++)
    {
        uint32 guildId = it->first;
        GuildCache cache;
        cache.name = it->second;
        cache.maxMembers = sPlayerbotAIConfig->randomBotGuildSizeMax;

        Guild* guild = sGuildMgr ->GetGuildById(guildId);
        if (!guild)
            continue;

        cache.memberCount = guild->GetMemberCount();
        ObjectGuid leaderGuid = guild->GetLeaderGUID();
        CharacterCacheEntry const* leaderEntry = sCharacterCache->GetCharacterCacheByGuid(leaderGuid);
        uint32 leaderAccount = leaderEntry->AccountId;
        cache.hasRealPlayer = !(sPlayerbotAIConfig->IsInRandomAccountList(leaderAccount));
        cache.faction = Player::TeamIdForRace(leaderEntry->Race);
        if (cache.memberCount == 0)
            cache.status = 0; // empty
        else if (cache.memberCount < cache.maxMembers)
            cache.status = 1; // partially filled
        else
            cache.status = 2; // full

        _guildCache.insert_or_assign(guildId, cache);
        for (auto& it : _guildNames)
        {
            if (it.first == cache.name)
            {
                it.second = false;
                break;
            }
        }
    }
}

void PlayerbotGuildMgr::DeleteBotGuilds()
{
    LOG_INFO("playerbots", "Deleting random bot guilds...");
    std::vector<uint32> randomBots;

    PlayerbotsDatabasePreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BOT);
    stmt->SetData(0, "add");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].Get<uint32>();
            randomBots.push_back(bot);
        } while (result->NextRow());
    }

    for (std::vector<uint32>::iterator i = randomBots.begin(); i != randomBots.end(); ++i)
    {
        if (Guild* guild = sGuildMgr->GetGuildByLeader(ObjectGuid::Create<HighGuid::Player>(*i)))
            guild->Disband();
    }
    LOG_INFO("playerbots", "Random bot guilds deleted");
}

bool PlayerbotGuildMgr::IsRealGuild(Player* bot)
{
    if (!bot)
        return false;
    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    return IsRealGuild(guildId);
}

bool PlayerbotGuildMgr::IsRealGuild(uint32 guildId)
{
    if (!guildId)
        return false;

    auto it = _guildCache.find(guildId);
    if (it == _guildCache.end())
        return false;

    return it->second.hasRealPlayer;
}

class BotGuildCacheWorldScript : public WorldScript
{
    public:

        BotGuildCacheWorldScript() : WorldScript("BotGuildCacheWorldScript"), _validateTimer(0){}

        void OnUpdate(uint32 diff) override
        {
            _validateTimer += diff;

            if (_validateTimer >= _validateInterval) // Validate every hour
            {
                _validateTimer = 0;
                sPlayerbotGuildMgr->ValidateGuildCache();
                LOG_INFO("playerbots", "Scheduled guild cache validation");
            }
        }

    private:
        uint32 _validateInterval = HOUR*IN_MILLISECONDS;
        uint32 _validateTimer;
};

void PlayerBotsGuildValidationScript()
{
    new BotGuildCacheWorldScript();
}