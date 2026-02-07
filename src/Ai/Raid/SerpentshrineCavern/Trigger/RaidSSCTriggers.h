#ifndef _PLAYERBOT_RAIDSSCTRIGGERS_H
#define _PLAYERBOT_RAIDSSCTRIGGERS_H

#include "Trigger.h"

// General

class SerpentShrineCavernBotIsNotInCombatTrigger : public Trigger
{
public:
    SerpentShrineCavernBotIsNotInCombatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "serpent shrine cavern bot is not in combat") {}
    bool IsActive() override;
};

// Trash

class UnderbogColossusSpawnedToxicPoolAfterDeathTrigger : public Trigger
{
public:
    UnderbogColossusSpawnedToxicPoolAfterDeathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "underbog colossus spawned toxic pool after death") {}
    bool IsActive() override;
};

class GreyheartTidecallerWaterElementalTotemSpawnedTrigger : public Trigger
{
public:
    GreyheartTidecallerWaterElementalTotemSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "greyheart tidecaller water elemental totem spawned") {}
    bool IsActive() override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableBotIsFrostTankTrigger : public Trigger
{
public:
    HydrossTheUnstableBotIsFrostTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable bot is frost tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableBotIsNatureTankTrigger : public Trigger
{
public:
    HydrossTheUnstableBotIsNatureTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable bot is nature tank") {}
    bool IsActive() override;
};

class HydrossTheUnstableElementalsSpawnedTrigger : public Trigger
{
public:
    HydrossTheUnstableElementalsSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable elementals spawned") {}
    bool IsActive() override;
};

class HydrossTheUnstableDangerFromWaterTombsTrigger : public Trigger
{
public:
    HydrossTheUnstableDangerFromWaterTombsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable danger from water tombs") {}
    bool IsActive() override;
};

class HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger : public Trigger
{
public:
    HydrossTheUnstableTankNeedsAggroUponPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable tank needs aggro upon phase change") {}
    bool IsActive() override;
};

class HydrossTheUnstableAggroResetsUponPhaseChangeTrigger : public Trigger
{
public:
    HydrossTheUnstableAggroResetsUponPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable aggro resets upon phase change") {}
    bool IsActive() override;
};

class HydrossTheUnstableNeedToManageTimersTrigger : public Trigger
{
public:
    HydrossTheUnstableNeedToManageTimersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hydross the unstable need to manage timers") {}
    bool IsActive() override;
};

// The Lurker Below

class TheLurkerBelowSpoutIsActiveTrigger : public Trigger
{
public:
    TheLurkerBelowSpoutIsActiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the lurker below spout is active") {}
    bool IsActive() override;
};

class TheLurkerBelowBossIsActiveForMainTankTrigger : public Trigger
{
public:
    TheLurkerBelowBossIsActiveForMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the lurker below boss is active for main tank") {}
    bool IsActive() override;
};

class TheLurkerBelowBossCastsGeyserTrigger : public Trigger
{
public:
    TheLurkerBelowBossCastsGeyserTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the lurker below boss casts geyser") {}
    bool IsActive() override;
};

class TheLurkerBelowBossIsSubmergedTrigger : public Trigger
{
public:
    TheLurkerBelowBossIsSubmergedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the lurker below boss is submerged") {}
    bool IsActive() override;
};

class TheLurkerBelowNeedToPrepareTimerForSpoutTrigger : public Trigger
{
public:
    TheLurkerBelowNeedToPrepareTimerForSpoutTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "the lurker below need to prepare timer for spout") {}
    bool IsActive() override;
};

// Leotheras the Blind

class LeotherasTheBlindBossIsInactiveTrigger : public Trigger
{
public:
    LeotherasTheBlindBossIsInactiveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss is inactive") {}
    bool IsActive() override;
};

class LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger : public Trigger
{
public:
    LeotherasTheBlindOnlyWarlockShouldTankDemonFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind only warlock should tank demon form") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossTransformedIntoDemonFormTrigger : public Trigger
{
public:
    LeotherasTheBlindBossTransformedIntoDemonFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss transformed into demon form") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossEngagedByRangedTrigger : public Trigger
{
public:
    LeotherasTheBlindBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss engaged by ranged") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossChannelingWhirlwindTrigger : public Trigger
{
public:
    LeotherasTheBlindBossChannelingWhirlwindTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss channeling whirlwind") {}
    bool IsActive() override;
};

class LeotherasTheBlindBotHasTooManyChaosBlastStacksTrigger : public Trigger
{
public:
    LeotherasTheBlindBotHasTooManyChaosBlastStacksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind bot has too many chaos blast stacks") {}
    bool IsActive() override;
};

class LeotherasTheBlindInnerDemonHasAwakenedTrigger : public Trigger
{
public:
    LeotherasTheBlindInnerDemonHasAwakenedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind inner demon has awakened") {}
    bool IsActive() override;
};

class LeotherasTheBlindEnteredFinalPhaseTrigger : public Trigger
{
public:
    LeotherasTheBlindEnteredFinalPhaseTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind entered final phase") {}
    bool IsActive() override;
};

class LeotherasTheBlindDemonFormTankNeedsAggro : public Trigger
{
public:
    LeotherasTheBlindDemonFormTankNeedsAggro(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind demon form tank needs aggro") {}
    bool IsActive() override;
};

class LeotherasTheBlindBossWipesAggroUponPhaseChangeTrigger : public Trigger
{
public:
    LeotherasTheBlindBossWipesAggroUponPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "leotheras the blind boss wipes aggro upon phase change") {}
    bool IsActive() override;
};

// Fathom-Lord Karathress

class FathomLordKarathressBossEngagedByMainTankTrigger : public Trigger
{
public:
    FathomLordKarathressBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress boss engaged by main tank") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress caribdis engaged by first assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressSharkkisEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressSharkkisEngagedBySecondAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress sharkkis engaged by second assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressTidalvessEngagedByThirdAssistTankTrigger : public Trigger
{
public:
    FathomLordKarathressTidalvessEngagedByThirdAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress tidalvess engaged by third assist tank") {}
    bool IsActive() override;
};

class FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger : public Trigger
{
public:
    FathomLordKarathressCaribdisTankNeedsDedicatedHealerTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress caribdis tank needs dedicated healer") {}
    bool IsActive() override;
};

class FathomLordKarathressPullingBossesTrigger : public Trigger
{
public:
    FathomLordKarathressPullingBossesTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress pulling bosses") {}
    bool IsActive() override;
};

class FathomLordKarathressDeterminingKillOrderTrigger : public Trigger
{
public:
    FathomLordKarathressDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress determining kill order") {}
    bool IsActive() override;
};

class FathomLordKarathressTanksNeedToEstablishAggroTrigger : public Trigger
{
public:
    FathomLordKarathressTanksNeedToEstablishAggroTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "fathom-lord karathress tanks need to establish aggro") {}
    bool IsActive() override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerPullingBossTrigger : public Trigger
{
public:
    MorogrimTidewalkerPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker pulling boss") {}
    bool IsActive() override;
};

class MorogrimTidewalkerBossEngagedByMainTankTrigger : public Trigger
{
public:
    MorogrimTidewalkerBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker boss engaged by main tank") {}
    bool IsActive() override;
};

class MorogrimTidewalkerWaterGlobulesAreIncomingTrigger : public Trigger
{
public:
    MorogrimTidewalkerWaterGlobulesAreIncomingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "morogrim tidewalker water globules are incoming") {}
    bool IsActive() override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjBossEngagedByMainTankTrigger : public Trigger
{
public:
    LadyVashjBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj boss engaged by main tank") {}
    bool IsActive() override;
};

class LadyVashjBossEngagedByRangedInPhase1Trigger : public Trigger
{
public:
    LadyVashjBossEngagedByRangedInPhase1Trigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj boss engaged by ranged in phase 1") {}
    bool IsActive() override;
};

class LadyVashjCastsShockBlastOnHighestAggroTrigger : public Trigger
{
public:
    LadyVashjCastsShockBlastOnHighestAggroTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj casts shock blast on highest aggro") {}
    bool IsActive() override;
};

class LadyVashjBotHasStaticChargeTrigger : public Trigger
{
public:
    LadyVashjBotHasStaticChargeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj bot has static charge") {}
    bool IsActive() override;
};

class LadyVashjPullingBossInPhase1AndPhase3Trigger : public Trigger
{
public:
    LadyVashjPullingBossInPhase1AndPhase3Trigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj pulling boss in phase 1 and phase 3") {}
    bool IsActive() override;
};

class LadyVashjAddsSpawnInPhase2AndPhase3Trigger : public Trigger
{
public:
    LadyVashjAddsSpawnInPhase2AndPhase3Trigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj adds spawn in phase 2 and phase 3") {}
    bool IsActive() override;
};

class LadyVashjCoilfangStriderIsApproachingTrigger : public Trigger
{
public:
    LadyVashjCoilfangStriderIsApproachingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj coilfang strider is approaching") {}
    bool IsActive() override;
};

class LadyVashjTaintedElementalCheatTrigger : public Trigger
{
public:
    LadyVashjTaintedElementalCheatTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj tainted elemental cheat") {}
    bool IsActive() override;
};

class LadyVashjTaintedCoreWasLootedTrigger : public Trigger
{
public:
    LadyVashjTaintedCoreWasLootedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj tainted core was looted") {}
    bool IsActive() override;
};

class LadyVashjTaintedCoreIsUnusableTrigger : public Trigger
{
public:
    LadyVashjTaintedCoreIsUnusableTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj tainted core is unusable") {}
    bool IsActive() override;
};

class LadyVashjNeedToResetCorePassingTrackersTrigger : public Trigger
{
public:
    LadyVashjNeedToResetCorePassingTrackersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj need to reset core passing trackers") {}
    bool IsActive() override;
};

class LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger : public Trigger
{
public:
    LadyVashjToxicSporebatsAreSpewingPoisonCloudsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj toxic sporebats are spewing poison clouds") {}
    bool IsActive() override;
};

class LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger : public Trigger
{
public:
    LadyVashjBotIsEntangledInToxicSporesOrStaticChargeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "lady vashj bot is entangled in toxic spores or static charge") {}
    bool IsActive() override;
};

#endif
