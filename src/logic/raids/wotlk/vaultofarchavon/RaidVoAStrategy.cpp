#include "RaidVoAStrategy.h"
#include "Action.h"
#include "Strategy.h"
#include "Trigger.h"
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
}
