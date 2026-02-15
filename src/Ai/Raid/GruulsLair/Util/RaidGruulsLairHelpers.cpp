#include "RaidGruulsLairHelpers.h"
#include "AiFactory.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include "Unit.h"

namespace GruulsLairHelpers
{
    // Olm does not chase properly due to the Core's caster movement issues
    // Thus, the below "OlmTankPosition" is beyond the actual desired tanking location
    // It is the spot to which the OlmTank runs to to pull Olm to a decent tanking location
    // "MaulgarRoomCenter" is to keep healers in a centralized location
    const Position MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
    const Position OLM_TANK_POSITION      = {  87.485f, 234.942f,  -3.635f };
    const Position BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
    const Position KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
    const Position MAULGAR_ROOM_CENTER    = {  88.754f, 150.759f, -11.569f };
    const Position GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

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

    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // (1) First loop: Return the first assistant Mage (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member == bot;
        }

        // (2) Fall back to bot Mage with highest HP
        Player* highestHpMage = nullptr;
        uint32 highestHp = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_MAGE)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Mage tank, or nullptr if none found
        return highestHpMage == bot;
    }

    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // (1) First loop: Return the first assistant Moonkin (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID)
                continue;

            if (group->IsAssistant(member->GetGUID()) &&
                AiFactory::GetPlayerSpecTab(member) == DRUID_TAB_BALANCE)
                return member == bot;
        }

        // (2) Fall back to bot Moonkin with highest HP
        Player* highestHpMoonkin = nullptr;
        uint32 highestHp = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID ||
                !GET_PLAYERBOT_AI(member) || AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpMoonkin || hp > highestHp)
            {
                highestHpMoonkin = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Moonkin tank, or nullptr if none found
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
            candidatePos.Relocate(bot->GetPositionX() + SEARCH_RADIUS * cos(angle),
                                  bot->GetPositionY() + SEARCH_RADIUS * sin(angle),
                                  bot->GetPositionZ());

            float destX = candidatePos.GetPositionX();
            float destY = candidatePos.GetPositionY();
            float destZ = candidatePos.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), destX, destY, destZ, true))
                continue;

            if (destX != candidatePos.GetPositionX() || destY != candidatePos.GetPositionY())
                continue;

            candidatePos.Relocate(destX, destY, destZ);

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
