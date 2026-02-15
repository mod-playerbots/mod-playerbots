#ifndef _PLAYERBOT_RAIDSSCMULTIPLIERS_H
#define _PLAYERBOT_RAIDSSCMULTIPLIERS_H

#include "Multiplier.h"

// Trash

class UnderbogColossusEscapeToxicPoolMultiplier : public Multiplier
{
public:
    UnderbogColossusEscapeToxicPoolMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "underbog colossus escape toxic pool") {}
    virtual float GetValue(Action* action);
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableTankActionsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable disable tank actions") {}
    virtual float GetValue(Action* action);
};

class HydrossTheUnstableWaitForDpsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable wait for dps") {}
    virtual float GetValue(Action* action);
};

class HydrossTheUnstableControlMisdirectionMultiplier : public Multiplier
{
public:
    HydrossTheUnstableControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable control misdirection") {}
    virtual float GetValue(Action* action);
};

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public Multiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below stay away from spout") {}
    virtual float GetValue(Action* action);
};

class TheLurkerBelowMaintainRangedSpreadMultiplier : public Multiplier
{
public:
    TheLurkerBelowMaintainRangedSpreadMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below maintain ranged spread") {}
    virtual float GetValue(Action* action);
};

class TheLurkerBelowDisableTankAssistMultiplier : public Multiplier
{
public:
    TheLurkerBelowDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below disable tank assist") {}
    virtual float GetValue(Action* action);
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public Multiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind avoid whirlwind") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDisableTankActionsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind disable tank actions") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier : public Multiplier
{
public:
    LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind melee dps avoid chaos blast") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public Multiplier
{
public:
    LeotherasTheBlindFocusOnInnerDemonMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind focus on inner demon") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindWaitForDpsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind wait for dps") {}
    virtual float GetValue(Action* action);
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable tank actions") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressDisableAoeMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable aoe") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressControlMisdirectionMultiplier : public Multiplier
{
public:
    FathomLordKarathressControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress control misdirection") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressWaitForDpsMultiplier : public Multiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress wait for dps") {}
    virtual float GetValue(Action* action);
};

class FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier : public Multiplier
{
public:
    FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress caribdis tank healer maintain position") {}
    virtual float GetValue(Action* action);
};

// Morogrim Tidewalker

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}
    virtual float GetValue(Action* action);
};

class MorogrimTidewalkerDisableTankActionsMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker disable tank actions") {}
    virtual float GetValue(Action* action);
};

class MorogrimTidewalkerMaintainPhase2StackingMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerMaintainPhase2StackingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker maintain phase2 stacking") {}
    virtual float GetValue(Action* action);
};

// Lady Vashj <Coilfang Matron>

class LadyVashjDelayCooldownsMultiplier : public Multiplier
{
public:
    LadyVashjDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj delay cooldowns") {}
    virtual float GetValue(Action* action);
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public Multiplier
{
public:
    LadyVashjMaintainPhase1RangedSpreadMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj maintain phase1 ranged spread") {}
    virtual float GetValue(Action* action);
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public Multiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj static charge stay away from group") {}
    virtual float GetValue(Action* action);
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public Multiplier
{
public:
    LadyVashjDoNotLootTheTaintedCoreMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj do not loot the tainted core") {}
    virtual float GetValue(Action* action);
};

class LadyVashjCorePassersPrioritizePositioningMultiplier : public Multiplier
{
public:
    LadyVashjCorePassersPrioritizePositioningMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj core passers prioritize positioning") {}
    virtual float GetValue(Action* action);
};

class LadyVashjDisableAutomaticTargetingAndMovementModifier : public Multiplier
{
public:
    LadyVashjDisableAutomaticTargetingAndMovementModifier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj disable automatic targeting and movement") {}
    virtual float GetValue(Action* action);
};

#endif
