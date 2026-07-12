#ifndef PLAYERBOTS_MECHANARSTRATEGY_H
#define PLAYERBOTS_MECHANARSTRATEGY_H

#include "AiObjectContext.h"
#include "Strategy.h"
#include "Multiplier.h"

// The Mechanar (map 554) instance combat strategy. Installed on every party bot
// while inside the instance (PlayerbotAI::ApplyInstanceStrategies), it is inert
// until Nethermancer Sepethrea's Raging Flames start fixating, then drives the
// kite / boss-focus behaviour. See MechanarTriggers.h for the mechanic.
class TbcDungeonMechanarStrategy : public Strategy
{
public:
    TbcDungeonMechanarStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mechanar"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
