#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar is main tank", {
        new NextAction("high king maulgar main tank attack maulgar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is first assist tank", {
        new NextAction("high king maulgar first assist tank attack olm", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is second assist tank", {
        new NextAction("high king maulgar second assist tank attack blindeye", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is mage tank", {
        new NextAction("high king maulgar mage tank attack krosh", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar is moonkin tank", {
        new NextAction("high king maulgar moonkin tank attack kiggler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar determining kill order", {
        new NextAction("high king maulgar assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar healer in danger", {
        new NextAction("high king maulgar healer find safe position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar boss channeling whirlwind", {
        new NextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("high king maulgar wild felstalker spawned", {
        new NextAction("high king maulgar banish felstalker", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high king maulgar pulling olm and blindeye", {
        new NextAction("high king maulgar misdirect olm and blindeye", ACTION_RAID + 2) }));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by main tank", {
        new NextAction("gruul the dragonkiller main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by range", {
        new NextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller incoming shatter", {
        new NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6) }));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableArcaneShotOnKroshMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableMageTankAOEMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerMainTankMovementMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerGroundSlamMultiplier(botAI));
}
