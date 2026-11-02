/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BWLHelpers.h"
#include "AiObjectContext.h"
#include "Group.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include <array>
#include <iterator>
#include <mutex>
#include <unordered_map>

namespace BlackwingLairHelpers
{
    static constexpr uint8 WYRMGUARD_PACK_SIZE = 3;
    static constexpr float WYRMGUARD_HOME_TOLERANCE = 5.0f;
    static constexpr float WYRMGUARD_IN_POSITION_DISTANCE = 8.0f;
    static constexpr uint32 WYRMGUARD_HOLD_LIMIT_MS = 20 * IN_MILLISECONDS;
    static constexpr uint32 WYRMGUARD_PACK_GAP_MS = 30 * IN_MILLISECONDS;
    static constexpr float DRAKE_SEARCH_RANGE = 55.0f;
    // A step that ends inside this is a step towards pulling the drake too.
    static constexpr float DRAKE_SAFE_DISTANCE = 35.0f;

    namespace
    {
        // Each Wyrmguard belongs to the slot of its spawn point, so the rest keep their tank when one of them dies.
        struct WyrmguardPack
        {
            std::array<Position, WYRMGUARD_PACK_SIZE> homes;
            std::array<Position, WYRMGUARD_PACK_SIZE> spots;
        };

        std::array<WyrmguardPack, 2> const WYRMGUARD_PACKS = {{
            // Before Ebonroc: back into the room the raid came from.
            { {{ Position(-7400.41f, -961.01f, 465.14f), Position(-7394.61f, -956.29f, 465.15f),
                 Position(-7389.70f, -951.74f, 465.15f) }},
              {{ Position(-7409.28f, -926.99f, 465.16f), Position(-7419.65f, -913.13f, 464.98f),
                 Position(-7428.69f, -901.37f, 465.16f) }} },
            // Before Chromaggus: back towards Flamegor's room.
            { {{ Position(-7450.65f, -1035.77f, 477.00f), Position(-7443.51f, -1045.13f, 477.02f),
                 Position(-7437.04f, -1053.58f, 477.04f) }},
              {{ Position(-7430.00f, -1025.00f, 477.35f), Position(-7410.00f, -1030.00f, 477.35f),
                 Position(-7415.00f, -1045.00f, 477.35f) }} },
        }};

        struct WyrmguardHold
        {
            uint32 startedMs = 0;
            uint32 lastCheckMs = 0;
        };

        std::mutex wyrmguardHoldMutex;
        std::unordered_map<uint32, WyrmguardHold> wyrmguardHolds;

        bool FindWyrmguardSlot(Unit const* wyrmguard, WyrmguardPack const*& pack, uint8& slot)
        {
            Creature const* creature = wyrmguard->ToCreature();
            if (!creature)
                return false;

            Position const& home = creature->GetHomePosition();
            for (WyrmguardPack const& candidate : WYRMGUARD_PACKS)
            {
                for (uint8 i = 0; i < WYRMGUARD_PACK_SIZE; ++i)
                {
                    if (home.GetExactDist(&candidate.homes[i]) < WYRMGUARD_HOME_TOLERANCE)
                    {
                        pack = &candidate;
                        slot = i;
                        return true;
                    }
                }
            }
            return false;
        }

        // Every tank on the map, the flagged main tank first. Dead ones keep their place, so a death moves nobody else.
        std::vector<Player*> GetWyrmguardTanks(Player* bot)
        {
            std::vector<Player*> tanks;
            Group* group = bot->GetGroup();
            if (!group)
                return tanks;

            ObjectGuid mainTank;
            for (Group::MemberSlot const& slot : group->GetMemberSlots())
            {
                if (slot.flags & MEMBER_FLAG_MAINTANK)
                    mainTank = slot.guid;
            }

            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member->GetMapId() != bot->GetMapId() || !PlayerbotAI::IsTank(member))
                    continue;

                tanks.insert(member->GetGUID() == mainTank ? tanks.begin() : tanks.end(), member);
            }
            return tanks;
        }

        // A dead tank's Wyrmguard goes to the next living tank. Returns the number of tanks if none is left.
        size_t GetOwnerIndex(std::vector<Player*> const& tanks, uint8 slot)
        {
            for (size_t i = 0; i < tanks.size(); ++i)
            {
                size_t const owner = (slot + i) % tanks.size();
                if (tanks[owner]->IsAlive())
                    return owner;
            }
            return tanks.size();
        }

        Position const* GetSpotOfOwner(std::vector<Player*> const& tanks, Unit const* wyrmguard)
        {
            WyrmguardPack const* pack = nullptr;
            uint8 slot = 0;
            if (!FindWyrmguardSlot(wyrmguard, pack, slot))
                return nullptr;

            // Beyond the pack's own spots a tank shares one: only three Wyrmguards need holding apart.
            size_t const owner = GetOwnerIndex(tanks, slot);
            return owner == tanks.size() ? nullptr : &pack->spots[owner % WYRMGUARD_PACK_SIZE];
        }
    }

    bool IsActiveSuppressionDeviceInRange(GameObject const* go, Player const* bot)
    {
        constexpr float suppressionDeviceInteractionDistance = 15.0f;
        return go &&
               go->GetEntry() == static_cast<uint32>(BlackwingLairGameObjects::GO_SUPPRESSION_DEVICE) &&
               go->GetDistance(bot) < suppressionDeviceInteractionDistance &&
               go->GetGoState() == GO_STATE_READY;
    }

    bool AreRazorgoreEggsAlive(PlayerbotAI* botAI)
    {
        GuidVector gos = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects")->Get();
        for (auto const& guid : gos)
        {
            GameObject const* go = botAI->GetGameObject(guid);
            if (go && go->GetEntry() == static_cast<uint32>(BlackwingLairGameObjects::GO_BLACK_DRAGON_EGG))
                return true;
        }
        return false;
    }

    bool IsRazorgoreOffTank(Player* bot)
    {
        return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
    }

    bool IsNonBABotNearPosition(Player const* bot, Position const& position, float distance)
    {
        Group const* group = bot->GetGroup();
        if (!group)
            return false;

        for (GroupReference const* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player const* p = gref->GetSource();
            if (!p || p == bot || !p->IsAlive() || p->GetMapId() != bot->GetMapId())
                continue;

            if (p->HasAura(static_cast<uint32>(BlackwingLairSpells::SPELL_BURNING_ADRENALINE)))
                continue;

            if (p->GetDistance2d(position.GetPositionX(), position.GetPositionY()) < distance)
                return true;
        }

        return false;
    }

    bool IsCorruptedWhelp(Unit const* unit)
    {
        if (!unit)
            return false;

        const uint32 entry = unit->GetEntry();
        return entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_RED_WHELP) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_GREEN_WHELP) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_BLUE_WHELP) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_BRONZE_WHELP);
    }

    bool IsCorruptedTotemEntry(uint32 entry)
    {
        return entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_FIRE_NOVA_TOTEM) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_STONESKIN_TOTEM) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_HEALING_TOTEM) ||
            entry == static_cast<uint32>(BlackwingLairNPCs::NPC_CORRUPTED_WINDFURY_TOTEM);
    }

    bool IsCorruptedTotem(Unit const* unit)
    {
        return unit && IsCorruptedTotemEntry(unit->GetEntry());
    }

    Unit* FindCorruptedTotem(PlayerbotAI* botAI)
    {
        // Ranged take the nearest totem, melee only one they can already reach.
        Player* bot = botAI->GetBot();
        Unit* nearest = nullptr;
        float nearestDistance = 0.0f;
        for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet())
        {
            if (!IsCorruptedTotemEntry(guid.GetEntry()))
                continue;

            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || (!bot->IsWithinMeleeRange(unit) && !PlayerbotAI::IsRanged(bot)))
                continue;

            float const distance = bot->GetExactDist(unit);
            if (!nearest || distance < nearestDistance)
            {
                nearest = unit;
                nearestDistance = distance;
            }
        }
        return nearest;
    }

    bool IsDeathTalonWyrmguard(Unit const* unit)
    {
        return unit && unit->GetEntry() == static_cast<uint32>(BlackwingLairNPCs::NPC_DEATH_TALON_WYRMGUARD);
    }

    std::vector<Unit*> GetWyrmguardAttackers(PlayerbotAI* botAI)
    {
        std::vector<Unit*> wyrmguards;
        for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->RefGet())
        {
            if (guid.GetEntry() != static_cast<uint32>(BlackwingLairNPCs::NPC_DEATH_TALON_WYRMGUARD))
                continue;

            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->IsInCombat())
                wyrmguards.push_back(unit);
        }
        return wyrmguards;
    }

    // The tank's Wyrmguard, the one it does not hold yet if it has several.
    Unit* GetAssignedWyrmguard(PlayerbotAI* botAI)
    {
        std::vector<Unit*> const wyrmguards = GetWyrmguardAttackers(botAI);
        if (wyrmguards.empty())
            return nullptr;

        Player* bot = botAI->GetBot();
        std::vector<Player*> const tanks = GetWyrmguardTanks(bot);
        Unit* assigned = nullptr;
        for (Unit* wyrmguard : wyrmguards)
        {
            WyrmguardPack const* pack = nullptr;
            uint8 slot = 0;
            if (!FindWyrmguardSlot(wyrmguard, pack, slot))
                continue;

            size_t const owner = GetOwnerIndex(tanks, slot);
            if (owner == tanks.size() || tanks[owner] != bot)
                continue;

            if (wyrmguard->GetVictim() != bot)
                return wyrmguard;

            assigned = wyrmguard;
        }
        return assigned;
    }

    // Green, red, bronze, blue is the order raids killed them in. Black is not in that order and goes last. Every bot
    // gets the same Wyrmguard, whatever order its attackers come in.
    Unit* GetWyrmguardKillTarget(std::vector<Unit*> const& wyrmguards)
    {
        static constexpr std::array<BlackwingLairSpells, 4> order = {
            BlackwingLairSpells::SPELL_GREEN_BROOD_POWER, BlackwingLairSpells::SPELL_RED_BROOD_POWER,
            BlackwingLairSpells::SPELL_BRONZE_BROOD_POWER, BlackwingLairSpells::SPELL_BLUE_BROOD_POWER};

        Unit* best = nullptr;
        size_t bestRank = 0;
        for (Unit* wyrmguard : wyrmguards)
        {
            size_t rank = 0;
            while (rank < order.size() && !wyrmguard->HasAura(static_cast<uint32>(order[rank])))
                ++rank;

            if (!best || rank < bestRank || (rank == bestRank && wyrmguard->GetGUID() < best->GetGUID()))
            {
                best = wyrmguard;
                bestRank = rank;
            }
        }
        return best;
    }

    bool IsAssignedToOtherTank(Player* bot, Unit const* wyrmguard)
    {
        WyrmguardPack const* pack = nullptr;
        uint8 slot = 0;
        if (!FindWyrmguardSlot(wyrmguard, pack, slot))
            return false;

        std::vector<Player*> const tanks = GetWyrmguardTanks(bot);
        size_t const owner = GetOwnerIndex(tanks, slot);
        return owner != tanks.size() && tanks[owner] != bot;
    }

    Position const* GetWyrmguardSpot(Player* bot, Unit const* wyrmguard)
    {
        return GetSpotOfOwner(GetWyrmguardTanks(bot), wyrmguard);
    }

    // Shared by the raid, so a bot that joins late does not wait on its own. The window starts at the first check on
    // a pack and a gap without checks ends the pack.
    bool IsWyrmguardHoldRunning(Player* bot)
    {
        std::lock_guard<std::mutex> lock(wyrmguardHoldMutex);
        uint32 const now = getMSTime();
        WyrmguardHold& hold = wyrmguardHolds[bot->GetInstanceId()];
        if (!hold.lastCheckMs || getMSTimeDiff(hold.lastCheckMs, now) >= WYRMGUARD_PACK_GAP_MS)
        {
            hold.startedMs = now;
            for (auto it = wyrmguardHolds.begin(); it != wyrmguardHolds.end();)
            {
                bool const stale = getMSTimeDiff(it->second.lastCheckMs, now) >= WYRMGUARD_PACK_GAP_MS;
                it = stale && &it->second != &hold ? wyrmguardHolds.erase(it) : std::next(it);
            }
        }

        hold.lastCheckMs = now;
        return getMSTimeDiff(hold.startedMs, now) < WYRMGUARD_HOLD_LIMIT_MS;
    }

    bool AreWyrmguardsInPosition(Player* bot, std::vector<Unit*> const& wyrmguards)
    {
        std::vector<Player*> const tanks = GetWyrmguardTanks(bot);
        for (Unit const* wyrmguard : wyrmguards)
        {
            Position const* spot = GetSpotOfOwner(tanks, wyrmguard);
            if (spot && wyrmguard->GetExactDist2d(spot) >= WYRMGUARD_IN_POSITION_DISTANCE)
                return false;
        }
        return true;
    }

    std::vector<Unit*> GetUnengagedDrakes(Player* bot)
    {
        std::vector<Unit*> drakes;
        for (BlackwingLairNPCs const npc : { BlackwingLairNPCs::NPC_FIREMAW, BlackwingLairNPCs::NPC_FLAMEGOR,
                                             BlackwingLairNPCs::NPC_EBONROC, BlackwingLairNPCs::NPC_CHROMAGGUS })
        {
            Creature* drake = bot->FindNearestCreature(static_cast<uint32>(npc), DRAKE_SEARCH_RANGE);
            if (drake && !drake->IsInCombat())
                drakes.push_back(drake);
        }
        return drakes;
    }

    // A bot already inside the radius may still step away from the drake.
    bool IsStepTowardsDrake(Player const* bot, std::vector<Unit*> const& drakes, float x, float y)
    {
        for (Unit const* drake : drakes)
        {
            float const distance = drake->GetDistance2d(x, y);
            if (distance < DRAKE_SAFE_DISTANCE && distance < drake->GetDistance2d(bot))
                return true;
        }
        return false;
    }
}
