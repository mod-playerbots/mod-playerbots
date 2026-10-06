/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PVPGEARLISTMGR_H
#define PLAYERBOTS_PVPGEARLISTMGR_H

#include "Define.h"
#include "PvpLoadoutEp.h"
#include "PvpLoadoutRules.h"
#include <mutex>
#include <unordered_map>
#include <vector>

class Player;
struct ItemTemplate;

// PvP gear for the arena loadout swap. The pool is every equippable armor or weapon with resilience that a spawned
// vendor outside GM Island sells, with the personal rating its vendors require; it is built once from the core's
// in-memory item, vendor and spawn data. Per-spec rankings are cached by class, PvP premade and level; match loadouts
// additionally by faction and rating tier. Neither cache is persisted: both only depend on config and world data,
// which change on restart.
class PvpGearListMgr
{
public:
    struct LoadoutItem
    {
        uint8 slot;
        uint32 itemId;
        char const* source;  // "set", "pvp" or "pve", for the debug lines
        uint32 requiredRating;
        bool unique;
        PvpLoadout::StatVector stats;  // template stats, without gems and enchants
    };

    // One decision of a loadout: the items it equips and the slots it decides. Without Titan's Grip, main hand and
    // off-hand are one decision, even for a two-hander.
    struct Pick
    {
        std::vector<LoadoutItem> items;
        std::vector<uint8> slots;
        PvpLoadout::StatVector stats;  // its items' stats
        bool setPiece;                 // the four-piece is kept over the bot's own items
    };

    struct Loadout
    {
        PvpLoadout::Role role;
        std::vector<Pick> picks;
    };

    static PvpGearListMgr& instance()
    {
        static PvpGearListMgr instance;

        return instance;
    }

    // World thread; the first call builds the pool and later ones are ignored, because rankings point into it.
    void LoadAll();

    // The bot's PvP items for an equipment slot, ranked best first for its spec by StatsWeightCalculator in PvP mode.
    // The bot must already be in the PvP premade specNo, so its talents match every bot sharing the cached ranking.
    std::vector<PvpLoadout::ItemCandidate> const& GetRanked(Player* bot, int32 specNo, uint8 slot);

    // The match gear for the bot's spec at this rating: a four-piece of one set, then the remaining slots biggest
    // marginal EP first, chosen once per rating tier and shared by every bot of the spec. Same precondition as
    // GetRanked; specNo is NO_SPEC when the bot's tree has no PvP premade, which leaves PvE items only.
    Loadout const& GetLoadout(Player* bot, int32 specNo, uint32 rating);

    static PvpLoadout::ItemCandidate ToCandidate(ItemTemplate const* proto, uint32 requiredRating);

    // A pick as an option for SelectBiggestGainFirst.
    static PvpLoadout::Option ToOption(Pick const& pick);

private:
    PvpGearListMgr() = default;
    ~PvpGearListMgr() = default;

    PvpGearListMgr(PvpGearListMgr const&) = delete;
    PvpGearListMgr& operator=(PvpGearListMgr const&) = delete;

    PvpGearListMgr(PvpGearListMgr&&) = delete;
    PvpGearListMgr& operator=(PvpGearListMgr&&) = delete;

    struct PoolItem
    {
        ItemTemplate const* proto;
        uint32 requiredRating;
    };

    using RankedSlots = std::vector<std::vector<PvpLoadout::ItemCandidate>>;  // indexed by equipment slot

    RankedSlots Rank(Player* bot) const;
    Loadout BuildLoadout(Player* bot, int32 specNo, uint32 rating, PvpLoadout::RatingRules const& rules);

    std::vector<PoolItem> _pool;
    std::vector<uint32> _requirements;  // the pool's distinct non-zero rating requirements, ascending
    bool _loaded = false;

    // Guards both caches. Entries are computed outside the lock and never erased, so references stay valid.
    std::mutex _cacheMutex;
    std::unordered_map<uint32, RankedSlots> _ranked;
    std::unordered_map<uint64, Loadout> _loadouts;
};

#endif
