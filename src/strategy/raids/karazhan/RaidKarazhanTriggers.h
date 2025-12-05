#ifndef _PLAYERBOT_RAIDKARAZHANTRIGGERS_H
#define _PLAYERBOT_RAIDKARAZHANTRIGGERS_H

#include "Trigger.h"

class ManaWarpIsAboutToExplodeTrigger : public Trigger
{
public:
    ManaWarpIsAboutToExplodeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "mana warp is about to explode") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanNeedTargetPriorityTrigger : public Trigger
{
public:
    AttumenTheHuntsmanNeedTargetPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman need target priority") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanAttumenSpawnedTrigger : public Trigger
{
public:
    AttumenTheHuntsmanAttumenSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman attumen spawned") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanAttumenIsMountedTrigger : public Trigger
{
public:
    AttumenTheHuntsmanAttumenIsMountedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman attumen is mounted") {}
    bool IsActive() override;
};

class AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger : public Trigger
{
public:
    AttumenTheHuntsmanBossWipesAggroWhenMountingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "attumen the huntsman boss wipes aggro when mounting") {}
    bool IsActive() override;
};

class MoroesBossEngagedByMainTankTrigger : public Trigger
{
public:
    MoroesBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "moroes boss engaged by main tank") {}
    bool IsActive() override;
};

class MoroesNeedTargetPriorityTrigger : public Trigger
{
public:
    MoroesNeedTargetPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "moroes need target priority") {}
    bool IsActive() override;
};

class MaidenOfVirtueHealersAreStunnedByRepentanceTrigger : public Trigger
{
public:
    MaidenOfVirtueHealersAreStunnedByRepentanceTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue healers are stunned by repentance") {}
    bool IsActive() override;
};

class MaidenOfVirtueHolyWrathDealsChainDamageTrigger : public Trigger
{
public:
    MaidenOfVirtueHolyWrathDealsChainDamageTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "maiden of virtue holy wrath deals chain damage") {}
    bool IsActive() override;
};

class BigBadWolfBossEngagedByTankTrigger : public Trigger
{
public:
    BigBadWolfBossEngagedByTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf boss engaged by tank") {}
    bool IsActive() override;
};

class BigBadWolfBossIsChasingLittleRedRidingHoodTrigger : public Trigger
{
public:
    BigBadWolfBossIsChasingLittleRedRidingHoodTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "big bad wolf boss is chasing little red riding hood") {}
    bool IsActive() override;
};

class RomuloAndJulianneBothBossesRevivedTrigger : public Trigger
{
public:
    RomuloAndJulianneBothBossesRevivedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "romulo and julianne both bosses revived") {}
    bool IsActive() override;
};

class WizardOfOzNeedTargetPriorityTrigger : public Trigger
{
public:
    WizardOfOzNeedTargetPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz need target priority") {}
    bool IsActive() override;
};

class WizardOfOzStrawmanIsVulnerableToFireTrigger : public Trigger
{
public:
    WizardOfOzStrawmanIsVulnerableToFireTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "wizard of oz strawman is vulnerable to fire") {}
    bool IsActive() override;
};
class TheCuratorAstralFlareSpawnedTrigger : public Trigger
{
public:
    TheCuratorAstralFlareSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the curator astral flare spawned") {}
    bool IsActive() override;
};

class TheCuratorBossEngagedByTanksTrigger : public Trigger
{
public:
    TheCuratorBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the curator boss engaged by tanks") {}
    bool IsActive() override;
};

class TheCuratorBossAstralFlaresCastArcingSearTrigger : public Trigger
{
public:
    TheCuratorBossAstralFlaresCastArcingSearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the curator astral flares cast arcing sear") {}
    bool IsActive() override;
};

class TerestianIllhoofNeedTargetPriorityTrigger : public Trigger
{
public:
    TerestianIllhoofNeedTargetPriorityTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "terestian illhoof need target priority") {}
    bool IsActive() override;
};

class ShadeOfAranArcaneExplosionIsCastingTrigger : public Trigger
{
public:
    ShadeOfAranArcaneExplosionIsCastingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "shade of aran arcane explosion is casting") {}
    bool IsActive() override;
};

class ShadeOfAranFlameWreathIsActiveTrigger : public Trigger
{
public:
    ShadeOfAranFlameWreathIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "shade of aran flame wreath is active") {}
    bool IsActive() override;
};

class ShadeOfAranConjuredElementalsSummonedTrigger : public Trigger
{
public:
    ShadeOfAranConjuredElementalsSummonedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "shade of aran conjured elementals summoned") {}
    bool IsActive() override;
};

class ShadeOfAranBossUsesCounterspellAndBlizzardTrigger : public Trigger
{
public:
    ShadeOfAranBossUsesCounterspellAndBlizzardTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "shade of aran boss uses counterspell and blizzard") {}
    bool IsActive() override;
};

class NetherspiteRedBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteRedBeamIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite red beam is active") {}
    bool IsActive() override;
};

class NetherspiteBlueBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteBlueBeamIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite blue beam is active") {}
    bool IsActive() override;
};

class NetherspiteGreenBeamIsActiveTrigger : public Trigger
{
public:
    NetherspiteGreenBeamIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite green beam is active") {}
    bool IsActive() override;
};

class NetherspiteBotIsNotBeamBlockerTrigger : public Trigger
{
public:
    NetherspiteBotIsNotBeamBlockerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite bot is not beam blocker") {}
    bool IsActive() override;
};

class NetherspiteBossIsBanishedTrigger : public Trigger
{
public:
    NetherspiteBossIsBanishedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite boss is banished") {}
    bool IsActive() override;
};

class NetherspiteNeedToManageTimersAndTrackersTrigger : public Trigger
{
public:
    NetherspiteNeedToManageTimersAndTrackersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "netherspite need to manage timers and trackers") {}
    bool IsActive() override;
};

class PrinceMalchezaarBotIsEnfeebledTrigger : public Trigger
{
public:
    PrinceMalchezaarBotIsEnfeebledTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar bot is enfeebled") {}
    bool IsActive() override;
};

class PrinceMalchezaarInfernalsAreSpawnedTrigger : public Trigger
{
public:
    PrinceMalchezaarInfernalsAreSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar infernals are spawned") {}
    bool IsActive() override;
};

class PrinceMalchezaarBossEngagedByMainTankTrigger : public Trigger
{
public:
    PrinceMalchezaarBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "prince malchezaar boss engaged by main tank") {}
    bool IsActive() override;
};

class NightbaneBossEngagedByMainTankTrigger : public Trigger
{
public:
    NightbaneBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane boss engaged by main tank") {}
    bool IsActive() override;
};

class NightbaneRangedBotsAreInCharredEarthTrigger : public Trigger
{
public:
    NightbaneRangedBotsAreInCharredEarthTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane ranged bots are in charred earth") {}
    bool IsActive() override;
};

class NightbaneMainTankIsSusceptibleToFearTrigger : public Trigger
{
public:
    NightbaneMainTankIsSusceptibleToFearTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane main tank is susceptible to fear") {}
    bool IsActive() override;
};

class NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger : public Trigger
{
public:
    NightbanePetsIgnoreCollisionToChaseFlyingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane pets ignore collision to chase flying boss") {}
    bool IsActive() override;
};

class NightbaneBossIsFlyingTrigger : public Trigger
{
public:
    NightbaneBossIsFlyingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane boss is flying") {}
    bool IsActive() override;
};

class NightbaneNeedToManageTimersAndTrackersTrigger : public Trigger
{
public:
    NightbaneNeedToManageTimersAndTrackersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nightbane need to manage timers and trackers") {}
    bool IsActive() override;
};

#endif
