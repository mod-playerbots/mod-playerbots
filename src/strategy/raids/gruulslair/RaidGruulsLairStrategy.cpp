#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High King Maulgar
    triggers.push_back(new TriggerNode("high king maulgar maulgar tank", NextAction::array(0,
        new NextAction("high king maulgar maulgar tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar olm tank", NextAction::array(0,
        new NextAction("high king maulgar olm tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar blindeye tank", NextAction::array(0,
        new NextAction("high king maulgar blindeye tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar krosh mage tank", NextAction::array(0,
        new NextAction("high king maulgar krosh mage tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar kiggler moonkin tank", NextAction::array(0,
        new NextAction("high king maulgar kiggler moonkin tank", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar melee dps priority", NextAction::array(0,
        new NextAction("high king maulgar melee dps priority", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar ranged dps priority", NextAction::array(0,
        new NextAction("high king maulgar ranged dps priority", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar healer avoidance", NextAction::array(0,
        new NextAction("high king maulgar healer avoidance", ACTION_RAID + 1), nullptr)));
    
    triggers.push_back(new TriggerNode("high king maulgar whirlwind run away", NextAction::array(0,
        new NextAction("high king maulgar whirlwind run away", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar banish felstalker", NextAction::array(0,
        new NextAction("high king maulgar banish felstalker", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("high king maulgar hunter misdirection", NextAction::array(0,
        new NextAction("high king maulgar hunter misdirection", ACTION_RAID + 2), nullptr)));

    // Gruul the Dragonkiller
    triggers.push_back(new TriggerNode("gruul the dragonkiller position boss", NextAction::array(0,
        new NextAction("gruul the dragonkiller position boss", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller spread ranged", NextAction::array(0,
        new NextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("gruul the dragonkiller shatter spread", NextAction::array(0,
        new NextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6), nullptr)));
}

void RaidGruulsLairStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HighKingMaulgarMultiplier(botAI));
    multipliers.push_back(new GruulTheDragonkillerMultiplier(botAI));
}
