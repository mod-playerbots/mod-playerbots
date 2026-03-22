#include "AuchenaiCryptsTriggers.h"
#include "AuchenaiCryptsStrategy.h"
#include "AuchenaiCryptsMultipliers.h"

void TbcDungeonAuchenaiCryptsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
  // Shirrak The Dead Watcher
    triggers.push_back(new TriggerNode("shirrak tank position boss", {
        NextAction("shirrak tank position boss", ACTION_RAID + 1) }));
        
    triggers.push_back(new TriggerNode("flee focus fire", {
        NextAction("flee focus fire", ACTION_EMERGENCY + 10) }));
}

void TbcDungeonAuchenaiCryptsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FleeFocusFireMultiplier(botAI));
}
