/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SSCMULTIPLIERS_H
#define PLAYERBOTS_SSCMULTIPLIERS_H

#include "Multiplier.h"

// Trash

class UnderbogColossusEscapeToxicPoolMultiplier : public Multiplier
{
public:
    UnderbogColossusEscapeToxicPoolMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "underbog colossus escape toxic pool") {}
    float GetValue(Action* action) override;
};

// Hydross the Unstable <Duke of Currents>

class HydrossTheUnstableDisableTankActionsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable disable tank actions") {}
    float GetValue(Action* action) override;
};

class HydrossTheUnstableWaitForDpsMultiplier : public Multiplier
{
public:
    HydrossTheUnstableWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable wait for dps") {}
    float GetValue(Action* action) override;
};

class HydrossTheUnstableControlMisdirectionMultiplier : public Multiplier
{
public:
    HydrossTheUnstableControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "hydross the unstable control misdirection") {}
    float GetValue(Action* action) override;
};

// The Lurker Below

class TheLurkerBelowStayAwayFromSpoutMultiplier : public Multiplier
{
public:
    TheLurkerBelowStayAwayFromSpoutMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below stay away from spout") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowMaintainRangedSpreadMultiplier : public Multiplier
{
public:
    TheLurkerBelowMaintainRangedSpreadMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below maintain ranged spread") {}
    float GetValue(Action* action) override;
};

class TheLurkerBelowDisableTankAssistMultiplier : public Multiplier
{
public:
    TheLurkerBelowDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the lurker below disable tank assist") {}
    float GetValue(Action* action) override;
};

// Leotheras the Blind

class LeotherasTheBlindAvoidWhirlwindMultiplier : public Multiplier
{
public:
    LeotherasTheBlindAvoidWhirlwindMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind avoid whirlwind") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDisableTankActionsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind disable tank actions") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier : public Multiplier
{
public:
    LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind melee dps avoid chaos blast") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindFocusOnInnerDemonMultiplier : public Multiplier
{
public:
    LeotherasTheBlindFocusOnInnerDemonMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind focus on inner demon") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindWaitForDpsMultiplier : public Multiplier
{
public:
    LeotherasTheBlindWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind wait for dps") {}
    float GetValue(Action* action) override;
};

class LeotherasTheBlindDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "leotheras the blind delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

// Fathom-Lord Karathress

class FathomLordKarathressDisableTankActionsMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable tank actions") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressDisableAoeMultiplier : public Multiplier
{
public:
    FathomLordKarathressDisableAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress disable aoe") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressControlMisdirectionMultiplier : public Multiplier
{
public:
    FathomLordKarathressControlMisdirectionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress control misdirection") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressWaitForDpsMultiplier : public Multiplier
{
public:
    FathomLordKarathressWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress wait for dps") {}
    float GetValue(Action* action) override;
};

class FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier : public Multiplier
{
public:
    FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "fathom-lord karathress caribdis tank healer maintain position") {}
    float GetValue(Action* action) override;
};

// Morogrim Tidewalker

class MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker delay bloodlust and heroism") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerDisableTankActionsMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerDisableTankActionsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker disable tank actions") {}
    float GetValue(Action* action) override;
};

class MorogrimTidewalkerMaintainPhase2StackingMultiplier : public Multiplier
{
public:
    MorogrimTidewalkerMaintainPhase2StackingMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "morogrim tidewalker maintain phase2 stacking") {}
    float GetValue(Action* action) override;
};

// Lady Vashj <Coilfang Matron>

class LadyVashjDelayCooldownsMultiplier : public Multiplier
{
public:
    LadyVashjDelayCooldownsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj delay cooldowns") {}
    float GetValue(Action* action) override;
};

class LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier : public Multiplier
{
public:
    LadyVashjMainTankGroupShamanUseGroundingTotemMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj main tank group shaman use grounding totem") {}
    float GetValue(Action* action) override;
};

class LadyVashjMaintainPhase1RangedSpreadMultiplier : public Multiplier
{
public:
    LadyVashjMaintainPhase1RangedSpreadMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj maintain phase1 ranged spread") {}
    float GetValue(Action* action) override;
};

class LadyVashjStaticChargeStayAwayFromGroupMultiplier : public Multiplier
{
public:
    LadyVashjStaticChargeStayAwayFromGroupMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj static charge stay away from group") {}
    float GetValue(Action* action) override;
};

class LadyVashjDoNotLootTheTaintedCoreMultiplier : public Multiplier
{
public:
    LadyVashjDoNotLootTheTaintedCoreMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj do not loot the tainted core") {}
    float GetValue(Action* action) override;
};

class LadyVashjCorePassersPrioritizePositioningMultiplier : public Multiplier
{
public:
    LadyVashjCorePassersPrioritizePositioningMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj core passers prioritize positioning") {}
    float GetValue(Action* action) override;
};

class LadyVashjDisableAutomaticTargetingAndMovementModifier : public Multiplier
{
public:
    LadyVashjDisableAutomaticTargetingAndMovementModifier(
        PlayerbotAI* botAI) : Multiplier(botAI, "lady vashj disable automatic targeting and movement") {}
    float GetValue(Action* action) override;
};

#endif
