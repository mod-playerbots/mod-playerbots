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
#include <optional>
#include <string>

// Shared

class Aq40EncounterMultiplier : public Multiplier
{
public:
    Aq40EncounterMultiplier(PlayerbotAI* botAI, std::string const name) : Multiplier(botAI, name) {}
    Aq40EncounterMultiplier(PlayerbotAI* botAI, std::string const name, Aq40Helpers::Aq40Encounter encounter)
        : Multiplier(botAI, name), _encounter(encounter)
    {
    }

    float GetValue(Action* action) final
    {
        if (!action)
            return 1.0f;
        bool const inProgress = _encounter ? Aq40Helpers::IsInEncounter(bot, *_encounter)
                                           : EncounterHelpers::IsEncounterInProgress(bot, Aq40Helpers::AQ40_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;

private:
    std::optional<Aq40Helpers::Aq40Encounter> const _encounter;
};

// Lets server-driven knockbacks and falls finish before any action, the strategy's own included, can replace them.
class Aq40WaitForKnockbackMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40WaitForKnockbackMultiplier(PlayerbotAI* botAI) : Aq40EncounterMultiplier(botAI, "aq40 wait for knockback") {}

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

class Aq40RangedHoldBossRangeMultiplier : public Aq40EncounterMultiplier
{
public:
    Aq40RangedHoldBossRangeMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "aq40 ranged hold boss range")
    {
    }

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
class TheProphetSkeramControlMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    TheProphetSkeramControlMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "the prophet skeram control movement", Aq40Helpers::Aq40Encounter::Skeram)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TheProphetSkeramHoldDamageInTransitMultiplier : public Aq40EncounterMultiplier
{
public:
    TheProphetSkeramHoldDamageInTransitMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "the prophet skeram hold damage in transit",
                                  Aq40Helpers::Aq40Encounter::Skeram)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// Bug Trio
class BugTrioControlTankMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    BugTrioControlTankMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "bug trio control tank movement", Aq40Helpers::Aq40Encounter::Trio)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// Viscidus
class ViscidusControlMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    ViscidusControlMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "viscidus control movement", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class ViscidusFrostOnlyMultiplier : public Aq40EncounterMultiplier
{
public:
    ViscidusFrostOnlyMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "viscidus frost only", Aq40Helpers::Aq40Encounter::Viscidus)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// Princess Huhuran
class PrincessHuhuranControlMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    PrincessHuhuranControlMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "princess huhuran control movement", Aq40Helpers::Aq40Encounter::Huhuran)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class PrincessHuhuranControlWyvernStingDispelMultiplier : public Aq40EncounterMultiplier
{
public:
    PrincessHuhuranControlWyvernStingDispelMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "princess huhuran control wyvern sting dispel",
                                  Aq40Helpers::Aq40Encounter::Huhuran)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class PrincessHuhuranDelayDpsCooldownsMultiplier : public Aq40EncounterMultiplier
{
public:
    PrincessHuhuranDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "princess huhuran delay dps cooldowns", Aq40Helpers::Aq40Encounter::Huhuran)
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
        : Aq40EncounterMultiplier(botAI, "twin emperors opening heal hold", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsPriestShieldOnlyEmperorTargetsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors priest shield only emperor targets",
                                  Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsHoldDamageInTransitMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsHoldDamageInTransitMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors hold damage in transit", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsSkipImmuneTargetsMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsSkipImmuneTargetsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors skip immune targets", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsVeklorThreatCapMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsVeklorThreatCapMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors vek'lor threat cap", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsSpareTankThreatCapMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsSpareTankThreatCapMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors spare tank threat cap", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class TwinEmperorsTanksHoldAnchorMultiplier : public Aq40EncounterMultiplier
{
public:
    TwinEmperorsTanksHoldAnchorMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "twin emperors tanks hold anchor", Aq40Helpers::Aq40Encounter::Twins)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

// C'Thun
class CthunDisableFollowMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunDisableFollowMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun disable follow", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachExitOnlyMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachExitOnlyMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach exit only", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachNoTreeOfLifeMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachNoTreeOfLifeMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach no tree of life", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachNoFloorHealsMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachNoFloorHealsMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach no floor heals", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunStomachHoldMovementMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunStomachHoldMovementMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun stomach hold movement", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

class CthunMeleeBurnRangeMultiplier : public Aq40EncounterMultiplier
{
public:
    CthunMeleeBurnRangeMultiplier(PlayerbotAI* botAI)
        : Aq40EncounterMultiplier(botAI, "c'thun melee burn range", Aq40Helpers::Aq40Encounter::Cthun)
    {
    }

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
