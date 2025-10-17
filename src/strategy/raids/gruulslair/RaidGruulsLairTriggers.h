#ifndef _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H
#define _PLAYERBOT_RAIDGRUULSLAIRTRIGGERS_H

#include "Trigger.h"

class HighKingMaulgarMaulgarTankTrigger : public Trigger
{
public:
    HighKingMaulgarMaulgarTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar maulgar tank") {}
    bool IsActive() override;
};

class HighKingMaulgarOlmTankTrigger : public Trigger
{
public:
    HighKingMaulgarOlmTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar olm tank") {}
    bool IsActive() override;
};

class HighKingMaulgarBlindeyeTankTrigger : public Trigger
{
public:
    HighKingMaulgarBlindeyeTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar blindeye tank") {}
    bool IsActive() override;
};

class HighKingMaulgarKroshMageTankTrigger : public Trigger
{
public:
    HighKingMaulgarKroshMageTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar krosh mage tank") {}
    bool IsActive() override;
};

class HighKingMaulgarKigglerMoonkinTankTrigger : public Trigger
{
public:
    HighKingMaulgarKigglerMoonkinTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar kiggler moonkin tank") {}
    bool IsActive() override;
};

class HighKingMaulgarMeleeDPSPriorityTrigger : public Trigger
{
public:
    HighKingMaulgarMeleeDPSPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar melee dps priority") {}
    bool IsActive() override;
};

class HighKingMaulgarRangedDPSPriorityTrigger : public Trigger
{
public:
    HighKingMaulgarRangedDPSPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar ranged dps priority") {}
    bool IsActive() override;
};

class HighKingMaulgarHealerAvoidanceTrigger : public Trigger
{
public:
    HighKingMaulgarHealerAvoidanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar healer avoidance") {}
    bool IsActive() override;
};

class HighKingMaulgarWhirlwindRunAwayTrigger : public Trigger
{
public:
    HighKingMaulgarWhirlwindRunAwayTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar whirlwind run away") {}
    bool IsActive() override;
};

class HighKingMaulgarBanishFelstalkerTrigger : public Trigger
{
public:
    HighKingMaulgarBanishFelstalkerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar banish felstalker") {}
    bool IsActive() override;
};

class HighKingMaulgarHunterMisdirectionTrigger : public Trigger
{
public:
    HighKingMaulgarHunterMisdirectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "high king maulgar hunter misdirection") {}
    bool IsActive() override;
};

class GruulTheDragonkillerPositionBossTrigger : public Trigger
{
public:
    GruulTheDragonkillerPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller position boss") {}
    bool IsActive() override;
};

class GruulTheDragonkillerSpreadRangedTrigger : public Trigger
{
public:
    GruulTheDragonkillerSpreadRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller spread ranged") {}
    bool IsActive() override;
};

class GruulTheDragonkillerShatterSpreadTrigger : public Trigger
{
public:
    GruulTheDragonkillerShatterSpreadTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gruul the dragonkiller shatter spread") {}
    bool IsActive() override;
};

#endif
