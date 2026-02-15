#include "RaidKarazhanStrategy.h"
#include "CreateNextAction.h"
#include "RaidKarazhanMultipliers.h"
#include "RaidKarazhanActions.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trash
    triggers.push_back(
        new TriggerNode(
            "mana warp is about to explode",
            {
                CreateNextAction<ManaWarpStunCreatureBeforeWarpBreachAction>(ACTION_EMERGENCY + 6.0f)
            }
    ));

    // Attumen the Huntsman
    triggers.push_back(
        new TriggerNode(
            "attumen the huntsman need target priority",
            {
                CreateNextAction<AttumenTheHuntsmanMarkTargetAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "attumen the huntsman attumen spawned",
            {
                CreateNextAction<AttumenTheHuntsmanSplitBossesAction>(ACTION_RAID + 2.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "attumen the huntsman attumen is mounted",
            {
                CreateNextAction<AttumenTheHuntsmanStackBehindAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "attumen the huntsman boss wipes aggro when mounting",
            {
                CreateNextAction<AttumenTheHuntsmanManageDpsTimerAction>(ACTION_RAID + 2.0f)
            }
    ));

    // Moroes
    triggers.push_back(
        new TriggerNode(
            "moroes boss engaged by main tank",
            {
                CreateNextAction<MoroesMainTankAttackBossAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "moroes need target priority",
            {
                CreateNextAction<MoroesMarkTargetAction>(ACTION_RAID + 1.0f)
            }
    ));

    // Maiden of Virtue
    triggers.push_back(
        new TriggerNode(
            "maiden of virtue healers are stunned by repentance",
            {
                CreateNextAction<MaidenOfVirtueMoveBossToHealerAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "maiden of virtue holy wrath deals chain damage",
            {
                CreateNextAction<MaidenOfVirtuePositionRangedAction>(ACTION_RAID + 1.0f)
            }
    ));

    // The Big Bad Wolf
    triggers.push_back(
        new TriggerNode(
            "big bad wolf boss is chasing little red riding hood",
            {
                CreateNextAction<BigBadWolfRunAwayFromBossAction>(ACTION_EMERGENCY + 6.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "big bad wolf boss engaged by tank",
            {
                CreateNextAction<BigBadWolfPositionBossAction>(ACTION_RAID + 1.0f)
            }
    ));

    // Romulo and Julianne
    triggers.push_back(
        new TriggerNode(
            "romulo and julianne both bosses revived",
            {
                CreateNextAction<RomuloAndJulianneMarkTargetAction>(ACTION_RAID + 1.0f)
            }
    ));

    // The Wizard of Oz
    triggers.push_back(
        new TriggerNode(
            "wizard of oz need target priority",
            {
                CreateNextAction<WizardOfOzMarkTargetAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "wizard of oz strawman is vulnerable to fire",
            {
                CreateNextAction<WizardOfOzScorchStrawmanAction>(ACTION_RAID + 2.0f)
            }
    ));

    // The Curator
    triggers.push_back(
        new TriggerNode(
            "the curator astral flare spawned",
            {
                CreateNextAction<TheCuratorMarkAstralFlareAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "the curator boss engaged by tanks",
            {
                CreateNextAction<TheCuratorPositionBossAction>(ACTION_RAID + 2.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "the curator astral flares cast arcing sear",
            {
                CreateNextAction<TheCuratorSpreadRangedAction>(ACTION_RAID + 2.0f)
            }
    ));

    // Terestian Illhoof
    triggers.push_back(
        new TriggerNode(
            "terestian illhoof need target priority",
            {
                CreateNextAction<TerestianIllhoofMarkTargetAction>(ACTION_RAID + 1.0f)
            }
    ));

    // Shade of Aran
    triggers.push_back(
        new TriggerNode(
            "shade of aran arcane explosion is casting",
            {
                CreateNextAction<ShadeOfAranRunAwayFromArcaneExplosionAction>(ACTION_EMERGENCY + 6.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "shade of aran flame wreath is active",
            {
                CreateNextAction<ShadeOfAranStopMovingDuringFlameWreathAction>(ACTION_EMERGENCY + 7.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "shade of aran conjured elementals summoned",
            {
                CreateNextAction<ShadeOfAranMarkConjuredElementalAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "shade of aran boss uses counterspell and blizzard",
            {
                CreateNextAction<ShadeOfAranRangedMaintainDistanceAction>(ACTION_RAID + 2.0f)
            }
    ));

    // Netherspite
    triggers.push_back(
        new TriggerNode(
            "netherspite red beam is active",
            {
                CreateNextAction<NetherspiteBlockRedBeamAction>(ACTION_EMERGENCY + 8.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "netherspite blue beam is active",
            {
                CreateNextAction<NetherspiteBlockBlueBeamAction>(ACTION_EMERGENCY + 8.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "netherspite green beam is active",
            {
                CreateNextAction<NetherspiteBlockGreenBeamAction>(ACTION_EMERGENCY + 8.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "netherspite bot is not beam blocker",
            {
                CreateNextAction<NetherspiteAvoidBeamAndVoidZoneAction>(ACTION_EMERGENCY + 7.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "netherspite boss is banished",
            {
                CreateNextAction<NetherspiteBanishPhaseAvoidVoidZoneAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "netherspite need to manage timers and trackers",
            {
                CreateNextAction<NetherspiteManageTimersAndTrackersAction>(ACTION_EMERGENCY + 10.0f)
            }
    ));

    // Prince Malchezaar
    triggers.push_back(
        new TriggerNode(
            "prince malchezaar bot is enfeebled",
            {
                CreateNextAction<PrinceMalchezaarEnfeebledAvoidHazardAction>(ACTION_EMERGENCY + 6.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "prince malchezaar infernals are spawned",
            {
                CreateNextAction<PrinceMalchezaarNonTankAvoidInfernalAction>(ACTION_EMERGENCY + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "prince malchezaar boss engaged by main tank",
            {
                CreateNextAction<PrinceMalchezaarMainTankMovementAction>(ACTION_EMERGENCY + 6.0f)
            }
    ));

    // Nightbane
    triggers.push_back(
        new TriggerNode(
            "nightbane boss engaged by main tank",
            {
                CreateNextAction<NightbaneGroundPhasePositionBossAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "nightbane ranged bots are in charred earth",
            {
                CreateNextAction<NightbaneGroundPhaseRotateRangedPositionsAction>(ACTION_EMERGENCY + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "nightbane main tank is susceptible to fear",
            {
                CreateNextAction<NightbaneCastFearWardOnMainTankAction>(ACTION_RAID + 2.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "nightbane pets ignore collision to chase flying boss",
            {
                CreateNextAction<NightbaneControlPetAggressionAction>(ACTION_RAID + 2.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "nightbane boss is flying",
            {
                CreateNextAction<NightbaneFlightPhaseMovementAction>(ACTION_RAID + 1.0f)
            }
    ));
    triggers.push_back(
        new TriggerNode(
            "nightbane need to manage timers and trackers",
            {
                CreateNextAction<NightbaneManageTimersAndTrackersAction>(ACTION_EMERGENCY + 10.0f)
            }
    ));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new AttumenTheHuntsmanDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanWaitForDpsMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new TheCuratorDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionDisableChargeMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteKeepBlockingBeamMultiplier(botAI));
    multipliers.push_back(new NetherspiteWaitForDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebleKeepDistanceMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new NightbaneDisablePetsMultiplier(botAI));
    multipliers.push_back(new NightbaneWaitForDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableAvoidAoeMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}
