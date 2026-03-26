#include "RaidNaxxTriggers.h"

#include "RaidNaxxSpellIds.h"
#include "Timer.h"
#include "Trigger.h"

bool MutatingInjectionMeleeTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grobbulus");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return MutatingInjectionTrigger::IsActive() && !this->botAI->IsRanged(this->bot);
}

bool MutatingInjectionRangedTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grobbulus");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return MutatingInjectionTrigger::IsActive() && this->botAI->IsRanged(this->bot);
}

bool AuraRemovedTrigger::IsActive()
{
    const bool check = this->botAI->HasAura(name, this->bot, false, false, -1, true);
    bool ret = false;

    if (this->prev_check && !check)
    {
        ret = true;
    }

    this->prev_check = check;

    return ret;
}

bool MutatingInjectionRemovedTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grobbulus");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return HasNoAuraTrigger::IsActive() && this->botAI->GetState() == BOT_STATE_COMBAT && this->botAI->IsRanged(this->bot);
}

bool GrobbulusCloudTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grobbulus");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    if (!this->botAI->IsMainTank(this->bot))
    {
        return false;
    }

    Value<bool>* const hasAggroValue = this->context->GetValue<bool>("has aggro", "boss target");

    if (hasAggroValue == nullptr)
    {
        return false;
    }

    if (!hasAggroValue->Get())
    {
        return false;
    }

    const uint32_t now = getMSTime();
    bool poison_cloud_casting = false;

    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        const Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);

        if (!spell)
        {
            spell = boss->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        }

        if (spell)
        {
            poison_cloud_casting = NaxxSpellIds::MatchesAnySpellId(spell->GetSpellInfo(), {NaxxSpellIds::PoisonCloud});
        }
    }

    if (!poison_cloud_casting && last_cloud_ms != 0 && now - last_cloud_ms < CloudRotationDelayMs)
    {
        return false;
    }

    last_cloud_ms = now;

    return true;
}

bool RazuviousTankTrigger::IsActive()
{
    const Difficulty diff = this->bot->GetRaidDifficulty();

    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return this->helper.UpdateBossAI() && this->botAI->IsTank(bot);
    }

    return this->helper.UpdateBossAI() && bot->getClass() == CLASS_PRIEST;
}

bool RazuviousNontankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();

    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return this->helper.UpdateBossAI() && !(this->botAI->IsTank(this->bot));
    }

    return this->helper.UpdateBossAI() && !(this->bot->getClass() == CLASS_PRIEST);
}

bool FourHorsemenAttractorsTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsAttracter(this->bot);
}

bool FourHorsemenExceptAttractorsTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return !this->helper.IsAttracter(this->bot);
}

bool SapphironGroundTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsPhaseGround();
}

bool SapphironFlightTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsPhaseFlight();
}

bool GluthTrigger::IsActive()
{
    return this->helper.UpdateBossAI();
}

bool GluthMainTankMortalWoundTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    if (!this->botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }

    Value<Unit*>* const mainTankValue = this->context->GetValue<Unit*>("main tank");

    if (mainTankValue == nullptr)
    {
        return false;
    }

    Unit* const mt = mainTankValue->Get();

    if (mt == nullptr)
    {
        return false;
    }

    const Aura* aura = NaxxSpellIds::GetAnyAura(mt, {NaxxSpellIds::MortalWound10, NaxxSpellIds::MortalWound25});

    if (aura == nullptr)
    {
        // Fallback to name for custom spell data.
        aura = this->botAI->GetAura("mortal wound", mt, false, true);
    }

    if (aura == nullptr || aura->GetStackAmount() < 5)
    {
        return false;
    }

    return true;
}

bool KelthuzadTrigger::IsActive()
{
    return this->helper.UpdateBossAI();
}

bool AnubrekhanTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "anub'rekhan");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return true;
}

bool FaerlinaTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "grand widow faerlina");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return true;
}

bool MaexxnaTrigger::IsActive()
{
    Value<Unit*>* const bossValue = this->context->GetValue<Unit*>("find target", "maexxna");

    if (bossValue == nullptr)
    {
        return false;
    }

    const Unit* const boss = bossValue->Get();

    if (boss == nullptr)
    {
        return false;
    }

    return true;
}

bool LoathebTrigger::IsActive()
{
    return this->helper.UpdateBossAI();
}

bool ThaddiusPhasePetTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsPhasePet();
}

bool ThaddiusPhaseTransitionTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsPhaseTransition();
}

bool ThaddiusPhaseThaddiusTrigger::IsActive()
{
    if (!this->helper.UpdateBossAI())
    {
        return false;
    }

    return this->helper.IsPhaseThaddius();
}
