/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCFACTIONCHAMPIONSGRIPCHAIN_H
#define PLAYERBOTS_RAID_TOCFACTIONCHAMPIONSGRIPCHAIN_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// Who grips a champion healer down the lane and when, each link off the last one's landing
namespace FactionChampionsGripChain
{
// One bot DK per post: the tank line, 54 yd and 78 yd down the lane. A 4th grip would land the healer past
// the far wall, about 83 yd off.
inline constexpr std::size_t MAX_LINKS = 3;
// a link's turn lapses this long after it came, the first one's has the walk in on top
inline constexpr uint32_t FIRST_LINK_MS = 15000;
inline constexpr uint32_t NEXT_LINK_MS = 10000;
// Death Grip further off its cooldown than this and the link is skipped
inline constexpr uint32_t WAIT_MS = 3000;
// the pull starts on the hit, so the jump only shows a moment after the cast
inline constexpr uint32_t LAND_MS = 300;

struct Dk
{
    uint64_t guid = 0;
    bool tank = false;
};

// Tank DKs first, then by guid. The first MAX_LINKS take the posts in order, the rest wait as spares.
inline std::vector<uint64_t> OrderLinks(std::vector<Dk> dks)
{
    std::sort(dks.begin(), dks.end(),
              [](Dk const& a, Dk const& b) { return a.tank != b.tank ? a.tank : a.guid < b.guid; });

    std::vector<uint64_t> order;
    for (Dk const& dk : dks)
        order.push_back(dk.guid);

    return order;
}

enum class Skip : uint8_t
{
    None,
    Gone,  // dead, off the map or out of control
    Cooldown,
    Timeout,
};

// The turn's link as the game sees it this tick
struct LinkState
{
    bool present = false;  // alive, on the map, in control
    uint32_t gripCooldownMs = 0;
};

struct Chain
{
    std::vector<uint64_t> links;  // by post
    std::size_t due = 0;          // whose turn, links.size() once closed
    uint32_t turnMs = 0;          // when that turn came
    uint32_t gripMs = 0;
    bool gripped = false;
};

inline Chain Open(std::vector<uint64_t> links, uint32_t now)
{
    if (links.size() > MAX_LINKS)
        links.resize(MAX_LINKS);

    Chain chain;
    chain.links = std::move(links);
    chain.turnMs = now;
    return chain;
}

inline bool Closed(Chain const& chain) { return chain.due >= chain.links.size(); }

// The DK's post, links.size() for one not in the chain
inline std::size_t PostOf(Chain const& chain, uint64_t guid)
{
    auto const found = std::find(chain.links.begin(), chain.links.end(), guid);
    return static_cast<std::size_t>(found - chain.links.begin());
}

struct Skipped
{
    uint64_t guid = 0;
    Skip reason = Skip::None;
};

// Passes the turn on from every link that can't grip. A timeout counts from when the turn came, unsigned
// so it holds across the ms clock's wrap. stateOf(guid) answers a LinkState.
template <typename StateOf>
std::vector<Skipped> Advance(Chain& chain, uint32_t now, StateOf stateOf)
{
    std::vector<Skipped> skipped;
    while (!Closed(chain))
    {
        uint64_t const guid = chain.links[chain.due];
        LinkState const link = stateOf(guid);
        uint32_t const limit = chain.due ? NEXT_LINK_MS : FIRST_LINK_MS;
        Skip reason = Skip::None;
        if (!link.present)
            reason = Skip::Gone;
        else if (link.gripCooldownMs > WAIT_MS)
            reason = Skip::Cooldown;
        else if (now - chain.turnMs > limit)
            reason = Skip::Timeout;

        if (reason == Skip::None)
            break;

        skipped.push_back({guid, reason});
        ++chain.due;
        chain.turnMs = now;
    }

    return skipped;
}

// The turn's link gripped the chain's champion: the next link's turn comes. A grip from any other link
// changes nothing.
inline void TrackGrip(Chain& chain, uint64_t guid, uint32_t now)
{
    if (Closed(chain) || chain.links[chain.due] != guid)
        return;

    chain.gripped = true;
    chain.gripMs = now;
    ++chain.due;
    chain.turnMs = now;
}

// The DK's turn, and the last grip has the champion on the ground
inline bool Due(Chain const& chain, uint64_t guid, uint32_t now, bool midJump)
{
    return !Closed(chain) && chain.links[chain.due] == guid &&
           (!chain.gripped || (now - chain.gripMs >= LAND_MS && !midJump));
}

// A link still to grip, the turn's own included until it's due
inline bool Pending(Chain const& chain, uint64_t guid)
{
    std::size_t const post = PostOf(chain, guid);
    return post < chain.links.size() && post >= chain.due;
}
}  // namespace FactionChampionsGripChain

#endif
