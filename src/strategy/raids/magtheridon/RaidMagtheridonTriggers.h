#ifndef _PLAYERBOT_RAIDMAGTHERIDONTRIGGERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

class MagtheridonSetBotSightTrigger : public Trigger
{
public:
    MagtheridonSetBotSightTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon set bot sight") {};
    bool IsActive() override;
};

class MagtheridonHellfireChannelerMainTankTrigger : public Trigger
{
public:
    MagtheridonHellfireChannelerMainTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon hellfire channeler main tank") {};
    bool IsActive() override;
};

class MagtheridonHellfireChannelerNWChannelerTankTrigger : public Trigger
{
public:
    MagtheridonHellfireChannelerNWChannelerTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon hellfire channeler nw channeler tank") {};
    bool IsActive() override;
};

class MagtheridonHellfireChannelerNEChannelerTankTrigger : public Trigger
{
public:
    MagtheridonHellfireChannelerNEChannelerTankTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon hellfire channeler ne channeler tank") {};
    bool IsActive() override;
};

class MagtheridonHellfireChannelerMisdirectionTrigger : public Trigger
{
public:
    MagtheridonHellfireChannelerMisdirectionTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon hellfire channeler misdirection") {};
    bool IsActive() override;
};

class MagtheridonHellfireChannelerDPSPriorityTrigger : public Trigger
{
public:
    MagtheridonHellfireChannelerDPSPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon hellfire channeler dps priority") {};
    bool IsActive() override;
};

class MagtheridonBurningAbyssalWarlockCCTrigger : public Trigger
{
public:
    MagtheridonBurningAbyssalWarlockCCTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon burning abyssal warlock cc") {};
    bool IsActive() override;
};

class MagtheridonPositionBossTrigger : public Trigger
{
public:
    MagtheridonPositionBossTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon position boss") {};
    bool IsActive() override;
};

class MagtheridonSpreadRangedTrigger : public Trigger
{
public:
    MagtheridonSpreadRangedTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon spread ranged") {};
    bool IsActive() override;
};

class MagtheridonSpreadHealerTrigger : public Trigger
{
public:
    MagtheridonSpreadHealerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon spread healer") {};
    bool IsActive() override;
};

class MagtheridonUseManticronCubeTrigger : public Trigger
{
public:
    MagtheridonUseManticronCubeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon use manticron cube") {};
    bool IsActive() override;
};

class MagtheridonUpdateTransitionTimerTrigger : public Trigger
{
public:
    MagtheridonUpdateTransitionTimerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon update transition timer") {};
    bool IsActive() override;
};

#endif
