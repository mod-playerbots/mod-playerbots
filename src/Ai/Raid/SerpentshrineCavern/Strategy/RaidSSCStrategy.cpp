#include "RaidSSCStrategy.h"
#include "CreateNextAction.h"
#include "RaidSSCActions.h"
#include "RaidSSCMultipliers.h"

void RaidSSCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(
        new TriggerNode(
            "serpent shrine cavern bot is not in combat",
            {
                CreateNextAction<SerpentShrineCavernEraseTimersAndTrackersAction>(ACTION_EMERGENCY + 11.0f)
            }
        )
    );

    // Trash Mobs
    triggers.push_back(
        new TriggerNode(
            "underbog colossus spawned toxic pool after death",
            {
                CreateNextAction<UnderbogColossusEscapeToxicPoolAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "greyheart tidecaller water elemental totem spawned",
            {
                CreateNextAction<GreyheartTidecallerMarkWaterElementalTotemAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(
        new TriggerNode(
            "hydross the unstable bot is frost tank",
            {
                CreateNextAction<HydrossTheUnstablePositionFrostTankAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable bot is nature tank",
            {
                CreateNextAction<HydrossTheUnstablePositionNatureTankAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable elementals spawned",
            {
                CreateNextAction<HydrossTheUnstablePrioritizeElementalAddsAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable danger from water tombs",
            {
                CreateNextAction<HydrossTheUnstableFrostPhaseSpreadOutAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable tank needs aggro upon phase change",
            {
                CreateNextAction<HydrossTheUnstableMisdirectBossToTankAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable aggro resets upon phase change",
            {
                CreateNextAction<HydrossTheUnstableStopDpsUponPhaseChangeAction>(ACTION_EMERGENCY + 9.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "hydross the unstable need to manage timers",
            {
                CreateNextAction<HydrossTheUnstableManageTimersAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    // The Lurker Below
    triggers.push_back(
        new TriggerNode(
            "the lurker below spout is active",
            {
                CreateNextAction<TheLurkerBelowRunAroundBehindBossAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "the lurker below boss is active for main tank",
            {
                CreateNextAction<TheLurkerBelowPositionMainTankAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "the lurker below boss casts geyser",
            {
                CreateNextAction<TheLurkerBelowSpreadRangedInArcAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "the lurker below boss is submerged",
            {
                CreateNextAction<TheLurkerBelowTanksPickUpAddsAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "the lurker below need to prepare timer for spout",
            {
                CreateNextAction<TheLurkerBelowManageSpoutTimerAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    // Leotheras the Blind
    triggers.push_back(
        new TriggerNode(
            "leotheras the blind boss is inactive",
            {
                CreateNextAction<LeotherasTheBlindTargetSpellbindersAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind boss transformed into demon form",
            {
                CreateNextAction<LeotherasTheBlindDemonFormTankAttackBossAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind only warlock should tank demon form",
            {
                CreateNextAction<LeotherasTheBlindMeleeTanksDontAttackDemonFormAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind boss engaged by ranged",
            {
                CreateNextAction<LeotherasTheBlindPositionRangedAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind boss channeling whirlwind",
            {
                CreateNextAction<LeotherasTheBlindRunAwayFromWhirlwindAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind bot has too many chaos blast stacks",
            {
                CreateNextAction<LeotherasTheBlindMeleeDpsRunAwayFromBossAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind inner demon has awakened",
            {
                CreateNextAction<LeotherasTheBlindDestroyInnerDemonAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind entered final phase",
            {
                CreateNextAction<LeotherasTheBlindFinalPhaseAssignDpsPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind demon form tank needs aggro",
            {
                CreateNextAction<LeotherasTheBlindMisdirectBossToDemonFormTankAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "leotheras the blind boss wipes aggro upon phase change",
            {
                CreateNextAction<LeotherasTheBlindManageDpsWaitTimersAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    // Fathom-Lord Karathress
    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress boss engaged by main tank",
            {
                CreateNextAction<FathomLordKarathressMainTankPositionBossAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress caribdis engaged by first assist tank",
            {
                CreateNextAction<FathomLordKarathressFirstAssistTankPositionCaribdisAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress sharkkis engaged by second assist tank",
            {
                CreateNextAction<FathomLordKarathressSecondAssistTankPositionSharkkisAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress tidalvess engaged by third assist tank",
            {
                CreateNextAction<FathomLordKarathressThirdAssistTankPositionTidalvessAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress caribdis tank needs dedicated healer",
            {
                CreateNextAction<FathomLordKarathressPositionCaribdisTankHealerAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress pulling bosses",
            {
                CreateNextAction<FathomLordKarathressMisdirectBossesToTanksAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress determining kill order",
            {
                CreateNextAction<FathomLordKarathressAssignDpsPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "fathom-lord karathress tanks need to establish aggro",
            {
                CreateNextAction<FathomLordKarathressManageDpsTimerAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    // Morogrim Tidewalker
    triggers.push_back(
        new TriggerNode(
            "morogrim tidewalker boss engaged by main tank",
            {
                CreateNextAction<MorogrimTidewalkerMoveBossToTankPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "morogrim tidewalker water globules are incoming",
            {
                CreateNextAction<MorogrimTidewalkerPhase2RepositionRangedAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "morogrim tidewalker pulling boss",
            {
                CreateNextAction<MorogrimTidewalkerMisdirectBossToMainTankAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Lady Vashj <Coilfang Matron>
    triggers.push_back(
        new TriggerNode(
            "lady vashj boss engaged by main tank",
            {
                CreateNextAction<LadyVashjMainTankPositionBossAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj boss engaged by ranged in phase 1",
            {
                CreateNextAction<LadyVashjPhase1SpreadRangedInArcAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj casts shock blast on highest aggro",
            {
                CreateNextAction<LadyVashjSetGroundingTotemInMainTankGroupAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj bot has static charge",
            {
                CreateNextAction<LadyVashjStaticChargeMoveAwayFromGroupAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj pulling boss in phase 1 and phase 3",
            {
                CreateNextAction<LadyVashjMisdirectBossToMainTankAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj tainted elemental cheat",
            {
                CreateNextAction<LadyVashjTeleportToTaintedElementalAction>(ACTION_EMERGENCY + 10.0f),
                CreateNextAction<LadyVashjLootTaintedCoreAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj tainted core was looted",
            {
                CreateNextAction<LadyVashjPassTheTaintedCoreAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj tainted core is unusable",
            {
                CreateNextAction<LadyVashjDestroyTaintedCoreAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj need to reset core passing trackers",
            {
                CreateNextAction<LadyVashjEraseCorePassingTrackersAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj adds spawn in phase 2 and phase 3",
            {
                CreateNextAction<LadyVashjAssignPhase2AndPhase3DpsPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj coilfang strider is approaching",
            {
                CreateNextAction<LadyVashjMisdirectStriderToFirstAssistTankAction>(ACTION_EMERGENCY + 2.0f),
                CreateNextAction<LadyVashjTankAttackAndMoveAwayStriderAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj toxic sporebats are spewing poison clouds",
            {
                CreateNextAction<LadyVashjAvoidToxicSporesAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "lady vashj bot is entangled in toxic spores or static charge",
            {
                CreateNextAction<LadyVashjUseFreeActionAbilitiesAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Trash Mobs
    multipliers.push_back(new UnderbogColossusEscapeToxicPoolMultiplier(botAI));

    // Hydross the Unstable <Duke of Currents>
    multipliers.push_back(new HydrossTheUnstableDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDpsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableControlMisdirectionMultiplier(botAI));

    // The Lurker Below
    multipliers.push_back(new TheLurkerBelowStayAwayFromSpoutMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMaintainRangedSpreadMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowDisableTankAssistMultiplier(botAI));

    // Leotheras the Blind
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindFocusOnInnerDemonMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDpsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(botAI));

    // Fathom-Lord Karathress
    multipliers.push_back(new FathomLordKarathressDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAoeMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDpsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(botAI));

    // Morogrim Tidewalker
    multipliers.push_back(new MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerMaintainPhase2StackingMultiplier(botAI));

    // Lady Vashj <Coilfang Matron>
    multipliers.push_back(new LadyVashjDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new LadyVashjMaintainPhase1RangedSpreadMultiplier(botAI));
    multipliers.push_back(new LadyVashjStaticChargeStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new LadyVashjDoNotLootTheTaintedCoreMultiplier(botAI));
    multipliers.push_back(new LadyVashjCorePassersPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new LadyVashjDisableAutomaticTargetingAndMovementModifier(botAI));
}
