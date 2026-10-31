/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BTVALUECONTEXT_H
#define PLAYERBOTS_BTVALUECONTEXT_H

#include "BTHelpers.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Value.h"
#include <vector>

// Trash

class SistersOfPleasureValue : public CalculatedValue<GuidVector>
{
public:
    SistersOfPleasureValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "sisters of pleasure", BtHelpers::SISTER_OF_PLEASURE_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindSisterOfPleasureGuids(botAI); }
};

class ShadowmoonReaversValue : public CalculatedValue<GuidVector>
{
public:
    ShadowmoonReaversValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "shadowmoon reavers", BtHelpers::SHADOWMOON_REAVER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindAttackerGuidsWithEntry(
            botAI, BtHelpers::Id(BtHelpers::BtNpcs::NPC_SHADOWMOON_REAVER));
    }
};

// Supremus

class SupremusHazardsValue : public CalculatedValue<std::vector<BtHelpers::SupremusHazard>>
{
public:
    SupremusHazardsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<BtHelpers::SupremusHazard>>(
            botAI, "supremus hazards", BtHelpers::SUPREMUS_HAZARD_CACHE_INTERVAL_MS) {}

protected:
    std::vector<BtHelpers::SupremusHazard> Calculate() override
    {
        return BtHelpers::FindSupremusHazards(botAI);
    }
};

// Shade of Akama

class ShadeOfAkamaAddsValue : public CalculatedValue<GuidVector>
{
public:
    ShadeOfAkamaAddsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "shade of akama adds", BtHelpers::SHADE_OF_AKAMA_ADD_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindShadeOfAkamaAddGuids(botAI); }
};

// Teron Gorefiend

class TeronGorefiendShadowyConstructsValue : public CalculatedValue<GuidVector>
{
public:
    TeronGorefiendShadowyConstructsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "teron gorefiend shadowy constructs",
            BtHelpers::GOREFIEND_CONSTRUCT_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindAttackerGuidsWithEntry(
            botAI, BtHelpers::Id(BtHelpers::BtNpcs::NPC_SHADOWY_CONSTRUCT));
    }
};

// Gurtogg Bloodboil

class GurtoggBloodboilSecondTankThreatValue : public FloatCalculatedValue
{
public:
    GurtoggBloodboilSecondTankThreatValue(PlayerbotAI* botAI)
        : FloatCalculatedValue(
            botAI, "gurtogg bloodboil second tank threat",
            BtHelpers::GURTOGG_TANK_THREAT_CACHE_INTERVAL_MS) {}

protected:
    float Calculate() override { return BtHelpers::FindGurtoggSecondTankThreat(botAI); }
};

// Illidan Stormrage <The Betrayer>

class IllidanStormrageSpeechInProgressValue : public BoolCalculatedValue
{
public:
    IllidanStormrageSpeechInProgressValue(PlayerbotAI* botAI)
        : BoolCalculatedValue(
            botAI, "illidan stormrage speech in progress",
            BtHelpers::ILLIDAN_SPEECH_CACHE_INTERVAL_MS) {}

protected:
    bool Calculate() override { return BtHelpers::IsIllidanSpeechInProgress(bot); }
};

class IllidanStormragePlayersWithParasiticShadowfiendValue : public CalculatedValue<GuidVector>
{
public:
    IllidanStormragePlayersWithParasiticShadowfiendValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "illidan stormrage players with parasitic shadowfiend",
            BtHelpers::ILLIDAN_PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override
    {
        return BtHelpers::FindPlayersWithParasiticShadowfiendGuids(bot);
    }
};

class IllidanStormrageTrapHuntersValue : public CalculatedValue<GuidVector>
{
public:
    IllidanStormrageTrapHuntersValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "illidan stormrage trap hunters",
            BtHelpers::ILLIDAN_TRAP_HUNTER_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindIllidanTrapHunterGuids(bot); }
};

class IllidanStormrageParasiticShadowfiendsValue : public CalculatedValue<GuidVector>
{
public:
    IllidanStormrageParasiticShadowfiendsValue(PlayerbotAI* botAI)
        : CalculatedValue<GuidVector>(
            botAI, "illidan stormrage parasitic shadowfiends",
            BtHelpers::ILLIDAN_PARASITIC_SHADOWFIEND_CACHE_INTERVAL_MS) {}

protected:
    GuidVector Calculate() override { return BtHelpers::FindParasiticShadowfiendGuids(botAI); }
};

class IllidanStormrageShadowDemonsValue
    : public CalculatedValue<std::vector<BtHelpers::ShadowDemon>>
{
public:
    IllidanStormrageShadowDemonsValue(PlayerbotAI* botAI)
        : CalculatedValue<std::vector<BtHelpers::ShadowDemon>>(
            botAI, "illidan stormrage shadow demons",
            BtHelpers::ILLIDAN_SHADOW_DEMON_CACHE_INTERVAL_MS) {}

protected:
    std::vector<BtHelpers::ShadowDemon> Calculate() override
    {
        return BtHelpers::FindShadowDemons(botAI);
    }
};

class RaidBlackTempleValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RaidBlackTempleValueContext()
    {
        creators["sisters of pleasure"] = &RaidBlackTempleValueContext::sisters_of_pleasure;
        creators["shadowmoon reavers"] = &RaidBlackTempleValueContext::shadowmoon_reavers;
        creators["supremus hazards"] = &RaidBlackTempleValueContext::supremus_hazards;
        creators["shade of akama adds"] = &RaidBlackTempleValueContext::shade_of_akama_adds;
        creators["teron gorefiend shadowy constructs"] =
            &RaidBlackTempleValueContext::teron_gorefiend_shadowy_constructs;
        creators["gurtogg bloodboil second tank threat"] =
            &RaidBlackTempleValueContext::gurtogg_bloodboil_second_tank_threat;
        creators["illidan stormrage speech in progress"] =
            &RaidBlackTempleValueContext::illidan_stormrage_speech_in_progress;
        creators["illidan stormrage players with parasitic shadowfiend"] =
            &RaidBlackTempleValueContext::illidan_stormrage_players_with_parasitic_shadowfiend;
        creators["illidan stormrage trap hunters"] =
            &RaidBlackTempleValueContext::illidan_stormrage_trap_hunters;
        creators["illidan stormrage parasitic shadowfiends"] =
            &RaidBlackTempleValueContext::illidan_stormrage_parasitic_shadowfiends;
        creators["illidan stormrage shadow demons"] =
            &RaidBlackTempleValueContext::illidan_stormrage_shadow_demons;
    }

private:
    static UntypedValue* sisters_of_pleasure(PlayerbotAI* botAI)
    {
        return new SistersOfPleasureValue(botAI);
    }
    static UntypedValue* shadowmoon_reavers(PlayerbotAI* botAI)
    {
        return new ShadowmoonReaversValue(botAI);
    }
    static UntypedValue* supremus_hazards(PlayerbotAI* botAI)
    {
        return new SupremusHazardsValue(botAI);
    }
    static UntypedValue* shade_of_akama_adds(PlayerbotAI* botAI)
    {
        return new ShadeOfAkamaAddsValue(botAI);
    }
    static UntypedValue* teron_gorefiend_shadowy_constructs(PlayerbotAI* botAI)
    {
        return new TeronGorefiendShadowyConstructsValue(botAI);
    }
    static UntypedValue* gurtogg_bloodboil_second_tank_threat(PlayerbotAI* botAI)
    {
        return new GurtoggBloodboilSecondTankThreatValue(botAI);
    }
    static UntypedValue* illidan_stormrage_speech_in_progress(PlayerbotAI* botAI)
    {
        return new IllidanStormrageSpeechInProgressValue(botAI);
    }
    static UntypedValue* illidan_stormrage_players_with_parasitic_shadowfiend(PlayerbotAI* botAI)
    {
        return new IllidanStormragePlayersWithParasiticShadowfiendValue(botAI);
    }
    static UntypedValue* illidan_stormrage_trap_hunters(PlayerbotAI* botAI)
    {
        return new IllidanStormrageTrapHuntersValue(botAI);
    }
    static UntypedValue* illidan_stormrage_parasitic_shadowfiends(PlayerbotAI* botAI)
    {
        return new IllidanStormrageParasiticShadowfiendsValue(botAI);
    }
    static UntypedValue* illidan_stormrage_shadow_demons(PlayerbotAI* botAI)
    {
        return new IllidanStormrageShadowDemonsValue(botAI);
    }
};

#endif
