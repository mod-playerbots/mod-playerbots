#include "AiObjectContext.h"
#include "ValueContext.h"
#include "Ai/Dungeon/Mech/MechValueContext.h"

void AiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new ValueContext());
    valueContexts.Add(new TbcDungeonMechValueContext());
}
