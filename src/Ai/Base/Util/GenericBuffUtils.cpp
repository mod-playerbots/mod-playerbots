/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "GenericBuffUtils.h"

#include "AiObjectContext.h"

#include "GameTime.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "SpellMgr.h"
#include "Unit.h"
#include "Value.h"

#include <map>

namespace ai::buff
{
    namespace
    {
        // Prevents bots from immediately casting already-present buffs upon logging in
        constexpr uint32 POST_LOGIN_BUFF_GRACE_MS = 5 * IN_MILLISECONDS;

        using MissingBuffNoticeKey = std::pair<uint32, std::string>;

        std::map<MissingBuffNoticeKey, uint32> lastMissingBuffNoticeTimes;

        bool IsWithinPostLoginBuffGrace(Player* player)
        {
            if (!player)
                return false;

            return getMSTimeDiff(
                player->GetInGameTime(), GameTime::GetGameTimeMS().count()) < POST_LOGIN_BUFF_GRACE_MS;
        }

        MissingBuffNoticeKey MakeMissingBuffNoticeKey(Player* bot, std::string const& groupName)
        {
            return std::make_pair(bot->GetGUID().GetCounter(), groupName);
        }

        void PruneExpiredNoticeTimes(
            std::map<MissingBuffNoticeKey, uint32>& lastNoticeTimes,
            uint32 cooldownMs,
            uint32 now)
        {
            if (!cooldownMs)
            {
                lastNoticeTimes.clear();
                return;
            }

            for (auto noticeIt = lastNoticeTimes.begin(); noticeIt != lastNoticeTimes.end();)
            {
                if (getMSTimeDiff(noticeIt->second, now) >= cooldownMs)
                    noticeIt = lastNoticeTimes.erase(noticeIt);
                else
                    ++noticeIt;
            }
        }
    }

    static bool HasEnoughSameMapMissingPlayersForGroupVariant(
        Player* bot, PlayerbotAI* botAI, std::string const& baseName,
        std::string const& groupName, uint32 requiredCount = 3)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        uint32 missingCount = 0;
        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsInWorld() || !member->IsAlive() ||
                member->GetMap() != bot->GetMap())
            {
                continue;
            }

            if (botAI->HasAura(baseName, member) || botAI->HasAura(groupName, member))
                continue;

            if (++missingCount >= requiredCount)
                return true;
        }

        return false;
    }

    static bool IsEligibleGroupForPartyBuffs(Group const* group)
    {
        if (!group)
            return false;

        switch (sPlayerbotAIConfig.autoPartyBuffs)
        {
            case AutoPartyBuffMode::RAID_ONLY:
                return group->isRaidGroup();
            case AutoPartyBuffMode::GROUP_OR_RAID:
                return true;
            case AutoPartyBuffMode::DISABLED:
                return false;
        }

        return false;
    }

    bool IsGroupVariantEnabled(Player* bot, std::string const& name)
    {
        if (!IsEligibleGroupForPartyBuffs(bot->GetGroup()))
            return false;

        return !GroupVariantFor(name).empty();
    }

    std::string MakeAuraQualifierForBuff(std::string const& name)
    {
        // Paladin
        if (name == "blessing of kings")        return "blessing of kings,greater blessing of kings";
        if (name == "blessing of might")        return "blessing of might,greater blessing of might";
        if (name == "blessing of wisdom")       return "blessing of wisdom,greater blessing of wisdom";
        if (name == "blessing of sanctuary")    return "blessing of sanctuary,greater blessing of sanctuary";
        // Druid
        if (name == "mark of the wild")         return "mark of the wild,gift of the wild";
        // Mage
        if (name == "arcane intellect")         return "arcane intellect,arcane brilliance";
        // Priest
        if (name == "power word: fortitude")    return "power word: fortitude,prayer of fortitude";
        if (name == "divine spirit")            return "divine spirit,prayer of spirit";
        if (name == "shadow protection")        return "shadow protection,prayer of shadow protection";

        return name;
    }

    std::string GroupVariantFor(std::string const& name)
    {
        // Druid
        if (name == "mark of the wild")         return "gift of the wild";
        // Mage
        if (name == "arcane intellect")         return "arcane brilliance";
        // Priest
        if (name == "power word: fortitude")    return "prayer of fortitude";
        if (name == "divine spirit")            return "prayer of spirit";
        if (name == "shadow protection")        return "prayer of shadow protection";

        // Paladin blessings are intentionally not included here because they are
        // coordinated by the auto greater blessing system instead.
        return std::string();
    }

    bool NeedsPostLoginBuffGrace(std::string const& name)
    {
        static char const* const trackedBuffs[] = {
            "mark of the wild",
            "arcane intellect",
            "power word: fortitude",
            "prayer of fortitude",
            "divine spirit",
            "prayer of spirit",
            "shadow protection",
            "prayer of shadow protection",
            "blessing of kings",
            "blessing of might",
            "blessing of wisdom",
            "blessing of sanctuary"
        };

        for (char const* trackedBuff : trackedBuffs)
        {
            if (name.find(trackedBuff) != std::string::npos)
                return true;
        }

        return false;
    }

    bool ShouldDeferPartyBuffEvaluationForRecentLogin(
        Player* bot, Unit* target, std::string const& spell)
    {
        if (!NeedsPostLoginBuffGrace(spell))
            return false;

        if (IsWithinPostLoginBuffGrace(bot))
            return true;

        Player* playerTarget = target ? target->ToPlayer() : nullptr;
        return IsWithinPostLoginBuffGrace(playerTarget);
    }

    bool ShouldDeferGreaterBlessingAssignmentForRecentLogin(Player* bot)
    {
        if (IsWithinPostLoginBuffGrace(bot))
            return true;

        Group* group = bot->GetGroup();
        if (!group)
            return false;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            if (!member || !member->IsInWorld())
                continue;

            if (IsWithinPostLoginBuffGrace(member))
                return true;
        }

        return false;
    }

    bool HasRequiredReagents(Player* bot, uint32 spellId)
    {
        if (!spellId)
            return false;

        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId))
        {
            for (int i = 0; i < MAX_SPELL_REAGENTS; ++i)
            {
                if (info->Reagent[i] > 0)
                {
                    uint32 const itemId = info->Reagent[i];
                    int32 const need = info->ReagentCount[i];
                    if ((int32)bot->GetItemCount(itemId, false) < need)
                        return false;
                }
            }
            return true;
        }
        return false;
    }

    void ClearMissingBuffReagentNotice(Player* bot, std::string const& groupName)
    {
        if (!bot || groupName.empty())
            return;

        lastMissingBuffNoticeTimes.erase(MakeMissingBuffNoticeKey(bot, groupName));
    }

    bool TryAnnounceMissingBuffReagents(
        PlayerbotAI* botAI, std::string const& baseName, std::string const& groupName)
    {
        if (!sPlayerbotAIConfig.tellWhenMissingBuffReagents)
            return false;

        Player* bot = botAI->GetBot();
        if (bot->InBattleground())
            return false;

        auto const cooldownMs = sPlayerbotAIConfig.missingBuffReagentMessageCooldown * IN_MILLISECONDS;
        auto const now = GameTime::GetGameTimeMS().count();
        auto const noticeKey = MakeMissingBuffNoticeKey(bot, groupName);

        PruneExpiredNoticeTimes(lastMissingBuffNoticeTimes, cooldownMs, now);

        auto const noticeIt = lastMissingBuffNoticeTimes.find(noticeKey);
        if (cooldownMs && noticeIt != lastMissingBuffNoticeTimes.end() &&
            getMSTimeDiff(noticeIt->second, now) < cooldownMs)
        {
            return false;
        }

        std::map<std::string, std::string> placeholders = {
            {"%base_spell", baseName},
            {"%group_spell", groupName}
        };

        std::string const message = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "missing_group_buff_reagent",
            "I am out of reagents for %group_spell and am casting %base_spell instead.",
            placeholders);

        Group* group = bot->GetGroup();
        const bool announced =
            group->isRaidGroup() ? botAI->SayToRaid(message) : botAI->SayToParty(message);

        if (announced)
            lastMissingBuffNoticeTimes[noticeKey] = now;

        return announced;
    }

    std::string UpgradeToGroupIfAppropriate(
        Player* bot,
        PlayerbotAI* botAI,
        std::string const& baseName,
        std::string* outMissingReagentGroupName)
    {
        if (outMissingReagentGroupName)
            outMissingReagentGroupName->clear();

        if (!IsGroupVariantEnabled(bot, baseName))
            return baseName;

        std::string const groupName = GroupVariantFor(baseName);
        if (groupName.empty())
            return baseName;

        // Prefer singles until at least three living, in-world group members on the bot's map
        // are missing both the single-target buff and its group variant.
        if (!HasEnoughSameMapMissingPlayersForGroupVariant(bot, botAI, baseName, groupName))
            return baseName;

        uint32 const groupSpellId = botAI->GetAiObjectContext()
            ->GetValue<uint32>("spell id", groupName)->Get();

        if (groupSpellId && HasRequiredReagents(bot, groupSpellId))
        {
            ClearMissingBuffReagentNotice(bot, groupName);
            return groupName;
        }

        if (groupSpellId && outMissingReagentGroupName)
            *outMissingReagentGroupName = groupName;

        return baseName;
    }
}

namespace ai::spell
{
    bool HasSpellOrCategoryCooldown(Player* bot, uint32 spellId)
    {
        if (bot->HasSpellCooldown(spellId))
            return true;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return false;

        uint32 category = spellInfo->GetCategory();
        if (!category)
            return false;

        for (auto const& [cooldownSpellId, cooldown] : bot->GetSpellCooldownMap())
        {
            if (cooldown.category == category && bot->GetSpellCooldownDelay(cooldownSpellId) > 0)
                return true;
        }

        return false;
    }
}
