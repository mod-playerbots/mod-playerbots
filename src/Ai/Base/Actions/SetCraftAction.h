/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SETCRAFTACTION_H
#define _PLAYERBOT_SETCRAFTACTION_H

#include <map>
#include <set>
#include <string>

#include "Action.h"
#include "CraftValue.h"

class PlayerbotAI;
class Player;

struct SkillLineAbilityEntry;

class SetCraftAction : public Action
{
public:
    struct CraftReplyData
    {
        uint32 itemId = 0;
        uint32 spellId = 0;
        uint32 skillId = 0;
        uint32 skillValue = 0;
        uint32 requiredSkillValue = 0;
        std::map<uint32, uint32> required;
        std::map<uint32, uint32> available;
        std::map<uint32, uint32> missing;

        bool IsEmpty() const { return !itemId || !spellId || !skillId; }
        bool HasAllReagents() const { return missing.empty(); }
        uint32 GetTotalMissingCount() const;
    };

    SetCraftAction(PlayerbotAI* botAI) : Action(botAI, "craft") {}

    bool Execute(Event event) override;

    static uint32 GetCraftFee(CraftData& craftData);
    static bool ParseCraftRequest(std::string const& text,
                                  std::set<uint32>& itemIds);
    static bool BuildCraftReplyData(Player* bot, uint32 itemId,
                                    CraftReplyData& data);
    static std::string FormatCraftReply(PlayerbotAI* botAI,
                                        CraftReplyData const& data);

private:
    void TellCraft();
    static bool IsSupportedCraftSkill(uint32 skillId);
};

#endif
