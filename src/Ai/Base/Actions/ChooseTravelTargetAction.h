/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CHOOSETRAVELTARGETACTION_H
#define PLAYERBOTS_CHOOSETRAVELTARGETACTION_H

#include "MovementActions.h"
#include "TravelMgr.h"

class Quest;
class PlayerbotAI;
class Unit;

struct QuestStatusData;

class ChooseTravelTargetAction : public MovementAction
{
public:
    ChooseTravelTargetAction(PlayerbotAI* botAI, std::string const name = "choose travel target")
        : MovementAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;

    static TravelDestination* FindDestination(Player* bot, std::string const name, bool zones = true, bool npcs = true, bool quests = true, bool mobs = true, bool bosses = true);

protected:
    void getNewTarget(TravelTarget* newTarget, TravelTarget* oldTarget);
    void setNewTarget(TravelTarget* newTarget, TravelTarget* oldTarget);
    void ReportTravelTarget(TravelTarget* newTarget, TravelTarget* oldTarget);

    bool getBestDestination(std::vector<TravelDestination*>* activeDestinations, std::vector<WorldPosition*>* activePoints);
    bool SetGroupTarget(TravelTarget* target);
    bool SetCurrentTarget(TravelTarget* target, TravelTarget* oldTarget);
    bool SetQuestTarget(TravelTarget* target, bool onlyCompleted = false, bool newQuests = true, bool activeQuests = true, bool completedQuests = true);
    bool SetNewQuestTarget(TravelTarget* target);
    bool SetRpgTarget(TravelTarget* target);
    bool SetGrindTarget(TravelTarget* target);
    bool SetBossTarget(TravelTarget* target);
    bool SetExploreTarget(TravelTarget* target);
    bool SetNpcFlagTarget(TravelTarget* target, std::vector<NPCFlags> flags, std::string const name = "", std::vector<uint32> items = { });
    bool SetNullTarget(TravelTarget* target);

private:
    virtual bool needForQuest(Unit* target);
    virtual bool needItemForQuest(uint32 itemId, Quest const* questTemplate, QuestStatusData const* questStatus);
};

#endif
