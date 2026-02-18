#pragma once

#include "Spell.h"
#include "AiObjectContext.h"
#include "MovementActions.h"
#include "Multiplier.h"
#include "PlayerbotAI.h"
#include "Unit.h"
#include "Value.h"

#include "GurubashiBatRiderUnstableConcoctionAction.h"
#include "definition/enum/GurubashiBatRiderEnum.h"

class GurubashiBatRiderUnstableConcoctionMultiplier : public Multiplier
{
public:
    GurubashiBatRiderUnstableConcoctionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gurubashi bat rider unstable concoction") {}

    float GetValue(Action& action) override
    {
        if (this->bot->GetMapId() != MAP_ZUL_GURUB)
        {
            return 1.0f;
        }

        if (this->bot->IsInCombat() == false)
        {
            return 1.0f;
        }

        Value<GuidVector>* const nearestUnitsValue = this->context->GetValue<GuidVector>("nearest hostile npcs");

        if (nearestUnitsValue == nullptr)
        {
            return 1.0f;
        }

        const GuidVector nearestUnitsGuids = nearestUnitsValue->Get();

        bool unstableConcoctionActive = false;

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

            unstableConcoctionActive = true;

            break;
        }

        if (!unstableConcoctionActive)
        {
            return 1.0f;
        }

        const GurubashiBatRiderUnstableConcoctionAction* const movementAction = dynamic_cast<GurubashiBatRiderUnstableConcoctionAction*>(&action);

        if (movementAction == nullptr)
        {
            return 0.0f;
        }

        return 1.0f;
    }
};
