#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

// Temporarily set bot sight distance to 150 to see the entire room
bool MagtheridonSetBotSightTrigger::IsActive()
{
    float originalSightDistance = sPlayerbotAIConfig->sightDistance;
    sPlayerbotAIConfig->sightDistance = 150.0f;

    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Group* group = bot->GetGroup();

    bool result = group && (magtheridon && magtheridon->IsAlive() ||
                  channeler && channeler->IsAlive());

    if (!result)
        sPlayerbotAIConfig->sightDistance = originalSightDistance;

    return result;
}

bool MagtheridonHellfireChannelerMainTankTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && group && botAI->IsMainTank(bot) && 
           magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonHellfireChannelerNWChannelerTankTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return magtheridon && botAI->IsAssistTankOfIndex(bot, 0) && 
           channelerDiamond && channelerDiamond->IsAlive();
}

bool MagtheridonHellfireChannelerNEChannelerTankTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return magtheridon && botAI->IsAssistTankOfIndex(bot, 1) &&
           channelerTriangle && channelerTriangle->IsAlive();
}

bool MagtheridonHellfireChannelerMisdirectionTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return magtheridon && group && bot->getClass() == CLASS_HUNTER &&
           (channelerStar && channelerStar->IsAlive() || channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonHellfireChannelerDPSPriorityTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Group* group = bot->GetGroup();
    
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (!magtheridon || !magtheridon->IsAlive() || !group || botAI->IsHeal(bot) || botAI->IsMainTank(bot) || 
        botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond && channelerDiamond->IsAlive() ||
        botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle && channelerTriangle->IsAlive())
    {
        return false;
    }

    return channeler && channeler->IsAlive() || magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonBurningAbyssalWarlockCCTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!magtheridon || !group || bot->getClass() != CLASS_WARLOCK)
    {
        return false;
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    return std::any_of(npcs.begin(), npcs.end(), [&](const ObjectGuid& npc) 
    {
        Unit* unit = botAI->GetUnit(npc);
        return unit && unit->GetEntry() == NPC_BURNING_ABYSSAL;
    });
}

bool MagtheridonPositionBossTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && magtheridon->IsAlive() && 
           !magtheridon->HasAura(SPELL_SHADOW_CAGE) && botAI->IsMainTank(bot) && group;
}

bool MagtheridonSpreadRangedTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Group* group = bot->GetGroup();

    if (!group || !magtheridon || !magtheridon->IsAlive() || !botAI->IsRanged(bot) || 
        botAI->IsHeal(bot) || channeler && channeler->IsAlive())
    {
        return false;
    }

    const int spreadWaitSeconds = 10;
    auto it = magtheridonSpreadWaitTimer.find(bot->GetMapId());
    if (it != magtheridonSpreadWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < spreadWaitSeconds)
        {
            return false;
        }
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);

        auto timerIt = magtheridonBlastNovaTimer.find(bot->GetMapId());
        if (timerIt != magtheridonBlastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
            {
                return false;
            }
        }
    }

    return true;
}

bool MagtheridonSpreadHealerTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");
    Group* group = bot->GetGroup();

    if (!group || !magtheridon || !magtheridon->IsAlive() || !botAI->IsHeal(bot) ||
        channeler && channeler->IsAlive())
    {
        return false;
    }

    const int spreadWaitSeconds = 10;
    auto it = magtheridonSpreadWaitTimer.find(bot->GetMapId());
    if (it != magtheridonSpreadWaitTimer.end())
    {
        time_t since = time(nullptr) - it->second;
        if (since < spreadWaitSeconds)
        {
            return false;
        }
    }

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);

        auto timerIt = magtheridonBlastNovaTimer.find(bot->GetMapId());
        if (timerIt != magtheridonBlastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
            {
                return false;
            }
        }
    }

    return true;
}

bool MagtheridonUseManticronCubeTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!group || !magtheridon || !magtheridon->IsAlive() || 
        magtheridon->HasAura(SPELL_SHADOW_CAGE))
    {
        return false;
    }

    bool needsReassign = botToCubeAssignment.empty();
    if (!needsReassign)
    {
        for (const auto& pair : botToCubeAssignment)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
            {
                needsReassign = true;
                break;
            }
        }
    }

    if (needsReassign)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
    }

    return botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
}

bool MagtheridonUpdateTransitionTimerTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();

    return magtheridon && magtheridon->IsAlive() && group;
}
