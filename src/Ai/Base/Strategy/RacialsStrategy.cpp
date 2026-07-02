/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RacialsStrategy.h"
#include "Playerbots.h"

namespace
{
    constexpr uint32 SPELL_ARCANE_TORRENT_ENERGY = 25046;
    constexpr uint32 SPELL_ARCANE_TORRENT_MANA = 28730;
    constexpr uint32 SPELL_ARCANE_TORRENT_RUNIC_POWER = 50613;
    constexpr uint32 SPELL_WAR_STOMP = 20549;
    constexpr uint32 SPELL_BERSERKING = 26297;
    constexpr uint32 SPELL_EVERY_MAN_FOR_HIMSELF = 59752;
    constexpr uint32 SPELL_WILL_OF_THE_FORSAKEN = 7744;
    constexpr uint32 SPELL_STONEFORM = 20594;
    constexpr uint32 SPELL_ESCAPE_ARTIST = 20589;
}

RacialsStrategy::RacialsStrategy(PlayerbotAI* botAI) : Strategy(botAI)
{
    // No custom ActionNodeFactory needed
}

void RacialsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_MANA))
    {
        triggers.push_back(new TriggerNode(
            "low mana", { NextAction("arcane torrent", ACTION_NORMAL + 5) }));
    }

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_ENERGY))
    {
        triggers.push_back(new TriggerNode(
            "low energy", { NextAction("arcane torrent", ACTION_NORMAL + 5) }));
    }

    if (bot->HasSpell(SPELL_ARCANE_TORRENT_RUNIC_POWER))
    {
        // No low runic power trigger exists; this trigger should be modified if one is added
        triggers.push_back(new TriggerNode(
            "generic boost", { NextAction("arcane torrent", ACTION_NORMAL + 5) }));
    }

    if (bot->HasSpell(SPELL_WAR_STOMP))
    {
        triggers.push_back(new TriggerNode(
            "medium aoe", { NextAction("war stomp", ACTION_NORMAL + 5) }));
    }

    if (bot->HasSpell(SPELL_BERSERKING))
    {
        triggers.push_back(new TriggerNode(
            "generic boost", { NextAction("berserking", ACTION_NORMAL + 5) }));
    }

    if (bot->HasSpell(SPELL_EVERY_MAN_FOR_HIMSELF))
    {
        triggers.push_back(new TriggerNode(
            "loss of control", { NextAction("every man for himself", ACTION_EMERGENCY + 1) }));
    }

    if (bot->HasSpell(SPELL_WILL_OF_THE_FORSAKEN))
    {
        triggers.push_back(new TriggerNode(
            "fear charm sleep", { NextAction("will of the forsaken", ACTION_EMERGENCY + 1) }));
    }

    if (bot->HasSpell(SPELL_STONEFORM))
    {
        triggers.push_back(new TriggerNode(
            "poison disease bleed", { NextAction("stoneform", ACTION_DISPEL) }));
    }

    if (bot->HasSpell(SPELL_ESCAPE_ARTIST))
    {
        triggers.push_back(new TriggerNode(
            "movement impaired", { NextAction("escape artist", ACTION_EMERGENCY + 1) }));
    }

    if (botAI->HasSpell("blood fury"))
    {
        triggers.push_back(new TriggerNode(
            "generic boost", { NextAction("blood fury", ACTION_NORMAL + 5) }));
    }

    if (botAI->HasSpell("gift of the naaru"))
    {
        // Currently targets self only
        triggers.push_back(new TriggerNode(
            "medium health", { NextAction("gift of the naaru", ACTION_LIGHT_HEAL + 5) }));
    }

    if (botAI->HasSpell("lifeblood"))
    {
        triggers.push_back(new TriggerNode(
            "medium health", { NextAction("lifeblood", ACTION_LIGHT_HEAL + 5) }));
    }

    triggers.push_back(new TriggerNode(
        "generic boost", { NextAction("use trinket", ACTION_NORMAL + 4) }));
}
