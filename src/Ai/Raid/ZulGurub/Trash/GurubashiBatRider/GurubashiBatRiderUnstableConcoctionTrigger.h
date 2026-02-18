#pragma once

#include "Spell.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Trigger.h"
#include "definition/enum/GurubashiBatRiderEnum.h"

class GurubashiBatRiderUnstableConcoctionTrigger : public Trigger
{
public:
    GurubashiBatRiderUnstableConcoctionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gurubashi bat rider unstable concoction") {}

    bool IsActive() override
    {
        if (this->bot->GetMapId() != MAP_ZUL_GURUB)
        {
            return false;
        }

        if (this->bot->IsInCombat() == false)
        {
            return false;
        }

        Value<GuidVector>* const nearestUnitsValue = this->context->GetValue<GuidVector>("nearest hostile npcs");

        if (nearestUnitsValue == nullptr)
        {
            return false;
        }

        const GuidVector nearestUnitsGuids = nearestUnitsValue->Get();

        for (const ObjectGuid& guid : nearestUnitsGuids)
        {
            Unit* const unit = this->botAI->GetUnit(guid);

            if (unit == nullptr)
            {
                continue;
            }

            if (unit->GetEntry() != uint32_t(GurubashiBatRiderEnum::ENTRY))
            {
                continue;
            }

            const Spell* const castedSpell = unit->GetFirstCurrentCastingSpell();

            if (castedSpell == nullptr)
            {
                continue;
            }

            const SpellInfo* const castedSpellInfo = castedSpell->GetSpellInfo();

            if (castedSpellInfo == nullptr)
            {
                continue;
            }

            if (castedSpellInfo->Id != uint32_t(GurubashiBatRiderEnum::SPELL_UNSTABLE_CONCOCTION))
            {
                continue;
            }

            return true;
        }

        return false;
    }
};
