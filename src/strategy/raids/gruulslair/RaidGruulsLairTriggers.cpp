#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarSetBotSightTrigger::IsActive()
{
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    sPlayerbotAIConfig->sightDistance = 150.0f;

    bool foundBosses = IsAnyOgreBossAlive(botAI);

    if (!foundBosses)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;

    return foundBosses;
}

bool HighKingMaulgarMaulgarTankTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Group* group = bot->GetGroup();
    return botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive() && group;
}

bool HighKingMaulgarOlmTankTrigger::IsActive()
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");

    return botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive();
}

bool HighKingMaulgarBlindeyeTankTrigger::IsActive()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive();
}

bool HighKingMaulgarKroshMageTankTrigger::IsActive()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Group* group = bot->GetGroup();

    return IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive() && group;
}

bool HighKingMaulgarKigglerMoonkinTankTrigger::IsActive()
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Group* group = bot->GetGroup();

    return IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive() && group;
}

bool HighKingMaulgarMeleeDPSPriorityTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsMelee(bot) &&
           !(botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive());
}

bool HighKingMaulgarRangedDPSPriorityTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsRanged(bot) && !botAI->IsHeal(bot) &&
           !(IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive()) &&
           !(IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive());
}

bool HighKingMaulgarHealerAvoidanceTrigger::IsActive()
{
    return botAI->IsHeal(bot) && IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarWhirlwindRunAwayTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return maulgar && maulgar->IsAlive() && maulgar->HasAura(SPELL_WHIRLWIND) && !botAI->IsMainTank(bot);
}

bool HighKingMaulgarBanishFelstalkerTrigger::IsActive()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarHunterMisdirectionTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
    {
        return false;
    }

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }
    if (hunterIndex == -1 || hunterIndex > 1)
    {
        return false;
    }

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
        {
            continue;
        }
        else if (botAI->IsAssistTankOfIndex(bot, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(bot, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
        case 0:
        return olm && olm->IsAlive() && olm->GetHealth() > 0.98f * olm->GetMaxHealth() && 
               olmTank && olmTank->IsAlive() && botAI->CanCastSpell("misdirection", olmTank);

        case 1:
        return blindeye && blindeye->IsAlive() && blindeye->GetHealth() > 0.90f * blindeye->GetMaxHealth() && 
               blindeyeTank && blindeyeTank->IsAlive() && botAI->CanCastSpell("misdirection", blindeyeTank);

        default:
            break;
    }

    return false;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerPositionBossTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsMainTank(bot);
}

bool GruulTheDragonkillerSpreadRangedTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    Group* group = bot->GetGroup();

    return gruul && gruul->IsAlive() && botAI->IsRanged(bot) && group;
}

bool GruulTheDragonkillerShatterSpreadTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");
    Group* group = bot->GetGroup();

    return gruul && gruul->IsAlive() && group &&
           (bot->HasAura(SPELL_GROUND_SLAM_1) || 
            bot->HasAura(SPELL_GROUND_SLAM_2));
}
