#include "GruulStrategy.h"
#include "GruulMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar boss engaged by main tank", {
        NextAction("high king maulgar main tank attack maulgar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar olm engaged by first assist tank", {
        NextAction("high king maulgar first assist tank attack olm", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar blindeye engaged by second assist tank", {
        NextAction("high king maulgar second assist tank attack blindeye", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar krosh engaged by mage tank", {
        NextAction("high king maulgar mage tank attack krosh", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar kiggler engaged by moonkin tank", {
        NextAction("high king maulgar moonkin tank attack kiggler", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar determining kill order", {
        NextAction("high king maulgar assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high king maulgar boss channeling whirlwind", {
        NextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("high king maulgar krosh casts blast wave", {
        NextAction("high king maulgar move away from blast nova danger", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("high king maulgar wild fel stalker spawned", {
        NextAction("high king maulgar banish fel stalker", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high king maulgar pulling ogre council", {
        NextAction("high king maulgar misdirect ogres to tanks", ACTION_RAID + 2) }));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by tanks", {
        NextAction("gruul the dragonkiller tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller boss engaged by ranged", {
        NextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gruul the dragonkiller incoming shatter", {
        NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6) }));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // High King Maulgar
    multipliers.push_back(new HighKingMaulgarDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarControlTankActionsMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableArcaneShotOnKroshMultiplier(botAI));
    multipliers.push_back(new HighKingMaulgarDisableMageTankAoeMultiplier(botAI));

    // Gruul the Dragonkiller
    multipliers.push_back(new GruulTheDragonkillerDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerControlTankMovementMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerStaySpreadForShatterMultiplier(botAI));
}
