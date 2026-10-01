/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "Aq40Strategy.h"

#include "Aq40Actions.h"
#include "Playerbots.h"

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Arcane Explosion must be handled before Skeram platform travel or target pickup.
    triggers.push_back(new TriggerNode(
        "aq40 encounter",
        {NextAction("aq40 safety", ACTION_EMERGENCY + 2), NextAction("aq40 skeram interrupt", ACTION_RAID + 3),
         NextAction("aq40 tactics", ACTION_RAID + 2), NextAction("aq40 control", ACTION_RAID + 1),
         NextAction("aq40 position", ACTION_LIGHT_HEAL - 1)}));
    // Out of combat: marked Twin Emperors warlocks take their start spots. Ranked just above follow (1.0)
    // and below everything else, so summoning the Voidwalker, buffing and drinking still happen there.
    triggers.push_back(new TriggerNode("aq40 twins prepull", {NextAction("aq40 twins prepull", 2.0f)}));
}

void RaidAq40Strategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new Aq40Multiplier(botAI));
}

void RaidAq40Strategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type)
{
    if (type != TargetValueExclusionType::Dps)
        return;
    Aq40ControlAction* control = Aq40ControlAction::Get(botAI);
    if (control->Encounter() == Aq40::None)
        return;
    Unit* target = control->Target(botAI->GetBot());
    for (Creature* unit : control->Units())
        if (unit != target || !control->AllowedDamage(botAI->GetBot(), unit))
            exclusions.insert(unit->GetGUID());
    for (Player* player : control->Members())
        if (player->HasAura(Aq40::MindControl))
            exclusions.insert(player->GetGUID());
}
