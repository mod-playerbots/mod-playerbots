#ifndef PLAYERBOTS_TRADESTATUSEXTENDEDACTION_H
#define PLAYERBOTS_TRADESTATUSEXTENDEDACTION_H

#include "QueryItemUsageAction.h"

class Player;
class PlayerbotAI;

class TradeStatusExtendedAction : public QueryItemUsageAction
{
public:
    TradeStatusExtendedAction(PlayerbotAI* botAI) : QueryItemUsageAction(botAI, "trade status extended") {}

    bool Execute(Event event) override;
};

#endif
