#include "RaidMcActions.h"

#include "Playerbots.h"
#include "RtiTargetValue.h"
#include "RaidMcTriggers.h"
#include "RaidMcHelpers.h"

static constexpr float LIVING_BOMB_DISTANCE = 20.0f;
static constexpr float INFERNO_DISTANCE = 20.0f;

// don't get hit by Arcane Explosion but still be in casting range
static constexpr float ARCANE_EXPLOSION_DISTANCE = 26.0f;

// dedicated tank positions; prevents assist tanks from positioning Core Ragers on steep walls on pull
static const Position GOLEMAGG_TANK_POSITION{795.7308, -994.8848, -207.18661};
static const Position CORE_RAGER_TANK_POSITION{846.6453, -1019.0639, -198.9819};

static constexpr float GOLEMAGGS_TRUST_DISTANCE = 30.0f;
static constexpr float CORE_RAGER_STEP_DISTANCE = 5.0f;

using namespace MoltenCoreHelpers;

bool McMoveFromGroupAction::Execute(Event event)
{
    return MoveFromGroup(LIVING_BOMB_DISTANCE);
}

bool McMoveFromBaronGeddonAction::Execute(Event event)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        float distToTravel = INFERNO_DISTANCE - bot->GetDistance(boss);
        if (distToTravel > 0)
        {
            // Stop current spell first
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(false);

            return MoveAway(boss, distToTravel);
        }
    }
    return false;
}

bool McShazzrahMoveAwayAction::Execute(Event event)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "shazzrah"))
    {
        float distToTravel = ARCANE_EXPLOSION_DISTANCE - bot->GetDistance(boss);
        if (distToTravel > 0)
            return MoveAway(boss, distToTravel);
    }
    return false;
}

bool McGolemaggMarkBossAction::Execute(Event event)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        if (Group* group = bot->GetGroup())
        {
            ObjectGuid currentSkullGuid = group->GetTargetIcon(RtiTargetValue::skullIndex);
            if (currentSkullGuid.IsEmpty() || currentSkullGuid != boss->GetGUID())
            {
                group->SetTargetIcon(RtiTargetValue::skullIndex, bot->GetGUID(), boss->GetGUID());
                return true;
            }
        }
    }
    return false;
}

bool McGolemaggTankAction::MoveUnitToPosition(Unit* target, const Position& tankPosition, float maxDistance)
{
    if (bot->GetVictim() != target)
        return Attack(target);
    if (target->GetVictim() == bot)
    {
        float distanceToTankPosition = bot->GetExactDist2d(tankPosition.GetPositionX(), tankPosition.GetPositionY());
        if (distanceToTankPosition > maxDistance)
        {
            float dX = tankPosition.GetPositionX() - bot->GetPositionX();
            float dY = tankPosition.GetPositionY() - bot->GetPositionY();
            float dist = sqrt(dX * dX + dY * dY);
            float moveX = bot->GetPositionX() + (dX / dist) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / dist) * maxDistance;
            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true,
                          true);
        }

        float orientation = atan2(target->GetPositionY() - bot->GetPositionY(),
                                  target->GetPositionX() - bot->GetPositionX());
        bot->SetFacingTo(orientation);
    }
    else if (!bot->IsWithinMeleeRange(target))
    {
        return MoveTo(target->GetMapId(), target->GetPositionX(), target->GetPositionY(),
                      target->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }
    return false;
}

bool McGolemaggTankAction::FindCoreRagers(Unit*& coreRager1, Unit*& coreRager2) const
{
    coreRager1 = coreRager2 = nullptr;
    for (auto const& target : AI_VALUE(GuidVector, "possible targets no los"))
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_CORE_RAGER)
        {
            if (coreRager1 == nullptr)
                coreRager1 = unit;
            else if (coreRager2 == nullptr)
            {
                coreRager2 = unit;
                break; // There should be no third Core Rager.
            }
        }
    }
    return coreRager1 != nullptr && coreRager2 != nullptr;
}

bool McGolemaggMainTankAttackGolemaggAction::Execute(Event event)
{
    // At this point, we know we are not the last living tank in the group.
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        Unit* coreRager1;
        Unit* coreRager2;
        if (!FindCoreRagers(coreRager1, coreRager2))
            return false; // safety check

        // We only need to move if the Core Ragers still have Golemagg's Trust
        if (coreRager1->HasAura(SPELL_GOLEMAGGS_TRUST) || coreRager2->HasAura(SPELL_GOLEMAGGS_TRUST))
            return MoveUnitToPosition(boss, GOLEMAGG_TANK_POSITION, boss->GetCombatReach());
    }
    return false;
}

bool McGolemaggAssistTankAttackCoreRagerAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "golemagg the incinerator");
    if (!boss)
        return false;

    // Step 0: Filter additional assist tanks. We only need 2.
    if (PlayerbotAI::GetAssistTankIndex(bot) > 1)
        return Attack(boss);

    // Step 1: Find both Core Ragers
    Unit* coreRager1;
    Unit* coreRager2;
    if (!FindCoreRagers(coreRager1, coreRager2))
        return false; // safety check

    // Step 2: Assign Core Rager to bot
    Unit* myCoreRager = nullptr;
    Unit* otherCoreRager = nullptr;
    if (PlayerbotAI::IsAssistTankOfIndex(bot, 0))
    {
        myCoreRager = coreRager1;
        otherCoreRager = coreRager2;
    }
    else if (PlayerbotAI::IsAssistTankOfIndex(bot, 1))
    {
        myCoreRager = coreRager2;
        otherCoreRager = coreRager1;
    }
    else
        return false; // safety check

    // Step 3: Select the right target
    if (myCoreRager->GetVictim() != bot)
    {
        // My Core Rager isn't attacking me. Attack until it does.
        return Attack(myCoreRager);
    }

    Unit* otherCoreRagerVictim = otherCoreRager->GetVictim();
    if (otherCoreRagerVictim) // Core Rager victim can be NULL
    {
        Player* otherCoreRagerPlayerVictim = otherCoreRagerVictim->ToPlayer();
        if (otherCoreRagerPlayerVictim &&
            !PlayerbotAI::IsAssistTankOfIndex(otherCoreRagerPlayerVictim, 0) &&
            !PlayerbotAI::IsAssistTankOfIndex(otherCoreRagerPlayerVictim, 1))
        {
            // Assume we are the only assist tank or the other assist tank is dead => pick up other Core Rager!
            return Attack(otherCoreRager);
        }
    }

    if (bot->GetVictim() == nullptr)
    {
        // Make sure, we are attacking something. Otherwise, we might start moving without ever attacking.
        return Attack(myCoreRager);
    }

    // Step 4: Prevent Golemagg's Trust on Core Ragers
    if (myCoreRager->HasAura(SPELL_GOLEMAGGS_TRUST) ||
        (otherCoreRagerVictim == bot && otherCoreRager->HasAura(SPELL_GOLEMAGGS_TRUST)))
    {
        // Step 4.1: Move Core Ragers to dedicated tank position (only if Golemagg is far enough away from said position)
        float bossDistanceToCoreRagerTankPosition = boss->GetExactDist2d(
            CORE_RAGER_TANK_POSITION.GetPositionX(), CORE_RAGER_TANK_POSITION.GetPositionY());
        if (bossDistanceToCoreRagerTankPosition > GOLEMAGGS_TRUST_DISTANCE)
        {
            float distanceToTankPosition = bot->GetExactDist2d(CORE_RAGER_TANK_POSITION.GetPositionX(),
                                                               CORE_RAGER_TANK_POSITION.GetPositionY());
            if (distanceToTankPosition > CORE_RAGER_STEP_DISTANCE)
                return MoveUnitToPosition(myCoreRager, CORE_RAGER_TANK_POSITION, CORE_RAGER_STEP_DISTANCE);
        }

        // Step 4.2: if boss is too close to tank position, or we are already there, move away from Golemagg to try to out-range Golemagg's Trust
        return MoveAway(boss, CORE_RAGER_STEP_DISTANCE, true);
    }

    return false;
}
