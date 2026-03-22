#include "Playerbots.h"
#include "AuchenaiCryptsTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"

// Shirrak the Dead Watcher

bool ShirrakTankPositionBossTrigger::IsActive()
{
    return botAI->IsTank(bot) &&
        AI_VALUE2(Unit*, "find target", "shirrak the dead watcher");
}

bool FleeFocusFireTrigger::IsActive()
{
    if (!AI_VALUE2(Unit*, "find target", "shirrak the dead watcher"))
        return false;

   constexpr float searchRadius = 20.0f;
        std::list<Creature*> creatureList;
        bot->GetCreatureListWithEntryInGrid(creatureList, static_cast<uint32>(AuchenaiCryptsIDs::NPC_FOCUS_FIRE), 20.0f);

    for (Creature* flare : creatureList)
    {
        if (flare && flare->IsAlive())
            return true;
    }
    return false;
}
