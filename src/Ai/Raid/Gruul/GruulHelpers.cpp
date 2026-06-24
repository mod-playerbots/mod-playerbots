#include "GruulHelpers.h"
#include "AiFactory.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include "Unit.h"

namespace GruulsLairHelpers
{

const Position MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
const Position OLM_TANK_POSITION      = { 101.050f, 219.359f,  -9.503f };
const Position BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
const Position KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
const Position MAULGAR_ROOM_CENTER    = {  88.754f, 150.759f, -11.569f };
const Position GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

Player* GetKroshMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // (1) First loop: Return the first assistant Mage (real player or bot)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
            continue;

        if (group->IsAssistant(member->GetGUID()))
            return member;

    }

    // (2) Fall back to bot Mage with highest HP
    Player* highestHpMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        uint32 hp = member->GetMaxHealth();
        if (!highestHpMage || hp > highestHp)
        {
            highestHpMage = member;
            highestHp = hp;
        }
    }

    return highestHpMage;
}

Player* GetKigglerMoonkinTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    uint8 tab = AiFactory::GetPlayerSpecTab(bot);

    // (1) First loop: Return the first assistant Moonkin (real player or bot)
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID)
            continue;

        if (group->IsAssistant(member->GetGUID()) && tab == DRUID_TAB_BALANCE)
            return member;
    }

    // (2) Fall back to bot Moonkin with highest HP
    Player* highestHpMoonkin = nullptr;
    uint32 highestHp = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID ||
            !GET_PLAYERBOT_AI(member) || tab != DRUID_TAB_BALANCE)
        {
            continue;
        }

        uint32 hp = member->GetMaxHealth();
        if (!highestHpMoonkin || hp > highestHp)
        {
            highestHpMoonkin = member;
            highestHp = hp;
        }
    }

    return highestHpMoonkin;
}

}
