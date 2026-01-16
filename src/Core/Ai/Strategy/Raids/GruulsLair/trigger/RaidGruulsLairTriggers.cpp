#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarIsMainTankTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive();
}

bool HighKingMaulgarIsFirstAssistTankTrigger::IsActive()
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");

    return botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive();
}

bool HighKingMaulgarIsSecondAssistTankTrigger::IsActive()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive();
}

bool HighKingMaulgarIsMageTankTrigger::IsActive()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive();
}

bool HighKingMaulgarIsMoonkinTankTrigger::IsActive()
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive();
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return (botAI->IsDps(bot) || botAI->IsTank(bot)) &&
           !(botAI->IsMainTank(bot) && maulgar && maulgar->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 0) && olm && olm->IsAlive()) &&
           !(botAI->IsAssistTankOfIndex(bot, 1) && blindeye && blindeye->IsAlive()) &&
           !(IsKroshMageTank(botAI, bot) && krosh && krosh->IsAlive()) &&
           !(IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive());
}

bool HighKingMaulgarHealerInDangerTrigger::IsActive()
{
    return botAI->IsHeal(bot) && IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return maulgar && maulgar->IsAlive() && maulgar->HasAura(SPELL_WHIRLWIND) &&
           !botAI->IsMainTank(bot);
}

bool HighKingMaulgarWildFelstalkerSpawnedTrigger::IsActive()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && felStalker->IsAlive() && bot->getClass() == CLASS_WARLOCK;
}

bool HighKingMaulgarPullingOlmAndBlindeyeTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group || bot->getClass() != CLASS_HUNTER)
        return false;

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
        return false;

    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Player* olmTank = nullptr;
    Player* blindeyeTank = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsAlive())
            continue;
        else if (botAI->IsAssistTankOfIndex(member, 0)) olmTank = member;
        else if (botAI->IsAssistTankOfIndex(member, 1)) blindeyeTank = member;
    }

    switch (hunterIndex)
    {
    case 0:
        return olm && olm->IsAlive() && olm->GetHealthPct() > 98.0f &&
               olmTank && olmTank->IsAlive() && botAI->CanCastSpell("misdirection", olmTank);

    case 1:
        return blindeye && blindeye->IsAlive() && blindeye->GetHealthPct() > 90.0f &&
               blindeyeTank && blindeyeTank->IsAlive() && botAI->CanCastSpell("misdirection", blindeyeTank);

    default:
        break;
    }

    return false;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossEngagedByMainTankTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsMainTank(bot);
}

bool GruulTheDragonkillerBossEngagedByRangeTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() && botAI->IsRanged(bot);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && gruul->IsAlive() &&
           (bot->HasAura(SPELL_GROUND_SLAM_1) ||
            bot->HasAura(SPELL_GROUND_SLAM_2));
}
