/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_FactionChampions.h"
#include "ToCHelpers_FactionChampionsBall.h"
#include "ToCHelpers_FactionChampionsDefence.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
constexpr uint32 PLAYER_DIVINE_SHIELD = 642;
constexpr uint32 FORBEARANCE = 25771;

// The focus trigger's test, so the rule and the node cover the same window. An ice duty DK hits its own
// champion.
bool FactionChampionsTargetOwned(PlayerbotAI* botAI)
{
    return (FactionChampionsFocusBot(botAI) && FactionChampionsKillTarget(botAI)) ||
           FactionChampionsHasIceDuty(botAI);
}

bool FactionChampionsStayPut(PlayerbotAI* botAI) { return FactionChampionsStaysPut(botAI); }

// Kill squad melee, tanks included, wait outside the pack while the player's pull is still flying, its healer
// takes the kill latch once it lands, or while a DK is on its way to grip the kill target out of the pack
bool FactionChampionsKillSquadWait(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsMelee(bot) || FactionChampionsHasIceDuty(botAI))
        return false;

    return !FactionChampionsChainTarget(bot).IsEmpty() || FactionChampionsGripPending(botAI);
}

bool FactionChampionsTotemHeld(PlayerbotAI* botAI) { return FactionChampionsTotemTarget(botAI) != nullptr; }

// On grip duty, or a grip chain link still to grip
bool FactionChampionsGripHeld(PlayerbotAI* botAI)
{
    Position stand;
    return FactionChampionsGripTarget(botAI, stand) != nullptr || FactionChampionsChainLink(botAI);
}

// Walking to its wall stand or waiting there for the grip
bool FactionChampionsWallGripHeld(PlayerbotAI* botAI)
{
    Position stand;
    uint32 spellId = 0;
    bool gripped = false;
    return FactionChampionsWallGrip(botAI, stand, spellId, &gripped) && !gripped;
}

bool FactionChampionsLive(PlayerbotAI* botAI) { return ToCEncounterIsLive(botAI, ToCEncounter::FactionChampions); }

// Its 30 s marker blocks Divine Shield, Divine Protection and Hand of Protection, and these paladins are
// the ones the champions focus
bool FactionChampionsWrathHeld(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return bot->getClass() == CLASS_PALADIN && (PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot)) &&
           FactionChampionsLive(botAI);
}

// The bot's own Divine Shield is ready but for the GCD, and Lay on Hands' Forbearance would lock it out 2 min
bool FactionChampionsBubbleReady(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->getClass() != CLASS_PALADIN || !FactionChampionsLive(botAI) || !bot->HasSpell(PLAYER_DIVINE_SHIELD) ||
        bot->HasSpellCooldown(PLAYER_DIVINE_SHIELD) || bot->HasAura(FORBEARANCE))
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(PLAYER_DIVINE_SHIELD);
    return info && !(info->ExcludeCasterAuraSpell && bot->HasAura(info->ExcludeCasterAuraSpell));
}

bool FactionChampionsHeroicLive(PlayerbotAI* botAI)
{
    Map* map = botAI->GetBot()->FindMap();
    return map && map->IsHeroic() && FactionChampionsLive(botAI);
}

void DefineFactionChampions(EncounterBuilder& e)
{
    // Assist makes its own pick, so on any tick the settled focus node returns false it would take the
    // bot off the kill target
    e.OwnTargeting("faction champions target guard multiplier", Role::Any, FactionChampionsTargetOwned,
                   Family::DpsAssist | Family::TankAssist);
    // assist hands a healer a Feared champion that "invalid target" drops next tick, so it loops there instead
    // of healing, and a healer's pick also holds our Polymorph and Fear off that champion
    e.OwnTargeting("faction champions healer target guard multiplier", Role::Heal, FactionChampionsLive,
                   Family::DpsAssist);

    // once the bot is on its totem the kill totem node returns false, and either of these would pull it off
    e.Block("faction champions totem guard multiplier", Role::Any, FactionChampionsTotemHeld, 0,
            {"faction champions focus priority", "faction champions physical switch"});

    // the DK walks out of melee to its grip stand, and these would walk it straight back
    e.Block("faction champions grip hold", Role::Any, FactionChampionsGripHeld,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Flee |
                Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink | Family::Disengage,
            {"reach melee", "reach spell"});

    // the same for a DK on its way to the wall, or waiting there for its grip
    e.Block("faction champions wall grip hold", Role::Any, FactionChampionsWallGripHeld,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Flee |
                Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink | Family::Disengage,
            {"reach melee", "reach spell"});

    // these would drag its champion onto the raid, the kite has its own node
    e.Block("faction champions ice hold", Role::Any, FactionChampionsHasIceDuty,
            Family::CombatFormationMove | Family::Follow | Family::Flee | Family::RunAway | Family::MoveRandom);

    // its Frost and Death runes go to the champion's Chains of Ice, cast by faction champions chains of ice
    e.Block("faction champions ice rune reserve", Role::Any, FactionChampionsHasIceDuty, 0,
            {"icy touch", "icy touch on attacker", "obliterate", "howling blast", "death strike", "scourge strike",
             "death and decay", "unbreakable armor", "army of the dead", "chains of ice"});

    // a healer or ranged bot on the move neither heals nor casts, so only the reaches walk it, to get its target in
    // range
    e.Block("faction champions stay put", Role::Any, FactionChampionsStayPut,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Flee |
                Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink | Family::Disengage);

    // a kiting bot walking or charging back into reach of its target, or its heal target, hands the chaser its
    // gap again
    e.Block("faction champions kite hold", Role::Any, FactionChampionsKiteHold, Family::Charge,
            {"reach spell", "reach melee", "reach party member to heal"});

    e.Block("faction champions kill squad wait", Role::Any, FactionChampionsKillSquadWait, Family::Charge,
            {"reach melee"});

    // the class Rebirth raises the master first and a corpse inside the pack too, faction champions battle
    // res picks the corpse instead
    e.Block("faction champions battle res guard", Role::Any, FactionChampionsLive, 0, {"rebirth"});

    // every paladin's class node fires at one dying bot at once; faction champions protect deals one
    e.Block("faction champions lay on hands guard", Role::Any, FactionChampionsLive, 0, {"lay on hands on party"});
    e.Block("faction champions self lay on hands hold", Role::Any, FactionChampionsBubbleReady, 0, {"lay on hands"});
    e.Block("faction champions avenging wrath hold", Role::Any, FactionChampionsWrathHeld, 0, {"avenging wrath"});
    // faction champions protect spends it on a focused bot
    e.Block("faction champions divine sacrifice hold", Role::Any, FactionChampionsLive, 0, {"divine sacrifice"});
    // the class node spends the 25 s cooldown on whoever it sees first; faction champions freedom keeps it for a
    // rooted kiter
    e.Block("faction champions freedom guard", Role::Any, FactionChampionsLive, 0, {"hand of freedom on party"});
    // the class nodes spend them closing on a target, the opening run-in too; faction champions speed boost keeps
    // them for a melee champion in contact
    e.Block("faction champions speed boost guard", Role::Any, FactionChampionsLive, 0, {"sprint", "dash"});
    // the Enh rotation pops it at full health for mana; faction champions personal defensive keeps it for a burst
    e.Block("faction champions personal reserve", Role::Any, FactionChampionsLive, 0, {"shamanistic rage"});

    // heroic champions are immune to taunt, and the generic Death Grip would spend the grip duty's cooldown
    e.Block("faction champions heroic taunt guard", Role::Any, FactionChampionsHeroicLive, Family::Taunt);

    // every link keeps its Death Grip for its turn in the chain
    e.Block("faction champions chain grip reserve", Role::Any, FactionChampionsChainLink, 0, {"death grip"});
}
}  // namespace

EncounterDefinition const& ToCFactionChampionsDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::FactionChampions), &ToCGateOpen,
                                                &DefineFactionChampions);
    return definition;
}
