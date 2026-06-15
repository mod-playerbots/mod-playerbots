#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "Multiplier.h"
#include "Playerbots.h"
#include "ICCActions.h"
#include "ICCTriggers.h"

bool IccDogsTankPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "stinky");
    if (!boss)
        boss = AI_VALUE2(Unit*, "find target", "precious");
    if (!boss)
        return false;

    auto CastClassTaunt = [&](Unit* target) -> bool
    {
        if (!target || !target->IsAlive())
            return false;

        switch (bot->getClass())
        {
            case CLASS_PALADIN:
            {
                bot->RemoveSpellCooldown(SPELL_TAUNT_PALADIN, true);
                if (botAI->CastSpell("hand of reckoning", target))
                    return true;
                break;
            }
            case CLASS_DEATH_KNIGHT:
            {
                bot->RemoveSpellCooldown(SPELL_TAUNT_DK, true);
                if (botAI->CastSpell("dark command", target))
                    return true;
                break;
            }
            case CLASS_DRUID:
            {
                bot->RemoveSpellCooldown(SPELL_TAUNT_DRUID, true);
                if (botAI->CastSpell("growl", target))
                    return true;
                break;
            }
            case CLASS_WARRIOR:
            {
                bot->RemoveSpellCooldown(SPELL_TAUNT_WARRIOR, true);
                if (botAI->CastSpell("taunt", target))
                    return true;
                break;
            }
            default:
                break;
        }

        if (botAI->CastSpell("shoot", target) || botAI->CastSpell("throw", target))
            return true;

        return false;
    };

    if (botAI->IsTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        bool const hasMortalWound = aura && aura->GetStackAmount() >= 8;

        if (hasMortalWound)
        {
            if (bot->GetVictim() == boss)
                bot->AttackStop();

            return true;
        }

        // Tank without high mortal wound stacks: taunt boss if current tank has the debuff
        Unit* currentTarget = boss->GetVictim();
        if (currentTarget && currentTarget != bot)
        {
            Aura* victimAura = botAI->GetAura("mortal wound", currentTarget, false, true);
            if (victimAura && victimAura->GetStackAmount() >= 8)
                CastClassTaunt(boss);
        }

        // Taunt nearby hostile adds not targeting a tank
        GuidVector const npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto const& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;

            if (unit == boss)
                continue;

            if (bot->GetDistance2d(unit) > 20.0f)
                continue;

            Unit* victim = unit->GetVictim();
            Player* victimPlayer = victim ? victim->ToPlayer() : nullptr;
            if (!victimPlayer || !botAI->IsTank(victimPlayer))
            {
                CastClassTaunt(unit);
                break;
            }
        }

        return false;
    }

    return false;
}
