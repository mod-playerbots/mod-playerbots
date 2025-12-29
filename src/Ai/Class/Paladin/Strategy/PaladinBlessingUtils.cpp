/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "PaladinBlessingUtils.h"

#include "AiFactory.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace ai::paladin
{
    std::string GetActualBlessingOfMight(Unit* target)
    {
        std::string result;
        Player* targetPlayer = target ? target->ToPlayer() : nullptr;
        if (!targetPlayer)
            result = "blessing of might";
        else
        {
            int specTab = AiFactory::GetPlayerSpecTab(targetPlayer);
            switch (targetPlayer->getClass())
            {
                case CLASS_MAGE:
                case CLASS_PRIEST:
                case CLASS_WARLOCK:
                    result = "blessing of wisdom";
                    break;
                case CLASS_SHAMAN:
                    if (specTab == SHAMAN_TAB_ELEMENTAL || specTab == SHAMAN_TAB_RESTORATION)
                        result = "blessing of wisdom";
                    break;
                case CLASS_DRUID:
                    if (specTab == DRUID_TAB_RESTORATION || specTab == DRUID_TAB_BALANCE)
                        result = "blessing of wisdom";
                    break;
                case CLASS_PALADIN:
                    if (specTab == PALADIN_TAB_HOLY)
                        result = "blessing of wisdom";
                    break;
                default:
                    break;
            }
            if (result.empty())
                result = "blessing of might";
        }
        LOG_DEBUG("playerbots", "[BlessingDecision] Might resolver -> target={} class={} specTab={} => {}",
                  (targetPlayer ? targetPlayer->GetName() : "non-player"),
                  (targetPlayer ? int(targetPlayer->getClass()) : -1),
                  (targetPlayer ? AiFactory::GetPlayerSpecTab(targetPlayer) : -1), result);
        return result;
    }

    std::string GetActualBlessingOfWisdom(Unit* target)
    {
        std::string result;
        Player* targetPlayer = target ? target->ToPlayer() : nullptr;
        if (!targetPlayer)
            result = "blessing of might";
        else
        {
            int specTab = AiFactory::GetPlayerSpecTab(targetPlayer);
            switch (targetPlayer->getClass())
            {
                case CLASS_WARRIOR:
                case CLASS_ROGUE:
                case CLASS_DEATH_KNIGHT:
                case CLASS_HUNTER:
                    result = "blessing of might";
                    break;
                case CLASS_SHAMAN:
                    if (specTab == SHAMAN_TAB_ENHANCEMENT)
                        result = "blessing of might";
                    break;
                case CLASS_DRUID:
                    if (specTab == DRUID_TAB_FERAL)
                        result = "blessing of might";
                    break;
                case CLASS_PALADIN:
                    if (specTab == PALADIN_TAB_PROTECTION || specTab == PALADIN_TAB_RETRIBUTION)
                        result = "blessing of might";
                    break;
                default:
                    break;
            }
            if (result.empty())
                result = "blessing of wisdom";
        }
        LOG_DEBUG("playerbots", "[BlessingDecision] Wisdom resolver -> target={} class={} specTab={} => {}",
                  (targetPlayer ? targetPlayer->GetName() : "non-player"),
                  (targetPlayer ? int(targetPlayer->getClass()) : -1),
                  (targetPlayer ? AiFactory::GetPlayerSpecTab(targetPlayer) : -1), result);
        return result;
    }

    std::string GetActualBlessingOfSanctuary(Unit* target, Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || !botAI)
            return "";

        Player* targetPlayer = target ? target->ToPlayer() : nullptr;
        if (!targetPlayer)
            return "";

        if (Unit* mt = botAI->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get())
        {
            if (mt == target)
                return "blessing of sanctuary";
        }

        if (targetPlayer->HasTankSpec())
        {
            LOG_DEBUG("playerbots", "[BlessingDecision] Sanctuary resolver -> target={} isTankSpec=1 => sanctuary",
                      targetPlayer->GetName());
            return "blessing of sanctuary";
        }

        LOG_DEBUG("playerbots", "[BlessingDecision] Sanctuary resolver -> target={} isTankSpec=0 => '' (no sanct)",
                  targetPlayer->GetName());

        return "";
    }
}  // namespace ai::paladin