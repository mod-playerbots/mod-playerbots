/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSSIBLERPGTARGETSVALUE_H
#define PLAYERBOTS_POSSIBLERPGTARGETSVALUE_H

#include "NearestGameObjects.h"
#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"
#include "SharedDefines.h"

class PlayerbotAI;

class PossibleRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleRpgTargetsValue(PlayerbotAI* botAI, float range = 70.0f);

    static const std::vector<uint32> allowedNpcFlags;

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

class PossibleNewRpgTargetsValue : public NearestUnitsValue
{
public:
    PossibleNewRpgTargetsValue(PlayerbotAI* botAI, float range = 150.0f);

    static const std::vector<uint32> allowedNpcFlags;
    GuidVector Calculate() override;
protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
private:
    float defaultRange;
};

class PossibleNewRpgGameObjectsValue : public ObjectGuidListCalculatedValue
{
public:
    PossibleNewRpgGameObjectsValue(PlayerbotAI* botAI, float range = 150.0f, bool ignoreLos = true)
        : ObjectGuidListCalculatedValue(botAI, "possible new rpg game objects"), range(range), ignoreLos(ignoreLos)
    {
    }

    static const std::vector<GameobjectTypes> allowedGOFlags;
    GuidVector Calculate() override;

private:
    float range;
    bool ignoreLos;
};

// GameObjects the "grab" strategy (QuestGrabStrategy) should walk over to and
// use. Unlike PossibleNewRpgGameObjectsValue (which only ever accepts
// GAMEOBJECT_TYPE_QUESTGIVER), this filters with GameObject::ActivateToQuest
// -- the same core-provided check the client itself uses to decide whether an
// object should sparkle for a player's quests, covering CHEST/GOOBER/GENERIC/
// SPELL_FOCUS/QUESTGIVER. Range is measured from the bot's own current
// position, not the leader -- see the comment on AiPlayerbot.QuestGrabDistance.
class PossibleQuestGrabTargetsValue : public ObjectGuidListCalculatedValue
{
public:
    PossibleQuestGrabTargetsValue(PlayerbotAI* botAI, float range = 0.0f)
        : ObjectGuidListCalculatedValue(botAI, "possible quest grab targets"),
          range(range ? range : sPlayerbotAIConfig.questGrabDistance)
    {
    }

    GuidVector Calculate() override;

private:
    float range;
};

#endif
