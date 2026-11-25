/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCActions_FactionChampions.h"
#include "Pet.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsDefence.h"
#include "Playerbots.h"
#include "SpellMgr.h"

using namespace TrialOfTheCrusaderHelpers;

bool FactionChampionsAntiFearAction::FearWindowActive() { return FactionChampionsFearWindowActive(botAI); }

bool FactionChampionsFocusPriorityAction::Execute(Event /*event*/)
{
    return AttackDirect(FactionChampionsKillTarget(botAI));
}

bool FactionChampionsCounterspellKillTargetAction::Execute(Event /*event*/)
{
    Unit* killTarget = FactionChampionsKillTarget(botAI);
    return killTarget && botAI->CastSpell("counterspell", killTarget);
}

// A queued basket can pop ticks after its trigger fired, once the heal is over
bool FactionChampionsCounterspellKillTargetAction::isUseful() { return FactionChampionsCounterspellDuty(botAI); }

// The non-combat toggle pet spell turns a taunt back on after the fight. False, so the bot still gets its
// own action this tick.
bool FactionChampionsPetAutocastAction::Execute(Event /*event*/)
{
    bool enable = false;
    uint32 const spellId = FactionChampionsPetAutocastToggle(botAI, enable);
    Pet* pet = bot->GetPet();
    if (spellId && pet)
        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
            pet->ToggleAutocast(info, enable);

    return false;
}
