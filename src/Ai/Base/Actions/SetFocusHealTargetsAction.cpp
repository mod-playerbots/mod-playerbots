/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "SetFocusHealTargetsAction.h"

#include "ObjectAccessor.h"
#include "Playerbots.h"

static std::string LowercaseString(std::string const& str)
{
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

static Player* FindGroupPlayerByName(Player* player, std::string const& playerName)
{
    if (!player)
        return nullptr;

    Group* group = player->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (member && LowercaseString(member->GetName()) == playerName)
            return member;
    }

    return nullptr;
}

bool SetFocusHealTargetsAction::Execute(Event event)
{
    if (!botAI->IsHeal(bot) && !botAI->HasStrategy("offheal", BOT_STATE_COMBAT))
    {
        botAI->TellMasterNoFacing("I'm not a healer or offhealer (please change my strats to heal or offheal)");
        return false;
    }

    std::string const param = LowercaseString(event.getParam());
    if (param.empty())
    {
        botAI->TellMasterNoFacing("Please provide one or more player names");
        return false;
    }

    std::list<ObjectGuid> focusHealTargets =
        AI_VALUE(std::list<ObjectGuid>, "focus heal targets");

    // Query current focus targets
    if (param.find('?') != std::string::npos)
    {
        std::stringstream ss;
        if (focusHealTargets.empty())
        {
            ss << "I don't have any focus heal targets";
        }
        else
        {
            ss << "My focus heal targets are ";
            for (auto it = focusHealTargets.begin(); it != focusHealTargets.end(); ++it)
            {
                Unit* target = botAI->GetUnit(*it);
                if (target)
                {
                    if (it != focusHealTargets.begin())
                        ss << ", ";
                    ss << target->GetName();
                }
            }
        }

        botAI->TellMasterNoFacing(ss.str());
        return true;
    }

    // Clear all targets
    if (param == "none" || param == "unset" || param == "clear")
    {
        focusHealTargets.clear();
        SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
        botAI->TellMasterNoFacing("Removed focus heal targets");
        return true;
    }

    // Parse multiple targets separated by commas
    std::vector<std::string> targetNames;
    if (param.find(',') != std::string::npos)
    {
        std::string targetName;
        std::stringstream ss(param);
        while (std::getline(ss, targetName, ','))
            targetNames.push_back(targetName);
    }
    else
    {
        targetNames.push_back(param);
    }

    if (targetNames.empty())
    {
        botAI->TellMasterNoFacing("Please provide one or more player names");
        return false;
    }

    if (!bot->GetGroup())
    {
        botAI->TellMasterNoFacing("I'm not in a group");
        return false;
    }

    for (std::string const& targetName : targetNames)
    {
        bool const add = targetName.find("+") != std::string::npos;
        bool const remove = targetName.find("-") != std::string::npos;
        if (!add && !remove)
        {
            botAI->TellMasterNoFacing("Please specify a + for add or - to remove a target");
            continue;
        }

        std::string const playerName = targetName.substr(1);
        Player* target = FindGroupPlayerByName(bot, playerName);
        if (!target)
        {
            std::ostringstream msg;
            msg << "I'm not in a group with " << playerName;
            botAI->TellMasterNoFacing(msg.str());
            continue;
        }

        ObjectGuid const& targetGuid = target->GetGUID();
        if (add)
        {
            if (std::find(focusHealTargets.begin(), focusHealTargets.end(), targetGuid) ==
                focusHealTargets.end())
            {
                focusHealTargets.push_back(targetGuid);
            }

            std::ostringstream msg;
            msg << "Added " << playerName << " to focus heal targets";
            botAI->TellMasterNoFacing(msg.str());
        }
        else
        {
            focusHealTargets.remove(targetGuid);
            std::ostringstream msg;
            msg << "Removed " << playerName << " from focus heal targets";
            botAI->TellMasterNoFacing(msg.str());
        }
    }

    SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);

    if (focusHealTargets.empty())
        botAI->ChangeStrategy("-focus heal targets", BOT_STATE_COMBAT);
    else
        botAI->ChangeStrategy("+focus heal targets", BOT_STATE_COMBAT);

    return true;
}
