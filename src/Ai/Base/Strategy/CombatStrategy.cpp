/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CombatStrategy.h"
#include "Strategy.h"

void CombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Pulled aggro (a boss or a pack picked a non-tank bot): pop a personal
    // defensive / threat-drop ability immediately instead of waiting for HP to
    // drop. Only the abilities the class actually has are registered, the
    // unknown ones are skipped by the engine.
    //
    // Deliberately NOT listed (already driven by class strategies, listing
    // them here would burn a global cooldown twice): fade (priest medium
    // threat), soulshatter (warlock), feint / cower (rogue / druid).
    // Divine Shield / Divine Protection are also excluded: protection and
    // retribution paladins lose their attacks while immune, so the bubble is
    // handled by a paladin-only trigger with a sudden-health-drop condition.
    triggers.push_back(
        new TriggerNode(
            "enemy looking at non tank",
            {
                NextAction("ice block", ACTION_EMERGENCY - 1),
                NextAction("dispersion", ACTION_EMERGENCY - 1),
                NextAction("icebound fortitude", ACTION_EMERGENCY - 1),
                NextAction("deterrence", ACTION_EMERGENCY - 2),
                NextAction("vanish", ACTION_EMERGENCY - 2),
                NextAction("invisibility", ACTION_EMERGENCY - 2),
                NextAction("feign death", ACTION_EMERGENCY - 2),
                NextAction("evasion", ACTION_EMERGENCY - 4),
                NextAction("barkskin", ACTION_EMERGENCY - 4),
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "enemy out of spell",
            {
                NextAction("reach spell", ACTION_HIGH)
            }
        )
    );
    // drop target relevance 99 (lower than Worldpacket triggers)
    triggers.push_back(
        new TriggerNode(
            "invalid target",
            {
                NextAction("drop target", 99)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mounted",
            {
                NextAction("check mount state", 54)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "combat stuck",
            {
                NextAction("reset", 1.0f)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "not facing target",
            {
                NextAction("set facing", ACTION_MOVE + 7)
            }
        )
    );
    // The pet-attack trigger is commented out because it was forcing the bot's pet to attack, overriding stay and follow commands.
    // Pets will automatically attack the bot's enemy if they are in "defensive" or "aggressive"
    // stance, or if the master issues an attack command.
}

AvoidAoeStrategy::AvoidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> AvoidAoeStrategy::getDefaultActions()
{
    return {
        NextAction("avoid aoe", ACTION_EMERGENCY)
    };
}

void AvoidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

void AvoidAoeStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}

TankFaceStrategy::TankFaceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> TankFaceStrategy::getDefaultActions()
{
    return {
        NextAction("tank face", ACTION_MOVE)
    };
}

void TankFaceStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

std::vector<NextAction> CombatFormationStrategy::getDefaultActions()
{
    return {
        NextAction("combat formation move", ACTION_NORMAL)
    };
}
