/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDZULAMANTRIGGERS_H
#define _PLAYERBOT_RAIDZULAMANTRIGGERS_H

#include "Trigger.h"

// Trash

class AmanishiMedicineManSummonedWardTrigger : public Trigger
{
public:
    AmanishiMedicineManSummonedWardTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "amani'shi medicine man summoned ward") {}
    bool IsActive() override;
};

// Akil'zon <Eagle Avatar>

class AkilzonPullingBossTrigger : public Trigger
{
public:
    AkilzonPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akil'zon pulling boss") {}
    bool IsActive() override;
};

class AkilzonBossEngagedByTanksTrigger : public Trigger
{
public:
    AkilzonBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akil'zon boss engaged by tanks") {}
    bool IsActive() override;
};

class AkilzonBossCastsStaticDisruptionTrigger : public Trigger
{
public:
    AkilzonBossCastsStaticDisruptionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akil'zon boss casts static disruption") {}
    bool IsActive() override;
};

class AkilzonElectricalStormIncomingTrigger : public Trigger
{
public:
    AkilzonElectricalStormIncomingTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akil'zon electrical storm incoming") {}
    bool IsActive() override;
};

class AkilzonBotsNeedToPrepareForElectricalStormTrigger : public Trigger
{
public:
    AkilzonBotsNeedToPrepareForElectricalStormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "akil'zon bots need to prepare for electrical storm") {}
    bool IsActive() override;
};

// Nalorakk <Bear Avatar>

class NalorakkPullingBossTrigger : public Trigger
{
public:
    NalorakkPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk pulling boss") {}
    bool IsActive() override;
};

class NalorakkBossSwitchesFormsTrigger : public Trigger
{
public:
    NalorakkBossSwitchesFormsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk boss switches forms") {}
    bool IsActive() override;
};

class NalorakkBossCastsSurgeTrigger : public Trigger
{
public:
    NalorakkBossCastsSurgeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "nalorakk boss casts surge") {}
    bool IsActive() override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiPullingBossTrigger : public Trigger
{
public:
    JanalaiPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jan'alai pulling boss") {}
    bool IsActive() override;
};

class JanalaiBossEngagedByTanksTrigger : public Trigger
{
public:
    JanalaiBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jan'alai boss engaged by tanks") {}
    bool IsActive() override;
};

class JanalaiBossCastsFlameBreathTrigger : public Trigger
{
public:
    JanalaiBossCastsFlameBreathTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jan'alai boss casts flame breath") {}
    bool IsActive() override;
};

class JanalaiBossSummoningFireBombsTrigger : public Trigger
{
public:
    JanalaiBossSummoningFireBombsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jan'alai boss summoning fire bombs") {}
    bool IsActive() override;
};

class JanalaiAmanishiHatchersSpawnedTrigger : public Trigger
{
public:
    JanalaiAmanishiHatchersSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "jan'alai amani'shi hatchers spawned") {}
    bool IsActive() override;
};

// Halazzi <Lynx Avatar>

class HalazziPullingBossTrigger : public Trigger
{
public:
    HalazziPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi pulling boss") {}
    bool IsActive() override;
};

class HalazziBossEngagedByMainTankTrigger : public Trigger
{
public:
    HalazziBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi boss engaged by main tank") {}
    bool IsActive() override;
};

class HalazziBossSummonsSpiritLynxTrigger : public Trigger
{
public:
    HalazziBossSummonsSpiritLynxTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi boss summons spirit lynx") {}
    bool IsActive() override;
};

class HalazziDeterminingDpsTargetTrigger : public Trigger
{
public:
    HalazziDeterminingDpsTargetTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "halazzi determining dps target") {}
    bool IsActive() override;
};

// Hex Lord Malacrass

class HexLordMalacrassPullingBossTrigger : public Trigger
{
public:
    HexLordMalacrassPullingBossTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass pulling boss") {}
    bool IsActive() override;
};

class HexLordMalacrassDeterminingKillOrderTrigger : public Trigger
{
public:
    HexLordMalacrassDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass determining kill order") {}
    bool IsActive() override;
};

class HexLordMalacrassBossIsChannelingWhirlwindTrigger : public Trigger
{
public:
    HexLordMalacrassBossIsChannelingWhirlwindTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass boss is channeling whirlwind") {}
    bool IsActive() override;
};

class HexLordMalacrassBossHasSpellReflectionTrigger : public Trigger
{
public:
    HexLordMalacrassBossHasSpellReflectionTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass boss has spell reflection") {}
    bool IsActive() override;
};

class HexLordMalacrassBossPlacedFreezingTrapTrigger : public Trigger
{
public:
    HexLordMalacrassBossPlacedFreezingTrapTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "hex lord malacrass boss placed freezing trap") {}
    bool IsActive() override;
};

// Zul'jin

class ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger : public Trigger
{
public:
    ZuljinMainTankNeedsAggroUponPullOrPhaseChangeTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zul'jin main tank needs aggro upon pull or phase change") {}
    bool IsActive() override;
};

class ZuljinBossEngagedByTanksTrigger : public Trigger
{
public:
    ZuljinBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zul'jin boss engaged by tanks") {}
    bool IsActive() override;
};

class ZuljinBossIsChannelingWhirlwindInTrollFormTrigger : public Trigger
{
public:
    ZuljinBossIsChannelingWhirlwindInTrollFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zul'jin boss is channeling whirlwind in troll form") {}
    bool IsActive() override;
};

class ZuljinBossIsSummoningCyclonesInEagleFormTrigger : public Trigger
{
public:
    ZuljinBossIsSummoningCyclonesInEagleFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zul'jin boss is summoning cyclones in eagle form") {}
    bool IsActive() override;
};

class ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger : public Trigger
{
public:
ZuljinBossCastsAoeAbilitiesInDragonhawkFormTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "zul'jin boss casts aoe abilities in dragonhawk form") {}
    bool IsActive() override;
};

#endif
