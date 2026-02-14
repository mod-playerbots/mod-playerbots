#include "RaidGruulsLairTriggers.h"
#include "RaidGruulsLairHelpers.h"
#include "Playerbots.h"

using namespace GruulsLairHelpers;

// High King Maulgar Triggers

bool HighKingMaulgarIsMainTankTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return botAI->IsMainTank(bot) && maulgar;
}

bool HighKingMaulgarIsFirstAssistTankTrigger::IsActive()
{
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");

    return botAI->IsAssistTankOfIndex(bot, 0, false) && olm;
}

bool HighKingMaulgarIsSecondAssistTankTrigger::IsActive()
{
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");

    return botAI->IsAssistTankOfIndex(bot, 1, false) && blindeye;
}

bool HighKingMaulgarIsMageTankTrigger::IsActive()
{
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return IsKroshMageTank(botAI, bot) && krosh;
}

bool HighKingMaulgarIsMoonkinTankTrigger::IsActive()
{
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");

    return IsKigglerMoonkinTank(botAI, bot) && kiggler;
}

bool HighKingMaulgarDeterminingKillOrderTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");
    Unit* kiggler = AI_VALUE2(Unit*, "find target", "kiggler the crazed");
    Unit* olm = AI_VALUE2(Unit*, "find target", "olm the summoner");
    Unit* blindeye = AI_VALUE2(Unit*, "find target", "blindeye the seer");
    Unit* krosh = AI_VALUE2(Unit*, "find target", "krosh firehand");

    return (botAI->IsDps(bot) || botAI->IsTank(bot)) &&
           !(botAI->IsMainTank(bot) && maulgar) &&
           !(botAI->IsAssistTankOfIndex(bot, 0, false) && olm) &&
           !(botAI->IsAssistTankOfIndex(bot, 1, false) && blindeye) &&
           !(IsKroshMageTank(botAI, bot) && krosh) &&
           !(IsKigglerMoonkinTank(botAI, bot) && kiggler);
}

bool HighKingMaulgarHealerInDangerTrigger::IsActive()
{
    return botAI->IsHeal(bot) && IsAnyOgreBossAlive(botAI);
}

bool HighKingMaulgarBossChannelingWhirlwindTrigger::IsActive()
{
    Unit* maulgar = AI_VALUE2(Unit*, "find target", "high king maulgar");

    return maulgar && maulgar->HasAura(SPELL_WHIRLWIND) &&
           !botAI->IsMainTank(bot);
}

bool HighKingMaulgarWildFelstalkerSpawnedTrigger::IsActive()
{
    Unit* felStalker = AI_VALUE2(Unit*, "find target", "wild fel stalker");

    return felStalker && bot->getClass() == CLASS_WARLOCK;
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
        return olm && olm->GetHealthPct() > 98.0f &&
               olmTank && botAI->CanCastSpell("misdirection", olmTank);

    case 1:
        return blindeye && blindeye->GetHealthPct() > 90.0f &&
               blindeyeTank && botAI->CanCastSpell("misdirection", blindeyeTank);

    default:
        break;
    }

    return false;
}

// Gruul the Dragonkiller Triggers

bool GruulTheDragonkillerBossEngagedByTanksTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && botAI->IsTank(bot);
}

bool GruulTheDragonkillerBossEngagedByRangedTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && botAI->IsRanged(bot);
}

bool GruulTheDragonkillerIncomingShatterTrigger::IsActive()
{
    Unit* gruul = AI_VALUE2(Unit*, "find target", "gruul the dragonkiller");

    return gruul && (bot->HasAura(SPELL_GROUND_SLAM_1) ||
           bot->HasAura(SPELL_GROUND_SLAM_2));
}
