/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PartyMemberToDispel.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SpellMgr.h"
#include <optional>

class PartyMemberToDispelPredicate : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    // Dispel Magic reaches 40 yd on a friendly unit but only 30 yd on a hostile (charmed) one.
    struct Range
    {
        float Friendly;
        float Hostile;
    };

    PartyMemberToDispelPredicate(PlayerbotAI* botAI, uint32 dispelType, std::optional<Range> range)
        : FindPlayerPredicate(), PlayerbotAIAware(botAI), _dispelType(dispelType), _range(range)
    {
    }

    bool Check(Unit* unit) override
    {
        if (!unit->IsAlive())
            return false;

        if (_range)
        {
            Player* bot = botAI->GetBot();
            float const maxRange = bot->IsHostileTo(unit) ? _range->Hostile : _range->Friendly;
            if (!bot->IsWithinCombatRange(unit, maxRange))
                return false;
        }

        return botAI->HasAuraToDispel(unit, _dispelType);
    }

private:
    uint32 _dispelType;
    std::optional<Range> _range;
};

// Qualifier "<dispel type>" or "<dispel type> <spell>". With a spell the bot knows, the pick is
// limited to that spell's range, tested the way the core tests the cast.
Unit* PartyMemberToDispel::Calculate()
{
    uint32 dispelType = atoi(qualifier.c_str());

    std::optional<PartyMemberToDispelPredicate::Range> range;
    std::size_t const spellStart = qualifier.find(' ');
    if (spellStart != std::string::npos)
    {
        uint32 const spellId = AI_VALUE2(uint32, "spell id", qualifier.substr(spellStart + 1));
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            range = PartyMemberToDispelPredicate::Range{spellInfo->GetMaxRange(true, bot),
                                                        spellInfo->GetMaxRange(false, bot)};
    }

    PartyMemberToDispelPredicate predicate(botAI, dispelType, range);
    return FindPartyMember(predicate);
}
