#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include "Unit.h"

namespace GruulsLairHelpers
{
    namespace GruulsLairLocations
    {
        // Olm does not chase properly due to the Core's caster movement issues
        // Thus, the below "OlmTankPosition" is beyond the actual desired tanking location
        // It is the spot to which the OlmTank runs to to pull Olm to a decent tanking location
        // "MaulgarRoomCenter" is to keep healers in a centralized location
        const Location MaulgarTankPosition  = {  90.686f, 167.047f, -13.234f };
        const Location OlmTankPosition      = {  87.485f, 234.942f,  -3.635f };
        const Location BlindeyeTankPosition = {  99.681f, 213.989f, -10.345f };
        const Location KroshTankPosition    = { 116.880f, 166.208f, -14.231f };
        const Location MaulgarRoomCenter    = {  88.754f, 150.759f, -11.569f };
        const Location GruulTankPosition    = { 241.238f, 365.025f,  -4.220f };
    }

    bool IsAnyOgreBossAlive(PlayerbotAI* botAI)
    {
        const char* ogreBossNames[] =
        {
            "high king maulgar",
            "kiggler the crazed",
            "krosh firehand",
            "olm the summoner",
            "blindeye the seer"
        };

        for (const char* name : ogreBossNames)
        {
            Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (!boss || !boss->IsAlive())
                continue;
            return true;
        }

        return false;
    }

    void MarkTargetWithIcon(Player* bot, Unit* target, uint8 iconId)
    {
        Group* group = bot->GetGroup();
        if (!target || !group)
            return;

        ObjectGuid currentGuid = group->GetTargetIcon(iconId);
        if (currentGuid != target->GetGUID())
        {
            group->SetTargetIcon(iconId, bot->GetGUID(), target->GetGUID());
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

    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        Player* highestHpMage = nullptr;
        uint32 highestHp = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_MAGE)
            {
                uint32 hp = member->GetMaxHealth();
                if (!highestHpMage || hp > highestHp)
                {
                    highestHpMage = member;
                    highestHp = hp;
                }
            }
        }

        return highestHpMage == bot;
    }

    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        Player* highestHpMoonkin = nullptr;
        uint32 highestHp = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                continue;

            if (member->getClass() == CLASS_DRUID)
            {
                int tab = AiFactory::GetPlayerSpecTab(member);
                if (tab == DRUID_TAB_BALANCE)
                {
                    uint32 hp = member->GetMaxHealth();
                    if (!highestHpMoonkin || hp > highestHp)
                    {
                        highestHpMoonkin = member;
                        highestHp = hp;
                    }
                }
            }
        }

        return highestHpMoonkin == bot;
    }

    bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos)
    {
        const float KROSH_SAFE_DISTANCE = 20.0f;
        const float MAULGAR_SAFE_DISTANCE = 10.0f;
        bool isSafe = true;

        Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
        if (krosh && krosh->IsAlive())
        {
            float dist = sqrt(pow(pos.GetPositionX() - krosh->GetPositionX(), 2) + pow(pos.GetPositionY() - krosh->GetPositionY(), 2));
            if (dist < KROSH_SAFE_DISTANCE)
                isSafe = false;
        }

        Unit* maulgar = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get();
        if (botAI->IsRanged(bot) && maulgar && maulgar->IsAlive())
        {
            float dist = sqrt(pow(pos.GetPositionX() - maulgar->GetPositionX(), 2) + pow(pos.GetPositionY() - maulgar->GetPositionY(), 2));
            if (dist < MAULGAR_SAFE_DISTANCE)
                isSafe = false;
        }

        return isSafe;
    }

    bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos)
    {
        const float SEARCH_RADIUS = 30.0f;
        const uint8 NUM_POSITIONS = 32;

        outPos = { bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() };
        if (IsPositionSafe(botAI, bot, outPos))
        {
            outPos = Position();
            return false;
        }

        float bestScore = std::numeric_limits<float>::max();
        bool foundSafeSpot = false;
        Position bestPos;

        for (int i = 0; i < NUM_POSITIONS; ++i)
        {
            float angle = 2 * M_PI * i / NUM_POSITIONS;
            Position candidatePos;
            candidatePos.m_positionX = bot->GetPositionX() + SEARCH_RADIUS * cos(angle);
            candidatePos.m_positionY = bot->GetPositionY() + SEARCH_RADIUS * sin(angle);
            candidatePos.m_positionZ = bot->GetPositionZ();

            float destX = candidatePos.m_positionX, destY = candidatePos.m_positionY, destZ = candidatePos.m_positionZ;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), destX, destY, destZ, true))
                continue;

            if (destX != candidatePos.m_positionX || destY != candidatePos.m_positionY)
                continue;

            candidatePos.m_positionX = destX;
            candidatePos.m_positionY = destY;
            candidatePos.m_positionZ = destZ;

            if (IsPositionSafe(botAI, bot, candidatePos))
            {
                float movementDistance = sqrt(pow(destX - bot->GetPositionX(), 2) + pow(destY - bot->GetPositionY(), 2));
                if (movementDistance < bestScore)
                {
                    bestScore = movementDistance;
                    bestPos = candidatePos;
                    foundSafeSpot = true;
                }
            }
        }

        if (foundSafeSpot)
        {
            outPos = bestPos;
            return true;
        }

        outPos = Position();
        return false;
    }
}
