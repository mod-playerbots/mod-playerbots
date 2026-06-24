#include "NaxxActions.h"

#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"

namespace
{
constexpr uint32 SPELL_UNDERSTUDY_TAUNT = 29060;
constexpr uint32 SPELL_BONE_BARRIER     = 29061;
constexpr uint32 SPELL_BLOOD_STRIKE     = 61696;
}

bool RazuviousUseObedienceCrystalAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    // bot->GetCharm
    if (Unit* charm = bot->GetCharm())
    {
        Unit* target = AI_VALUE2(Unit*, "find target", "instructor razuvious");
        if (!target)
            return false;

        if (charm->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_ACTIVE) == NULL_MOTION_TYPE)
        {
            charm->GetMotionMaster()->Clear();
            charm->GetMotionMaster()->MoveChase(target);
            charm->GetAI()->AttackStart(target);
        }
        Aura* forceObedience = botAI->GetAura("force obedience", charm);
        uint32 duration_time;
        if (!forceObedience)
        {
            forceObedience = botAI->GetAura("mind control", charm);
            duration_time = 60000;
        }
        else
            duration_time = 90000;

        if (!forceObedience)
            return false;

        if (charm->GetDistance(target) <= 0.51f)
        {
            // taunt
            bool tauntUseful = true;
            if (forceObedience->GetDuration() <= (duration_time - 5000))
            {
                Unit* victim = target->GetVictim();
                if (victim && victim->HasAura(SPELL_BONE_BARRIER))
                    tauntUseful = false;

                if (forceObedience->GetDuration() <= 3000)
                    tauntUseful = false;

            }
            if (forceObedience->GetDuration() >= (duration_time - 500))
                tauntUseful = false;

            if (tauntUseful && !charm->HasSpellCooldown(29060))
            {
                // shield
                if (!charm->HasSpellCooldown(SPELL_BONE_BARRIER))
                {
                    charm->CastSpell(charm, SPELL_BONE_BARRIER, true);
                    charm->AddSpellCooldown(SPELL_BONE_BARRIER, 0, 30 * 1000);
                }
                charm->CastSpell(target, SPELL_UNDERSTUDY_TAUNT, true);
                charm->AddSpellCooldown(SPELL_UNDERSTUDY_TAUNT, 0, 20 * 1000);
            }
            // strike
            if (!charm->HasSpellCooldown(SPELL_BLOOD_STRIKE))
            {
                charm->CastSpell(target, SPELL_BLOOD_STRIKE, true);
                charm->AddSpellCooldown(SPELL_BLOOD_STRIKE, 0, 4 * 1000);
            }
        }
    }
    else
    {
        Difficulty diff = bot->GetRaidDifficulty();
        if (diff == RAID_DIFFICULTY_10MAN_NORMAL)
        {
            GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
            for (auto i = npcs.begin(); i != npcs.end(); i++)
            {
                Creature* unit = botAI->GetCreature(*i);
                if (!unit)
                    continue;

                if (botAI->IsMainTank(bot) && unit->GetSpawnId() != 128352)
                    continue;

                if (!botAI->IsMainTank(bot) && unit->GetSpawnId() != 128353)
                    continue;

                if (MoveTo(unit, 0.0f, MovementPriority::MOVEMENT_COMBAT))
                    return true;

                Creature* creature = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_SPELLCLICK);
                if (!creature)
                    continue;

                creature->HandleSpellClick(bot);
                return true;
            }
        }
        else
        {
            GuidVector attackers = context->GetValue<GuidVector>("attackers")->Get();
            Unit* target = nullptr;
            for (auto i = attackers.begin(); i != attackers.end(); ++i)
            {
                Unit* unit = botAI->GetUnit(*i);
                if (!unit)
                    continue;
                if (botAI->EqualLowercaseName(unit->GetName(), "death knight understudy"))
                {
                    target = unit;
                    break;
                }
            }
            if (target)
            {
                if (bot->GetDistance2d(target) > sPlayerbotAIConfig.spellDistance)
                    return MoveNear(target, sPlayerbotAIConfig.spellDistance, MovementPriority::MOVEMENT_COMBAT);
                else
                    return botAI->CastSpell("mind control", target);
            }
        }
    }
    return false;
}

bool RazuviousTargetAction::Execute(Event /*event*/)
{
    if (!helper.UpdateBossAI())
        return false;

    Unit* razuvious = AI_VALUE2(Unit*, "find target", "instructor razuvious");
    Unit* understudy = AI_VALUE2(Unit*, "find target", "death knight understudy");
    Unit* target = nullptr;
    if (botAI->IsTank(bot))
        target = understudy;
    else
        target = razuvious;

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}
