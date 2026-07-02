#include "Aq40Multipliers.h"

#include <initializer_list>
#include <string>

#include "Action.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "Pet.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Spell.h"
#include "Action/Aq40Actions.h"
#include "Aq40BossHelper.h"
#include "Aq40SpellIds.h"
#include "Util/Aq40Helpers_Cthun.h"
#include "Util/Aq40Helpers_Shared.h"
#include "Util/Aq40Helpers_Skeram.h"
#include "Aq40Scripts.h"

namespace
{
float constexpr kTwinArcaneBurstAvoidRadius = 10.0f;

bool StartsWith(std::string const& value, char const* prefix)
{
    return value.compare(0, std::string(prefix).size(), prefix) == 0;
}

bool IsAq40Action(std::string const& actionName)
{
    return StartsWith(actionName, "aq40 ");
}

bool IsTwinAction(std::string const& actionName)
{
    return StartsWith(actionName, "aq40 twin ");
}

bool IsSharedAq40Action(std::string const& actionName)
{
    return actionName == "aq40 erase timers and trackers" ||
           actionName == "aq40 manage resistance strategies";
}

bool IsActionNamed(Action* action, std::initializer_list<char const*> names)
{
    if (!action)
        return false;

    std::string const actionName = action->getName();
    for (char const* name : names)
    {
        if (actionName == name)
            return true;
    }

    return false;
}

GuidVector GetActiveUnits(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return GuidVector();

    return Aq40BossHelper::GetActiveCombatUnits(botAI,
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
}

GuidVector GetEncounterUnits(PlayerbotAI* botAI)
{
    if (!botAI || !botAI->GetAiObjectContext())
        return GuidVector();

    return Aq40BossHelper::GetEncounterUnits(botAI,
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get());
}

bool HasActiveNamedUnit(PlayerbotAI* botAI, std::initializer_list<char const*> names)
{
    return Aq40BossHelper::HasAnyNamedUnit(botAI, GetActiveUnits(botAI), names);
}

bool HasActiveTwin(PlayerbotAI* botAI)
{
    GuidVector const activeUnits = GetActiveUnits(botAI);
    GuidVector const encounterUnits = GetEncounterUnits(botAI);
    return Aq40BossHelper::HasAnyNamedUnit(botAI, activeUnits, { "emperor vek'lor", "emperor vek'nilash" }) ||
           Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits) ||
           Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
}

bool IsAttackOrReachAction(Action* action)
{
    return dynamic_cast<AttackAction*>(action) ||
           dynamic_cast<ReachTargetAction*>(action) ||
           dynamic_cast<CastReachTargetSpellAction*>(action);
}

bool IsOffensiveSpellAction(Action* action)
{
    if (!dynamic_cast<CastSpellAction*>(action))
        return false;

    return !dynamic_cast<CastHealingSpellAction*>(action);
}

bool IsGenericPressureAction(Action* action)
{
    return IsAttackOrReachAction(action) ||
           IsOffensiveSpellAction(action) ||
           dynamic_cast<PetAttackAction*>(action) ||
           action->getName() == "shoot";
}

bool IsGenericMovementAction(Action* action)
{
    return dynamic_cast<CombatFormationMoveAction*>(action) ||
           dynamic_cast<FollowAction*>(action) ||
           dynamic_cast<FleeAction*>(action) ||
           dynamic_cast<MovementAction*>(action);
}

Unit* GetActionTarget(Player* bot, PlayerbotAI* botAI, Action* action)
{
    if (!bot || !botAI || !botAI->GetAiObjectContext() || !action)
        return nullptr;

    if (Unit* target = action->GetTarget())
        return target;

    if (Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get())
        return currentTarget;

    if (Unit* victim = bot->GetVictim())
        return victim;

    if (!bot->GetTarget().IsEmpty())
        return botAI->GetUnit(bot->GetTarget());

    return nullptr;
}

bool IsTargetingEntry(Unit* target, uint32 entry)
{
    return target && target->GetEntry() == entry;
}

bool PetIsTargetingVeklor(Player* bot)
{
    if (!bot)
        return false;

    Pet* pet = bot->GetPet();
    Unit* petTarget = pet ? pet->GetVictim() : nullptr;
    return IsTargetingEntry(petTarget, Aq40SpellIds::TwinVeklorNpcEntry);
}

bool IsTrashMovementCase(PlayerbotAI* botAI, Player* bot, GuidVector const& encounterUnits)
{
    if (!botAI || !bot || (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot)))
        return false;

    for (ObjectGuid const guid : encounterUnits)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell &&
            Aq40SpellIds::MatchesAnySpellId(spell->GetSpellInfo(), { Aq40SpellIds::Aq40DefenderThunderclap }) &&
            bot->GetDistance2d(unit) < 24.0f)
        {
            return true;
        }
    }

    return false;
}
}    // namespace

float Aq40GenericMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    std::string const actionName = action->getName();
    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::Aq40DefenderPlague }))
    {
        if (actionName == "aq40 trash avoid dangerous aoe")
            return 4.0f;

        if (IsGenericMovementAction(action))
            return 0.0f;

        if (!PlayerbotAI::IsRanged(bot) && !botAI->IsHeal(bot) && IsAttackOrReachAction(action))
            return 0.0f;
    }

    GuidVector const encounterUnits = GetEncounterUnits(botAI);
    if (!Aq40BossHelper::IsEncounterTank(bot, bot) && !encounterUnits.empty() &&
        !Aq40BossHelper::IsBossEncounterActive(botAI, encounterUnits) &&
        Aq40BossHelper::IsTrashEncounterActive(botAI, encounterUnits) &&
        IsTrashMovementCase(botAI, bot, encounterUnits))
    {
        if (actionName == "aq40 trash avoid dangerous aoe")
            return 3.5f;

        if (IsGenericMovementAction(action) || IsAttackOrReachAction(action))
            return 0.0f;
    }

    return 1.0f;
}

float Aq40SkeramMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    GuidVector const attackers = AI_VALUE(GuidVector, "attackers");
    if (!Aq40Helpers::IsSkeramEncounterLive(bot, botAI, attackers))
        return 1.0f;

    std::string const actionName = action->getName();
    if (StartsWith(actionName, "aq40 trash "))
        return 0.0f;

    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float Aq40BugTrioMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) ||
        !HasActiveNamedUnit(botAI, { "lord kri", "princess yauj", "vem", "yauj brood" }))
    {
        return 1.0f;
    }

    Unit* kri = Aq40BossHelper::FindUnitByAnyName(botAI, GetEncounterUnits(botAI), { "lord kri" });
    if (!kri || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    bool const poisonCloudWindow = kri->GetHealthPct() <= 5.0f ||
                                   Aq40SpellIds::HasAnyAura(botAI, kri, { Aq40SpellIds::BugTrioPoisonCloud });
    if (!poisonCloudWindow || bot->GetDistance2d(kri) > 12.0f)
        return 1.0f;

    if (dynamic_cast<Aq40BugTrioAvoidPoisonCloudAction*>(action))
        return 3.5f;

    if (IsGenericMovementAction(action) || IsAttackOrReachAction(action))
        return 0.0f;

    return 1.0f;
}

float Aq40SarturaMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    bool whirlwindRisk = false;
    bool const isBackline = botAI->IsRanged(bot) || botAI->IsHeal(bot);
    for (ObjectGuid const guid : GetEncounterUnits(botAI))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!Aq40BossHelper::IsSarturaSpinning(botAI, unit))
            continue;

        float const distance = bot->GetDistance2d(unit);
        bool const isClosingOnBot = unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID();
        if (distance <= 18.0f || (isBackline && isClosingOnBot && distance <= 24.0f))
        {
            whirlwindRisk = true;
            break;
        }
    }

    if (!whirlwindRisk)
        return 1.0f;

    if (dynamic_cast<Aq40SarturaAvoidWhirlwindAction*>(action))
        return 3.5f;

    if (dynamic_cast<CastReachTargetSpellAction*>(action) ||
        (dynamic_cast<MovementAction*>(action) && !dynamic_cast<Aq40SarturaAvoidWhirlwindAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float Aq40FankrissMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || !HasActiveNamedUnit(botAI, { "fankriss the unyielding" }))
        return 1.0f;

    std::string const actionName = action->getName();
    if (StartsWith(actionName, "aq40 trash "))
        return 0.0f;

    if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (!Aq40BossHelper::IsEncounterTank(bot, bot) &&
        (dynamic_cast<FleeAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action)))
    {
        return 0.0f;
    }

    return 1.0f;
}

float Aq40HuhuranMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || Aq40BossHelper::IsEncounterTank(bot, bot))
        return 1.0f;

    Unit* huhuran = Aq40BossHelper::FindUnitByAnyName(botAI, GetActiveUnits(botAI), { "princess huhuran" });
    if (!huhuran || (!botAI->IsRanged(bot) && !botAI->IsHeal(bot)))
        return 1.0f;

    bool const poisonPhase = huhuran->GetHealthPct() <= 32.0f ||
                             Aq40SpellIds::HasAnyAura(botAI, huhuran, { Aq40SpellIds::HuhuranFrenzy });
    if (!poisonPhase)
        return 1.0f;

    if (dynamic_cast<Aq40HuhuranPoisonSpreadAction*>(action))
        return 3.0f;

    if (IsGenericMovementAction(action))
        return 0.0f;

    return 1.0f;
}

float Aq40TwinMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) || !HasActiveTwin(botAI))
        return 1.0f;

    std::string const actionName = action->getName();
    if (IsTwinAction(actionName))
    {
        if (actionName == "aq40 twin avoid hazard")
            return 4.0f;
        if (actionName == "aq40 twin avoid veklor")
            return 3.5f;
        if (actionName == "aq40 twin tank" || actionName == "aq40 twin warlock tank")
            return 3.0f;
        if (actionName == "aq40 twin healer anchor")
            return 3.0f;
        if (actionName == "aq40 twin choose target")
            return 2.0f;

        return 1.0f;
    }

    if (IsAq40Action(actionName) && !IsSharedAq40Action(actionName))
        return 0.0f;

    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, GetEncounterUnits(botAI));
    Unit* target = GetActionTarget(bot, botAI, action);
    bool const isWarlockTank = Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI);
    bool const isTankPairMember = Aq40BossHelper::Twin::IsTankPairMember(bot);
    bool const isSelectedMeleeTank = Aq40BossHelper::Twin::IsSelectedMeleeTank(bot);
    bool const isAssignedHealer = botAI->IsHeal(bot) && Aq40BossHelper::Twin::IsAssignedHealer(bot);
    bool const targetsVeklor = IsTargetingEntry(target, Aq40SpellIds::TwinVeklorNpcEntry);
    bool const targetsVeknilash = IsTargetingEntry(target, Aq40SpellIds::TwinVeknilashNpcEntry);
    bool const targetsTwinBug = target && Aq40SpellIds::IsTwinBugEntry(target->GetEntry());
    bool const nearVeklorDanger = veklor && bot->GetDistance2d(veklor) <= kTwinArcaneBurstAvoidRadius;
    bool const isActiveVeklorTank =
        isWarlockTank && veklor && Aq40BossHelper::IsUnitFocusedOnPlayer(veklor, bot);

    if (isSelectedMeleeTank)
    {
        if (dynamic_cast<CastHealingSpellAction*>(action) && bot->GetHealthPct() > 35.0f)
            return 0.0f;

        if (IsGenericMovementAction(action) && !IsActionNamed(action, { "avoid aoe" }))
            return 0.0f;
    }

    if (isTankPairMember && IsGenericPressureAction(action))
        return 0.0f;

    if (isAssignedHealer && IsGenericMovementAction(action) &&
        !IsActionNamed(action,
            { "aq40 twin healer anchor", "aq40 twin avoid hazard", "aq40 twin avoid veklor", "avoid aoe" }))
    {
        return 0.0f;
    }

    if (targetsTwinBug && IsGenericPressureAction(action))
    {
        bool const canAttackBug = !botAI->IsHeal(bot) && !isTankPairMember &&
                                  (bot->getClass() == CLASS_HUNTER || PlayerbotAI::IsRanged(bot));
        if (!canAttackBug || !Aq40BossHelper::Twin::IsTwinKillBug(botAI, target))
            return 0.0f;
    }

    if (nearVeklorDanger && !isActiveVeklorTank)
    {
        if (IsGenericPressureAction(action) || dynamic_cast<CastReachTargetSpellAction*>(action))
            return 0.0f;

        if (IsGenericMovementAction(action) && !IsActionNamed(action, { "avoid aoe" }))
            return 0.0f;
    }

    if (targetsVeklor && Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI, false) &&
        IsGenericPressureAction(action))
        return 0.0f;

    if (targetsVeknilash && Aq40BossHelper::Twin::IsTrueCasterProfile(bot, botAI) &&
        IsGenericPressureAction(action))
        return 0.0f;

    if (!isWarlockTank && PetIsTargetingVeklor(bot) &&
        (dynamic_cast<PetAttackAction*>(action) || dynamic_cast<SetPetStanceAction*>(action) ||
         dynamic_cast<TogglePetSpellAutoCastAction*>(action)))
    {
        return 0.0f;
    }

    if ((Aq40Scripts::IsTwinBlizzardWindow(bot) || Aq40Scripts::IsTwinExplodeBugWindow(bot)) &&
        IsGenericMovementAction(action) &&
        !IsActionNamed(action, { "aq40 twin avoid hazard", "aq40 twin avoid veklor", "avoid aoe" }))
    {
        return 0.0f;
    }

    return 1.0f;
}

float Aq40OuroMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    Unit* ouro = Aq40BossHelper::FindUnitByAnyName(botAI, GetActiveUnits(botAI), { "ouro" });
    if (!ouro)
        return 1.0f;

    std::string const actionName = action->getName();
    bool const isEncounterTank = Aq40BossHelper::IsEncounterTank(bot, bot);

    if (actionName == "aq40 choose target")
        return 0.0f;

    if (isEncounterTank && bot->GetDistance2d(ouro) > 8.0f)
    {
        if (actionName == "aq40 ouro hold melee contact")
            return 3.0f;

        if (!dynamic_cast<MovementAction*>(action))
            return 0.5f;
    }

    if (!isEncounterTank && ouro->isInFront(bot, 10.0f) && bot->GetDistance2d(ouro) <= 15.0f)
    {
        if (dynamic_cast<Aq40OuroAvoidSandBlastAction*>(action))
            return 3.5f;

        if (IsGenericMovementAction(action) &&
            !dynamic_cast<Aq40OuroAvoidSweepAction*>(action) &&
            !dynamic_cast<Aq40OuroAvoidSubmergeAction*>(action))
        {
            return 0.0f;
        }
    }

    return 1.0f;
}

float Aq40ViscidusMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot))
        return 1.0f;

    Unit* viscidus = Aq40BossHelper::FindUnitByAnyName(botAI, GetActiveUnits(botAI), { "viscidus" });
    if (!viscidus)
        return 1.0f;

    std::string const actionName = action->getName();
    if (actionName == "aq40 choose target")
        return 0.0f;

    bool const frozen = Aq40SpellIds::HasAnyAura(botAI, viscidus, { Aq40SpellIds::ViscidusFreeze });
    if (frozen)
    {
        if (actionName == "aq40 viscidus shatter")
            return 2.8f;
        if (actionName == "aq40 viscidus use frost")
            return 0.0f;
    }
    else
    {
        if (actionName == "aq40 viscidus use frost" && !botAI->IsHeal(bot) &&
            !Aq40BossHelper::IsEncounterTank(bot, bot))
        {
            return 2.2f;
        }
        if (actionName == "aq40 viscidus shatter")
            return 0.4f;
    }

    return 1.0f;
}

float Aq40CthunMultiplier::GetValue(Action* action)
{
    if (!action || !Aq40BossHelper::IsInAq40(bot) ||
        !HasActiveNamedUnit(botAI, { "c'thun", "eye of c'thun", "eye tentacle", "claw tentacle",
                                    "giant eye tentacle", "giant claw tentacle", "flesh tentacle" }))
    {
        return 1.0f;
    }

    std::string const actionName = action->getName();
    if (actionName == "aq40 choose target")
        return 0.0f;

    bool const inStomach = Aq40Helpers::IsCthunInStomach(bot, botAI);
    if (actionName == "aq40 cthun maintain spread")
        return inStomach ? 0.0f : 1.0f;

    if (inStomach)
    {
        if (dynamic_cast<Aq40CthunStomachExitAction*>(action))
        {
            Aura* acid = Aq40SpellIds::GetAnyAura(bot, { Aq40SpellIds::CthunDigestiveAcid });
            if (!acid)
                acid = botAI->GetAura("digestive acid", bot, false, true);

            uint32 exitStacks = Aq40BossHelper::IsEncounterPrimaryTank(bot, bot) ? 1 : (botAI->IsHeal(bot) ? 5 : 10);
            if (acid && acid->GetStackAmount() >= exitStacks)
                return 4.0f;
        }

        if (dynamic_cast<Aq40CthunStomachDpsAction*>(action))
            return 3.0f;

        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<Aq40CthunStomachExitAction*>(action))
            return 0.0f;
    }

    Unit* eye = Aq40BossHelper::FindUnitByAnyName(botAI, GetEncounterUnits(botAI), { "eye of c'thun" });
    bool const darkGlare = eye &&
        ((eye->GetCurrentSpell(CURRENT_GENERIC_SPELL) &&
          Aq40SpellIds::MatchesAnySpellId(eye->GetCurrentSpell(CURRENT_GENERIC_SPELL)->GetSpellInfo(),
              { Aq40SpellIds::CthunDarkGlare })) ||
         Aq40SpellIds::HasAnyAura(botAI, eye, { Aq40SpellIds::CthunDarkGlare }) ||
         botAI->HasAura("dark glare", eye));

    if (darkGlare && dynamic_cast<Aq40CthunAvoidDarkGlareAction*>(action))
        return 4.0f;

    if (Aq40Helpers::IsCthunVulnerableNow(botAI, GetEncounterUnits(botAI)) &&
        dynamic_cast<Aq40CthunVulnerableBurstAction*>(action))
    {
        return 3.0f;
    }

    return 1.0f;
}
