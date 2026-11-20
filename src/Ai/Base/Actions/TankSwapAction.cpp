/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TankSwapAction.h"
#include "Event.h"
#include "Playerbots.h"

uint32 TankSwapAction::TauntSpellFor(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:
            return 355;  // Taunt
        case CLASS_PALADIN:
            return 62124;  // Hand of Reckoning
        case CLASS_DEATH_KNIGHT:
            return 56222;  // Dark Command
        case CLASS_DRUID:
            return 6795;  // Growl
        default:
            return 0;
    }
}

void TankSwapAction::TellHumanTanks(std::string const& text) const
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        // Only real players: bot tanks coordinate through their own actions.
        if (GET_PLAYERBOT_AI(member))
            continue;

        if (!PlayerbotAI::IsTank(member))
            continue;

        botAI->Whisper(text, member->GetName());
    }
}

bool TankSwapAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE(Unit*, "current target");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Someone else holds the boss: this bot is the one that has to taunt.
    if (boss->GetVictim() != bot)
    {
        uint32 const spell = TauntSpellFor(bot->getClass());
        if (!spell || !botAI->CanCastSpell(spell, boss))
            return false;

        bot->CastSpell(boss, spell, false);
        TellHumanTanks(std::string("tank swap: I taunt ") + boss->GetName() + ", you are free");
        return true;
    }

    // The bot is currently tanking and a swap was requested: ask the human
    // tank to pick the boss up before the debuff becomes lethal.
    TellHumanTanks(std::string("tank swap: taunt ") + boss->GetName() + " now, my stacks are too high");
    return false;
}

bool TankSwapAction::isUseful()
{
    // Only tanks ever perform this, and only with a live boss in reach.
    if (!PlayerbotAI::IsTank(bot))
        return false;

    Unit* boss = AI_VALUE(Unit*, "current target");
    return boss && boss->IsAlive() && boss->IsInCombat();
}
