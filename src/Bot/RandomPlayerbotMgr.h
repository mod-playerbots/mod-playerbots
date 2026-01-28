/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTMGR_H
#define _PLAYERBOT_RANDOMPLAYERBOTMGR_H

#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "NewRpgInfo.h"
#include "ObjectGuid.h"
#include "PlayerbotMgr.h"
#include "GameTime.h"

struct CreatureData;

struct BattlegroundInfo
{
    std::vector<uint32> bgInstances;
    std::vector<uint32> ratedArenaInstances;
    std::vector<uint32> skirmishArenaInstances;
    uint32 bgInstanceCount = 0;
    uint32 ratedArenaInstanceCount = 0;
    uint32 skirmishArenaInstanceCount = 0;
    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    uint32 activeRatedArenaQueue = 0;     // 0 = Inactive, 1 = Active
    uint32 activeSkirmishArenaQueue = 0;  // 0 = Inactive, 1 = Active
    uint32 activeBgQueue = 0;             // 0 = Inactive, 1 = Active

    // Bots (Arena)
    uint32 ratedArenaBotCount = 0;
    uint32 skirmishArenaBotCount = 0;

    // Bots (Battleground)
    uint32 bgHordeBotCount = 0;
    uint32 bgAllianceBotCount = 0;

    // Players (Arena)
    uint32 ratedArenaPlayerCount = 0;
    uint32 skirmishArenaPlayerCount = 0;

    // Players (Battleground)
    uint32 bgHordePlayerCount = 0;
    uint32 bgAlliancePlayerCount = 0;
};

class ChatHandler;
class PerfMonitorOperation;
class WorldLocation;

struct CachedEvent
{
    uint32 value = 0;
    uint32 lastChangeTime = 0;
    uint32 validIn = 0;
    std::string data;

    bool IsEmpty() const { return !lastChangeTime; }
};

struct BotEventCache
{
    bool loaded = false;
    std::unordered_map<std::string, CachedEvent> events;
};

// https://gist.github.com/bradley219/5373998

class botPIDImpl;
class botPID
{
public:
    // Kp -  proportional gain
    // Ki -  Integral gain
    // Kd -  derivative gain
    // dt -  loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    botPID(double dt, double max, double min, double Kp, double Ki, double Kd);
    void adjust(double Kp, double Ki, double Kd);
    void reset();

    double calculate(double setpoint, double pv);
    ~botPID();

private:
    botPIDImpl* pimpl;
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
public:
    RandomPlayerbotMgr();
    virtual ~RandomPlayerbotMgr();
    static RandomPlayerbotMgr* instance()
    {
        static RandomPlayerbotMgr instance;
        return &instance;
    }

    void LogPlayerLocation();
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

    uint32 activeBots = 0;
    static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);
    bool IsRandomBot(Player* bot);
    bool IsRandomBot(ObjectGuid::LowType bot);
    bool IsAddclassBot(Player* bot);
    bool IsAddclassBot(ObjectGuid::LowType bot);

    // Fast lookup: real-player master -> controlled random-bots (so players without bots can skip packet forwarding).
    bool HasMasterControlledRandomBots(ObjectGuid const& masterGuid) const;
    void GetMasterControlledRandomBotGuidsSnapshot(ObjectGuid const& masterGuid, std::vector<ObjectGuid>& out) const;
    void OnRandomBotMasterChanged(ObjectGuid const& botGuid, ObjectGuid const& oldMasterGuid, ObjectGuid const& newMasterGuid);
    void OnRandomBotLoggedOut(ObjectGuid const& botGuid);
    void Randomize(Player* bot);
    void Clear(Player* bot);
    void RandomizeFirst(Player* bot);
    void RandomizeMin(Player* bot);
    void IncreaseLevel(Player* bot);
    void ScheduleTeleport(uint32 bot, uint32 time = 0);
    void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);
    void HandleCommand(uint32 type, std::string const text, Player* fromPlayer, std::string channelName = "");
    std::string const HandleRemoteCommand(std::string const request);
    void OnPlayerLogout(Player* player);
    void OnPlayerLogin(Player* player);
    void OnPlayerLoginError(uint32 bot);
    Player* GetRandomPlayer();
    std::vector<Player*> GetPlayers() { return players; };
    PlayerBotMap GetAllBots() { return playerBots; };
    void PrintStats();
    double GetBuyMultiplier(Player* bot);
    double GetSellMultiplier(Player* bot);
    void AddTradeDiscount(Player* bot, Player* master, int32 value);
    void SetTradeDiscount(Player* bot, Player* master, uint32 value);
    uint32 GetTradeDiscount(Player* bot, Player* master);
    void Refresh(Player* bot);
    void RandomTeleportForLevel(Player* bot);
    void RandomTeleportGrindForLevel(Player* bot);
    void RandomTeleportForRpg(Player* bot);
    uint32 GetMaxAllowedBotCount();
    bool ProcessBot(Player* player);
    void Revive(Player* player);
    void ChangeStrategy(Player* player);
    void ChangeStrategyOnce(Player* player);
    uint32 GetValue(Player* bot, std::string const& type);
    uint32 GetValue(uint32 bot, std::string const& type);
    std::string GetData(uint32 bot, std::string const& type);
    void SetValue(uint32 bot, std::string const& type, uint32 value, std::string const& data = "");
    void SetValue(Player* bot, std::string const& type, uint32 value, std::string const& data = "");
    void Remove(Player* bot);
    ObjectGuid GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId);
    CreatureData const* GetCreatureDataByEntry(uint32 entry);
    void LoadBattleMastersCache();
    std::map<uint32, std::map<uint32, BattlegroundInfo>> BattlegroundData;
    std::map<uint32, std::map<uint32, std::map<TeamId, uint32>>> VisualBots;
    std::map<uint32, std::map<uint32, std::map<uint32, uint32>>> Supporters;
    std::map<TeamId, std::vector<uint32>> LfgDungeons;
    void CheckBgQueue();
    void CheckLfgQueue();
    void CheckPlayers();
    void LogBattlegroundInfo();

    // Prevent "wild" random-bots (no real-player master) from re-joining recently evacuated empty battleground instances.
    // Used by BG status handler when a bot receives an invite to a specific BG instance.
    bool IsWildBgJoinBlocked(uint32 instanceId);
    void ClearWildBgJoinBlock(uint32 instanceId);

    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> getBattleMastersCache()
    {
        return BattleMastersCache;
    }

    float getActivityMod() { return activityMod; }
    float getActivityPercentage() { return activityMod * 100.0f; }
    void setActivityPercentage(float percentage) { activityMod = percentage / 100.0f; }
    static uint8 GetTeamClassIdx(bool isAlliance, uint8 claz) { return isAlliance * 20 + claz; }

    void PrepareAddclassCache();
    void PrepareZone2LevelBracket();
    void PrepareTeleportCache();
    void Init();
    std::map<uint8, std::unordered_set<ObjectGuid>> addclassCache;
    std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;
    std::map<uint8, std::vector<WorldLocation>> allianceStarterPerLevelCache;
    std::map<uint8, std::vector<WorldLocation>> hordeStarterPerLevelCache;

    struct LevelBracket {
        uint32 low;
        uint32 high;
        bool InsideBracket(uint32 val) { return val >= low && val <= high; }
    };
    std::map<uint32, LevelBracket> zone2LevelBracket;
    struct BankerLocation {
        WorldLocation loc;
        uint32 entry;
    };
    std::map<uint8, std::vector<BankerLocation>> bankerLocsPerLevelCache;

    // Account type management
    void AssignAccountTypes();
    bool IsAccountType(uint32 accountId, uint8 accountType);

protected:
    void OnBotLoginInternal(Player* const bot) override;

private:
    // pid values are set in constructor
    botPID pid = botPID(1, 50, -50, 0, 0, 0);
    float activityMod = 0.25;
    bool _isBotInitializing = true;
    bool _isBotLogging = true;
    // Optional smoothing of heavy random-bot ticks by splitting them into smaller slices.
    enum class SlicedPhase : uint8
    {
        None = 0,
        Updating,
        LoginPrep,
        LoggingIn
    };

    void BeginSlicedCycle();
    void ProcessSlicedSlice();
    void EndSlicedCycle();

    bool _slicedCycleActive = false;
    SlicedPhase _slicedPhase = SlicedPhase::None;
    uint32 _slicedLogicalIntervalMs = 0;
    uint32 _slicedElapsedMs = 0;

    uint32 _slicedDtMs = 0;
    uint64 _slicedUpdateBudgetAcc = 0;
    uint64 _slicedLoginBudgetAcc = 0;
    uint32 _slicedUpdateTargetInitial = 0;
    uint32 _slicedLoginTargetTotal = 0;
    bool _slicedDidAttemptLoginPhase = false;

    uint32 _slicedUpdateTarget = 0;
    uint32 _slicedLoginTarget = 0;
    uint32 _slicedLoginTargetInitial = 0;
    uint32 _slicedUpdateDone = 0;
    uint32 _slicedLoginDone = 0;
    uint32 _slicedMaxNewBots = 0;

    bool _slicedRealPlayerIsLogged = false;
    bool _slicedCanAttemptLogin = false;

    std::vector<uint32> _slicedBotSnapshot;
    size_t _slicedUpdateIndex = 0;
    size_t _slicedLoginIndex = 0;

    NewRpgStatistic rpgStasticTotal;
    CachedEvent* FindEvent(uint32 bot, std::string const& event);
    uint32 GetEventValue(uint32 bot, std::string const& event);
    std::string GetEventData(uint32 bot, std::string const& event);
    uint32 SetEventValue(uint32 bot, std::string const& event, uint32 value, uint32 validIn,
                         std::string const& data = "");
    void FlushPendingEventDbWrites(bool force = false);
    void GetBots();
    void BuildCreatureDataEntryIndex();
    std::vector<uint32> GetBgBots(uint32 bracket);
    time_t BgCheckTimer;
    time_t LfgCheckTimer;
    time_t PlayersCheckTimer;
    time_t RealPlayerLastTimeSeen = 0;
    time_t DelayLoginBotsTimer;
    time_t printStatsTimer;
    uint32 AddRandomBots();
    bool ProcessBot(uint32 bot);
    void ScheduleRandomize(uint32 bot, uint32 time);
    void RandomTeleport(Player* bot);
    void RandomTeleport(Player* bot, std::vector<WorldLocation>& locs, bool hearth = false);
    uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);
    typedef void (RandomPlayerbotMgr::*ConsoleCommandHandler)(Player*);
    std::vector<Player*> players;
    uint32 processTicks;

    // std::map<uint32, std::vector<WorldLocation>> rpgLocsCache;
    std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> rpgLocsCacheLevel;
    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<uint32>>> BattleMastersCache;
    std::unordered_map<uint32, BotEventCache> eventCache;

    // Batched persistence of random-bot events to reduce DB commit overhead.
    struct PendingBotEventWrite
    {
        uint32 value = 0;
        uint32 validIn = 0;
        uint32 changeTime = 0;     // NowSeconds() at the time of scheduling
        std::string data;
    };

    std::unordered_map<uint32, std::unordered_map<std::string, PendingBotEventWrite>> _pendingEventWrites;
    size_t _pendingEventWritesCount = 0;
    uint32 _pendingEventLastFlushMs = 0;

    // Fast lookup for CreatureData by entry (avoids full scans of GetAllCreatureData()).
    // Store the spawn guid (low) instead of a raw pointer so the lookup stays valid even if
    // creature data is reloaded (pointers can be invalidated by container rehash/rebuild).
    std::unordered_map<uint32, ObjectGuid::LowType> _creatureDataByEntry;
    bool _creatureEntryIndexBuilt = false;
    std::list<uint32> currentBots;
    uint32 bgBotsCount;
    uint32 playersLevel;

    // --- Wild random-bot battleground maintenance ---
    // We track empty BG instances (no real players) to evict "wild" random-bots after a short delay,
    // and to temporarily block immediate re-joins to the same instance (stale BG queue snapshots, etc.).
    uint32 _wildBgMaintenanceNextSec = 0;
    std::unordered_map<uint32, uint32> _wildBgEmptySinceSec;         // instanceId -> first time we saw it empty
    std::unordered_map<uint32, uint32> _wildBgJoinBlockedUntilSec;    // instanceId -> block expiration timestamp

    void UpdateWildBotBattlegroundMaintenance();

    // Account lists
    std::vector<uint32> rndBotTypeAccounts;             // Accounts marked as RNDbot (type 1)
    std::vector<uint32> addClassTypeAccounts;           // Accounts marked as AddClass (type 2)

    // Fast master->controlled random-bots index (maintained via PlayerbotAI::SetMaster()).
    // Note: only real-player masters are tracked (bot masters are ignored).
    std::unordered_map<ObjectGuid, std::unordered_set<ObjectGuid>> _controlledRandomBotsByMaster;
    std::unordered_map<ObjectGuid, ObjectGuid> _controlledRandomBotMaster;

    //void ScaleBotActivity();      // Deprecated function
    static inline uint32 NowSeconds() { return static_cast<uint32>(GameTime::GetGameTime().count()); }
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
