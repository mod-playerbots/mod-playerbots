/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VoAStrategy.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "Strategy.h"
#include "Trigger.h"
#include "VoATriggers.h"
#include "vector"

void RaidVoAStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Emalon the Storm Watcher
    //
    triggers.push_back(new TriggerNode(
        "emalon lighting nova trigger",
        { NextAction("emalon lighting nova action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "emalon mark boss trigger",
        { NextAction("emalon mark boss action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon overcharge trigger",
        { NextAction("emalon overcharge action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon fall from floor trigger",
        { NextAction("emalon fall from floor action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon nature resistance trigger",
        { NextAction("emalon nature resistance action", ACTION_RAID) }));

    //
    // Koralon the Flame Watcher
    //

    triggers.push_back(new TriggerNode(
        "koralon fire resistance trigger",
        { NextAction("koralon fire resistance action", ACTION_RAID) }));

    //
    // Toravon the Ice Watcher
    //
    // Ice orbs are a DPS job. The tank must keep the boss still: chasing an
    // orb turns the boss and its breath/cleave sweeps the raid. The action
    // itself refuses tanks, and AppendTargetExclusions removes orbs from the
    // tank target value so the tank never even walks toward them.
    triggers.push_back(new TriggerNode(
        "voa toravon frozen orb",
        { NextAction("voa toravon frozen orb action", ACTION_RAID + 2) }));

    // Frozen Mallet stacks on the tank: the off tank taunts at 3 stacks.
    triggers.push_back(new TriggerNode(
        "voa toravon tank swap",
        { NextAction("taunt spell", ACTION_EMERGENCY) }));
}

void RaidVoAStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::TankTarget)
        return;

    // Called for every tank target-value recalculation: bail out with the
    // cheapest check first.
    if (botAI->GetBot()->GetMapId() != 624)
        return;

    // Strategies have no AI_VALUE macro (no action context), so read the
    // values through the context directly.
    AiObjectContext* context = botAI->GetAiObjectContext();

    // Only during the Toravon fight.
    Unit* boss = context->GetValue<Unit*>("find target", "toravon the ice watcher")->Get();
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return;

    // Remove every live ice orb from the tank's candidate targets.
    for (ObjectGuid const guid : context->GetValue<GuidVector>("nearest hostile npcs")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        uint32 entry = unit->GetEntry();
        if (entry == NPC_FROZEN_ORB_10 || entry == NPC_FROZEN_ORB_25)
            exclusions.insert(guid);
    }
}
