#ifndef _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H
#define _PLAYERBOT_RAIDGRUULSLAIRACTIONS_H

#include "Action.h"
#include "AttackAction.h"
#include "MovementActions.h"

class HighKingMaulgarMainTankAttackMaulgarAction : public AttackAction
{
public:
    HighKingMaulgarMainTankAttackMaulgarAction(PlayerbotAI* botAI, std::string const name = "high king maulgar main tank attack maulgar") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarFirstAssistTankAttackOlmAction : public AttackAction
{
public:
    HighKingMaulgarFirstAssistTankAttackOlmAction(PlayerbotAI* botAI, std::string const name = "high king maulgar first assist tank attack olm") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarSecondAssistTankAttackBlindeyeAction : public AttackAction
{
public:
    HighKingMaulgarSecondAssistTankAttackBlindeyeAction(PlayerbotAI* botAI, std::string const name = "high king maulgar second assist tank attack blindeye") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMageTankAttackKroshAction : public AttackAction
{
public:
    HighKingMaulgarMageTankAttackKroshAction(PlayerbotAI* botAI, std::string const name = "high king maulgar mage tank attack krosh") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMoonkinTankAttackKigglerAction : public AttackAction
{
public:
    HighKingMaulgarMoonkinTankAttackKigglerAction(PlayerbotAI* botAI, std::string const name = "high king maulgar moonkin tank attack kiggler") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarAssignDPSPriorityAction : public AttackAction
{
public:
    HighKingMaulgarAssignDPSPriorityAction(PlayerbotAI* botAI, std::string const name = "high king maulgar assign dps priority") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarHealerFindSafePositionAction : public MovementAction
{
public:
    HighKingMaulgarHealerFindSafePositionAction(PlayerbotAI* botAI, std::string const name = "high king maulgar healer find safe position") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarRunAwayFromWhirlwindAction : public MovementAction
{
public:
    HighKingMaulgarRunAwayFromWhirlwindAction(PlayerbotAI* botAI, std::string const name = "high king maulgar run away from whirlwind") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarBanishFelstalkerAction : public AttackAction
{
public:
    HighKingMaulgarBanishFelstalkerAction(PlayerbotAI* botAI, std::string const name = "high king maulgar banish felstalker") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class HighKingMaulgarMisdirectOlmAndBlindeyeAction : public AttackAction
{
public:
    HighKingMaulgarMisdirectOlmAndBlindeyeAction(PlayerbotAI* botAI, std::string const name = "high king maulgar misdirect olm and blindeye") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class GruulTheDragonkillerMainTankPositionBossAction : public AttackAction
{
public:
    GruulTheDragonkillerMainTankPositionBossAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller main tank position boss") : AttackAction(botAI, name) {};

    bool Execute(Event event) override;
};

class GruulTheDragonkillerSpreadRangedAction : public MovementAction
{
public:
    GruulTheDragonkillerSpreadRangedAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller spread ranged") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

class GruulTheDragonkillerShatterSpreadAction : public MovementAction
{
public:
    GruulTheDragonkillerShatterSpreadAction(PlayerbotAI* botAI, std::string const name = "gruul the dragonkiller shatter spread") : MovementAction(botAI, name) {};

    bool Execute(Event event) override;
};

#endif
