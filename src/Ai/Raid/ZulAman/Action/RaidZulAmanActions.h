/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RAIDZULAMANACTIONS_H
#define _PLAYERBOT_RAIDZULAMANACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

// Trash

class AmanishiMedicineManMarkWardAction : public Action
{
public:
    AmanishiMedicineManMarkWardAction(
        PlayerbotAI* botAI, std::string const name = "amani'shi medicine man mark ward") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Akil'zon <Eagle Avatar>

class AkilzonMisdirectBossToMainTankAction : public AttackAction
{
public:
    AkilzonMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "akil'zon misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonTanksPositionBossAction : public AttackAction
{
public:
    AkilzonTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "akil'zon tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonSpreadRangedAction : public MovementAction
{
public:
    AkilzonSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "akil'zon spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonMoveToEyeOfTheStormAction : public MovementAction
{
public:
    AkilzonMoveToEyeOfTheStormAction(
        PlayerbotAI* botAI, std::string const name = "akil'zon move to eye of the storm") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class AkilzonManageElectricalStormTimerAction : public Action
{
public:
    AkilzonManageElectricalStormTimerAction(
        PlayerbotAI* botAI, std::string const name = "akil'zon manage electrical storm timer") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Nalorakk <Bear Avatar>

class NalorakkMisdirectBossToMainTankAction : public AttackAction
{
public:
    NalorakkMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class NalorakkTanksPositionBossAction : public AttackAction
{
public:
    NalorakkTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;

private:
    bool MainTankPositionTrollForm(Unit* nalorakk);
    bool FirstAssistTankPositionBearForm(Unit* nalorakk);
};

class NalorakkSpreadRangedAction : public MovementAction
{
public:
    NalorakkSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "nalorakk spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Jan'alai <Dragonhawk Avatar>

class JanalaiMisdirectBossToMainTankAction : public AttackAction
{
public:
    JanalaiMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "jan'alai misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiTanksPositionBossAction : public AttackAction
{
public:
    JanalaiTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "jan'alai tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiSpreadRangedInCircleAction : public MovementAction
{
public:
    JanalaiSpreadRangedInCircleAction(
        PlayerbotAI* botAI, std::string const name = "jan'alai spread ranged in circle") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiAvoidFireBombsAction : public MovementAction
{
public:
    JanalaiAvoidFireBombsAction(PlayerbotAI* botAI, std::string const name = "jan'alai avoid fire bombs") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class JanalaiMarkAmanishiHatchersAction : public Action
{
public:
    JanalaiMarkAmanishiHatchersAction(
        PlayerbotAI* botAI, std::string const name = "jan'alai mark amani'shi hatchers") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// Halazzi <Lynx Avatar>

class HalazziMisdirectBossToMainTankAction : public AttackAction
{
public:
    HalazziMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "halazzi misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziMainTankPositionBossAction : public AttackAction
{
public:
    HalazziMainTankPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "halazzi main tank position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziFirstAssistTankAttackSpiritLynxAction : public AttackAction
{
public:
    HalazziFirstAssistTankAttackSpiritLynxAction(
        PlayerbotAI* botAI, std::string const name = "halazzi first assist tank attack spirit lynx") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HalazziAssignDpsPriorityAction : public AttackAction
{
public:
    HalazziAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "halazzi assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Hex Lord Malacrass

class HexLordMalacrassMisdirectBossToMainTankAction : public AttackAction
{
public:
    HexLordMalacrassMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassAssignDpsPriorityAction : public AttackAction
{
public:
    HexLordMalacrassAssignDpsPriorityAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass assign dps priority") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassRunAwayFromWhirlwindAction : public MovementAction
{
public:
    HexLordMalacrassRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass run away from whirlwind") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassCastersStopAttackingAction : public Action
{
public:
    HexLordMalacrassCastersStopAttackingAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass casters stop attacking") : Action(botAI, name) {}
    bool Execute(Event event) override;
};

class HexLordMalacrassMoveAwayFromFreezingTrapAction : public MovementAction
{
public:
    HexLordMalacrassMoveAwayFromFreezingTrapAction(
        PlayerbotAI* botAI, std::string const name = "hex lord malacrass move away from freezing trap") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

// Zul'jin

class ZuljinMisdirectBossToMainTankAction : public AttackAction
{
public:
    ZuljinMisdirectBossToMainTankAction(
        PlayerbotAI* botAI, std::string const name = "zul'jin misdirect boss to main tank") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinTanksPositionBossAction : public AttackAction
{
public:
    ZuljinTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "zul'jin tanks position boss") : AttackAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinRunAwayFromWhirlwindAction : public MovementAction
{
public:
    ZuljinRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const name = "zul'jin run away from whirlwind") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinAvoidCyclonesAction : public MovementAction
{
public:
    ZuljinAvoidCyclonesAction(PlayerbotAI* botAI, std::string const name = "zul'jin avoid cyclones") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

class ZuljinSpreadRangedAction : public MovementAction
{
public:
    ZuljinSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "zul'jin spread ranged") : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};

#endif
