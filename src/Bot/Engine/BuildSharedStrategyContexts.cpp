#include "AiObjectContext.h"
#include "Ai/Dungeon/DungeonStrategyContext.h"
#include "Ai/Raid/RaidStrategyContext.h"
#include "StrategyContext.h"

void AiObjectContext::BuildSharedStrategyContexts(SharedNamedObjectContextList<Strategy>& strategyContexts)
{
    strategyContexts.Add(new StrategyContext());
    strategyContexts.Add(new MovementStrategyContext());
    strategyContexts.Add(new AssistStrategyContext());
    strategyContexts.Add(new QuestStrategyContext());
    strategyContexts.Add(new DungeonStrategyContext());
    strategyContexts.Add(new RaidStrategyContext());
}
