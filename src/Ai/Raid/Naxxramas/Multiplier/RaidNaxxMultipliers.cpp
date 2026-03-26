#include "RaidNaxxMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "RaidNaxxSpellIds.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "Spell.h"
#include "UniversalTauntAction.h"
#include "UseMeetingStoneAction.h"

float GrobbulusMultiplier::GetValue(Action& action)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grobbulus");

    if (bossValue == nullptr)
    {
        return 1.0f;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return 1.0f;
    }

    if (dynamic_cast<AvoidAoeAction*>(&action))
    {
        return this->botAI->IsMainTank(this->bot) ? 0.0f : 1.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(&action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float LoathebGenericMultiplier::GetValue(Action& action)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "loatheb");

    if (bossValue == nullptr)
    {
        return 1.0f;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return 1.0f;
    }

    Value<bool>* const neglectThreatValue = this->context->GetValue<bool>("neglect threat");

    if (neglectThreatValue == nullptr)
    {
        return 1.0f;
    }

    neglectThreatValue->Set(true);

    if (this->botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(&action) || dynamic_cast<FleeAction*>(&action) ||
         dynamic_cast<CombatFormationMoveAction*>(&action)))
    {
        return 0.0f;
    }

    if (!dynamic_cast<CastHealingSpellAction*>(&action))
    {
        return 1.0f;
    }

    const Aura* aura = NaxxSpellIds::GetAnyAura(bot, {NaxxSpellIds::NecroticAura10});

    if (aura == nullptr)
    {
        // Fallback to name for custom spell data.
        aura = this->botAI->GetAura("necrotic aura", bot);
    }

    if (aura == nullptr || aura->GetDuration() <= 1500)
    {
        return 1.0f;
    }

    return 0.0f;
}

float ThaddiusGenericMultiplier::GetValue(Action& action)
{
    if (!this->helper.UpdateBossAI())
    {
        return 1.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(&action))
    {
        return 0.0f;
    }

    // pet phase
    if (this->helper.IsPhasePet() &&
        (dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(&action) ||
         dynamic_cast<ReachPartyMemberToHealAction*>(&action) || dynamic_cast<BuffOnMainTankAction*>(&action)))
    {
        return 0.0f;
    }

    // die at the same time
    Unit* const target = AI_VALUE(Unit*, "current target");
    Unit* const feugen = AI_VALUE2(Unit*, "find target", "feugen");
    Unit* const stalagg = AI_VALUE2(Unit*, "find target", "stalagg");

    if (helper.IsPhasePet() && target && feugen && stalagg && target->GetHealthPct() <= 40 &&
        (feugen->GetHealthPct() >= target->GetHealthPct() + 3 || stalagg->GetHealthPct() >= target->GetHealthPct() + 3))
    {
        if (dynamic_cast<CastSpellAction*>(&action) && !dynamic_cast<CastHealingSpellAction*>(&action))
            return 0.0f;
    }

    return 1.0f;
}

float SapphironGenericMultiplier::GetValue(Action& action)
{
    if (!this->helper.UpdateBossAI())
    {
        return 1.0f;
    }

    if (dynamic_cast<CastDeathGripAction*>(&action) || dynamic_cast<CombatFormationMoveAction*>(&action))
    {
        return 0.0f;
    }

    return 1.0f;
}

float InstructorRazuviousGenericMultiplier::GetValue(Action& action)
{
    if (!this->helper.UpdateBossAI())
    {
        return 1.0f;
    }

    Value<bool>* const neglectThreatValue = this->context->GetValue<bool>("neglect threat");

    if (neglectThreatValue == nullptr)
    {
        return 1.0f;
    }

    neglectThreatValue->Set(true);

    if (this->botAI->GetState() == BOT_STATE_COMBAT &&
        (dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action) ||
         dynamic_cast<CastTauntAction*>(&action) || dynamic_cast<CastDarkCommandAction*>(&action) ||
         dynamic_cast<CastHandOfReckoningAction*>(&action) || dynamic_cast<CastGrowlAction*>(&action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float KelthuzadGenericMultiplier::GetValue(Action& action)
{
    if (!this->helper.UpdateBossAI())
    {
        return 1.0f;
    }

    if ((dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action) ||
         dynamic_cast<CastDebuffSpellOnAttackerAction*>(&action) || dynamic_cast<FleeAction*>(&action)))
    {
        return 0.0f;
    }

    if (this->helper.IsPhaseOne())
    {
        if (dynamic_cast<CastTotemAction*>(&action) || dynamic_cast<CastShadowfiendAction*>(&action) ||
            dynamic_cast<CastRaiseDeadAction*>(&action) || dynamic_cast<CastFeignDeathAction*>(&action) ||
            dynamic_cast<CastInvisibilityAction*>(&action) || dynamic_cast<CastVanishAction*>(&action) ||
            dynamic_cast<PetAttackAction*>(&action))
        {
            return 0.0f;
        }
    }

    if (this->helper.IsPhaseTwo())
    {
        if (dynamic_cast<CastBlizzardAction*>(&action) || dynamic_cast<CastFrostNovaAction*>(&action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float AnubrekhanGenericMultiplier::GetValue(Action& action)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "anub'rekhan");

    if (bossValue == nullptr)
    {
        return 1.0f;
    }

    Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return 1.0f;
    }

    const bool hasLocustSwarmAuraById = NaxxSpellIds::HasAnyAura(
        this->botAI,
        boss,
        { NaxxSpellIds::LocustSwarm10, NaxxSpellIds::LocustSwarm10Alt, NaxxSpellIds::LocustSwarm25 }
    );
    const bool hasLocustSwarmAuraByName = this->botAI->HasAura("locust swarm", boss);
    const bool hasLocustSwarmAura = hasLocustSwarmAuraById || hasLocustSwarmAuraByName;

    if (hasLocustSwarmAura)
    {
        if (dynamic_cast<FleeAction*>(&action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float FourHorsemenGenericMultiplier::GetValue(Action& action)
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "sir zeliek");

    if (bossValue == nullptr)
    {
        return 1.0f;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return 1.0f;
    }

    Value<bool>* const neglectThreatValue = this->context->GetValue<bool>("neglect threat");

    if (neglectThreatValue == nullptr)
    {
        return 1.0f;
    }

    neglectThreatValue->Set(true);

    if ((dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float GluthGenericMultiplier::GetValue(Action& action)
{
    if (!this->helper.UpdateBossAI())
    {
        return 1.0f;
    }

    if ((dynamic_cast<DpsAssistAction*>(&action) || dynamic_cast<TankAssistAction*>(&action) ||
         dynamic_cast<FleeAction*>(&action) || dynamic_cast<CastDebuffSpellOnAttackerAction*>(&action) ||
         dynamic_cast<CastStarfallAction*>(&action)))
    {
        return 0.0f;
    }

    if (this->botAI->IsMainTank(bot))
    {
        const Aura* aura = NaxxSpellIds::GetAnyAura(this->bot, { NaxxSpellIds::MortalWound10, NaxxSpellIds::MortalWound25 });

        if (aura == nullptr)
        {
            // Fallback to name for custom spell data.
            aura = this->botAI->GetAura("mortal wound", bot, false, true);
        }
        if (aura != nullptr && aura->GetStackAmount() >= 5)
        {
            if (dynamic_cast<UniversalTauntAction*>(&action) || dynamic_cast<CastDarkCommandAction*>(&action) ||
                dynamic_cast<CastHandOfReckoningAction*>(&action) || dynamic_cast<CastGrowlAction*>(&action))
            {
                return 0.0f;
            }
        }
    }

    if (dynamic_cast<PetAttackAction*>(&action))
    {
        Value<Unit*>* const currentTargetValue = this->context->GetValue<Unit*>("current target");

        if (currentTargetValue == nullptr)
        {
            return 1.0f;
        }

        Unit* const target = currentTargetValue->Get();

        if (this->helper.IsZombieChow(target))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}
