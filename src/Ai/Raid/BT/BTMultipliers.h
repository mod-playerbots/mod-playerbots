/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTMULTIPLIERS_H
#define PLAYERBOTS_BTMULTIPLIERS_H

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Multiplier.h"
#include <optional>
#include <string>

// Shared

class BlackTempleEncounterMultiplier : public Multiplier
{
public:
    BlackTempleEncounterMultiplier(PlayerbotAI* botAI, std::string const name)
        : Multiplier(botAI, name) {}
    BlackTempleEncounterMultiplier(
        PlayerbotAI* botAI, std::string const name, BtHelpers::BtBosses boss)
        : Multiplier(botAI, name), _boss(boss) {}

    float GetValue(Action* action) final
    {
        bool const inProgress = _boss ?
            EncounterHelpers::IsBossInProgress(
                bot, BtHelpers::BT_MAP_ID, BtHelpers::Id(*_boss)) :
            EncounterHelpers::IsEncounterInProgress(bot, BtHelpers::BT_MAP_ID);
        return inProgress ? GetValueInEncounter(action) : 1.0f;
    }

protected:
    virtual float GetValueInEncounter(Action* action) = 0;

    Creature* GetEncounterBoss()
    {
        return _boss ? BtHelpers::GetBtBoss(bot, *_boss) : nullptr;
    }

private:
    std::optional<BtHelpers::BtBosses> const _boss;
};

class BlackTempleDelayDpsCooldownsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleDelayDpsCooldownsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple delay dps cooldowns") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BlackTempleDisableKillingSpreeMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleDisableKillingSpreeMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple disable killing spree") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class BlackTempleNoThreatRedirectsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    BlackTempleNoThreatRedirectsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(botAI, "black temple no threat redirects") {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Trash

class ShadowmoonReaverHoldChargeBuildingSpellsMultiplier : public Multiplier
{
public:
    ShadowmoonReaverHoldChargeBuildingSpellsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "shadowmoon reaver hold charge building spells") {}

    float GetValue(Action* action) override;
};

// High Warlord Naj'entus

class HighWarlordNajentusDisableCombatFormationMoveMultiplier
    : public BlackTempleEncounterMultiplier
{
public:
    HighWarlordNajentusDisableCombatFormationMoveMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "high warlord naj'entus disable combat formation move",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class HighWarlordNajentusTidalShieldHoldPositionMultiplier : public BlackTempleEncounterMultiplier
{
public:
    HighWarlordNajentusTidalShieldHoldPositionMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "high warlord naj'entus tidal shield hold position",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Supremus

class SupremusRunAwayWhenFixatedMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusRunAwayWhenFixatedMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "supremus run away when fixated", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SupremusDoNotReachThroughFireMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusDoNotReachThroughFireMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "supremus do not reach through fire", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class SupremusControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    SupremusControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "supremus control movement", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Shade of Akama

class ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "shade of akama do not drop out of sight target",
            BtHelpers::BtBosses::DATA_SHADE_OF_AKAMA) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Teron Gorefiend

class TeronGorefiendControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "teron gorefiend control movement", BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TeronGorefiendMarkedBotOnlyMoveToDieMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "teron gorefiend marked bot only move to die",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class TeronGorefiendSpiritsAttackShadowyConstructsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    TeronGorefiendSpiritsAttackShadowyConstructsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "teron gorefiend spirits attack shadowy constructs",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "gurtogg bloodboil control movement",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GurtoggBloodboilFelRageTargetHoldsAggroMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilFelRageTargetHoldsAggroMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "gurtogg bloodboil fel rage target holds aggro",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class GurtoggBloodboilHoldThreatMultiplier : public BlackTempleEncounterMultiplier
{
public:
    GurtoggBloodboilHoldThreatMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "gurtogg bloodboil hold threat", BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Reliquary of Souls

class ReliquaryOfSoulsDoNotWasteHealingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsDoNotWasteHealingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "reliquary of souls do not waste healing",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ReliquaryOfSoulsControlMeleeMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsControlMeleeMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "reliquary of souls control melee movement",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "reliquary of souls let mages steal rune shield",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier : public BlackTempleEncounterMultiplier
{
public:
    ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "reliquary of souls save spell reflection for deaden",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Mother Shahraz

class MotherShahrazControlMovementMultiplier : public BlackTempleEncounterMultiplier
{
public:
    MotherShahrazControlMovementMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "mother shahraz control movement", BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class MotherShahrazFatalAttractionRunAwayMultiplier : public BlackTempleEncounterMultiplier
{
public:
    MotherShahrazFatalAttractionRunAwayMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "mother shahraz fatal attraction run away",
            BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Illidari Council

class IllidariCouncilControlMovementAndTargetingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council control movement and targeting",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    float GetValueInEncounter(Action* action) override;

private:
    float GetTankValue(Action* action);
    float GetNonTankValue(Action* action);
    float GetHealerValue(Action* action);
};

class IllidariCouncilDisableMageTankActionsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilDisableMageTankActionsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council disable mage tank actions",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilHoldMisdirectionAndDispelsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilHoldMisdirectionAndDispelsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council hold misdirection and dispels",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilManageInterruptsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilManageInterruptsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council manage interrupts",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidariCouncilWaitForDpsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidariCouncilWaitForDpsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidari council wait for dps", BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageHoldPullPositionsMultiplier : public Multiplier
{
public:
    IllidanStormrageHoldPullPositionsMultiplier(PlayerbotAI* botAI)
        : Multiplier(botAI, "illidan stormrage hold pull positions") {}

    float GetValue(Action* action) override;
};

class IllidanStormrageControlMovementAndTargetingMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageControlMovementAndTargetingMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage control movement and targeting",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;

private:
    float GetTargetingValue(
        BtHelpers::IllidanPhase phase, Action* action, bool isTankAssist, bool isDpsAssist);
    float GetTankMovementValue(Unit* illidan, BtHelpers::IllidanPhase phase, Action* action);
    float GetNonTankMovementValue(BtHelpers::IllidanPhase phase, Action* action);
};

class IllidanStormrageHoldNearNewShadowfiendMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldNearNewShadowfiendMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage hold near new shadowfiend",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageHoldGuardianSummonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldGuardianSummonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage hold guardian summons",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageHoldTotemsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageHoldTotemsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage hold totems", BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageWaitForDpsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageWaitForDpsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage wait for dps", BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageDisableTankSoulshatterMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableTankSoulshatterMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage disable tank soulshatter",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

class IllidanStormrageDisableAoeOnShadowDemonsMultiplier : public BlackTempleEncounterMultiplier
{
public:
    IllidanStormrageDisableAoeOnShadowDemonsMultiplier(PlayerbotAI* botAI)
        : BlackTempleEncounterMultiplier(
            botAI, "illidan stormrage disable aoe on shadow demons",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    float GetValueInEncounter(Action* action) override;
};

#endif
