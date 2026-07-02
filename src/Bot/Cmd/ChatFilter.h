/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHATFILTER_H
#define PLAYERBOTS_CHATFILTER_H

#include <vector>

#include "Common.h"
#include "PlayerbotAIAware.h"

class PlayerbotAI;

class ChatFilter : public PlayerbotAIAware
{
public:
    ChatFilter(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) {}
    virtual ~ChatFilter() {}

    virtual std::string const Filter(std::string& message);
};

class CompositeChatFilter : public ChatFilter
{
public:
    CompositeChatFilter(PlayerbotAI* botAI);

    virtual ~CompositeChatFilter();
    std::string const Filter(std::string& message) override;

private:
    std::vector<ChatFilter*> filters;
};

#endif
