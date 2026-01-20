#include "RaidMcMultipliers.h"

#include "Playerbots.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "DruidActions.h"
#include "HunterActions.h"
#include "PaladinActions.h"
#include "ShamanActions.h"
#include "WarriorActions.h"
#include "DKActions.h"
#include "RaidMcActions.h"
#include "RaidMcHelpers.h"

using namespace MoltenCoreHelpers;

static bool IsDpsBotWithAoeAction(Player* bot, Action* action)
{
    if (PlayerbotAI::IsDps(bot))
    {
        if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastConsecrationAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<CastWhirlwindAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastExplosiveTrapAction*>(action) ||
            dynamic_cast<CastDeathAndDecayAction*>(action))
            return true;

        if (auto castSpellAction = dynamic_cast<CastSpellAction*>(action))
        {
            if (castSpellAction->getThreatType() == Action::ActionThreatType::Aoe)
                return true;
        }
    }
    return false;
}

float GarrDisableDpsAoeMultiplier::GetValue(Action* action)
{
    if (AI_VALUE2(Unit*, "find target", "garr"))
    {
        if (IsDpsBotWithAoeAction(bot, action))
            return 0.0f;
    }
    return 1.0f;
}

static bool IsAllowedGeddonMovementAction(Action* action)
{
    if (dynamic_cast<MovementAction*>(action) &&
                !dynamic_cast<McMoveFromGroupAction*>(action) &&
                !dynamic_cast<McMoveFromBaronGeddonAction*>(action))
        return false;

    if (dynamic_cast<CastReachTargetSpellAction*>(action))
        return false;

    return true;
}

float BaronGeddonAbilityMultiplier::GetValue(Action* action)
{
    if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
    {
        if (boss->HasAura(SPELL_INFERNO))
        {
            if (!IsAllowedGeddonMovementAction(action))
                return 0.0f;
        }
    }

    // No check for Baron Geddon, because bots may have the bomb even after Geddon died.
    if (bot->HasAura(SPELL_LIVING_BOMB))
    {
        if (!IsAllowedGeddonMovementAction(action))
            return 0.0f;
    }

    return 1.0f;
}

static bool IsSingleLivingTankInGroup(Player* bot)
{
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || member == bot)
                continue;
            if (PlayerbotAI::IsTank(member))
                return false;
        }
    }
    return true;
}

float GolemaggMultiplier::GetValue(Action* action)
{
    if (AI_VALUE2(Unit*, "find target", "golemagg the incinerator"))
    {
        if (PlayerbotAI::IsTank(bot) && IsSingleLivingTankInGroup(bot))
        {
            // Only one tank => Pick up Golemagg and the two Core Ragers
            if (dynamic_cast<McGolemaggMainTankAttackGolemaggAction*>(action) ||
                dynamic_cast<McGolemaggAssistTankAttackCoreRagerAction*>(action))
                return 0.0f;
        }
        if (PlayerbotAI::IsAssistTank(bot))
        {
            // The first two assist tanks manage the Core Ragers. The remaining assist tanks attack the boss.
            if (dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
        if (IsDpsBotWithAoeAction(bot, action))
            return 0.0f;
    }
    return 1.0f;
}
