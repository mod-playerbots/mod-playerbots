#pragma once

#include "Spell.h"
#include "AiObjectContext.h"
#include "MovementActions.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "definition/enum/GurubashiBatRiderEnum.h"

class GurubashiBatRiderUnstableConcoctionAction : public MovementAction
{
public:
    GurubashiBatRiderUnstableConcoctionAction(
        PlayerbotAI* botAI,
        const std::string name = "gurubashi bat rider unstable concoction"
    ) : MovementAction(botAI, name) {}

    bool Execute(Event) override
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

            const float safeDistance = float(GurubashiBatRiderEnum::SPELL_UNSTABLE_CONCOCTION_RADIUS) - this->bot->GetDistance2d(unit);

            if (safeDistance <= 0.0f)
            {
                continue;
            }

            this->MoveAway(unit, safeDistance);

            return true;
        }

        return false;
    }
};
