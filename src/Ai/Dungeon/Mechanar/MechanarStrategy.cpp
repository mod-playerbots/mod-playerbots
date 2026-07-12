#include "MechanarTriggers.h"
#include "MechanarStrategy.h"
#include "MechanarMultipliers.h"

void TbcDungeonMechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Nethermancer Sepethrea: Raging Flames.

    // The fixated bot kites its flame. Highest priority (above all stock combat
    // movement) so nothing drags it back into the fire; the multiplier below zeros
    // the competing movers, and it only actually moves when the slow flame closes.
    triggers.push_back(new TriggerNode("sepethrea kite flame", {
        NextAction("sepethrea kite flame", ACTION_EMERGENCY + 6) }));

    // Anyone else standing in an Inferno steps out (safety for the fixate handoff).
    triggers.push_back(new TriggerNode("sepethrea avoid flame", {
        NextAction("sepethrea avoid flame", ACTION_EMERGENCY + 5) }));

    // Step out of the ribbon of persistent fire patches the elemental trails behind it.
    // A dedicated repulsion action (not the stock "avoid aoe"): it repels off the whole
    // overlapping trail in one hop, where the stock single-patch flee just hopped a bot
    // from one patch of the ribbon into the next and let combat move drag it back in.
    triggers.push_back(new TriggerNode("sepethrea trail", {
        NextAction("sepethrea avoid trail", ACTION_EMERGENCY + 4) }));

    // Everyone burns the boss, never the flame.
    triggers.push_back(new TriggerNode("sepethrea focus boss", {
        NextAction("sepethrea focus boss", ACTION_RAID + 2) }));
}

void TbcDungeonMechanarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SepethreaKiteFlameMultiplier(botAI));
    multipliers.push_back(new SepethreaTankFocusMultiplier(botAI));
}
