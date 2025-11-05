#ifndef _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H

#include "Trigger.h"

class HighKingMaulgarIsMainTankTrigger : public Trigger
{
public:
    HighKingMaulgarIsMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar is main tank") {}
    bool IsActive() override;
};

class HighKingMaulgarIsFirstAssistTankTrigger : public Trigger
{
public:
    HighKingMaulgarIsFirstAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar is first assist tank") {}
    bool IsActive() override;
};

class HighKingMaulgarIsSecondAssistTankTrigger : public Trigger
{
public:
    HighKingMaulgarIsSecondAssistTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar is second assist tank") {}
    bool IsActive() override;
};

class HighKingMaulgarIsMageTankTrigger : public Trigger
{
public:
    HighKingMaulgarIsMageTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar is mage tank") {}
    bool IsActive() override;
};

class HighKingMaulgarIsMoonkinTankTrigger : public Trigger
{
public:
    HighKingMaulgarIsMoonkinTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar is moonkin tank") {}
    bool IsActive() override;
};

class HighKingMaulgarDeterminingKillOrderTrigger : public Trigger
{
public:
    HighKingMaulgarDeterminingKillOrderTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar determining kill order") {}
    bool IsActive() override;
};

class HighKingMaulgarHealerInDangerTrigger : public Trigger
{
public:
    HighKingMaulgarHealerInDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar healers in danger") {}
    bool IsActive() override;
};

class HighKingMaulgarBossChannelingWhirlwindTrigger : public Trigger
{
public:
    HighKingMaulgarBossChannelingWhirlwindTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar boss channeling whirlwind") {}
    bool IsActive() override;
};

class HighKingMaulgarWildFelstalkerSpawnedTrigger : public Trigger
{
public:
    HighKingMaulgarWildFelstalkerSpawnedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar wild felstalker spawned") {}
    bool IsActive() override;
};

class HighKingMaulgarPullingOlmAndBlindeyeTrigger : public Trigger
{
public:
    HighKingMaulgarPullingOlmAndBlindeyeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar pulling olm and blindeye") {}
    bool IsActive() override;
};

class GruulTheDragonkillerBossEngagedByMainTankTrigger : public Trigger
{
public:
    GruulTheDragonkillerBossEngagedByMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller boss engaged by main tank") {}
    bool IsActive() override;
};

class GruulTheDragonkillerBossEngagedByRangeTrigger : public Trigger
{
public:
    GruulTheDragonkillerBossEngagedByRangeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller boss engaged by range") {}
    bool IsActive() override;
};

class GruulTheDragonkillerIncomingShatterTrigger : public Trigger
{
public:
    GruulTheDragonkillerIncomingShatterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller incoming shatter") {}
    bool IsActive() override;
};

#endif
