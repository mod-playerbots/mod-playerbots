/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RogueActions.h"

#include "Event.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 BG_WS_SPELL_WARSONG_FLAG = 23333;
constexpr uint32 BG_WS_SPELL_SILVERWING_FLAG = 23335;
constexpr uint32 BG_EY_NETHERSTORM_FLAG_SPELL = 34976;
constexpr uint32 SPELL_MASTER_POISONER_RANK_3 = 58410;
}

bool CastStealthAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && bot->GetDistance(target) >= sPlayerbotAIConfig.spellDistance)
        return false;
    return true;
}

bool CastStealthAction::isPossible()
{
    // do not use with WSG flag or EYE flag
    return !bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) && !bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) &&
           !bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL);
}

bool UnstealthAction::Execute(Event /*event*/)
{
    botAI->RemoveAura("stealth");
    // botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);

    return true;
}

bool CheckStealthAction::Execute(Event /*event*/)
{
    if (botAI->HasAura("stealth", bot))
    {
        botAI->ChangeStrategy("-dps,+stealthed", BOT_STATE_COMBAT);
    }
    else
    {
        botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);
    }

    return true;
}

bool CastVanishAction::isUseful()
{
    // do not use with WSG flag or EYE flag
    return !bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) && !bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) &&
           !bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL);
}

bool CastEnvenomAction::isUseful()
{
    return AI_VALUE2(uint8, "energy", "self target") >= 35;
}

bool CastEnvenomAction::isPossible()
{
    // alternate to eviscerate if talents unlearned
    return bot->HasAura(SPELL_MASTER_POISONER_RANK_3);
}

bool CastTricksOfTheTradeOnMainTankAction::isUseful()
{
    return CastSpellAction::isUseful() && AI_VALUE2(float, "distance", GetTargetName()) < 20.0f;
}

bool UseDeadlyPoisonAction::Execute(Event /*event*/)
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Deadly Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    if (items.empty())
    {
        return false;
    }
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return UseItem(*items.begin(), ObjectGuid::Empty, itemForSpell);
    // return UseItemAuto(*items.begin());
}

bool UseDeadlyPoisonAction::isPossible()
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Deadly Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    return !items.empty();
}

bool UseInstantPoisonAction::Execute(Event /*event*/)
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Instant Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    if (items.empty())
    {
        return false;
    }
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return UseItem(*items.begin(), ObjectGuid::Empty, itemForSpell);
}

bool UseInstantPoisonAction::isPossible()
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Instant Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    return !items.empty();
}

bool UseInstantPoisonOffHandAction::Execute(Event /*event*/)
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Instant Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    if (items.empty())
    {
        return false;
    }
    Item* const itemForSpell = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return UseItem(*items.begin(), ObjectGuid::Empty, itemForSpell);
}

bool UseInstantPoisonOffHandAction::isPossible()
{
    std::vector<std::string> poison_suffixs = {" IX", " VIII", " VII", " VI", " V", " IV", " III", " II", ""};
    std::vector<Item*> items;
    std::string poison_name;
    for (std::string& suffix : poison_suffixs)
    {
        poison_name = "Instant Poison" + suffix;
        items = AI_VALUE2(std::vector<Item*>, "inventory items", poison_name);
        if (!items.empty())
        {
            break;
        }
    }
    return !items.empty();
}
