/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GruulHelpers.h"
#include "AiFactory.h"
#include "Playerbots.h"

namespace GruulHelpers
{

bool IsMaulgarTank(Player* bot)
{
    // Note: IsMainTank() is not necessarily a tank (by either strategy or spec). It can be anybody
    // with the main tank flag. Raid strategies will have problems with non-tank main tanks so this
    // assumes you are using a real tank for your main tank.
    return PlayerbotAI::IsTank(bot) && PlayerbotAI::IsMainTank(bot);
}

bool IsOlmTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 0, true);
}

bool IsBlindeyeTank(Player* bot)
{
    return PlayerbotAI::IsAssistTankOfIndex(bot, 1, true);
}

Player* GetKroshMageTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // If an assistant Mage (player or bot) is found, return immediately.
    // Otherwise, return the bot Mage with the highest HP as fallback.
    Player* highestHpBotMage = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_MAGE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMage || hp > highestHp)
        {
            highestHpBotMage = member;
            highestHp = hp;
        }
    }

    return highestHpBotMage;
}

bool IsKroshMageTank(Player* bot)
{
    return bot->getClass() == CLASS_MAGE && GetKroshMageTank(bot) == bot;
}

Player* GetKigglerMoonkinTank(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return nullptr;

    // If an assistant Balance Druid (player or bot) is found, return immediately.
    // Otherwise, return the bot Balance Druid with the highest HP as fallback.
    Player* highestHpBotMoonkin = nullptr;
    uint32 highestHp = 0;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != GRUUL_MAP_ID ||
            member->getClass() != CLASS_DRUID ||
            AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
        {
            continue;
        }

        if (group->IsAssistant(member->GetGUID()))
            return member;

        if (!GET_PLAYERBOT_AI(member))
            continue;

        uint32 const hp = member->GetMaxHealth();
        if (!highestHpBotMoonkin || hp > highestHp)
        {
            highestHpBotMoonkin = member;
            highestHp = hp;
        }
    }

    return highestHpBotMoonkin;
}

bool IsKigglerMoonkinTank(Player* bot)
{
    return bot->getClass() == CLASS_DRUID && GetKigglerMoonkinTank(bot) == bot;
}

bool HasGroundSlam(Player* bot)
{
    return bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_1)) ||
        bot->HasAura(Id(GruulSpells::SPELL_GROUND_SLAM_2));
}

}
