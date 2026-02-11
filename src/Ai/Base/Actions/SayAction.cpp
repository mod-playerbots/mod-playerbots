/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "AiFactory.h"
#include "SayAction.h"

#include <regex>
#include <string>

#include "Event.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

static const std::unordered_set<std::string> noReplyMsgs = {
    "join",
    "leave",
    "follow",
    "attack",
    "pull",
    "flee",
    "reset",
    "reset ai",
    "all ?",
    "talents",
    "talents list",
    "talents auto",
    "talk",
    "stay",
    "stats",
    "who",
    "items",
    "leave",
    "join",
    "repair",
    "summon",
    "nc ?",
    "co ?",
    "de ?",
    "dead ?",
    "follow",
    "los",
    "guard",
    "do accept invitation",
    "stats",
    "react ?",
    "reset strats",
    "home",
};
static const std::unordered_set<std::string> noReplyMsgParts = {
    "+", "-", "@", "follow target", "focus heal", "cast ", "accept [", "e [", "destroy [", "go zone"};
static const std::unordered_set<std::string> noReplyMsgStarts = {"e ", "accept ", "cast ", "destroy "};

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified() {}

bool SayAction::Execute(Event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target)
        placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        {
            switch (pItem->GetTemplate()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    placeholders["<ammo>"] = "bullets";
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    placeholders["<ammo>"] = "arrows";
                    break;
            }
        }
    }

    if (bot->GetMap())
    {
        if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetMap()->GetZoneId(bot->GetPhaseMask(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
            placeholders["<subzone>"] = zone->area_name[sWorld->GetDefaultDbcLocale()];
    }

    // set delay before next say
    uint32 nextTime = time(nullptr) + urand(1, 30);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (auto& member : members)
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                memberAi->GetAiObjectContext()
                    ->GetValue<time_t>("last said", qualifier)
                    ->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!PlayerbotTextMgr::instance().GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}

bool SayAction::isUseful()
{
    if (!botAI->allowActivity())
        return false;

    if (botAI->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT))
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}

// @TODO: This is not a real action.
// This is actually a hack that is called directly during each update tick.
// This should be moved to a chat service attached to a bot.
void ChatReplyAction::ChatReplyDo(Player* bot, const uint32 type, uint32 guid1, std::string& msg, const std::string& chanName, std::string& name)
{
    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.contains(msg))
    {
        return;
    }

    // second one is for partial matches like + or - where we change strats
    for (const std::string& noReplyMsg : noReplyMsgParts)
    {
        if (msg.find(noReplyMsg) != std::string::npos)
        {
            return;
        }
    }

    for (const std::string& noReplyMsg : noReplyMsgStarts)
    {
        // Check if the start matches
        if (msg.starts_with(noReplyMsg))
        {
            return;
        }
    }

    PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(bot);

    if (botAI == nullptr)
    {
        return;
    }

    const ChatChannelSource chatChannelSource = botAI->GetChatChannelSource(bot, type, chanName);

    if (
        (
            msg.starts_with("LFG")
            || msg.starts_with("LFM")
        )
        && ChatReplyAction::HandleLFGQuestsReply(*bot, chatChannelSource, msg, name)
    )
    {
        return;
    }

    if (
        msg.starts_with("WTB")
        && ChatReplyAction::HandleWTBItemsReply(*bot, chatChannelSource, msg, name)
    )
    {
        return;
    }

    const ChatHelper& chatHelper = botAI->GetChatHelper();

    const std::set<uint32_t> itemIds = chatHelper.ExtractAllItemIds(msg);
    const std::set<uint32_t> questIds = chatHelper.ExtractAllQuestIds(msg);

    //toxic links
    if (
        msg.starts_with(PlayerbotAIConfig::instance().toxicLinksPrefix)
        && (
            itemIds.empty() == false
            || questIds.empty() == false
        )
    )
    {
        ChatReplyAction::HandleToxicLinksReply(*bot, chatChannelSource, msg, name);

        return;
    }

    //thunderfury
    if (itemIds.count(19019) != 0)
    {
        ChatReplyAction::HandleThunderfuryReply(bot, chatChannelSource, msg, name);

        return;
    }

    std::string messageRepy = ChatReplyAction::GenerateReplyMessage(bot, msg, guid1, name);

    ChatReplyAction::SendGeneralResponse(bot, chatChannelSource, messageRepy, name);
}

bool ChatReplyAction::HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource, std::string&, std::string&)
{
    std::map<std::string, std::string> placeholders;
    const auto thunderfury = sObjectMgr->GetItemTemplate(19019);
    placeholders["%thunderfury_link"] = GET_PLAYERBOT_AI(bot)->GetChatHelper().FormatItem(thunderfury);

    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("thunderfury_spam", placeholders);

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
            break;
        }
        default:
            break;
    }

    GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
    return true;
}

bool ChatReplyAction::HandleToxicLinksReply(Player& bot, ChatChannelSource chatChannelSource, const std::string&, const std::string&)
{
    PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(&bot);

    if (botAI == nullptr)
    {
        return false;
    }

    const ChatHelper& chatHelper = botAI->GetChatHelper();

    std::vector<uint32> incompleteQuests{};

    for (uint16_t slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        const uint32_t questId = bot.GetQuestSlotQuestId(slot);

        if (questId == 0)
        {
            continue;
        }

        const QuestStatus status = bot.GetQuestStatus(questId);

        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
        {
            incompleteQuests.push_back(questId);
        }
    }

    //items
    std::vector<Item*> botItems = botAI->GetInventoryAndEquippedItems();

    std::map<std::string, std::string> placeholders{
        { "%random_inventory_item_link", PlayerbotTextMgr::instance().GetBotText("string_empty_link") },
        { "%prefix", PlayerbotAIConfig::instance().toxicLinksPrefix },
        { "%my_role", ChatHelper::FormatClass(&bot, AiFactory::GetPlayerSpecTab(&bot)) },
        { "%area_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%zone_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%my_class", chatHelper.FormatClass(bot.getClass()) },
        { "%my_race", chatHelper.FormatRace(bot.getRace()) },
        { "%my_level", std::to_string(bot.GetLevel()) }
    };

    if (!botItems.empty())
    {
        placeholders.at("%random_inventory_item_link") = chatHelper.FormatItem(botItems[rand() % botItems.size()]->GetTemplate());
    }

    placeholders.at("%random_taken_quest_or_item_link") = placeholders.at("%random_inventory_item_link");

    if (!incompleteQuests.empty())
    {
        const Quest* const quest = sObjectMgr->GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);

        placeholders.at("%random_taken_quest_or_item_link") = chatHelper.FormatQuest(quest);
    }

    const AreaTableEntry* const current_area = botAI->GetCurrentArea();

    if (current_area != nullptr)
    {
        placeholders.at("%area_name") = botAI->GetLocalizedAreaName(current_area);
    }

    const AreaTableEntry* const current_zone = botAI->GetCurrentZone();

    if (current_zone != nullptr)
    {
        placeholders.at("%zone_name") = botAI->GetLocalizedAreaName(current_zone);
    }

    const std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders);

    if (chatChannelSource == ChatChannelSource::SRC_WORLD)
    {
        botAI->SayToWorld(responseMessage);
    }

    if (chatChannelSource == ChatChannelSource::SRC_GENERAL)
    {
        botAI->SayToChannel(responseMessage, ChatChannelId::GENERAL);
    }

    if (chatChannelSource == ChatChannelSource::SRC_GUILD)
    {
        botAI->SayToGuild(responseMessage);
    }

    const time_t now = std::time(nullptr);
    const uint8_t randomDelay = urand(5, 60);
    Value<time_t>* const lastSaidValue = botAI->GetAiObjectContext()->GetValue<time_t>("last said", "chat");

    if (lastSaidValue == nullptr)
    {
        return true;
    }

    lastSaidValue->Set(now + randomDelay);

    return true;
}

bool ChatReplyAction::HandleWTBItemsReply(Player& bot, ChatChannelSource chatChannelSource, const std::string& msg, const std::string& name)
{
    PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(&bot);

    if (botAI == nullptr)
    {
        return false;
    }

    const ChatHelper& chatHelper = botAI->GetChatHelper();

    const std::set<uint32_t> messageItemIds = chatHelper.ExtractAllItemIds(msg);

    if (messageItemIds.empty())
    {
        return false;
    }

    std::set<uint32_t> matchingItemIds;

    for (const uint32_t messageItemId : messageItemIds)
    {
        if (botAI->HasItemInInventory(messageItemId))
        {
            matchingItemIds.insert(messageItemId);
        }
    }

    if (matchingItemIds.empty())
    {
        return true;
    }

    std::map<std::string, std::string> placeholders{
        { "%other_name", name },
        { "%area_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%zone_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%my_class", chatHelper.FormatClass(bot.getClass()) },
        { "%my_race", chatHelper.FormatRace(bot.getRace()) },
        { "%my_level", std::to_string(bot.GetLevel()) },
        { "%my_role", ChatHelper::FormatClass(&bot, AiFactory::GetPlayerSpecTab(&bot)) },
        { "%formatted_item_links", "" }
    };

    const AreaTableEntry* const current_area = botAI->GetCurrentArea();

    if (current_area != nullptr)
    {
        placeholders.at("%area_name") = botAI->GetLocalizedAreaName(current_area);
    }

    const AreaTableEntry* const current_zone = botAI->GetCurrentZone();

    if (current_zone != nullptr)
    {
        placeholders.at("%zone_name") = botAI->GetLocalizedAreaName(current_zone);
    }

    std::string& formattedLinks = placeholders.at("%formatted_item_links");

    for (const uint32_t matchingItemId : matchingItemIds)
    {
        const ItemTemplate* const proto = ObjectMgr::instance()->GetItemTemplate(matchingItemId);

        if (proto == nullptr)
        {
            continue;
        }

        formattedLinks += chatHelper.FormatItem(proto, botAI->GetInventoryItemsCountWithId(matchingItemId));
        formattedLinks += " ";
    }

    const time_t now = std::time(nullptr);
    const uint8_t randomDelay = urand(5, 60);
    Value<time_t>* const lastSaidValue = botAI->GetAiObjectContext()->GetValue<time_t>("last said", "chat");

    if (lastSaidValue == nullptr)
    {
        return true;
    }

    if (
        chatChannelSource != ChatChannelSource::SRC_WORLD
        && chatChannelSource != ChatChannelSource::SRC_GENERAL
        && chatChannelSource != ChatChannelSource::SRC_TRADE
    )
    {
        lastSaidValue->Set(now + randomDelay);

        return true;
    }

    const std::string whisperMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_whisper", placeholders);

    const bool sayToChannel = urand(0, 1) == 1;

    if (!sayToChannel)
    {
        botAI->Whisper(whisperMessage, name);
        lastSaidValue->Set(now + randomDelay);

        return true;
    }

    const std::string channelMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_channel", placeholders);

    if (chatChannelSource == ChatChannelSource::SRC_WORLD)
    {
        botAI->SayToWorld(channelMessage);
    }

    if (chatChannelSource == ChatChannelSource::SRC_GENERAL)
    {
        botAI->SayToChannel(channelMessage, ChatChannelId::GENERAL);
    }

    if (chatChannelSource == ChatChannelSource::SRC_TRADE)
    {
        botAI->SayToChannel(channelMessage, ChatChannelId::TRADE);
    }

    lastSaidValue->Set(now + randomDelay);

    return true;
}

// @TODO: This should be move to a dedicated chat service attached to the bot, not a hacky static method in an action.
/**
 * Attempts to respond to LFG/LFM quest messages by matching linked quests
 * against the bot's current quest log.
 *
 * Parses quest links in the incoming message, intersects them with the bot's
 * active quests, and, if any match, replies either in the source channel
 * (World/General) or via whisper depending on the channel and RNG rules.
 * For LookingForGroup it only whispers. When a response is sent, updates the
 * bot's "last said" chat timer to throttle further replies.
 *
 * @param bot The bot player instance to evaluate and respond as.
 * @param chatChannelSource The channel source of the incoming message.
 * @param msg The incoming chat message text.
 * @param name The sender's character name (used for whispers/placeholders).
 * @return true if the message was handled (including when no quest matched),
 *         false if it could not be processed (e.g., no AI, no quests parsed).
 */
bool ChatReplyAction::HandleLFGQuestsReply(Player& bot, ChatChannelSource chatChannelSource, const std::string& msg, const std::string& name)
{
    PlayerbotAI* const botAI = PlayerbotsMgr::instance().GetPlayerbotAI(&bot);

    if (botAI == nullptr)
    {
        return false;
    }

    const ChatHelper& chatHelper = botAI->GetChatHelper();
    const std::set<uint32_t> messageQuestIds = chatHelper.ExtractAllQuestIds(msg);

    if (messageQuestIds.empty())
    {
        return false;
    }

    const std::set<uint32_t> botQuestIds = botAI->GetAllCurrentQuestIds();
    std::set<uint32_t> matchingQuestIds;

    for (const uint32_t botQuestId : botQuestIds)
    {
        if (messageQuestIds.count(botQuestId) != 0)
        {
            matchingQuestIds.insert(botQuestId);
        }
    }

    if (matchingQuestIds.empty())
    {
        return true;
    }

    const AreaTableEntry* const current_area = botAI->GetCurrentArea();
    const AreaTableEntry* const current_zone = botAI->GetCurrentZone();

    std::map<std::string, std::string> placeholders{
        { "%other_name", name },
        { "%quest_links", "" },
        { "%area_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%zone_name", PlayerbotTextMgr::instance().GetBotText("string_unknown_area") },
        { "%my_class", chatHelper.FormatClass(bot.getClass()) },
        { "%my_race", chatHelper.FormatRace(bot.getRace()) },
        { "%my_level", std::to_string(bot.GetLevel()) },
        { "%my_role", ChatHelper::FormatClass(&bot, AiFactory::GetPlayerSpecTab(&bot)) },
    };

    if (current_area != nullptr)
    {
        placeholders.at("%area_name") = botAI->GetLocalizedAreaName(current_area);
    }

    if (current_zone != nullptr)
    {
        placeholders.at("%zone_name") = botAI->GetLocalizedAreaName(current_zone);
    }

    std::string& formattedQuestLinks = placeholders.at("%quest_links");

    for (const uint32_t matchingQuestId : matchingQuestIds)
    {
        Quest const* quest = ObjectMgr::instance()->GetQuestTemplate(matchingQuestId);

        if (quest == nullptr)
        {
            continue;
        }

        formattedQuestLinks += chatHelper.FormatQuest(quest);
    }

    const time_t now = std::time(nullptr);
    const uint8_t randomDelay = urand(5, 60);
    Value<time_t>* const lastSaidValue = botAI->GetAiObjectContext()->GetValue<time_t>("last said", "chat");

    if (lastSaidValue == nullptr)
    {
        return true;
    }

    if (
        chatChannelSource != ChatChannelSource::SRC_WORLD
        && chatChannelSource != ChatChannelSource::SRC_GENERAL
        && chatChannelSource != ChatChannelSource::SRC_LOOKING_FOR_GROUP
    )
    {
        lastSaidValue->Set(now + randomDelay);

        return true;
    }

    const std::string whisperResponse = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_whisper", placeholders);
    const bool sayToChannel = urand(0, 1) == 1;

    if (!sayToChannel || chatChannelSource == ChatChannelSource::SRC_LOOKING_FOR_GROUP)
    {
        botAI->Whisper(whisperResponse, name);
        lastSaidValue->Set(now + randomDelay);

        return true;
    }

    const std::string channelResponse = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_channel", placeholders);

    if (chatChannelSource == ChatChannelSource::SRC_WORLD)
    {
        botAI->SayToWorld(channelResponse);
    }

    if (chatChannelSource == ChatChannelSource::SRC_GENERAL)
    {
        botAI->SayToChannel(channelResponse, ChatChannelId::GENERAL);
    }

    lastSaidValue->Set(now + randomDelay);

    return true;
}

bool ChatReplyAction::SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string& responseMessage, std::string& name)
{
    // send responds
    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            //may reply to the same channel or whisper
            GET_PLAYERBOT_AI(bot)->SayToWorld(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            //may reply to the same channel 80% or whisper
            if (urand(0, 100) < 80)
                GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::GENERAL);
            else
                GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_TRADE:
        {
            //do not reply to the chat
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOCAL_DEFENSE:
        {
            //may reply to the same channel or whisper
            GET_PLAYERBOT_AI(bot)->SayToChannel(responseMessage, ChatChannelId::LOCAL_DEFENSE);
            break;
        }
        case ChatChannelSource::SRC_WORLD_DEFENSE:
        {
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_GUILD_RECRUITMENT:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_WHISPER:
        {
            GET_PLAYERBOT_AI(bot)->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_SAY:
        {
            GET_PLAYERBOT_AI(bot)->Say(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_YELL:
        {
            GET_PLAYERBOT_AI(bot)->Yell(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            GET_PLAYERBOT_AI(bot)->SayToGuild(responseMessage);
            break;
        }
        default:
            break;
    }
    GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));

    return true;
}

std::string ChatReplyAction::GenerateReplyMessage(Player* bot, std::string& incomingMessage, uint32& guid1, std::string& name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand

    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(incomingMessage);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (incomingMessage.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = ObjectAccessor::FindPlayer(ObjectGuid(HighGuid::Player, guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2;  // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3;  // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1;  // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (incomingMessage.find(bot->GetName()) == std::string::npos)
                {
                    continue;  // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found)
    {
        // Name Responds
        if (incomingMessage.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else  // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // load text if needed
    if (respondsText.empty())
    {
        respondsText = PlayerbotTextMgr::instance().GetBotText(replyType, name);
    }

    if (respondsText.size() > 255)
    {
        respondsText.resize(255);
    }

    return respondsText;
}
