#ifndef PLAYERBOT_NEWRPGDOGATHER_H
#define PLAYERBOT_NEWRPGDOGATHER_H

#include "NewRpgBaseAction.h"

// Chat command "rpg do gather": force the bot into the gathering state in
// its current zone, mirroring "rpg do quest". Intended for maintainers to
// test the gather behavior on demand (requires the "new rpg" strategy to be
// active, e.g. via "nc +new rpg").
class StartRpgDoGatherAction : public Action
{
public:
    StartRpgDoGatherAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do gather") {}

    bool Execute(Event event) override;
};

class NewRpgDoGatherAction : public NewRpgBaseAction
{
public:
    NewRpgDoGatherAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do gather") {}
    bool Execute(Event event) override;

protected:
    // Release the loot pipeline's target if it points at the given spawn.
    // Called whenever the gather action abandons a node, so a half-done
    // harvest can't leave a stale loot target behind (which would gate off
    // loot-target replacement and wedge all subsequent harvesting).
    void ClearLootTargetForNode(ObjectGuid::LowType spawnId);

    // Drop the current node target (releasing the loot target with it) and
    // reset the per-node state; markVisited writes the spawn off for this
    // gather session.
    void AbandonNode(NewRpgInfo::DoGather& data, bool markVisited = false);

    // Max time waiting at a reached node for the loot pipeline to harvest
    // it before giving up and moving on.
    const uint32 nodeStayTime = 30 * IN_MILLISECONDS;
    // Cadence for the periodic target-liveness and switch checks.
    const uint32 passiveCheckInterval = 2 * IN_MILLISECONDS;
    // A live node passed within this range takes priority over the current
    // (possibly unverified) target.
    const float nodeSwitchDistance = 80.0f;
    // Required distance gain before switching to a live node that is not
    // in immediate pickup range.
    const float switchMinGain = 20.0f;
    // A live node this close is always picked up (if strictly closer than
    // the current target) - the cost of grabbing it is negligible and
    // skipping it reads as "ran past a herb".
    const float nodePickupDistance = 30.0f;
};

#endif
