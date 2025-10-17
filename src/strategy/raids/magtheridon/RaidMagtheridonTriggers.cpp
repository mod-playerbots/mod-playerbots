#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonHellfireChannelerMainTankTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon && botAI->IsMainTank(bot) && 
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

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return magtheridon && bot->getClass() == CLASS_HUNTER &&
           (channelerStar && channelerStar->IsAlive() || 
            channelerCircle && channelerCircle->IsAlive());
}

bool MagtheridonDPSPriorityTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");

    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (!magtheridon || botAI->IsHeal(bot) || botAI->IsMainTank(bot) || 
        botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond && channelerDiamond->IsAlive() ||
        botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle && channelerTriangle->IsAlive())
        return false;

    return channeler && channeler->IsAlive() || magtheridon && 
           !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonBurningAbyssalWarlockCCTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    if (!magtheridon || bot->getClass() != CLASS_WARLOCK)
        return false;

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

    return magtheridon && botAI->IsMainTank(bot) &&
           !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonSpreadRangedTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Unit* channeler = AI_VALUE2(Unit*, "find target", "hellfire channeler");

    return magtheridon && botAI->IsRanged(bot) && 
           !(channeler && channeler->IsAlive());
}

bool MagtheridonUseManticronCubeTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");
    Group* group = bot->GetGroup();
    if (!group || !magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;

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

bool MagtheridonManageTimersAndAssignmentsTrigger::IsActive()
{
    Unit* magtheridon = AI_VALUE2(Unit*, "find target", "magtheridon");

    return magtheridon;
}
