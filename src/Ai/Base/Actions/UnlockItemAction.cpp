#include "BotChatService.h"
#include "UnlockItemAction.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"
#include "BotSpellService.h"
#include "BotItemService.h"

static constexpr uint32 PICK_LOCK_SPELL_ID = 1804;

bool UnlockItemAction::Execute(Event event)
{
    bool foundLockedItem = false;

    Item* item = botAI->GetServices().GetItemService().FindLockedItem();
    if (item)
    {
        UnlockItem(item);
        foundLockedItem = true;
    }

    return foundLockedItem;
}

void UnlockItemAction::UnlockItem(Item* item)
{
    // Use CastSpell to unlock the item
    if (botAI->GetServices().GetSpellService().CastSpell(PICK_LOCK_SPELL_ID, bot, item))
    {
        std::ostringstream out;
        out << "Used Pick Lock on: " << item->GetTemplate()->Name1;
        botAI->GetServices().GetChatService().TellMaster(out.str());
    }
    else
    {
        botAI->GetServices().GetChatService().TellError("Failed to cast Pick Lock.");
    }
}
