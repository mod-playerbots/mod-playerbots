/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_MAGTRIGGERS_H
#define PLAYERBOTS_MAGTRIGGERS_H

#include "Trigger.h"
#include "PlayerbotAI.h"

class MagtheridonFirstThreeChannelersEngagedByMainTankTrigger : public Trigger
{
public:
    MagtheridonFirstThreeChannelersEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon first three channelers engaged by main tank") {}
    bool IsActive() override;
};

class MagtheridonNWChannelerEngagedByFirstAssistTankTrigger : public Trigger
{
public:
    MagtheridonNWChannelerEngagedByFirstAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon nw channeler engaged by first assist tank") {}
    bool IsActive() override;
};

class MagtheridonNEChannelerEngagedBySecondAssistTankTrigger : public Trigger
{
public:
    MagtheridonNEChannelerEngagedBySecondAssistTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon ne channeler engaged by second assist tank") {}
    bool IsActive() override;
};

class MagtheridonPullingWestAndEastChannelersTrigger : public Trigger
{
public:
    MagtheridonPullingWestAndEastChannelersTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon pulling west and east channelers") {}
    bool IsActive() override;
};

class MagtheridonDeterminingKillOrderTrigger : public Trigger
{
public:
    MagtheridonDeterminingKillOrderTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon determining kill order") {}
    bool IsActive() override;
};

class MagtheridonBurningAbyssalSpawnedTrigger : public Trigger
{
public:
    MagtheridonBurningAbyssalSpawnedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon burning abyssal spawned") {}
    bool IsActive() override;
};

class MagtheridonBossEngagedByMainTankTrigger : public Trigger
{
public:
    MagtheridonBossEngagedByMainTankTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon boss engaged by main tank") {}
    bool IsActive() override;
};

class MagtheridonBossEngagedByRangedTrigger : public Trigger
{
public:
    MagtheridonBossEngagedByRangedTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon boss engaged by ranged") {}
    bool IsActive() override;
};

class MagtheridonStandingInDebrisTrigger : public Trigger
{
public:
    MagtheridonStandingInDebrisTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon standing in debris") {}
    bool IsActive() override;
};

class MagtheridonIncomingBlastNovaTrigger : public Trigger
{
public:
    MagtheridonIncomingBlastNovaTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon incoming blast nova") {}
    bool IsActive() override;
};

class MagtheridonNeedToManageTimersAndAssignmentsTrigger : public Trigger
{
public:
    MagtheridonNeedToManageTimersAndAssignmentsTrigger(
        PlayerbotAI* botAI) : Trigger(botAI, "magtheridon need to manage timers and assignments") {}
    bool IsActive() override;
};

class MagtheridonBotIsNotInCombatTrigger : public Trigger
{
public:
    MagtheridonBotIsNotInCombatTrigger(PlayerbotAI* botAI) : Trigger(botAI, "magtheridon bot is not in combat") {};
    bool IsActive() override;
};

#endif
