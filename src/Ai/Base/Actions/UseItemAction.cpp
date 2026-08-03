/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UseItemAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemPackets.h"
#include "ItemUsageValue.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

bool UseItemAction::Execute(Event event)
{
    std::string name = event.getParam();
    if (name.empty())
        name = getName();

    std::vector<Item*> items = AI_VALUE2(std::vector<Item*>, "inventory items", name);
    GuidVector gos = chat->parseGameobjects(name);

    if (gos.empty())
    {
        if (!items.empty())
        {
            return UseItemAuto(*items.begin());
        }
    }
    else
    {
        if (items.empty())
            return UseGameObject(*gos.begin());
        else
            return UseItemOnGameObject(*items.begin(), *gos.begin());
    }

    botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "use_item_none_available", "No items (or game objects) available", {}));
    return false;
}

bool UseItemAction::UseGameObject(ObjectGuid guid)
{
    GameObject* go = botAI->GetGameObject(guid);
    if (!go || !go->isSpawned() /* || go->GetGoState() != GO_STATE_READY*/)
        return false;

    go->Use(bot);

    std::ostringstream out;
    botAI->TellMasterNoFacing(PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "use_gameobject",
        "Using %gameobject",
        {{"%gameobject", chat->FormatGameobject(go)}}));
    return true;
}

bool UseItemAction::UseItemAuto(Item* item) { return UseItem(item, ObjectGuid::Empty, nullptr); }

bool UseItemAction::UseItemOnGameObject(Item* item, ObjectGuid go) { return UseItem(item, go, nullptr); }

bool UseItemAction::UseItemOnItem(Item* item, Item* itemTarget) { return UseItem(item, ObjectGuid::Empty, itemTarget); }

bool UseItemAction::UseItem(Item* item, ObjectGuid goGuid, Item* itemTarget, Unit* unitTarget)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 cast_count = 1;
    ObjectGuid item_guid = item->GetGUID();
    uint32 glyphIndex = 0;
    uint8 castFlags = 0;
    uint32 targetFlag = TARGET_FLAG_NONE;
    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetTemplate()->Spells[i].SpellId > 0)
        {
            spellId = item->GetTemplate()->Spells[i].SpellId;
            if (!botAI->CanCastSpell(spellId, bot, false, itemTarget, item))
            {
                return false;
            }
        }
    }

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << castFlags;

    bool targetSelected = false;

    std::string itemText = chat->FormatItem(item->GetTemplate());
    std::string targetText;

    if (item->GetTemplate()->Stackable > 1)
    {
        uint32 count = item->GetCount();
        if (count > 1)
            itemText += " (" + std::to_string(count) + " available)";
        else
            itemText += " (the last one!)";
    }

    if (goGuid)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !go->isSpawned())
            return false;

        targetFlag = TARGET_FLAG_GAMEOBJECT;

        packet << targetFlag;
        packet << goGuid.WriteAsPacked();
        targetText = chat->FormatGameobject(go);
        targetSelected = true;
    }

    if (itemTarget)
    {
        if (item->GetTemplate()->Class == ITEM_CLASS_GEM)
        {
            bool fit = SocketItem(itemTarget, item) || SocketItem(itemTarget, item, true);
            if (!fit)
                botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                    "socket_does_not_fit", "Socket does not fit", {}));

            return fit;
        }
        else
        {
            targetFlag = TARGET_FLAG_ITEM;
            packet << targetFlag;
            packet << itemTarget->GetGUID().WriteAsPacked();
            targetText = chat->FormatItem(itemTarget->GetTemplate());
            targetSelected = true;
        }
    }

    Player* master = GetMaster();
    if (!targetSelected && item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE && master &&
        botAI->HasActivePlayerMaster() && !selfOnly)
    {
        if (ObjectGuid masterSelection = master->GetTarget())
        {
            Unit* unit = botAI->GetUnit(masterSelection);
            if (unit)
            {
                targetFlag = TARGET_FLAG_UNIT;
                packet << targetFlag << masterSelection.WriteAsPacked();
                targetText = unit->GetName();
                targetSelected = true;
            }
        }
    }

    if (!targetSelected && item->GetTemplate()->Class != ITEM_CLASS_CONSUMABLE && unitTarget)
    {
        targetFlag = TARGET_FLAG_UNIT;
        packet << targetFlag << unitTarget->GetGUID().WriteAsPacked();
        targetText = unitTarget->GetName();
        targetSelected = true;
    }

    if (uint32 questid = item->GetTemplate()->StartQuest)
    {
        if (Quest const* qInfo = sObjectMgr->GetQuestTemplate(questid))
        {
            WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST, 8 + 4 + 4);
            packet << item_guid;
            packet << questid;
            packet << uint32(0);
            bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

            botAI->TellMasterNoFacing("Got quest " + chat->FormatQuest(qInfo));
            return true;
        }
    }

    bot->ClearUnitState(UNIT_STATE_CHASE);
    bot->ClearUnitState(UNIT_STATE_FOLLOW);

    if (bot->isMoving())
    {
        bot->StopMoving();
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.globalCoolDown);
        return false;
    }

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        uint32 spellId = item->GetTemplate()->Spells[i].SpellId;
        if (!spellId)
            continue;

        if (!botAI->CanCastSpell(spellId, bot, false))
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (spellInfo->Targets & TARGET_FLAG_ITEM)
        {
            Item* itemForSpell = AI_VALUE2(Item*, "item for spell", spellId);
            if (!itemForSpell)
                continue;

            if (itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                continue;

            if (bot->GetTrader())
            {
                if (selfOnly)
                    return false;

                targetFlag = TARGET_FLAG_TRADE_ITEM;
                packet << targetFlag << (uint8)1 << ObjectGuid((uint64)TRADE_SLOT_NONTRADED).WriteAsPacked();
                targetSelected = true;
                targetText = "traded item";
            }
            else
            {
                targetFlag = TARGET_FLAG_ITEM;
                packet << targetFlag;
                packet << itemForSpell->GetGUID().WriteAsPacked();
                targetSelected = true;
                targetText = chat->FormatItem(itemForSpell->GetTemplate());
            }
            uint32 castTime = spellInfo->CalcCastTime();
            botAI->SetNextCheckDelay(castTime + sPlayerbotAIConfig.reactDelay);
        }

        break;
    }

    if (!targetSelected)
    {
        targetFlag = TARGET_FLAG_NONE;
        packet << targetFlag;

        // Use the actual target if provided
        if (unitTarget)
        {
            packet << unitTarget->GetGUID();
            targetSelected = true;

            if (unitTarget == bot || !unitTarget->IsInWorld() || unitTarget->IsDuringRemoveFromWorld())
                targetText = "self";
            else if (unitTarget->IsHostileTo(bot))
                targetText = "self";
            else
                targetText = unitTarget->GetName();
        }
        else
        {
            packet << bot->GetPackGUID();
            targetSelected = true;
            targetText = "self";
        }
    }

    ItemTemplate const* proto = item->GetTemplate();
    bool isDrink = proto->Spells[0].SpellCategory == 59;
    bool isFood = proto->Spells[0].SpellCategory == 11;
    if (proto->Class == ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ITEM_SUBCLASS_FOOD || proto->SubClass == ITEM_SUBCLASS_CONSUMABLE) && (isFood || isDrink))
    {
        if (bot->IsInCombat())
            return false;

        // bot->SetStandState(UNIT_STAND_STATE_SIT);
        botAI->InterruptSpell();
        float hp = bot->GetHealthPct();
        float mp = bot->GetPower(POWER_MANA) * 100.0f / bot->GetMaxPower(POWER_MANA);
        float p = 0.f;
        if (isDrink && isFood)
        {
            p = std::min(hp, mp);
            TellConsumableUse(item, "Feasting", p);
        }
        else if (isDrink)
        {
            p = mp;
            TellConsumableUse(item, "Drinking", p);
        }
        else if (isFood)
        {
            p = std::min(hp, mp);
            TellConsumableUse(item, "Eating", p);
        }

        if (!bot->IsInCombat() && !bot->InBattleground())
            botAI->SetNextCheckDelay(std::max(10000.0f, 27000.0f * (100 - p) / 100.0f));

        if (!bot->IsInCombat() && bot->InBattleground())
            botAI->SetNextCheckDelay(std::max(10000.0f, 20000.0f * (100 - p) / 100.0f));

        // botAI->SetNextCheckDelay(27000.0f * (100 - p) / 100.0f);
        //  botAI->SetNextCheckDelay(20000);
        bot->GetSession()->HandleUseItemOpcode(packet);

        return true;
    }

    if (!spellId)
        return false;

    // botAI->SetNextCheckDelay(sPlayerbotAIConfig.globalCoolDown);
    std::string useText = targetSelected
        ? PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "use_item_on_target", "Using %item on %target", {{"%item", itemText}, {"%target", targetText}})
        : PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "use_item", "Using %item", {{"%item", itemText}});
    botAI->TellMasterNoFacing(useText);
    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

void UseItemAction::TellConsumableUse(Item* item, std::string const action, float percent)
{
    std::ostringstream out;
    out << action << " " << chat->FormatItem(item->GetTemplate());

    if (item->GetTemplate()->Stackable > 1)
        out << "/x" << item->GetCount();

    out << " (" << round(percent) << "%)";
    botAI->TellMasterNoFacing(out.str());
}

bool UseItemAction::SocketItem(Item* item, Item* gem, bool replace)
{
    WorldPacket packet(CMSG_SOCKET_GEMS);
    packet << item->GetGUID();

    bool fits = false;
    for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS;
         ++enchant_slot)
    {
        uint8 SocketColor = item->GetTemplate()->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
        GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gem->GetTemplate()->GemProperties);
        if (gemProperty && (gemProperty->color & SocketColor))
        {
            if (fits)
            {
                packet << ObjectGuid::Empty;
                continue;
            }

            uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(enchant_slot));
            if (!enchant_id)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }

            SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
            if (!enchantEntry || !enchantEntry->GemID)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }

            if (replace && enchantEntry->GemID != gem->GetTemplate()->ItemId)
            {
                packet << gem->GetGUID();
                fits = true;
                continue;
            }
        }

        packet << ObjectGuid::Empty;
    }

    if (fits)
    {
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "socketing_item_with_gem",
            "Socketing %item with %gem",
            {{"%item", chat->FormatItem(item->GetTemplate())}, {"%gem", chat->FormatItem(gem->GetTemplate())}}));

        WorldPackets::Item::SocketGems nicePacket(std::move(packet));
        nicePacket.Read();
        bot->GetSession()->HandleSocketOpcode(nicePacket);
    }

    return fits;
}

bool UseItemAction::isPossible() { return getName() == "use" || AI_VALUE2(uint32, "item count", getName()) > 0; }

bool UseSpellItemAction::isUseful() { return AI_VALUE2(bool, "spell cast useful", getName()); }

bool UseHealingPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

bool UseManaPotion::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

bool UseHearthStone::Execute(Event event)
{
    if (bot->isMoving())
    {
        MotionMaster& mm = *bot->GetMotionMaster();
        bot->StopMoving();
        mm.Clear();
    }

    bool used = UseItemAction::Execute(event);
    if (used)
    {
        RESET_AI_VALUE(bool, "combat::self target");
        RESET_AI_VALUE(WorldPosition, "current position");
        botAI->SetNextCheckDelay(10 * IN_MILLISECONDS);
    }

    return used;
}

bool UseHearthStone::isUseful() { return !bot->InBattleground(); }

bool UseRandomRecipe::Execute(Event /*event*/)
{
    std::vector<Item*> recipes = AI_VALUE2(std::vector<Item*>, "inventory items", "recipe");

    std::string recipeName = "";

    for (auto& recipe : recipes)
    {
        recipeName = recipe->GetTemplate()->Name1;
    }

    if (recipeName.empty())
        return false;

    bool used = UseItemAction::Execute(Event(name, recipeName));

    if (used)
        botAI->SetNextCheckDelay(3.0 * IN_MILLISECONDS);

    return used;
}

bool UseRandomRecipe::isUseful()
{
    return !bot->IsInCombat() && !botAI->HasActivePlayerMaster() && !bot->InBattleground();
}

bool UseRandomRecipe::isPossible() { return AI_VALUE2(uint32, "item count", "recipe") > 0; }

bool UseRandomQuestItem::Execute(Event /*event*/)
{
    Unit* unitTarget = nullptr;
    ObjectGuid goTarget;

    std::vector<Item*> questItems = AI_VALUE2(std::vector<Item*>, "inventory items", "quest");
    if (questItems.empty())
        return false;

    Item* item = nullptr;
    for (uint8 i = 0; i < 5; i++)
    {
        auto itr = questItems.begin();
        std::advance(itr, urand(0, questItems.size() - 1));
        Item* questItem = *itr;

        ItemTemplate const* proto = questItem->GetTemplate();
        if (proto->StartQuest)
        {
            Quest const* qInfo = sObjectMgr->GetQuestTemplate(proto->StartQuest);
            if (bot->CanTakeQuest(qInfo, false))
            {
                item = questItem;
                break;
            }
        }
    }

    if (!item)
        return false;

    bool used = UseItem(item, goTarget, nullptr, unitTarget);
    if (used)
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.globalCoolDown);

    return used;
}

bool UseRandomQuestItem::isUseful()
{
    return !botAI->HasActivePlayerMaster() && !bot->InBattleground() && !bot->HasUnitState(UNIT_STATE_IN_FLIGHT);
}

bool UseRandomQuestItem::isPossible() { return AI_VALUE2(uint32, "item count", "quest") > 0; }
