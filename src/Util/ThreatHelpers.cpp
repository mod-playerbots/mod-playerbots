/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ThreatHelpers.h"
#include "Creature.h"
#include "Player.h"
#include "ThreatManager.h"

namespace
{

char LowerAscii(char c) { return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c; }

bool EqualsIgnoringAsciiCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        if (LowerAscii(a[i]) != LowerAscii(b[i]))
            return false;
    }

    return true;
}

} // end anonymous namespace

Unit* FindThreatTarget(Player* bot, std::string_view name)
{
    if (name.empty() || !bot->GetGroup())
        return nullptr;

    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (unit && EqualsIgnoringAsciiCase(unit->GetName(), name))
            return unit->IsInWorld() ? unit : nullptr;
    }

    return nullptr;
}
