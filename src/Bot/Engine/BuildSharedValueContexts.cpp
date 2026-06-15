#include "AiObjectContext.h"
#include "ValueContext.h"

void AiObjectContext::BuildSharedValueContexts(SharedNamedObjectContextList<UntypedValue>& valueContexts)
{
    valueContexts.Add(new ValueContext());
}
