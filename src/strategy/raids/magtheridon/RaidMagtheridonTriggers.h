#ifndef _PLAYERBOT_RAIDMAGTHERIDONTRIGGERS_H
#define _PLAYERBOT_RAIDMAGTHERIDONTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

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

class MagtheridonDPSPriorityTrigger : public Trigger
{
public:
    MagtheridonDPSPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon dps priority") {};
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

class MagtheridonUseManticronCubeTrigger : public Trigger
{
public:
    MagtheridonUseManticronCubeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon use manticron cube") {};
    bool IsActive() override;
};

class MagtheridonManageTimersAndAssignmentsTrigger : public Trigger
{
public:
    MagtheridonManageTimersAndAssignmentsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon manage timers and assignments") {};
    bool IsActive() override;
};

#endif
