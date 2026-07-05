#include "MCTriggers.h"

#include "SharedDefines.h"
#include "MCHelpers.h"

using namespace MoltenCoreHelpers;

bool McLivingBombDebuffTrigger::IsActive()
{
    // No check for Baron Geddon, because bots may have the bomb even after Geddon died.
    return bot->HasAura(SPELL_LIVING_BOMB);
}

bool McBaronGeddonInfernoTrigger::IsActive()
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
        return boss->HasAura(SPELL_INFERNO);
    return false;
}

bool McShazzrahRangedTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "shazzrah") && PlayerbotAI::IsRanged(bot);
}

bool McGolemaggMarkBossTrigger::IsActive()
{
    // any tank may mark the boss
    return AI_VALUE2(Unit*, "find target", "golemagg the incinerator") && PlayerbotAI::IsTank(bot);
}

bool McGolemaggIsMainTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "golemagg the incinerator") && PlayerbotAI::IsMainTank(bot);
}

bool McGolemaggIsAssistTankTrigger::IsActive()
{
    return AI_VALUE2(Unit*, "find target", "golemagg the incinerator") && PlayerbotAI::IsAssistTank(bot);
}

bool McCoreHoundMarkTrigger::IsActive()
{
    return PlayerbotAI::IsMainTank(bot) && AI_VALUE2(Unit*, "find target", "core hound");
}
