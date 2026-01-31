#include "RaidGruulsLairStrategy.h"
#include "RaidGruulsLairMultipliers.h"
#include "CreateNextAction.h"
#include "RaidGruulsLairActions.h"

void RaidGruulsLairStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // High King Maulgar
    triggers.push_back(
        new TriggerNode(
            "high king maulgar is main tank",
            {
                CreaateNextAction("high king maulgar main tank attack maulgar", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is first assist tank",
            {
                CreaateNextAction("high king maulgar first assist tank attack olm", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is second assist tank",
            {
                CreaateNextAction("high king maulgar second assist tank attack blindeye", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is mage tank",
            {
                CreaateNextAction("high king maulgar mage tank attack krosh", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is moonkin tank",
            {
                CreaateNextAction("high king maulgar moonkin tank attack kiggler", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar determining kill order",
            {
                CreaateNextAction("high king maulgar assign dps priority", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar healer in danger",
            {
                CreaateNextAction("high king maulgar healer find safe position", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar boss channeling whirlwind",
            {
                CreaateNextAction("high king maulgar run away from whirlwind", ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar wild felstalker spawned",
            {
                CreaateNextAction("high king maulgar banish felstalker", ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar pulling olm and blindeye",
            {
                CreaateNextAction("high king maulgar misdirect olm and blindeye", ACTION_RAID + 2.0f)
            }
        )
    );

    // Gruul the Dragonkiller
    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller boss engaged by main tank",
            {
                CreaateNextAction("gruul the dragonkiller main tank position boss", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller boss engaged by range",
            {
                CreaateNextAction("gruul the dragonkiller spread ranged", ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller incoming shatter",
            {
                CreaateNextAction("gruul the dragonkiller shatter spread", ACTION_EMERGENCY + 6.0f)
            }
        )
    );
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
