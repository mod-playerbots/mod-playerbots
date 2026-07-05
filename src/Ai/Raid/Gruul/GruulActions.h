#ifndef PLAYERBOTS_GRUULACTIONS_H
#define PLAYERBOTS_GRUULACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HighKingMaulgarMainTankAttackMaulgarAction : public AttackAction
{
public:
    HighKingMaulgarMainTankAttackMaulgarAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar main tank attack maulgar") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarFirstAssistTankAttackOlmAction : public AttackAction
{
public:
    HighKingMaulgarFirstAssistTankAttackOlmAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar first assist tank attack olm") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarSecondAssistTankAttackBlindeyeAction : public AttackAction
{
public:
    HighKingMaulgarSecondAssistTankAttackBlindeyeAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar second assist tank attack blindeye") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarMageTankAttackKroshAction : public AttackAction
{
public:
    HighKingMaulgarMageTankAttackKroshAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar mage tank attack krosh") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarMoonkinTankAttackKigglerAction : public AttackAction
{
public:
    HighKingMaulgarMoonkinTankAttackKigglerAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar moonkin tank attack kiggler") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarAssignDPSPriorityAction : public AttackAction
{
public:
    HighKingMaulgarAssignDPSPriorityAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar assign dps priority") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarRunAwayFromWhirlwindAction : public MovementAction
{
public:
    HighKingMaulgarRunAwayFromWhirlwindAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar run away from whirlwind") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarMoveAwayFromBlastNovaDangerAction : public MovementAction
{
public:
    HighKingMaulgarMoveAwayFromBlastNovaDangerAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar move away from blast nova danger") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarBanishFelStalkerAction : public AttackAction
{
public:
    HighKingMaulgarBanishFelStalkerAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar banish fel stalker") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class HighKingMaulgarMisdirectOgresToTanksAction : public AttackAction
{
public:
    HighKingMaulgarMisdirectOgresToTanksAction(
        PlayerbotAI* botAI, std::string const name = "high king maulgar misdirect ogres to tanks") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class GruulTheDragonkillerTanksPositionBossAction : public AttackAction
{
public:
    GruulTheDragonkillerTanksPositionBossAction(
        PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller tanks position boss") : AttackAction(botAI, name) {};
    bool Execute(Event event) override;
};

class GruulTheDragonkillerSpreadRangedAction : public MovementAction
{
public:
    GruulTheDragonkillerSpreadRangedAction(
        PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller spread ranged") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;

private:
    Position _initialPosition;
    bool _hasReachedInitialPosition = false;
};

class GruulTheDragonkillerShatterSpreadAction : public MovementAction
{
public:
    GruulTheDragonkillerShatterSpreadAction(
        PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller shatter spread") : MovementAction(botAI, name) {};
    bool Execute(Event event) override;
};

#endif
