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
    if (target && bot->GetDistance(target) >= sPlayerbotAIConfig.SpellDistance)
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
    // botAI->ChangeStrategy("+combat,-stealthed", BOT_STATE_COMBAT);

    return true;
}

bool CheckStealthAction::Execute(Event /*event*/)
{
    if (botAI->HasAura("stealth", bot))
    {
        botAI->ChangeStrategy("-combat,+stealthed", BOT_STATE_COMBAT);
    }
    else
    {
        botAI->ChangeStrategy("+combat,-stealthed", BOT_STATE_COMBAT);
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

namespace
{
Item* FindHighestRankPoison(PlayerbotAI* botAI, bool deadly)
{
    static constexpr uint32 deadlyPoisons[] = {
        DEADLY_POISON_IX, DEADLY_POISON_VIII, DEADLY_POISON_VII, DEADLY_POISON_VI, DEADLY_POISON_V,
        DEADLY_POISON_IV, DEADLY_POISON_III, DEADLY_POISON_II, DEADLY_POISON
    };
    static constexpr uint32 instantPoisons[] = {
        INSTANT_POISON_IX, INSTANT_POISON_VIII, INSTANT_POISON_VII, INSTANT_POISON_VI, INSTANT_POISON_V,
        INSTANT_POISON_IV, INSTANT_POISON_III, INSTANT_POISON_II, INSTANT_POISON
    };

    if (deadly)
    {
        for (uint32 id : deadlyPoisons)
            if (Item* const poison = botAI->FindConsumable(id))
                return poison;
    }
    else
    {
        for (uint32 id : instantPoisons)
            if (Item* const poison = botAI->FindConsumable(id))
                return poison;
    }

    return nullptr;
}
}

bool UseDeadlyPoisonAction::Execute(Event /*event*/)
{
    Item* const poison = FindHighestRankPoison(botAI, true);
    if (!poison)
        return false;

    return UseItem(poison, ObjectGuid::Empty,
                   bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND));
}

bool UseInstantPoisonAction::Execute(Event /*event*/)
{
    Item* const poison = FindHighestRankPoison(botAI, false);
    if (!poison)
        return false;

    return UseItem(poison, ObjectGuid::Empty,
                   bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND));
}

bool UseInstantPoisonOffHandAction::Execute(Event /*event*/)
{
    Item* const poison = FindHighestRankPoison(botAI, false);
    if (!poison)
        return false;

    return UseItem(poison, ObjectGuid::Empty,
                   bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND));
}

namespace
{
uint32 GetWeaponDelay(Item const* item)
{
    if (!item)
        return 0;
    ItemTemplate const* proto = item->GetTemplate();
    return proto ? proto->Delay : 0;
}
}

bool UseBestMainHandPoisonAction::Execute(Event /*event*/)
{
    Item* const mh = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!mh)
        return false;

    Item* const oh = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    bool const useDeadly = !oh || GetWeaponDelay(mh) <= GetWeaponDelay(oh);

    Item* const poison = FindHighestRankPoison(botAI, useDeadly);
    if (!poison)
        return false;

    return UseItem(poison, ObjectGuid::Empty, mh);
}

bool UseBestOffHandPoisonAction::Execute(Event /*event*/)
{
    Item* const oh = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!oh)
        return false;

    Item* const mh = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    bool const useDeadly = mh && GetWeaponDelay(oh) < GetWeaponDelay(mh);

    Item* const poison = FindHighestRankPoison(botAI, useDeadly);
    if (!poison)
        return false;

    return UseItem(poison, ObjectGuid::Empty, oh);
}
