/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SUGGESTWHATTODOACTION_H
#define PLAYERBOTS_SUGGESTWHATTODOACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class SuggestWhatToDoAction : public InventoryAction
{
public:
    SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name = "suggest what to do");

    bool Execute(Event event) override;
    bool isUseful() override;

protected:
    using Suggestion = std::function<void()>;
    std::vector<Suggestion> suggestions;
    void specificQuest();
    void grindReputation();
    void grindMaterials();
    void something();
    void toxicLinks();
    void somethingToxic();
    void thunderfury();

    std::vector<uint32> GetIncompletedQuests();

private:
    static std::map<std::string, uint8> factions;
    const int32_t _dbc_locale;
};

class SuggestTradeAction : public SuggestWhatToDoAction
{
public:
    SuggestTradeAction(PlayerbotAI* botAI);
    bool Execute(Event event) override;
};

class SuggestDungeonAction : public SuggestWhatToDoAction
{
public:
    SuggestDungeonAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
private:
    static std::map<std::string, uint8> instances;
};

#endif
