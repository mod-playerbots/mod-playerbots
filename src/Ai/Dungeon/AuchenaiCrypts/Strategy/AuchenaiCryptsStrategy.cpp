#include "AuchenaiCryptsTriggers.h"
#include "AuchenaiCryptsStrategy.h"
#include "AuchenaiCryptsMultipliers.h"

void TbcDungeonAuchenaiCryptsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shirrak The Dead Watcher
    triggers.push_back(new TriggerNode("shirrak tank position boss", {
        NextAction("shirrak tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("shirrak flee focus fire", {
        NextAction("shirrak flee focus fire", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("shirrak ranged keep distance", {
        NextAction("shirrak ranged keep distance", ACTION_RAID + 1) }));
}

void TbcDungeonAuchenaiCryptsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ShirrakFleeFocusFireMultiplier(botAI));
}
