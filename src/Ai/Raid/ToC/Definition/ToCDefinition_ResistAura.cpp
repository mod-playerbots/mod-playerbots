/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "PlayerbotAI.h"
#include "RaidResistAura.h"
#include "Strategy.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_ResistAura.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Role = RaidEncounterRules::Role;

namespace
{
void DefineResistAura(EncounterBuilder& e)
{
    // Above the staging walk: the aura is instant, and the walk holds the tick while in flight
    e.Node(
        "toc resistance aura",
        [](PlayerbotAI* botAI) -> Trigger*
        { return new RaidResistAuraTrigger(botAI, "toc resistance aura", TOC_RESIST_AURA); },
        "toc raise resistance aura",
        [](PlayerbotAI* botAI) -> Action*
        { return new RaidRaiseResistAuraAction(botAI, "toc raise resistance aura", TOC_RESIST_AURA); },
        ACTION_RAID + 1);

    // The aura slot is exclusive, so the paladin's own buff strategy trades it back on the next GCD. The
    // resistance auras are on the list too: the raise action casts outside the queue.
    e.Block("toc resistance aura hold", Role::Any, IsToCResistPaladinHolding, 0,
            {"devotion aura", "retribution aura", "concentration aura", "crusader aura", "sanctity aura",
             "fire resistance aura", "frost resistance aura", "shadow resistance aura"});

    // Every aspect, not just Dragonhawk: a vetoed node still pushes its alternatives, and the hunter
    // buff chain runs dragonhawk, hawk, monkey.
    e.Block("toc nature aspect hold", Role::Any, IsToCResistHunterHolding, 0,
            {"aspect of the viper", "aspect of the dragonhawk", "aspect of the hawk", "aspect of the monkey",
             "aspect of the cheetah", "aspect of the pack"});
}
}  // namespace

EncounterDefinition const& ToCResistAuraDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::None), &ToCRaidWideGateOpen,
                                                &DefineResistAura);
    return definition;
}
