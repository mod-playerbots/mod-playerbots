#include "RaidTempestKeepStrategy.h"
#include "CreateNextAction.h"
#include "RaidTempestKeepActions.h"
#include "RaidTempestKeepMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(
        new TriggerNode(
            "crimson hand centurion casts arcane volley",
            {
                CreateNextAction<CrimsonHandCenturionCastPolymorphAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Al'ar <Phoenix God>
    triggers.push_back(
        new TriggerNode(
            "al'ar pulling boss",
            {
                CreateNextAction<AlarMisdirectBossToMainTankAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar boss is flying between platforms",
            {
                CreateNextAction<AlarBossTanksMoveBetweenPlatformsAction>(ACTION_RAID + 1.0f),
                CreateNextAction<AlarMeleeDpsMoveBetweenPlatformsAction>(ACTION_RAID + 1.0f),
                CreateNextAction<AlarRangedAndEmberTankMoveUnderPlatformsAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar embers of al'ar explode upon death",
            {
                CreateNextAction<AlarAssistTanksPickUpEmbersAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar killing embers of al'ar damages boss",
            {
                CreateNextAction<AlarRangedDpsPrioritizeEmbersAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar incoming flame quills",
            {
                CreateNextAction<AlarJumpFromPlatformAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar rising from the ashes",
            {
                CreateNextAction<AlarMoveAwayFromRebirthAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar everything is on fire in phase 2",
            {
                CreateNextAction<AlarSwapTanksOnBossAction>(ACTION_EMERGENCY + 2.0f),
                CreateNextAction<AlarAvoidFlamePatchesAndDiveBombsAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar phase 2 encounter is at room center",
            {
                CreateNextAction<AlarReturnToRoomCenterAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "al'ar strategy changes between phases",
            {
                CreateNextAction<AlarManagePhaseTrackerAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    // Void Reaver
    triggers.push_back(
        new TriggerNode(
            "void reaver boss casts pounding",
            {
                CreateNextAction<VoidReaverTanksPositionBossAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "void reaver knock away reduces tank aggro",
            {
                CreateNextAction<VoidReaverUseAggroDumpAbilityAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "void reaver boss launches arcane orbs",
            {
                CreateNextAction<VoidReaverSpreadRangedAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "void reaver arcane orb is incoming",
            {
                CreateNextAction<VoidReaverAvoidArcaneOrbAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "void reaver bot is not in combat",
            {
                CreateNextAction<VoidReaverEraseTrackersAction>(ACTION_EMERGENCY + 11.0f)
            }
        )
    );

    // High Astromancer Solarian
    triggers.push_back(
        new TriggerNode(
            "high astromancer solarian boss casts wrath of the astromancer",
            {
                CreateNextAction<HighAstromancerSolarianRangedLeaveSpaceForMeleeAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high astromancer solarian bot has wrath of the astromancer",
            {
                CreateNextAction<HighAstromancerSolarianMoveAwayFromGroupAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high astromancer solarian boss has vanished",
            {
                CreateNextAction<HighAstromancerSolarianStackForAoeAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high astromancer solarian solarium priests spawned",
            {
                CreateNextAction<HighAstromancerSolarianTargetSolariumPriestsAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "high astromancer solarian boss casts psychic scream",
            {
                CreateNextAction<HighAstromancerSolarianCastFearWardOnMainTankAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider thaladred is fixated on bot",
            {
                CreateNextAction<KaelthasSunstriderKiteThaladredAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider pulling tankable advisors",
            {
                CreateNextAction<KaelthasSunstriderMisdirectAdvisorsToTanksAction>(ACTION_EMERGENCY + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider sanguinar engaged by main tank",
            {
                CreateNextAction<KaelthasSunstriderMainTankPositionSanguinarAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider sanguinar casts bellowing roar",
            {
                CreateNextAction<KaelthasSunstriderCastFearWardOnSanguinarTankAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider capernian should be tanked by a warlock",
            {
                CreateNextAction<KaelthasSunstriderWarlockTankPositionCapernianAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider capernian casts arcane burst and conflagration",
            {
                CreateNextAction<KaelthasSunstriderSpreadAndMoveAwayFromCapernianAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider telonicus engaged by first assist tank",
            {
                CreateNextAction<KaelthasSunstriderFirstAssistTankPositionTelonicusAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider bots have specific roles in phase 3",
            {
                CreateNextAction<KaelthasSunstriderHandleAdvisorRolesInPhase3Action>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider determining advisor kill order",
            {
                CreateNextAction<KaelthasSunstriderAssignAdvisorDpsPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider waiting for tanks to get aggro on advisors",
            {
                CreateNextAction<KaelthasSunstriderManageAdvisorDpsTimerAction>(ACTION_EMERGENCY + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider legendary weapons are alive",
            {
                CreateNextAction<KaelthasSunstriderAssignLegendaryWeaponDpsPriorityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider legendary axe casts whirlwind",
            {
                CreateNextAction<KaelthasSunstriderMoveDevastationAwayAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider legendary weapons are dead and lootable",
            {
                CreateNextAction<KaelthasSunstriderLootLegendaryWeaponsAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider legendary weapons are equipped",
            {
                CreateNextAction<KaelthasSunstriderUseLegendaryWeaponsAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider legendary weapons were lost",
            {
                CreateNextAction<KaelthasSunstriderReequipGearAction>(ACTION_EMERGENCY + 11.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider boss has entered the fight",
            {
                CreateNextAction<KaelthasSunstriderMainTankPositionBossAction>(ACTION_RAID + 1.0f),
                CreateNextAction<KaelthasSunstriderAvoidFlameStrikeAction>(ACTION_EMERGENCY + 8.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider phoenixes and eggs are spawning",
            {
                CreateNextAction<KaelthasSunstriderHandlePhoenixesAndEggsAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider raid member is mind controlled",
            {
                CreateNextAction<KaelthasSunstriderBreakMindControlAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider boss is casting pyroblast",
            {
                CreateNextAction<KaelthasSunstriderBreakThroughShockBarrierAction>(ACTION_EMERGENCY + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kael'thas sunstrider boss is manipulating gravity",
            {
                CreateNextAction<KaelthasSunstriderSpreadOutInMidairAction>(ACTION_EMERGENCY + 1.0f)
            }
        )
    );
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Alar <Phoenix God>
    multipliers.push_back(new AlarMoveBetweenPlatformsMultiplier(botAI));
    multipliers.push_back(new AlarDisableDisperseMultiplier(botAI));
    multipliers.push_back(new AlarDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AlarStayAwayFromRebirthMultiplier(botAI));
    multipliers.push_back(new AlarPhase2NoTankingIfArmorMeltedMultiplier(botAI));

    // Void Reaver
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));

    // High Astromancer Solarian
    multipliers.push_back(new HighAstromancerSolarianDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianMaintainPositionMultiplier(botAI));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    multipliers.push_back(new KaelthasSunstriderWaitForDpsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderKiteThaladredMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderKeepDistanceFromCapernianMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderManageWeaponTankingMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableAdvisorTankAssistMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableDisperseMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(botAI));
}
