/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAID_TOCTRIGGERS_ANUBARAK_H
#define PLAYERBOTS_RAID_TOCTRIGGERS_ANUBARAK_H

#include "Trigger.h"

class AnubarakKiterNeedsImmunityTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak kiter needs immunity";

    AnubarakKiterNeedsImmunityTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakPursuedBySpikeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak pursued by spike";

    AnubarakPursuedBySpikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakKiterNeedsProtectionTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak kiter needs protection";

    AnubarakKiterNeedsProtectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakKiterNeedsFreedomTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak kiter needs freedom";

    AnubarakKiterNeedsFreedomTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakHopEscortTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak hop escort";

    AnubarakHopEscortTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakRedirectThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak redirect threat";

    AnubarakRedirectThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSpikeNearbyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak spike nearby";

    AnubarakSpikeNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakBurrowerCastingShadowStrikeTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak burrower casting shadow strike";

    AnubarakBurrowerCastingShadowStrikeTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakLeechingSwarmOnTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak leeching swarm on tank";

    AnubarakLeechingSwarmOnTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakRangedShouldSeedPermafrostTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak ranged should seed permafrost";

    AnubarakRangedShouldSeedPermafrostTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakDpsTargetTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak dps target";

    AnubarakDpsTargetTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakBurrowerNeedsAssistTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak burrower needs assist tank";

    AnubarakBurrowerNeedsAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSideTankReadyTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak side tank ready";

    AnubarakSideTankReadyTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSideTankAreaThreatTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak side tank area threat";

    AnubarakSideTankAreaThreatTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakCleaveBuffNearBurrowerTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak cleave buff near burrower";

    AnubarakCleaveBuffNearBurrowerTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSideTankNeedsDefensiveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak side tank needs defensive";

    AnubarakSideTankNeedsDefensiveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakColdSelfSaveTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak penetrating cold self save";

    AnubarakColdSelfSaveTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSideTankDeadTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak side tank dead";

    AnubarakSideTankDeadTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakGripFrostSphereTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak grip frost sphere";

    AnubarakGripFrostSphereTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakRaidGathersTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak raid gathers";

    AnubarakRaidGathersTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakHealerStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak healer stand";

    AnubarakHealerStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakRangedStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak ranged stand";

    AnubarakRangedStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakOnPermafrostTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak on permafrost";

    AnubarakOnPermafrostTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakMeleeBurrowerStandTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak melee burrower stand";

    AnubarakMeleeBurrowerStandTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakBurrowerLeadTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak burrower lead";

    AnubarakBurrowerLeadTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakKiteCatDropTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak kite cat drop";

    AnubarakKiteCatDropTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakTankNeedsRescueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak tank needs rescue";

    AnubarakTankNeedsRescueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakLustDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak lust due";

    AnubarakLustDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakDemonicCircleDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak demonic circle due";

    AnubarakDemonicCircleDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakScarabOnRaidTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak scarab on raid";

    AnubarakScarabOnRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakEngagedByMainTankTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak engaged by main tank";

    AnubarakEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakPenetratingColdOnRaidTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak penetrating cold on raid";

    AnubarakPenetratingColdOnRaidTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSwarmHealUrgentTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak swarm heal urgent";

    AnubarakSwarmHealUrgentTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakSwarmHealTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak swarm heal";

    AnubarakSwarmHealTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakAuraMasteryDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak aura mastery due";

    AnubarakAuraMasteryDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakDivinePleaTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak divine plea";

    AnubarakDivinePleaTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakInnervateHealerTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak innervate healer";

    AnubarakInnervateHealerTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakScarabKiteTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak scarab kite";

    AnubarakScarabKiteTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakManaTopUpTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak mana top up";

    AnubarakManaTopUpTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakLightJudgeDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak light judge due";

    AnubarakLightJudgeDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

class AnubarakHealingStreamDueTrigger : public Trigger
{
public:
    static constexpr char const* Name = "anubarak healing stream due";

    AnubarakHealingStreamDueTrigger(PlayerbotAI* botAI) : Trigger(botAI, Name) {}
    bool IsActive() override;
};

#endif
