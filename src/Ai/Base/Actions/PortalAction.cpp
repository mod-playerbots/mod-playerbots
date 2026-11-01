/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PortalAction.h"
#include "ChatHelper.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <ctime>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    struct PortalClaims
    {
        time_t expires = 0;
        std::vector<uint32> spellIds;
    };

    std::mutex portalClaimsMutex;
    std::unordered_map<uint64, PortalClaims> portalClaims;
    constexpr time_t PORTAL_CLAIM_SECONDS = 60;
    constexpr float PORTAL_MAX_MASTER_DISTANCE = 30.0f;

    SpellInfo const* FindPortal(std::vector<SpellInfo const*> const& portals, std::string const& destination)
    {
        for (SpellInfo const* spellInfo : portals)
        {
            std::string name = spellInfo->SpellName[LOCALE_enUS];
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (name.find(destination) != std::string::npos)
                return spellInfo;
        }

        return nullptr;
    }

    bool IsClaimed(PortalClaims const& claims, uint32 spellId)
    {
        return std::find(claims.spellIds.begin(), claims.spellIds.end(), spellId) != claims.spellIds.end();
    }

    void CleanupPortalClaims(time_t now)
    {
        for (auto it = portalClaims.begin(); it != portalClaims.end();)
        {
            if (it->second.expires <= now)
                it = portalClaims.erase(it);
            else
                ++it;
        }
    }

    SpellInfo const* ReservePortal(uint64 key, std::vector<SpellInfo const*> const& order)
    {
        std::lock_guard<std::mutex> lock(portalClaimsMutex);
        time_t const now = time(nullptr);

        CleanupPortalClaims(now);
        PortalClaims& claims = portalClaims[key];

        for (SpellInfo const* candidate : order)
        {
            if (IsClaimed(claims, candidate->Id))
                continue;

            claims.spellIds.push_back(candidate->Id);
            claims.expires = now + PORTAL_CLAIM_SECONDS;
            return candidate;
        }

        return nullptr;
    }

    void ReleasePortal(uint64 key, uint32 spellId)
    {
        std::lock_guard<std::mutex> lock(portalClaimsMutex);
        auto it = portalClaims.find(key);
        if (it == portalClaims.end())
            return;

        std::vector<uint32>& ids = it->second.spellIds;
        ids.erase(std::remove(ids.begin(), ids.end(), spellId), ids.end());
    }
}

bool PortalAction::isUseful() { return bot->getClass() == CLASS_MAGE; }

bool PortalAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (master && !bot->IsWithinDistInMap(master, PORTAL_MAX_MASTER_DISTANCE))
        return false;

    std::string destination = event.getParam();
    std::transform(destination.begin(), destination.end(), destination.begin(), ::tolower);

    bool const allMode = destination == "all" || destination.rfind("all ", 0) == 0;
    if (allMode)
        destination = destination.size() > 4 ? destination.substr(4) : "";

    std::vector<SpellInfo const*> portals = GetKnownPortals();
    std::sort(portals.begin(), portals.end(),
        [](SpellInfo const* lhs, SpellInfo const* rhs) { return lhs->Id < rhs->Id; });

    if (portals.empty())
    {
        Tell("portal_none_known", "I don't know any portals");
        return false;
    }

    // Plain "portal": list what we can open
    if (destination.empty() && !allMode)
    {
        std::string list;
        for (SpellInfo const* spellInfo : portals)
            list += ChatHelper::FormatSpell(spellInfo) + " ";

        Tell("portal_list", "I can open portals to: %portals", {{"%portals", list}});
        return true;
    }

    SpellInfo const* requested = destination.empty() ? nullptr : FindPortal(portals, destination);
    if (!destination.empty() && !requested)
    {
        Tell("portal_unknown", "I don't know a portal to %destination", {{"%destination", destination}});
        return false;
    }

    Group* group = bot->GetGroup();
    uint64 const claimKey = group ? group->GetGUID().GetRawValue() : bot->GetGUID().GetRawValue();

    std::vector<SpellInfo const*> order;
    if (!allMode)
        order.push_back(requested);
    else
    {
        if (requested)
        {
            portals.erase(std::find(portals.begin(), portals.end(), requested));
            portals.insert(portals.begin(), requested);
        }

        order = portals;
    }

    // Already claimed by another mage.
    SpellInfo const* portal = ReservePortal(claimKey, order);
    if (!portal)
        return false;

    if (!HasReagents(portal))
    {
        ReleasePortal(claimKey, portal->Id);
        Tell("portal_no_reagents", "I need a Rune of Portals for %portal",
             {{"%portal", ChatHelper::FormatSpell(portal)}});
        return false;
    }

    if (bot->isMoving())
        bot->StopMoving();

    // Turn away from the master to spread the portals out
    if (master)
    {
        float const angle = master->GetAngle(bot);
        bot->SetOrientation(angle);
        bot->SendMovementFlagUpdate(true);
    }

    if (!botAI->CastSpell(portal->Id, bot))
    {
        ReleasePortal(claimKey, portal->Id);
        Tell("portal_cannot_cast", "I cannot open %portal right now",
             {{"%portal", ChatHelper::FormatSpell(portal)}});
        return false;
    }

    Tell("portal_casting", "Opening %portal", {{"%portal", ChatHelper::FormatSpell(portal)}});
    return true;
}

std::vector<SpellInfo const*> PortalAction::GetKnownPortals() const
{
    std::vector<SpellInfo const*> portals;
    for (auto const& [spellId, playerSpell] : bot->GetSpellMap())
    {
        if (playerSpell->State == PLAYERSPELL_REMOVED || !playerSpell->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive() || !spellInfo->SpellName[LOCALE_enUS])
            continue;

        if (std::string(spellInfo->SpellName[LOCALE_enUS]).rfind("Portal: ", 0) == 0)
            portals.push_back(spellInfo);
    }

    return portals;
}

bool PortalAction::HasReagents(SpellInfo const* spellInfo) const
{
    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; ++i)
        if (spellInfo->Reagent[i] > 0 && !bot->HasItemCount(spellInfo->Reagent[i], spellInfo->ReagentCount[i]))
            return false;

    return true;
}

void PortalAction::Tell(std::string const& key, std::string const& defaultText,
                        std::map<std::string, std::string> const& placeholders)
{
    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(key, defaultText, placeholders));
}
