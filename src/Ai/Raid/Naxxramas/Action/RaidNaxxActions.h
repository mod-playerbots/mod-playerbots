#pragma once

#include "Action.h"
#include "AttackAction.h"
#include "GenericActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "RaidNaxxBossHelper.h"

class GrobbulusGoBehindAction : public MovementAction
{
public:
    GrobbulusGoBehindAction(PlayerbotAI* ai, float distance = 24.0f, float delta_angle = M_PI / 8)
        : MovementAction(ai, "grobbulus go behind")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

class GrobbulusRotateAction : public RotateAroundTheCenterPointAction
{
public:
    GrobbulusRotateAction(PlayerbotAI* botAI)
        : RotateAroundTheCenterPointAction(botAI, "rotate grobbulus", 3281.23f, -3310.38f, 35.0f, 8, true, M_PI) {}
    virtual bool isUseful() override
    {
        const Value<bool>* const hasAggro = this->context->GetValue<bool>("has aggro", "boss target");

        if (hasAggro == nullptr)
        {
            return false;
        }

        return RotateAroundTheCenterPointAction::isUseful()
                && this->botAI->IsMainTank(this->bot)
                && hasAggro;
    }
    uint32 GetCurrWaypoint() override;
};

class GrobbulusMoveCenterAction : public MoveInsideAction
{
public:
    GrobbulusMoveCenterAction(PlayerbotAI* ai) : MoveInsideAction(ai, 3281.23f, -3310.38f, 5.0f) {}
};

class GrobbulusMoveAwayAction : public MovementAction
{
public:
    GrobbulusMoveAwayAction(PlayerbotAI* ai, float distance = 18.0f)
        : MovementAction(ai, "grobbulus move away"), distance(distance)
    {
    }
    bool Execute(Event event) override;

private:
    float distance;
};

class ThaddiusAttackNearestPetAction : public AttackAction
{
public:
    ThaddiusAttackNearestPetAction(PlayerbotAI* ai) : AttackAction(ai, "thaddius attack nearest pet"), helper(ai) {}
    virtual bool Execute(Event event);
    virtual bool isUseful();

private:
    ThaddiusBossHelper helper;
};

class ThaddiusMoveToPlatformAction : public MovementAction
{
public:
    ThaddiusMoveToPlatformAction(PlayerbotAI* ai) : MovementAction(ai, "thaddius move to platform") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class ThaddiusMovePolarityAction : public MovementAction
{
public:
    ThaddiusMovePolarityAction(PlayerbotAI* ai) : MovementAction(ai, "thaddius move polarity") {}
    virtual bool Execute(Event event);
    virtual bool isUseful();
};

class RazuviousUseObedienceCrystalAction : public MovementAction
{
public:
    RazuviousUseObedienceCrystalAction(PlayerbotAI* ai)
        : MovementAction(ai, "razuvious use obedience crystal"), helper(ai)
    {
    }
    bool Execute(Event event) override;

private:
    RazuviousBossHelper helper;
};

class RazuviousTargetAction : public AttackAction
{
public:
    RazuviousTargetAction(PlayerbotAI* ai) : AttackAction(ai, "razuvious target"), helper(ai) {}
    bool Execute(Event event) override;

private:
    RazuviousBossHelper helper;
};

class FourHorsemenAttractAlternativelyAction : public AttackAction
{
public:
    FourHorsemenAttractAlternativelyAction(PlayerbotAI* ai) : AttackAction(ai, "horseman attract alternatively"), helper(ai)
    {
    }
    bool Execute(Event event) override;

protected:
    FourhorsemanBossHelper helper;
};

class FourHorsemenAttactInOrderAction : public AttackAction
{
public:
    FourHorsemenAttactInOrderAction(PlayerbotAI* ai) : AttackAction(ai, "horseman attact in order"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    FourhorsemanBossHelper helper;
};

class SapphironGroundPositionAction : public MovementAction
{
public:
    SapphironGroundPositionAction(PlayerbotAI* ai) : MovementAction(ai, "sapphiron ground position"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    SapphironBossHelper helper;
};

class SapphironFlightPositionAction : public MovementAction
{
public:
    SapphironFlightPositionAction(PlayerbotAI* ai) : MovementAction(ai, "sapphiron flight position"), helper(ai) {}
    bool Execute(Event event) override;

protected:
    SapphironBossHelper helper;
    bool MoveToNearestIcebolt();
};

class KelthuzadChooseTargetAction : public AttackAction
{
public:
    KelthuzadChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "kel'thuzad choose target"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    KelthuzadBossHelper helper;
};

class KelthuzadPositionAction : public MovementAction
{
public:
    KelthuzadPositionAction(PlayerbotAI* ai) : MovementAction(ai, "kel'thuzad position"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    KelthuzadBossHelper helper;
};

class AnubrekhanChooseTargetAction : public AttackAction
{
public:
    AnubrekhanChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "anub'rekhan choose target") {}
    bool Execute(Event event) override;
};

class AnubrekhanPositionAction : public RotateAroundTheCenterPointAction
{
public:
    AnubrekhanPositionAction(PlayerbotAI* ai)
        : RotateAroundTheCenterPointAction(ai, "anub'rekhan position", 3272.49f, -3476.27f, 45.0f, 16) {}
    bool Execute(Event event) override;
};

class GluthChooseTargetAction : public AttackAction
{
public:
    GluthChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "gluth choose target"), helper(ai) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class GluthPositionAction : public RotateAroundTheCenterPointAction
{
public:
    GluthPositionAction(PlayerbotAI* ai)
        : RotateAroundTheCenterPointAction(ai, "gluth position", 3293.61f, -3149.01f, 12.0f, 12), helper(ai) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class GluthSlowdownAction : public Action
{
public:
    GluthSlowdownAction(PlayerbotAI* ai) : Action(ai, "gluth slowdown"), helper(ai) {}
    bool Execute(Event event) override;

private:
    GluthBossHelper helper;
};

class LoathebPositionAction : public MovementAction
{
public:
    LoathebPositionAction(PlayerbotAI* ai) : MovementAction(ai, "loatheb position"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    LoathebBossHelper helper;
};

class LoathebChooseTargetAction : public AttackAction
{
public:
    LoathebChooseTargetAction(PlayerbotAI* ai) : AttackAction(ai, "loatheb choose target"), helper(ai) {}
    virtual bool Execute(Event event);

private:
    LoathebBossHelper helper;
};
