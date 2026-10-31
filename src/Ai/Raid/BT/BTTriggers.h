/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTTRIGGERS_H
#define PLAYERBOTS_BTTRIGGERS_H

#include "BTHelpers.h"
#include "EncounterHelpers.h"
#include "Trigger.h"
#include <optional>
#include <string>

// Shared

class BlackTempleEncounterTrigger : public Trigger
{
public:
    BlackTempleEncounterTrigger(
        PlayerbotAI* botAI, std::string const name, BtHelpers::BtBosses boss,
        int32 checkInterval = 1)
        : Trigger(botAI, name, checkInterval), _boss(boss) {}

    bool IsActive() final
    {
        return EncounterHelpers::IsBossInProgress(
            bot, BtHelpers::BT_MAP_ID, BtHelpers::Id(_boss)) && IsActiveInEncounter();
    }

protected:
    virtual bool IsActiveInEncounter() = 0;

    Creature* GetEncounterBoss() { return BtHelpers::GetBtBoss(bot, _boss); }

private:
    BtHelpers::BtBosses const _boss;
};

class BlackTempleNoEncounterInProgressTrigger : public Trigger
{
public:
    BlackTempleNoEncounterInProgressTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "black temple no encounter in progress", 1000) {}
    bool IsActive() override;
};

class BlackTempleClassRolesShouldBeAssignedTrigger : public Trigger
{
public:
    BlackTempleClassRolesShouldBeAssignedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "black temple class roles should be assigned", 1000) {}
    bool IsActive() override;
};

class BlackTempleHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    BlackTempleHunterShouldMisdirectTrigger(
        PlayerbotAI* botAI, std::string const& name, BtHelpers::BtBosses boss,
        std::optional<BtHelpers::BtCreatureData> target = std::nullopt)
        : BlackTempleEncounterTrigger(botAI, name, boss), _target(target) {}

protected:
    bool IsActiveInEncounter() override;

private:
    std::optional<BtHelpers::BtCreatureData> const _target;
};

// Trash

// The throttle compensates for this trigger not being encounter gated. The first Shell of Pain is
// applied 20-30s into the pull.
class SisterOfPleasureShouldBeFocusedTrigger : public Trigger
{
public:
    SisterOfPleasureShouldBeFocusedTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "sister of pleasure should be focused", 1000) {}
    bool IsActive() override;
};

class ShadowmoonReaverWandBuildsChargesTrigger : public Trigger
{
public:
    ShadowmoonReaverWandBuildsChargesTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shadowmoon reaver wand builds charges") {}
    bool IsActive() override;
};

class ShadowmoonReaverShouldControlCasterPetTrigger : public Trigger
{
public:
    ShadowmoonReaverShouldControlCasterPetTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "shadowmoon reaver should control caster pet") {}
    bool IsActive() override;
};

// High Warlord Naj'entus

class HighWarlordNajentusShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus should be tanked",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus ranged should spread",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusImpaledPlayerNeedsRemoverTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus impaled player needs remover",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusImpalingSpineOnGroupMemberTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusImpalingSpineOnGroupMemberTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus impaling spine on group member",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusNeedsSpineThrowerTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusNeedsSpineThrowerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus needs spine thrower",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS, 1000) {}

protected:
    bool IsActiveInEncounter() override;
};

class HighWarlordNajentusHasTidalShieldTrigger : public BlackTempleEncounterTrigger
{
public:
    HighWarlordNajentusHasTidalShieldTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "high warlord naj'entus has tidal shield",
            BtHelpers::BtBosses::DATA_HIGH_WARLORD_NAJENTUS) {}

protected:
    bool IsActiveInEncounter() override;
};

// Supremus

class SupremusShouldBePulledBackByTankTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusShouldBePulledBackByTankTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "supremus should be pulled back by tank", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "supremus ranged should spread", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusFixatedOnBotTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusFixatedOnBotTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "supremus fixated on bot", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusNearFireTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusNearFireTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "supremus near fire", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    bool IsActiveInEncounter() override;
};

class SupremusReachBlockedByFireTrigger : public BlackTempleEncounterTrigger
{
public:
    SupremusReachBlockedByFireTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "supremus reach blocked by fire", BtHelpers::BtBosses::DATA_SUPREMUS) {}

protected:
    bool IsActiveInEncounter() override;
};

// Shade of Akama

class ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger : public BlackTempleEncounterTrigger
{
public:
    ShadeOfAkamaShouldAttackChannelersAndSorcerersTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "shade of akama should attack channelers and sorcerers",
            BtHelpers::BtBosses::DATA_SHADE_OF_AKAMA) {}

protected:
    bool IsActiveInEncounter() override;
};

// Teron Gorefiend

class TeronGorefiendShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend should be tanked", BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendRangedShouldPositionOnBalconyTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendRangedShouldPositionOnBalconyTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend ranged should position on balcony",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendCastsShadowOfDeathTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendCastsShadowOfDeathTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend casts shadow of death",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendShouldPositionForVengefulSpiritTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendShouldPositionForVengefulSpiritTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend should position for vengeful spirit",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    bool IsActiveInEncounter() override;
};

class TeronGorefiendTransformedIntoVengefulSpiritTrigger : public BlackTempleEncounterTrigger
{
public:
    TeronGorefiendTransformedIntoVengefulSpiritTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "teron gorefiend transformed into vengeful spirit",
            BtHelpers::BtBosses::DATA_TERON_GOREFIEND) {}

protected:
    bool IsActiveInEncounter() override;
};

// Gurtogg Bloodboil

class GurtoggBloodboilShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil should be tanked",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilShouldPositionForBloodboilTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilShouldPositionForBloodboilTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil should position for bloodboil",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilMeleeShouldStayBehindTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilMeleeShouldStayBehindTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil melee should stay behind",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilRangedShouldAvoidFrontalConeTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil ranged should avoid frontal cone",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class GurtoggBloodboilFelRageOnBotTrigger : public BlackTempleEncounterTrigger
{
public:
    GurtoggBloodboilFelRageOnBotTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "gurtogg bloodboil fel rage on bot",
            BtHelpers::BtBosses::DATA_GURTOGG_BLOODBOIL) {}

protected:
    bool IsActiveInEncounter() override;
};

// Reliquary of Souls

class ReliquaryOfSoulsHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls hunter should misdirect",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsShouldPositionForSufferingTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsShouldPositionForSufferingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls should position for suffering",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsHealersShouldAttackSufferingTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsHealersShouldAttackSufferingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls healers should attack suffering",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireHasRuneShieldTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls essence of desire has rune shield",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    bool IsActiveInEncounter() override;
};

class ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger : public BlackTempleEncounterTrigger
{
public:
    ReliquaryOfSoulsEssenceOfDesireCastingDeadenTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "reliquary of souls essence of desire casting deaden",
            BtHelpers::BtBosses::DATA_RELIQUARY_OF_SOULS) {}

protected:
    bool IsActiveInEncounter() override;
};

// Mother Shahraz

class MotherShahrazShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "mother shahraz should be tanked", BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazBeingPositionedTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazBeingPositionedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "mother shahraz being positioned", BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazRangedShouldPositionUnderStatueTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazRangedShouldPositionUnderStatueTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "mother shahraz ranged should position under statue",
            BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    bool IsActiveInEncounter() override;
};

class MotherShahrazFatalAttractionTrigger : public BlackTempleEncounterTrigger
{
public:
    MotherShahrazFatalAttractionTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "mother shahraz fatal attraction", BtHelpers::BtBosses::DATA_MOTHER_SHAHRAZ) {}

protected:
    bool IsActiveInEncounter() override;
};

// Illidari Council

class IllidariCouncilGathiosShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilGathiosShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council gathios should be tanked",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilGathiosCastingJudgementOfCommandTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilGathiosCastingJudgementOfCommandTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council gathios casting judgement of command",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMalandeCastingCircleOfHealingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMalandeCastingCircleOfHealingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council malande casting circle of healing",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMalandeShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMalandeShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council malande should be tanked",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilVerasShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilVerasShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council veras should be tanked",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilZerevorShouldBeTankedByMageTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilZerevorShouldBeTankedByMageTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council zerevor should be tanked by mage",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilMageTankHealerShouldRepositionTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilMageTankHealerShouldRepositionTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council mage tank healer should reposition",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council ranged should spread",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilPetShouldSwitchTargetTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilPetShouldSwitchTargetTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council pet should switch target",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilShouldAssignDpsPriorityTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilShouldAssignDpsPriorityTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council should assign dps priority",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidariCouncilShouldManageDpsTimerTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidariCouncilShouldManageDpsTimerTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidari council should manage dps timer",
            BtHelpers::BtBosses::DATA_ILLIDARI_COUNCIL) {}

protected:
    bool IsActiveInEncounter() override;
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageShouldPositionForPullTrigger : public Trigger
{
public:
    IllidanStormrageShouldPositionForPullTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "illidan stormrage should position for pull", 1000) {}
    bool IsActive() override;
};

class IllidanStormrageHunterShouldMisdirectTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageHunterShouldMisdirectTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage hunter should misdirect",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should be tanked",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageParasiticShadowfiendOnGroupMemberTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage parasitic shadowfiend on group member",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldTrackInfectionsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldTrackInfectionsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should track infections",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageFlamesOfAzzinothShouldBeTankedTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage flames of azzinoth should be tanked",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetsDieToHazardsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetsDieToHazardsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage pets die to hazards",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetShouldAttackShadowfiendTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetShouldAttackShadowfiendTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage pet should attack shadowfiend",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormragePetShouldAttackShadowDemonTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormragePetShouldAttackShadowDemonTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage pet should attack shadow demon",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldPositionAboveGrateTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldPositionAboveGrateTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should position above grate",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageImmunityCanClearDebuffTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageImmunityCanClearDebuffTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage immunity can clear debuff",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageImmunityNoLongerNeededTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageImmunityNoLongerNeededTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage immunity no longer needed",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger
    : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShamanHasTotemsBeforeTanksHaveAggroTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage shaman has totems before tanks have aggro",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldPositionForLandingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldPositionForLandingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should position for landing",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageMeleeShouldStopAttackingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageMeleeShouldStopAttackingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage melee should stop attacking",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldStopShootingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldStopShootingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should stop shooting",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageRangedShouldSpreadTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageRangedShouldSpreadTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage ranged should spread",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageRangedShouldReachShadowDemonTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageRangedShouldReachShadowDemonTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage ranged should reach shadow demon",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageMeleeShouldStayAwayFromDemonFormTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage melee should stay away from demon form",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageWarlockShouldTankDemonFormTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageWarlockShouldTankDemonFormTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage warlock should tank demon form",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageWarlockShouldSeedShadowDemonsTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageWarlockShouldSeedShadowDemonsTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage warlock should seed shadow demons",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldAssignTargetPriorityTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldAssignTargetPriorityTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should assign target priority",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldManageDpsTimersTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldManageDpsTimersTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should manage dps timers",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

class IllidanStormrageShouldClearHazardsBeforeLandingTrigger : public BlackTempleEncounterTrigger
{
public:
    IllidanStormrageShouldClearHazardsBeforeLandingTrigger(PlayerbotAI* botAI)
        : BlackTempleEncounterTrigger(
            botAI, "illidan stormrage should clear hazards before landing",
            BtHelpers::BtBosses::DATA_ILLIDAN_STORMRAGE) {}

protected:
    bool IsActiveInEncounter() override;
};

#endif
