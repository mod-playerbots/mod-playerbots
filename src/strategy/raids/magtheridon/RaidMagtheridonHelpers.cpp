#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "GameObject.h"
#include "GroupReference.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"

namespace MagtheridonHelpers
{
    namespace MagtheridonsLairLocations
    {
        const Location WaitingForMagtheridonPosition = {   1.359f,   2.048f, -0.406f, 3.135f };
        const Location MagtheridonTankPosition =       {  22.827f,   2.105f, -0.406f, 3.135f };
        const Location NWChannelerTankPosition =       { -11.764f,  30.818f, -0.411f,   0.0f };
        const Location NEChannelerTankPosition =       { -12.490f, -26.211f, -0.411f,   0.0f };
        const Location RangedSpreadPosition =          { -14.890f,   1.995f, -0.406f,   0.0f };
        const Location HealerSpreadPosition =          {  -2.265f,   1.874f, -0.404f,   0.0f };
    }

    // Identify channelers by their database GUIDs
    Creature* GetChanneler(Player* bot, uint32 dbGuid)
    {
        Map* map = bot->GetMap();
        if (!map)
            return nullptr;

        auto it = map->GetCreatureBySpawnIdStore().find(dbGuid);
        if (it == map->GetCreatureBySpawnIdStore().end())
            return nullptr;

        return it->second;
    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        Group* group = bot->GetGroup();
        if (!target || !group)
            return;

        ObjectGuid currentGuid = group->GetTargetIcon(iconId);
        if (currentGuid != target->GetGUID())
            group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
    }

    void SetRtiTarget(PlayerbotAI* botAI, const std::string& rtiName, Unit* target)
    {
        if (!target)
            return;

        std::string currentRti = botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Get();
        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Get();

        if (currentRti != rtiName || currentTarget != target)
        {
            botAI->GetAiObjectContext()->GetValue<std::string>("rti")->Set(rtiName);
            botAI->GetAiObjectContext()->GetValue<Unit*>("rti target")->Set(target);
        }
    }

    void MarkTargetWithSquare(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::squareIndex);
    }

    void MarkTargetWithStar(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::starIndex);
    }

    void MarkTargetWithCircle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::circleIndex);
    }

    void MarkTargetWithDiamond(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::diamondIndex);
    }

    void MarkTargetWithTriangle(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::triangleIndex);
    }

    void MarkTargetWithCross(Player* bot, Unit* target)
    {
        MarkTargetWithIcon(bot, target, RtiTargetValue::crossIndex);
    }

    const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

    // Get the positions of all Manticron Cubes by their database GUIDs
    std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, const std::vector<uint32>& cubeDbGuids)
    {
        std::vector<CubeInfo> cubes;
        if (!map)
            return cubes;

        for (uint32 dbGuid : cubeDbGuids)
        {
            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
            if (bounds.first == bounds.second)
                continue;

            GameObject* go = bounds.first->second;
            if (!go)
                continue;

            CubeInfo info;
            info.guid = go->GetGUID();
            info.x = go->GetPositionX();
            info.y = go->GetPositionY();
            info.z = go->GetPositionZ();
            cubes.push_back(info);
        }

        return cubes;
    }

    std::unordered_map<ObjectGuid, CubeInfo> botToCubeAssignment;

    void AssignBotsToCubesByGuidAndCoords(Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI)
    {
        botToCubeAssignment.clear();
        if (!group)
            return;

        size_t cubeIndex = 0;
        std::vector<Player*> candidates;

        // Assign ranged DPS (excluding Warlocks) to cubes first
        for (GroupReference* ref = group->GetFirstMember(); ref && cubeIndex < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRangedDps(member, true) ||
                member->getClass() == CLASS_WARLOCK || !GET_PLAYERBOT_AI(member))
                continue;

            candidates.push_back(member);
            if (candidates.size() >= cubes.size())
                break;
        }

        // If there are still cubes left, assign any other non-tank bots
        if (candidates.size() < cubes.size())
        {
            for (GroupReference* ref = group->GetFirstMember();
                ref && candidates.size() < cubes.size(); ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
                    continue;

                if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                    candidates.push_back(member);
            }
        }

        for (Player* member : candidates)
        {
            if (cubeIndex >= cubes.size())
                break;

            if (!member || !member->IsAlive())
                continue;

            botToCubeAssignment[member->GetGUID()] = cubes[cubeIndex++];
        }
    }

    std::unordered_map<uint32, bool> lastBlastNovaState;
    std::unordered_map<uint32, time_t> blastNovaTimer;
    std::unordered_map<uint32, time_t> spreadWaitTimer;
    std::unordered_map<uint32, time_t> dpsWaitTimer;

    bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z)
    {
        // Debris
        std::vector<Unit*> debrisHazards;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || unit->GetEntry() != NPC_TARGET_TRIGGER)
                continue;
            debrisHazards.push_back(unit);
        }
        for (Unit* hazard : debrisHazards)
        {
            float dist = std::sqrt(std::pow(x - hazard->GetPositionX(), 2) + std::pow(y - hazard->GetPositionY(), 2));
            if (dist < 9.0f)
                return false;
        }

        // Conflagration
        GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects");
        for (auto const& goGuid : gos)
        {
            GameObject* go = botAI->GetGameObject(goGuid);
            if (!go || go->GetEntry() != GO_BLAZE)
                continue;

            float dist = std::sqrt(std::pow(x - go->GetPositionX(), 2) + std::pow(y - go->GetPositionY(), 2));
            if (dist < 5.0f)
                return false;
        }

        return true;
    }

    bool IsInstanceTimerManager(PlayerbotAI* botAI, Player* bot)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && botAI->IsDps(member) && GET_PLAYERBOT_AI(member))
                    return member == bot;
            }
        }

        return true;
    }
}
