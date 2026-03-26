#include "MotionMaster.h"
#include "RaidNaxxActions.h"

#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "UnitAI.h"

// @TODO: This needs a complete rewrite.
bool RazuviousUseObedienceCrystalAction::Execute(Event /*event*/)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    Unit* const charmedUnit = this->bot->GetCharm();

    if (charmedUnit != nullptr)
    {
        Value<Unit*>* targetValue = this->context->GetValue<Unit*>("find target", "instructor razuvious");

        if (targetValue == nullptr)
        {
            return false;
        }

        Unit* const target = targetValue->Get();

        if (target == nullptr)
        {
            return false;
        }

        MotionMaster* const charmedUnitMotionMaster = charmedUnit->GetMotionMaster();

        if (charmedUnitMotionMaster == nullptr)
        {
            return false;
        }

        if (charmedUnitMotionMaster->GetMotionSlotType(MOTION_SLOT_ACTIVE) == NULL_MOTION_TYPE)
        {
            charmedUnitMotionMaster->Clear();
            charmedUnitMotionMaster->MoveChase(target);
            charmedUnit->GetAI()->AttackStart(target);
        }

        const Aura* forceObedience = this->botAI->GetAura("force obedience", charmedUnit);
        int32_t duration_time = 90000;

        if (forceObedience == nullptr)
        {
            forceObedience = this->botAI->GetAura("mind control", charmedUnit);
            duration_time = 60000;
        }

        if (forceObedience == nullptr)
        {
            return false;
        }

        if (charmedUnit->GetDistance(target) <= 0.51f)
        {
            // taunt
            bool tauntUseful = true;

            if (forceObedience->GetDuration() <= (duration_time - 5000))
            {
                if (target->GetVictim() && botAI->HasAura(29061, target->GetVictim()))
                {
                    tauntUseful = false;
                }

                if (forceObedience->GetDuration() <= 3000)
                {
                    tauntUseful = false;
                }
            }

            if (forceObedience->GetDuration() >= (duration_time - 500))
            {
                tauntUseful = false;
            }

            if (tauntUseful && !charmedUnit->HasSpellCooldown(29060))
            {
                // shield
                if (!charmedUnit->HasSpellCooldown(29061))
                {
                    charmedUnit->CastSpell(charmedUnit, 29061, true);
                    charmedUnit->AddSpellCooldown(29061, 0, 30 * 1000);
                }

                charmedUnit->CastSpell(target, 29060, true);
                charmedUnit->AddSpellCooldown(29060, 0, 20 * 1000);
            }

            // strike
            if (!charmedUnit->HasSpellCooldown(61696))
            {
                charmedUnit->CastSpell(target, 61696, true);
                charmedUnit->AddSpellCooldown(61696, 0, 4 * 1000);
            }
        }

        return false;
    }

    const Difficulty diff = this->bot->GetRaidDifficulty();

    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        Value<GuidVector>* const npcsValue = this->context->GetValue<GuidVector>("nearest npcs");

        if (npcsValue == nullptr)
        {
            return false;
        }

        const GuidVector npcs = npcsValue->Get();

        for (GuidVector::const_iterator i = npcs.begin(); i != npcs.end(); i++)
        {
            Creature* const unit = this->botAI->GetCreature(*i);

            if (unit == nullptr)
            {
                continue;
            }

            if (this->botAI->IsMainTank(bot) && unit->GetSpawnId() != 128352)
            {
                continue;
            }

            if (!this->botAI->IsMainTank(bot) && unit->GetSpawnId() != 128353)
            {
                continue;
            }

            if (this->MoveTo(unit, 0.0f, MovementPriority::MOVEMENT_COMBAT))
            {
                return true;
            }

            Creature* const creature = this->bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_SPELLCLICK);

            if (!creature)
            {
                continue;
            }

            creature->HandleSpellClick(bot);

            return true;
        }

        return false;
    }

    Value<GuidVector>* const attackersValue = this->context->GetValue<GuidVector>("attackers");

    if (attackersValue == nullptr)
    {
        return false;
    }

    const GuidVector attackers = attackersValue->Get();
    Unit* target = nullptr;

    for (GuidVector::const_iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* const unit = this->botAI->GetUnit(*i);

        if (unit == nullptr)
        {
            continue;
        }

        if (!this->botAI->EqualLowercaseName(unit->GetName(), "death knight understudy"))
        {
            continue;
        }

        target = unit;
    }

    if (target == nullptr)
    {
        return false;
    }

    const float spellDistance = PlayerbotAIConfig::instance().spellDistance;

    if (this->bot->GetDistance2d(target) > spellDistance)
    {
        return this->MoveNear(target, spellDistance, MovementPriority::MOVEMENT_COMBAT);
    }

    return this->botAI->CastSpell("mind control", target);
}

bool RazuviousTargetAction::Execute(Event /*event*/)
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    Value<Unit*>* const razuviousValue = this->context->GetValue<Unit*>("find target", "inspector razuvious");
    Value<Unit*>* const understudyValue = this->context->GetValue<Unit*>("find target", "death knight understudy");

    if (razuviousValue == nullptr || understudyValue == nullptr)
    {
        return false;
    }

    Unit* const razuvious = razuviousValue->Get();
    Unit* const understudy = understudyValue->Get();
    Unit* target = razuvious;

    if (this->botAI->IsTank(bot))
    {
        target = understudy;
    }

    Value<Unit*>* const currentTargetValue = this->context->GetValue<Unit*>("current target");

    if (currentTargetValue == nullptr)
    {
        return false;
    }

    if (currentTargetValue->Get() == target)
    {
        return false;
    }

    return this->Attack(target);
}
