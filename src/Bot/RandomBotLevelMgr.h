/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported with permission from Dustin HendricksonBased from mod-player-bot-level-brackets
 * and mod-player-bot-reset modules with contributors NoxMax (level reset), jimm0thy (friend-list exclusion),
 * Jered Little (arena-team exclusion).
 */

#ifndef PLAYERBOTS_RANDOMBOTLEVELMGR_H
#define PLAYERBOTS_RANDOMBOTLEVELMGR_H

#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "SharedDefines.h"
#include <vector>

class Player;

// Owns two ported sub-features: periodic redistribution of random bots across per-faction level
// brackets, and resetting random bots that reach max level. Config lives in PlayerbotAIConfig;
// this class holds the runtime working state (working bracket copies, pending-reset queue, timers).
class RandomBotLevelMgr
{
public:
    static RandomBotLevelMgr& instance()
    {
        static RandomBotLevelMgr instance;

        return instance;
    }

    void LoadConfig();
    void LogStartupSummary() const;
    void Update(uint32 diff);
    void OnBotLogin(Player* player);
    void OnBotLevelChanged(Player* player, uint8 oldLevel);
    void OnPlayerLogout(Player* player);

private:
    RandomBotLevelMgr() = default;
    ~RandomBotLevelMgr() = default;

    RandomBotLevelMgr(RandomBotLevelMgr const&) = delete;
    RandomBotLevelMgr& operator=(RandomBotLevelMgr const&) = delete;

    RandomBotLevelMgr(RandomBotLevelMgr&&) = delete;
    RandomBotLevelMgr& operator=(RandomBotLevelMgr&&) = delete;

    // Resolved back to the live _allianceRanges/_hordeRanges vector at process time, since those
    // vectors can be resized on a config reload.
    struct PendingResetEntry
    {
        ObjectGuid botGuid;
        int targetRange;
        TeamId team;
    };

    // ---- Level brackets sub-feature ----
    std::vector<LevelBracketConfig>& GetFactionRanges(TeamId team);
    void ClampAndBalanceBrackets();
    void ApplyBracketWeights(std::vector<LevelBracketConfig>& ranges, std::vector<float> const& weights);
    int GetLevelRangeIndex(uint8 level, TeamId team);
    void AdjustBotToRange(Player* bot, int targetRangeIndex, TeamId team);
    void LoadSocialFriendList();
    int GetOrFlagPlayerBracket(Player* player);
    void RunLevelBracketsDistribution();
    void ProcessFactionDistribution(TeamId team, uint32 totalBots, std::vector<int>& actualCounts,
        std::vector<std::vector<Player*>>& botsByRange);
    void RedistributeSurplusBots(std::vector<Player*>& sourceBots, int fromRange, TeamId team,
        std::vector<int>& actualCounts, std::vector<int> const& desiredCounts, std::vector<int> const& targetRanges);
    void ProcessPendingLevelResets();

    // ---- Level reset sub-feature ----
    uint8 ComputeResetChance(uint8 level) const;
    void ResetBot(Player* player, uint8 currentLevel);
    void SkipBotLevel(Player* player, uint8 currentLevel);
    void RunResetPlayedTimeCheck();

    // Level brackets: working copies, since dynamic distribution and the clamp/rebalance pass
    // mutate percentages at runtime and must never write back into PlayerbotAIConfig.
    std::vector<LevelBracketConfig> _allianceRanges;
    std::vector<LevelBracketConfig> _hordeRanges;
    uint8 _numRanges = 9;
    uint8 _randomBotMinLevel = 1;
    uint8 _randomBotMaxLevel = 80;
    std::vector<PendingResetEntry> _pendingLevelResets;
    std::vector<uint32> _socialFriendsList;

    uint32 _bracketsTimer = 0; // Level brackets: distribution adjustments
    uint32 _flaggedTimer = 0;  // Level brackets: pending reset checks
    uint32 _resetTimer = 0;    // Level reset: played-time based reset checks
};

// Registers the random bot level brackets + level reset world/player scripts.
void AddSC_randombot_level_mgr();

#endif // PLAYERBOTS_RANDOMBOTLEVELMGR_H
