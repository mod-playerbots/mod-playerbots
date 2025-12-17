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
        { new NextAction("emalon lighting nova action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "emalon mark boss trigger",
        { new NextAction("emalon mark boss action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon overcharge trigger",
        { new NextAction("emalon overcharge action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon fall from floor trigger",
        { new NextAction("emalon fall from floor action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "emalon nature resistance trigger",
        { new NextAction("emalon nature resistance action", ACTION_RAID) }));

    //
    // Koralon the Flame Watcher
    //

    triggers.push_back(new TriggerNode(
        "koralon fire resistance trigger",
        { new NextAction("koralon fire resistance action", ACTION_RAID) }));
}
