#ifndef PLAYERBOTS_GRUULTRIGGERS_H
#define PLAYERBOTS_GRUULTRIGGERS_H

#include "Trigger.h"

class HighKingMaulgarBossEngagedByMainTankTrigger : public Trigger
{
public:
    HighKingMaulgarBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar engaged by main tank") {}
    bool IsActive() override;
};

class HighKingMaulgarOlmEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    HighKingMaulgarOlmEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar olm engaged by first assist tank") {}
    bool IsActive() override;
};

class HighKingMaulgarBlindeyeEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    HighKingMaulgarBlindeyeEngagedBySecondAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar blindeye engaged by second assist tank") {}
    bool IsActive() override;
};

class HighKingMaulgarKroshEngagedByMageTankTrigger : public Trigger
{
public:
    HighKingMaulgarKroshEngagedByMageTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar krosh engaged by mage tank") {}
    bool IsActive() override;
};

class HighKingMaulgarKigglerEngagedByMoonkinTankTrigger : public Trigger
{
public:
    HighKingMaulgarKigglerEngagedByMoonkinTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar kiggler engaged by moonkin tank") {}
    bool IsActive() override;
};

class HighKingMaulgarDeterminingKillOrderTrigger : public Trigger
{
public:
    HighKingMaulgarDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar determining kill order") {}
    bool IsActive() override;
};

class HighKingMaulgarBossChannelingWhirlwindTrigger : public Trigger
{
public:
    HighKingMaulgarBossChannelingWhirlwindTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar boss channeling whirlwind") {}
    bool IsActive() override;
};

class HighKingMaulgarKroshCastsBlastWaveTrigger : public Trigger
{
public:
    HighKingMaulgarKroshCastsBlastWaveTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar krosh casts blast wave") {}
    bool IsActive() override;
};

class HighKingMaulgarWildFelStalkerSpawnedTrigger : public Trigger
{
public:
    HighKingMaulgarWildFelStalkerSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar wild fel stalker spawned") {}
    bool IsActive() override;
};

class HighKingMaulgarPullingOgreCouncilTrigger : public Trigger
{
public:
    HighKingMaulgarPullingOgreCouncilTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar pulling ogre council") {}
    bool IsActive() override;
};

class GruulTheDragonkillerBossEngagedByTanksTrigger : public Trigger
{
public:
    GruulTheDragonkillerBossEngagedByTanksTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller boss engaged by tanks") {}
    bool IsActive() override;
};

class GruulTheDragonkillerBossEngagedByRangedTrigger : public Trigger
{
public:
    GruulTheDragonkillerBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller boss engaged by ranged") {}
    bool IsActive() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public Trigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller incoming shatter") {}
    bool IsActive() override;
};

#endif
