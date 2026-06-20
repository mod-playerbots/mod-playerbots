#ifndef PLAYERBOTS_UNLOCKTRADEDITEMACTION_H
#define PLAYERBOTS_UNLOCKTRADEDITEMACTION_H

#include "Action.h"

class PlayerbotAI;

class UnlockTradedItemAction : public Action
{
public:
    UnlockTradedItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock traded item") {}

    bool Execute(Event event) override;

private:
    bool CanUnlockItem(Item* item);
    void UnlockItem(Item* item);
};

#endif
