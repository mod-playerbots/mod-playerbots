#ifndef _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H
#define _PLAYERBOT_RAIDKARAZHANMULTIPLIERS_H

#include "Multiplier.h"

class AttumenTheHuntsmanDisableTankAssistMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "attumen the huntsman disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class AttumenTheHuntsmanStayStackedMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanStayStackedMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "attumen the huntsman stay stacked multiplier") {}
    virtual float GetValue(Action* action);
};

class AttumenTheHuntsmanWaitForDpsMultiplier : public Multiplier
{
public:
    AttumenTheHuntsmanWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "attumen the huntsman wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class TheCuratorDisableTankAssistMultiplier : public Multiplier
{
public:
    TheCuratorDisableTankAssistMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the curator disable tank assist multiplier") {}
    virtual float GetValue(Action* action);
};

class TheCuratorDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    TheCuratorDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "the curator delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

class ShadeOfAranArcaneExplosionDisableChargeMultiplier : public Multiplier
{
public:
    ShadeOfAranArcaneExplosionDisableChargeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "shade of aran arcane explosion disable charge multiplier") {}
    virtual float GetValue(Action* action);
};

class ShadeOfAranFlameWreathDisableMovementMultiplier : public Multiplier
{
public:
    ShadeOfAranFlameWreathDisableMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "shade of aran flame wreath disable movement multiplier") {}
    virtual float GetValue(Action* action);
};

class NetherspiteKeepBlockingBeamMultiplier : public Multiplier
{
public:
    NetherspiteKeepBlockingBeamMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "netherspite keep blocking beam multiplier") {}
    virtual float GetValue(Action* action);
};

class NetherspiteWaitForDpsMultiplier : public Multiplier
{
public:
    NetherspiteWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "netherspite wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class PrinceMalchezaarDisableAvoidAoeMultiplier : public Multiplier
{
public:
    PrinceMalchezaarDisableAvoidAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "prince malchezaar disable avoid aoe multiplier") {}
    virtual float GetValue(Action* action);
};

class PrinceMalchezaarEnfeebleKeepDistanceMultiplier : public Multiplier
{
public:
    PrinceMalchezaarEnfeebleKeepDistanceMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "prince malchezaar enfeeble keep distance multiplier") {}
    virtual float GetValue(Action* action);
};

class PrinceMalchezaarDelayBloodlustAndHeroismMultiplier : public Multiplier
{
public:
    PrinceMalchezaarDelayBloodlustAndHeroismMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "prince malchezaar delay bloodlust and heroism multiplier") {}
    virtual float GetValue(Action* action);
};

class NightbaneDisablePetsMultiplier : public Multiplier
{
public:
    NightbaneDisablePetsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nightbane disable pets multiplier") {}
    virtual float GetValue(Action* action);
};

class NightbaneWaitForDpsMultiplier : public Multiplier
{
public:
    NightbaneWaitForDpsMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nightbane wait for dps multiplier") {}
    virtual float GetValue(Action* action);
};

class NightbaneDisableAvoidAoeMultiplier : public Multiplier
{
public:
    NightbaneDisableAvoidAoeMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nightbane disable avoid aoe multiplier") {}
    virtual float GetValue(Action* action);
};

class NightbaneDisableMovementMultiplier : public Multiplier
{
public:
    NightbaneDisableMovementMultiplier(
        PlayerbotAI* botAI) : Multiplier(botAI, "nightbane disable movement multiplier") {}
    virtual float GetValue(Action* action);
};

#endif
