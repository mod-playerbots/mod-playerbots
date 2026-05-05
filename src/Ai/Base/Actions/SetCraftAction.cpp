/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetCraftAction.h"

#include <algorithm>
#include <cctype>

#include "ChatHelper.h"
#include "CraftValue.h"
#include "Event.h"
#include "PlayerbotSpellRepository.h"
#include "Playerbots.h"

namespace
{
std::string ToLower(std::string const& text)
{
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower;
}

bool IsBetterCraftReplyData(SetCraftAction::CraftReplyData const& candidate,
                            SetCraftAction::CraftReplyData const& current)
{
    if (current.IsEmpty())
        return true;

    if (candidate.HasAllReagents() != current.HasAllReagents())
        return candidate.HasAllReagents();

    if (candidate.GetTotalMissingCount() != current.GetTotalMissingCount())
        return candidate.GetTotalMissingCount() < current.GetTotalMissingCount();

    if (candidate.skillValue != current.skillValue)
        return candidate.skillValue > current.skillValue;

    if (candidate.requiredSkillValue != current.requiredSkillValue)
        return candidate.requiredSkillValue < current.requiredSkillValue;

    return candidate.spellId < current.spellId;
}

void AppendFormattedItems(PlayerbotAI* botAI, std::ostringstream& out,
                          std::map<uint32, uint32> const& items)
{
    bool first = true;
    for (auto const& [itemId, count] : items)
    {
        if (!first)
            out << ", ";

        first = false;

        if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
            out << botAI->GetChatHelper()->FormatItem(proto, count);
        else
            out << "item " << itemId << " x" << count;
    }
}
} // namespace

uint32 SetCraftAction::CraftReplyData::GetTotalMissingCount() const
{
    uint32 totalMissingCount = 0;
    for (auto const& [itemId, count] : missing)
        totalMissingCount += count;

    return totalMissingCount;
}

bool SetCraftAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const link = event.getParam();

    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (link == "reset")
    {
        data.Reset();
        botAI->TellMaster("I will not craft anything");
        return true;
    }

    if (link == "?")
    {
        TellCraft();
        return true;
    }

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
    {
        botAI->TellMaster("Usage: 'craft [itemId]' or 'craft reset'");
        return false;
    }

    uint32 itemId = *itemIds.begin();
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    data.required.clear();
    data.obtained.clear();

    CraftReplyData craftReplyData;
    if (!BuildCraftReplyData(bot, itemId, craftReplyData))
    {
        botAI->TellMaster("I cannot craft this");
        return false;
    }

    data.itemId = itemId;
    data.required = craftReplyData.required;
    for (auto const& [reagentId, count] : craftReplyData.required)
        data.obtained[reagentId] = 0;

    TellCraft();
    return true;
}

bool SetCraftAction::ParseCraftRequest(std::string const& text,
                                       std::set<uint32>& itemIds)
{
    itemIds = ChatHelper::ExtractAllItemIds(text);
    if (itemIds.empty())
        return false;

    std::string const lower = ToLower(text);
    return lower.find("craft") != std::string::npos ||
           lower.find("make") != std::string::npos;
}

bool SetCraftAction::BuildCraftReplyData(Player* bot, uint32 itemId,
                                         CraftReplyData& data)
{
    if (!bot || !itemId)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    CraftReplyData bestCandidate;

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin();
         itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->State == PLAYERSPELL_REMOVED || !itr->second->Active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            continue;

        SkillLineAbilityEntry const* skillLine =
            PlayerbotSpellRepository::Instance().GetSkillLine(spellId);
        if (!skillLine || !skillLine->SkillLine ||
            !IsSupportedCraftSkill(skillLine->SkillLine))
            continue;

        if (!botAI->HasSkill(static_cast<SkillType>(skillLine->SkillLine)))
            continue;

        uint32 skillValue = bot->GetSkillValue(skillLine->SkillLine);
        if (skillValue < skillLine->MinSkillLineRank)
            continue;

        bool craftsRequestedItem = false;
        for (uint8 i = 0; i < 3; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM &&
                itemId == spellInfo->Effects[i].ItemType)
            {
                craftsRequestedItem = true;
                break;
            }
        }

        if (!craftsRequestedItem)
            continue;

        CraftReplyData candidate;
        candidate.itemId = itemId;
        candidate.spellId = spellId;
        candidate.skillId = skillLine->SkillLine;
        candidate.skillValue = skillValue;
        candidate.requiredSkillValue = skillLine->MinSkillLineRank;

        for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
        {
            if (spellInfo->Reagent[x] <= 0)
                continue;

            uint32 reagentId = spellInfo->Reagent[x];
            uint32 reagentCount = spellInfo->ReagentCount[x];
            if (!reagentId || !reagentCount)
                continue;

            candidate.required[reagentId] = reagentCount;

            uint32 availableCount =
                std::min<uint32>(reagentCount, bot->GetItemCount(reagentId, false));
            if (availableCount)
                candidate.available[reagentId] = availableCount;

            if (availableCount < reagentCount)
                candidate.missing[reagentId] = reagentCount - availableCount;
        }

        if (IsBetterCraftReplyData(candidate, bestCandidate))
            bestCandidate = candidate;
    }

    if (bestCandidate.IsEmpty())
        return false;

    data = bestCandidate;
    return true;
}

std::string SetCraftAction::FormatCraftReply(PlayerbotAI* botAI,
                                             CraftReplyData const& data)
{
    if (!botAI || data.IsEmpty())
        return "";

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return "";

    CraftData craftData;
    craftData.itemId = data.itemId;

    std::ostringstream out;
    out << "I can craft " << botAI->GetChatHelper()->FormatItem(proto)
        << " with " << ChatHelper::FormatSkill(data.skillId) << " ("
        << data.skillValue << ") for "
        << ChatHelper::formatMoney(GetCraftFee(craftData))
        << ". ";

    if (data.HasAllReagents())
    {
        out << "I already have all required materials.";
    }
    else
    {
        if (!data.available.empty())
        {
            out << "I have: ";
            AppendFormattedItems(botAI, out, data.available);
            out << ". ";
        }

        out << "I still need: ";
        AppendFormattedItems(botAI, out, data.missing);
        out << ".";
    }

    std::string message = out.str();
    if (message.size() > 255)
        message = message.substr(0, 252) + "...";

    return message;
}

void SetCraftAction::TellCraft()
{
    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (data.IsEmpty())
    {
        botAI->TellMaster("I will not craft anything");
        return;
    }

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return;

    std::ostringstream out;
    out << "I will craft " << chat->FormatItem(proto) << " using reagents: ";

    bool first = true;
    for (std::map<uint32, uint32>::iterator i = data.required.begin(); i != data.required.end(); ++i)
    {
        uint32 item = i->first;
        uint32 required = i->second;

        if (ItemTemplate const* reagent = sObjectMgr->GetItemTemplate(item))
        {
            if (first)
                first = false;

            else
                out << ", ";

            out << chat->FormatItem(reagent, required);

            uint32 given = data.obtained[item];
            if (given)
                out << "|cffffff00(x" << given << " given)|r ";
        }
    }

    out << " (craft fee: " << chat->formatMoney(GetCraftFee(data)) << ")";
    botAI->TellMaster(out.str());
}

uint32 SetCraftAction::GetCraftFee(CraftData& data)
{
    if (data.IsEmpty())
        return 0;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(data.itemId);
    if (!proto)
        return 0;

    uint32 level = std::max(proto->ItemLevel, proto->RequiredLevel);
    return level * level / 40;
}

bool SetCraftAction::IsSupportedCraftSkill(uint32 skillId)
{
    switch (skillId)
    {
        case SKILL_ALCHEMY:
        case SKILL_BLACKSMITHING:
        case SKILL_ENCHANTING:
        case SKILL_ENGINEERING:
        case SKILL_INSCRIPTION:
        case SKILL_JEWELCRAFTING:
        case SKILL_LEATHERWORKING:
        case SKILL_TAILORING:
            return true;
        default:
            return false;
    }
}
