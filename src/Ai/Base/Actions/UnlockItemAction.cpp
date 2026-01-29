#include "UnlockItemAction.h"
#include "PlayerbotAI.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "SpellInfo.h"

inline constexpr uint32_t PICK_LOCK_SPELL_ID = 1804;

bool UnlockItemAction::Execute(Event)
{
    Item* const item = botAI->FindLockedItem();

    if (item == nullptr)
    {
        return false;
    }

    this->UnlockItem(item);

    return true;
}

void UnlockItemAction::UnlockItem(Item* item)
{
    // Use CastSpell to unlock the item
    const bool unlocked = this->botAI->CastSpell(PICK_LOCK_SPELL_ID, bot, item);

    if (unlocked)
    {
        std::ostringstream out;
        out << "Used Pick Lock on: " << item->GetTemplate()->Name1;
        this->botAI->TellMaster(out.str());

        return;
    }

    this->botAI->TellError("Failed to cast Pick Lock.");
}
