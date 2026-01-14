#include "RaidNaxxTriggers.h"

#include "Playerbots.h"
#include "RaidNaxxSpellIds.h"
#include "Timer.h"
#include "Trigger.h"

bool MutatingInjectionMeleeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return false;
    }
    return MutatingInjectionTrigger::IsActive() && !botAI->IsRanged(bot);
}

bool MutatingInjectionRangedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return false;
    }
    return MutatingInjectionTrigger::IsActive() && botAI->IsRanged(bot);
}

bool AuraRemovedTrigger::IsActive()
{
    bool check = botAI->HasAura(name, bot, false, false, -1, true);
    bool ret = false;
    if (prev_check && !check)
    {
        ret = true;
    }
    prev_check = check;
    return ret;
}

bool MutatingInjectionRemovedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return false;
    }
    return HasNoAuraTrigger::IsActive() && botAI->GetState() == BOT_STATE_COMBAT && botAI->IsRanged(bot);
}

bool GrobbulusCloudTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grobbulus");
    if (!boss)
    {
        return false;
    }
    if (!botAI->IsMainTank(bot))
    {
        return false;
    }
    // bot->Yell("has aggro on " + boss->GetName() + " : " + to_string(AI_VALUE2(bool, "has aggro", "boss target")),
    // LANG_UNIVERSAL);
    if (!AI_VALUE2(bool, "has aggro", "boss target"))
    {
        return false;
    }
    uint32 now = getMSTime();
    bool poison_cloud_casting = false;
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        Spell* spell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
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

bool HeiganMeleeTrigger::IsActive()
{
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan)
    {
        return false;
    }
    return !botAI->IsRanged(bot);
}

bool HeiganRangedTrigger::IsActive()
{
    Unit* heigan = AI_VALUE2(Unit*, "find target", "heigan the unclean");
    if (!heigan)
    {
        return false;
    }
    return botAI->IsRanged(bot);
}

bool RazuviousTankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return helper.UpdateBossAI() && botAI->IsTank(bot);
    }
    return helper.UpdateBossAI() && bot->getClass() == CLASS_PRIEST;
}

bool RazuviousNontankTrigger::IsActive()
{
    Difficulty diff = bot->GetRaidDifficulty();
    if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
    {
        return helper.UpdateBossAI() && !(botAI->IsTank(bot));
    }
    return helper.UpdateBossAI() && !(bot->getClass() == CLASS_PRIEST);
}

bool HorsemanAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsAttracter(bot);
}

bool HorsemanExceptAttractorsTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return !helper.IsAttracter(bot);
}

bool SapphironGroundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseGround();
}

bool SapphironFlightTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseFlight();
}

bool GluthTrigger::IsActive() { return helper.UpdateBossAI(); }

bool GluthMainTankMortalWoundTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    if (!botAI->IsAssistTankOfIndex(bot, 0))
    {
        return false;
    }
    Unit* mt = AI_VALUE(Unit*, "main tank");
    if (!mt)
    {
        return false;
    }
    Aura* aura = NaxxSpellIds::GetAnyAura(mt, {NaxxSpellIds::MortalWound10, NaxxSpellIds::MortalWound25});
    if (!aura)
    {
        // Fallback to name for custom spell data.
        aura = botAI->GetAura("mortal wound", mt, false, true);
    }
    if (!aura || aura->GetStackAmount() < 5)
    {
        return false;
    }
    return true;
}

bool KelthuzadTrigger::IsActive() { return helper.UpdateBossAI(); }

bool AnubrekhanTrigger::IsActive() {
    Unit* boss = AI_VALUE2(Unit*, "find target", "anub'rekhan");
    if (!boss)
    {
        return false;
    }
    return true;
}

bool FaerlinaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand widow faerlina");
    if (!boss)
    {
        return false;
    }
    return true;
}

bool MaexxnaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "maexxna");
    if (!boss)
    {
        return false;
    }
    return !botAI->IsTank(bot);
}

bool PatchwerkTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
    {
        return false;
    }
    return !botAI->IsTank(bot) && !botAI->IsRanged(bot);
}

bool PatchwerkRangedTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
    {
        return false;
    }
    return !botAI->IsTank(bot) && botAI->IsRanged(bot);
}

bool PatchwerkNonTankTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "patchwerk");
    if (!boss)
    {
        return false;
    }
    return !botAI->IsTank(bot);
}

bool LoathebTrigger::IsActive() { return helper.UpdateBossAI(); }

bool ThaddiusPhasePetTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhasePet();
}

bool ThaddiusPhaseTransitionTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseTransition();
}

bool ThaddiusPhaseThaddiusTrigger::IsActive()
{
    if (!helper.UpdateBossAI())
    {
        return false;
    }
    return helper.IsPhaseThaddius();
}
