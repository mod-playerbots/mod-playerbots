/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ZGHelpers.h"
#include "Player.h"
#include "Spell.h"
#include "SpellInfo.h"

namespace ZulGurubHelpers
{
bool IsGazedByMandokir(Player* bot)
{
    uint32 const gaze = static_cast<uint32>(ZulGurubSpells::SPELL_THREATENING_GAZE);
    if (bot->HasAura(gaze))
        return true;

    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || unit->GetEntry() != static_cast<uint32>(ZulGurubNpcs::NPC_BLOODLORD_MANDOKIR))
            continue;

        Spell const* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        return spell && spell->GetSpellInfo()->Id == gaze && spell->m_targets.GetUnitTargetGUID() == bot->GetGUID();
    }

    return false;
}
}  // namespace ZulGurubHelpers
