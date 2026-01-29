#include "OpenItemAction.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "WorldPacket.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "LootObjectStack.h"
#include "AiObjectContext.h"

bool OpenItemAction::Execute(Event)
{
    Item* const item = botAI->FindOpenableItem();

    if (item == nullptr)
    {
        return false;
    }

    uint8_t bag = item->GetBagSlot();  // Retrieves the bag slot (255 for main inventory)
    uint8_t slot = item->GetSlot();    // Retrieves the actual slot inside the bag

    this->OpenItem(item, bag, slot);

    return true;
}

void OpenItemAction::OpenItem(Item* item, uint8_t bag, uint8_t slot)
{
    WorldPacket packet(CMSG_OPEN_ITEM);
    packet << bag << slot;
    this->bot->GetSession()->HandleOpenItemOpcode(packet);

    // Store the item GUID as the loot target
    LootObject lootObject;
    lootObject.guid = item->GetGUID();
    this->botAI->GetAiObjectContext()->GetValue<LootObject>("loot target")->Set(lootObject);

    std::ostringstream out;
    out << "Opened item: " << item->GetTemplate()->Name1;
    this->botAI->TellMaster(out.str());
}
