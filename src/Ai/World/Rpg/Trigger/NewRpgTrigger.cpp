#include "NewRpgTriggers.h"
#include "PlayerbotAI.h"

bool NewRpgStatusTrigger::IsActive()
{
    return this->status == this->botAI->rpgInfo.GetStatus();
}
