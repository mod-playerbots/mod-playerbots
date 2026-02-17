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
                CreateNextAction<HighKingMaulgarMainTankAttackMaulgarAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is first assist tank",
            {
                CreateNextAction<HighKingMaulgarFirstAssistTankAttackOlmAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is second assist tank",
            {
                CreateNextAction<HighKingMaulgarSecondAssistTankAttackBlindeyeAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is mage tank",
            {
                CreateNextAction<HighKingMaulgarMageTankAttackKroshAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar is moonkin tank",
            {
                CreateNextAction<HighKingMaulgarMoonkinTankAttackKigglerAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar determining kill order",
            {
                CreateNextAction<HighKingMaulgarAssignDPSPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar healer in danger",
            {
                CreateNextAction<HighKingMaulgarHealerFindSafePositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar boss channeling whirlwind",
            {
                CreateNextAction<HighKingMaulgarRunAwayFromWhirlwindAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar wild felstalker spawned",
            {
                CreateNextAction<HighKingMaulgarBanishFelstalkerAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high king maulgar pulling olm and blindeye",
            {
                CreateNextAction<HighKingMaulgarMisdirectOlmAndBlindeyeAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    // Gruul the Dragonkiller
    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller boss engaged by main tank",
            {
                CreateNextAction<GruulTheDragonkillerTanksPositionBossAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller boss engaged by range",
            {
                CreateNextAction<GruulTheDragonkillerSpreadRangedAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "gruul the dragonkiller incoming shatter",
            {
                CreateNextAction<GruulTheDragonkillerShatterSpreadAction>(ACTION_EMERGENCY + 6.0f)
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
