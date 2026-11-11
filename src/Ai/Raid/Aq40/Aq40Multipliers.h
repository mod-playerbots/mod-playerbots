/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AQ40MULTIPLIERS_H
#define PLAYERBOTS_AQ40MULTIPLIERS_H

#include "Aq40Helpers.h"
#include "EncounterHelpers.h"
#include "Multiplier.h"
#include <string>

// Shared

class Aq40EncounterMultiplier : public Multiplier
{
public:
    Aq40EncounterMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}

    float GetValue(Action* action) final
    {
        return action && EncounterHelpers::IsEncounterInProgress(bot, Aq40Helpers::AQ40_MAP_ID)
                   ? GetValueInEncounter(action)
                   : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;
};

// Lets server-driven knockbacks and falls finish before any action, the strategy's own included, can replace them.
class Aq40WaitForKnockbackMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40WaitForKnockbackMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 wait for knockback") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40HazardEscapeMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40HazardEscapeMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 hazard escape") {}

protected:
    float GetValueInEncounter(Action* action) override;
};


class Aq40DontAttackMindControlledMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40DontAttackMindControlledMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "aq40 don't attack mind controlled")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40ControlTauntingMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40ControlTauntingMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 control taunting") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40TanksDisableTankAssistMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40TanksDisableTankAssistMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "aq40 tanks disable tank assist")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40DisableAutoTargetMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40DisableAutoTargetMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 disable auto target") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40AllowedDamageMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40AllowedDamageMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 allowed damage") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40DisableAoeMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40DisableAoeMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 disable aoe") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class Aq40MaintainFormationMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40MaintainFormationMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 maintain formation") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

// Not encounter gated: an Anubisath Defender reflects two schools for its whole fight.
class AnubisathDefenderAvoidReflectedSchoolsMultiplier : public Multiplier
{
public:
    AnubisathDefenderAvoidReflectedSchoolsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "anubisath defender avoid reflected schools")
    {
    }
    float GetValue(Action* action) override;
};

// The Prophet Skeram
class SkeramStayOnPlatformMultiplier : public Aq40EncounterMultiplier
{
public:
    SkeramStayOnPlatformMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "the prophet skeram stay on platform")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// Viscidus
class ViscidusFrostOnlyMultiplier : public Aq40EncounterMultiplier
{
public:
    ViscidusFrostOnlyMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "viscidus frost only") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Princess Huhuran
class HuhuranControlWyvernStingDispelMultiplier : public Aq40EncounterMultiplier
{
public:
    HuhuranControlWyvernStingDispelMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "princess huhuran control wyvern sting dispel")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class HuhuranDelayDpsCooldownsMultiplier : public Aq40EncounterMultiplier
{
public:
    HuhuranDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "princess huhuran delay dps cooldowns")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// Twin Emperors
class TwinEmperorsOpeningHealHoldMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsOpeningHealHoldMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors opening heal hold")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors priest shield only emperor targets")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsHoldDamageInTransitMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsHoldDamageInTransitMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors hold damage in transit")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsSkipImmuneTargetsMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsSkipImmuneTargetsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors skip immune targets")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsVeklorThreatCapMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsVeklorThreatCapMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors vek'lor threat cap")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsSpareTankThreatCapMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsSpareTankThreatCapMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors spare tank threat cap")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsTanksHoldAnchorMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsTanksHoldAnchorMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors tanks hold anchor")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// C'Thun
class CthunDisableFollowMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunDisableFollowMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "c'thun disable follow") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachExitOnlyMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachExitOnlyMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "c'thun stomach exit only") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachNoTreeOfLifeMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachNoTreeOfLifeMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach no tree of life")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachNoFloorHealsMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachNoFloorHealsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach no floor heals")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachHoldMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachHoldMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach hold movement")
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunMeleeBurnRangeMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunMeleeBurnRangeMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "c'thun melee burn range") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
