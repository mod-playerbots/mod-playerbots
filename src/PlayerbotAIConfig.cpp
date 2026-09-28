/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PlayerbotAIConfig.h"
#include "BisListMgr.h"
#include "Config.h"
#include "NewRpgInfo.h"
#include "PlayerbotDungeonRepository.h"
#include "PlayerbotFactory.h"
#include "PlayerbotGuildMgr.h"
#include "Playerbots.h"
#include "RandomItemMgr.h"
#include "RandomPlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "Talentspec.h"
#include "TravelMgr.h"
#include <cctype>
#include <iostream>
#include <sstream>

template <class T>
void LoadList(std::string const value, T& list)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        list.push_back(id);
    }
}

template <class T>
void LoadSet(std::string const value, T& set)
{
    std::vector<std::string> ids = split(value, ',');
    for (std::vector<std::string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        uint32 id = atoi((*i).c_str());
        // if (!id)
        //     continue;
        set.insert(id);
    }
}

template <class T>
void LoadListString(std::string const value, T& list)
{
    std::vector<std::string> strings = split(value, ',');
    for (std::vector<std::string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        std::string const string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}

// Parses a comma-separated, whitespace-tolerant bot name list (as used by both
// AiPlayerbot.LevelBrackets.ExcludeNames and AiPlayerbot.ResetBotLevel.ExcludeNames) into out.
static void ParseLevelMgrExcludeNames(std::string const& csv, std::vector<std::string>& out)
{
    out.clear();
    std::istringstream f(csv);
    std::string s;
    while (getline(f, s, ','))
    {
        s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
        if (!s.empty())
            out.push_back(s);
    }
}

bool PlayerbotAIConfig::Initialize()
{
    LOG_INFO("server.loading", "Initializing mod-playerbots, based on AI Playerbots by ike3 and the original Playerbots by blueboy");

    Enabled = sConfigMgr->GetOption<bool>("AiPlayerbot.Enabled", true);
    if (!Enabled)
    {
        LOG_INFO("server.loading", "Playerbots Module is disabled in playerbots.conf");
        return false;
    }

    GlobalCoolDown = sConfigMgr->GetOption<int32>("AiPlayerbot.GlobalCooldown", 500);
    MaxWaitForMove = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxWaitForMove", 5000);
    DisableMoveSplinePath = sConfigMgr->GetOption<int32>("AiPlayerbot.DisableMoveSplinePath", 0);
    MaxMovementSearchTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxMovementSearchTime", 3);
    ExpireActionTime = sConfigMgr->GetOption<int32>("AiPlayerbot.ExpireActionTime", 5000);
    DispelAuraDuration = sConfigMgr->GetOption<int32>("AiPlayerbot.DispelAuraDuration", 700);
    ReactDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReactDelay", 100);
    DynamicReactDelay = sConfigMgr->GetOption<bool>("AiPlayerbot.DynamicReactDelay", true);
    PassiveDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.PassiveDelay", 10000);
    RepeatDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RepeatDelay", 2000);
    ErrorDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ErrorDelay", 100);
    RpgDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgDelay", 10000);
    SitDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.SitDelay", 20000);
    ReturnDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.ReturnDelay", 2000);
    LootDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.LootDelay", 1000);
    DisabledWithoutRealPlayerLoginDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLoginDelay", 30);
    DisabledWithoutRealPlayerLogoutDelay = sConfigMgr->GetOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLogoutDelay", 300);

    FarDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FarDistance", 20.0f);
    SightDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SightDistance", 100.0f);
    SpellDistance = sConfigMgr->GetOption<float>("AiPlayerbot.SpellDistance", 28.5f);
    ShootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ShootDistance", 5.0f);
    HealDistance = sConfigMgr->GetOption<float>("AiPlayerbot.HealDistance", 38.5f);
    LootDistance = sConfigMgr->GetOption<float>("AiPlayerbot.LootDistance", 15.0f);
    FleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FleeDistance", 5.0f);
    AggroDistance = sConfigMgr->GetOption<float>("AiPlayerbot.AggroDistance", 22.0f);
    TooCloseDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TooCloseDistance", 5.0f);
    MeleeDistance = sConfigMgr->GetOption<float>("AiPlayerbot.MeleeDistance", 0.75f);
    FollowDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FollowDistance", 1.5f);
    WhisperDistance = sConfigMgr->GetOption<float>("AiPlayerbot.WhisperDistance", 6000.0f);
    ContactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ContactDistance", 0.45f);
    AoeRadius = sConfigMgr->GetOption<float>("AiPlayerbot.AoeRadius", 10.0f);
    RpgDistance = sConfigMgr->GetOption<float>("AiPlayerbot.RpgDistance", 200.0f);
    GrindDistance = sConfigMgr->GetOption<float>("AiPlayerbot.GrindDistance", 75.0f);
    ReactDistance = sConfigMgr->GetOption<float>("AiPlayerbot.ReactDistance", 150.0f);

    CriticalHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.CriticalHealth", 25);
    LowHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.LowHealth", 45);
    MediumHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumHealth", 65);
    AlmostFullHealth = sConfigMgr->GetOption<int32>("AiPlayerbot.AlmostFullHealth", 85);
    LowMana = sConfigMgr->GetOption<int32>("AiPlayerbot.LowMana", 15);
    MediumMana = sConfigMgr->GetOption<int32>("AiPlayerbot.MediumMana", 40);
    HighMana = sConfigMgr->GetOption<int32>("AiPlayerbot.HighMana", 65);
    AutoSaveMana = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoSaveMana", true);
    SaveManaThreshold = sConfigMgr->GetOption<int32>("AiPlayerbot.SaveManaThreshold", 60);
    switch (sConfigMgr->GetOption<uint32>("AiPlayerbot.AutoGreaterBlessings", 1))
    {
        case 0:
            AutoGreaterBlessings = AutoPartyBuffMode::DISABLED;
            break;
        case 2:
            AutoGreaterBlessings = AutoPartyBuffMode::GROUP_OR_RAID;
            break;
        case 1:
        default:
            AutoGreaterBlessings = AutoPartyBuffMode::RAID_ONLY;
            break;
    }
    switch (sConfigMgr->GetOption<uint32>("AiPlayerbot.AutoPartyBuffs", 2))
    {
        case 0:
            AutoPartyBuffs = AutoPartyBuffMode::DISABLED;
            break;
        case 1:
            AutoPartyBuffs = AutoPartyBuffMode::RAID_ONLY;
            break;
        case 2:
        default:
            AutoPartyBuffs = AutoPartyBuffMode::GROUP_OR_RAID;
            break;
    }
    TellWhenMissingBuffReagents = sConfigMgr->GetOption<bool>("AiPlayerbot.TellWhenMissingBuffReagents", true);
    MissingBuffReagentMessageCooldown = sConfigMgr->GetOption<uint32>(
        "AiPlayerbot.MissingBuffReagentMessageCooldown", 300);
    ForceRebuffOnReadyCheck = sConfigMgr->GetOption<bool>("AiPlayerbot.ForceRebuffOnReadyCheck", false);
    ForceRebuffMarginSecs = std::min(sConfigMgr->GetOption<uint32>("AiPlayerbot.ForceRebuffMarginSecs", 60), 3600u);
    AutoAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoAvoidAoe", true);
    MaxAoeAvoidRadius = sConfigMgr->GetOption<float>("AiPlayerbot.MaxAoeAvoidRadius", 15.0f);
    LoadSet<std::set<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AoeAvoidSpellWhitelist", "50759,57491,13810,29946"),
                              AoeAvoidSpellWhitelist);
    TellWhenAvoidAoe = sConfigMgr->GetOption<bool>("AiPlayerbot.TellWhenAvoidAoe", false);

    RandomGearLoweringChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomGearLoweringChance", 0.0f);
    RandomGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearQualityLimit", 3);
    RandomGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomGearScoreLimit", 0);
    PreferClassArmorType  = sConfigMgr->GetOption<bool>("AiPlayerbot.PreferClassArmorType", false);
    PreferredSpecWeapons  = sConfigMgr->GetOption<bool>("AiPlayerbot.PreferredSpecWeapons", false);

    RandomBotMinLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMinLevelChance", 0.1f);
    RandomBotMaxLevelChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotMaxLevelChance", 0.1f);
    RandomBotRpgChance = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotRpgChance", 0.20f);

    IterationsPerTick = sConfigMgr->GetOption<int32>("AiPlayerbot.IterationsPerTick", 10);

    AllowAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowAccountBots", true);
    AllowGuildBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowGuildBots", true);
    AllowTrustedAccountBots = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowTrustedAccountBots", true);
    DisabledWithoutRealPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.DisabledWithoutRealPlayer", false);
    RandomBotGuildNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGuildNearby", false);
    RandomBotInvitePlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotInvitePlayer", false);
    InviteChat = sConfigMgr->GetOption<bool>("AiPlayerbot.InviteChat", false);

    RandomBotMapsAsString = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32>>(RandomBotMapsAsString, RandomBotMaps);
    ProbTeleToBankers = sConfigMgr->GetOption<float>("AiPlayerbot.ProbTeleToBankers", 0.25f);
    EnableWeightTeleToCityBankers = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableWeightTeleToCityBankers", false);
    WeightTeleToStormwind = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToStormwindWeight", 2);
    WeightTeleToIronforge = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToIronforgeWeight", 1);
    WeightTeleToDarnassus = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToDarnassusWeight", 1);
    WeightTeleToExodar = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToExodarWeight", 1);
    WeightTeleToOrgrimmar = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToOrgrimmarWeight", 2);
    WeightTeleToUndercity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToUndercityWeight", 1);
    WeightTeleToThunderBluff = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToThunderBluffWeight", 1);
    WeightTeleToSilvermoonCity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToSilvermoonCityWeight", 1);
    WeightTeleToShattrathCity = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToShattrathCityWeight", 1);
    WeightTeleToDalaran = sConfigMgr->GetOption<int>("AiPlayerbot.TeleToDalaranWeight", 1);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestItems",
                                           "5175,5176,5177,5178,6948,11000,12382,13704,16309"),
        RandomBotQuestItems);
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotSpellIds", "54197"),
                                  RandomBotSpellIds);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedZoneIds",
                                           "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,"
                                           "3539,3623,4152,3988,4658,4284,4418,4436,4275,4323,4395,3703,4298,3951"),
        PvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.PvpProhibitedAreaIds",
                                           "976,35,392,2268,4161,4010,4317,4312,3649,3887,3958,3724,4080,3938,3754,3786,"
                                           "3973,4085,4086,4087,4088"),
        PvpProhibitedAreaIds);
    FastReactInBG = sConfigMgr->GetOption<bool>("AiPlayerbot.FastReactInBG", true);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotQuestIds", "3802,5505,6502,7761,7848,10277,10285,11492,"
                                           "13188,13189,24499,24511,24710,24712"),
        RandomBotQuestIds);

    LoadSet<std::set<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.DisallowedGameObjects",
                                           "176213,17155,2656,74448,19020,3719,3658,3705,3706,105579,75293,2857,"
                                           "179490,141596,160836,160845,179516,176224,181085,176112,128308,128403,"
                                           "165739,165738,175245,175970,176325,176327,123329,2560"),
        DisallowedGameObjects);
    LoadSet<std::set<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.AttunementQuests", "10279,10277,10282,10283,10284,10285,10296,"
                                           "10297,10298,11481,11482,11488,11490,11492,10901,10888,10445,10985"),
        AttunementQuests);

    LoadSet<std::set<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.UnobtainableItems", "12468,44869,44870,46978"),
        UnobtainableItems);

    BotAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.BotAutologin", false);
    RandomBotAutologin = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutologin", true);
    MinRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBots", 500);
    MaxRandomBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBots", 500);
    RandomBotUpdateInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotUpdateInterval", 20);
    RandomBotCountChangeMinInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMinInterval", 30 * MINUTE);
    RandomBotCountChangeMaxInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * HOUR);
    MinRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotInWorldTime", 2 * HOUR);
    MaxRandomBotInWorldTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * HOUR);
    MinRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotRandomizeTime", 2 * HOUR);
    MaxRandomBotRandomizeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotRandomizeTime", 14 * 24 * HOUR);
    MinRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotChangeStrategyTime", 30 * MINUTE);
    MaxRandomBotChangeStrategyTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * HOUR);
    MinRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotReviveTime", MINUTE);
    MaxRandomBotReviveTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotReviveTime", 5 * MINUTE);
    MinRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotTeleportInterval", 1 * HOUR);
    MaxRandomBotTeleportInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotTeleportInterval", 5 * HOUR);
    PermanentlyInWorldTime =
        sConfigMgr->GetOption<int32>("AiPlayerbot.PermanentlyInWorldTime", 1 * YEAR);
    RandomBotTeleportDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleportDistance", 100);
    RandomBotsPerInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotsPerInterval", 60);
    RandomBotPrintStatsInterval = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotPrintStatsInterval", 300);
    MinRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * HOUR);
    MaxRandomBotsPriceChangeInterval =
        sConfigMgr->GetOption<int32>("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * HOUR);
    RandomBotJoinLfg = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinLfg", true);

    RestrictHealerDPS = sConfigMgr->GetOption<bool>("AiPlayerbot.HealerDPSMapRestriction", false);
    LoadList<std::vector<uint32>>(
        sConfigMgr->GetOption<std::string>("AiPlayerbot.RestrictedHealerDPSMaps",
                                             "33,34,36,43,47,48,70,90,109,129,209,229,230,329,349,389,429,1001,1004,"
                                             "1007,269,540,542,543,545,546,547,552,553,554,555,556,557,558,560,585,574,"
                                             "575,576,578,595,599,600,601,602,604,608,619,632,650,658,668,409,469,509,"
                                             "531,532,534,544,548,550,564,565,580,249,533,603,615,616,624,631,649,724"),
        RestrictedHealerDPSMaps);

    //////////////////////////// ICC

    EnableICCBuffs = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableICCBuffs", true);

    //////////////////////////// Professions
    ClassMatchingProfessionChance =
        std::min<uint32>(100, sConfigMgr->GetOption<uint32>("AiPlayerbot.ClassMatchingProfessionChance", 30));
    FishingDistanceFromMaster = sConfigMgr->GetOption<float>("AiPlayerbot.FishingDistanceFromMaster", 10.0f);
    EndFishingWithMaster = sConfigMgr->GetOption<float>("AiPlayerbot.EndFishingWithMaster", 30.0f);
    FishingDistance = sConfigMgr->GetOption<float>("AiPlayerbot.FishingDistance", 40.0f);
    EnableFishingWithMaster = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableFishingWithMaster", true);
    //////////////////////////// CHAT
    EnableBroadcasts = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableBroadcasts", true);
    RandomBotTalk = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotTalk", false);
    RandomBotEmote = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotEmote", false);
    RandomBotSuggestDungeons = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSuggestDungeons", true);
    RandomBotSayWithoutMaster = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotSayWithoutMaster", false);
    AnnounceConsumableUse = sConfigMgr->GetOption<bool>("AiPlayerbot.AnnounceConsumableUse", true);

    // broadcastChanceMaxValue is used in urand(1, broadcastChanceMaxValue) for broadcasts,
    // lowering it will increase the chance, setting it to 0 will disable broadcasts
    // for internal use, not intended to be change by the user
    BroadcastChanceMaxValue = EnableBroadcasts ? 30000 : 0;

    // all broadcast chances should be in range 1-broadcastChanceMaxValue, value of 0 will disable this particular
    // broadcast setting value to max does not guarantee the broadcast, as there are some internal randoms as well
    BroadcastToGuildGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
    BroadcastToWorldGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
    BroadcastToGeneralGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
    BroadcastToTradeGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
    BroadcastToLFGGlobalChance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
    BroadcastToLocalDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
    BroadcastToWorldDefenseGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
    BroadcastToGuildRecruitmentGlobalChance =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);

    BroadcastChanceLootingItemPoor = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
    BroadcastChanceLootingItemNormal =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemNormal", 300);
    BroadcastChanceLootingItemUncommon =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
    BroadcastChanceLootingItemRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
    BroadcastChanceLootingItemEpic = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
    BroadcastChanceLootingItemLegendary =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
    BroadcastChanceLootingItemArtifact =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);

    BroadcastChanceQuestAccepted = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
    BroadcastChanceQuestUpdateObjectiveCompleted =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
    BroadcastChanceQuestUpdateObjectiveProgress =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
    BroadcastChanceQuestUpdateFailedTimer =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
    BroadcastChanceQuestUpdateComplete =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
    BroadcastChanceQuestTurnedIn = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);

    BroadcastChanceKillNormal = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillNormal", 30);
    BroadcastChanceKillElite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillElite", 300);
    BroadcastChanceKillRareelite = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
    BroadcastChanceKillWorldboss = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
    BroadcastChanceKillRare = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillRare", 10000);
    BroadcastChanceKillUnknown = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillUnknown", 100);
    BroadcastChanceKillPet = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPet", 10);
    BroadcastChanceKillPlayer = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceKillPlayer", 30);

    BroadcastChanceLevelupGeneric = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
    BroadcastChanceLevelupTenX = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
    BroadcastChanceLevelupMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);

    BroadcastChanceSuggestInstance = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
    BroadcastChanceSuggestQuest = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
    BroadcastChanceSuggestGrindMaterials =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
    BroadcastChanceSuggestGrindReputation =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
    BroadcastChanceSuggestSell = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSell", 300);
    BroadcastChanceSuggestSomething =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);

    BroadcastChanceSuggestSomethingToxic =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);

    BroadcastChanceSuggestToxicLinks = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
    ToxicLinksPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.ToxicLinksPrefix", "gnomes");

    BroadcastChanceSuggestThunderfury =
        sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);

    // does not depend on global chance
    BroadcastChanceGuildManagement = sConfigMgr->GetOption<int32>("AiPlayerbot.BroadcastChanceGuildManagement", 30000);

    ToxicLinksRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ToxicLinksRepliesChance", 30);    // 0-100
    ThunderfuryRepliesChance = sConfigMgr->GetOption<int32>("AiPlayerbot.ThunderfuryRepliesChance", 40);  // 0-100
    GuildRepliesRate = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildRepliesRate", 100);                 // 0-100

    RandomBotJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotJoinBG", true);
    RandomBotAutoJoinBG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotAutoJoinBG", false);

    RandomBotAutoJoinArenaBracket = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinArenaBracket", 14);

    RandomBotAutoJoinWSBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinWSBrackets", "7");
    RandomBotAutoJoinABBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinABBrackets", "6");
    RandomBotAutoJoinAVBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinAVBrackets", "3");
    RandomBotAutoJoinEYBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinEYBrackets", "2");
    RandomBotAutoJoinICBrackets = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAutoJoinICBrackets", "1");

    RandomBotAutoJoinBGWSCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGWSCount", 1);
    RandomBotAutoJoinBGABCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGABCount", 1);
    RandomBotAutoJoinBGAVCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGAVCount", 0);
    RandomBotAutoJoinBGEYCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGEYCount", 1);
    RandomBotAutoJoinBGICCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGICCount", 0);

    RandomBotAutoJoinBGRatedArena2v2Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena2v2Count", 0);
    RandomBotAutoJoinBGRatedArena3v3Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena3v3Count", 0);
    RandomBotAutoJoinBGRatedArena5v5Count =
        sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena5v5Count", 0);
    LogInGroupOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.LogInGroupOnly", true);
    LogValuesPerTick = sConfigMgr->GetOption<bool>("AiPlayerbot.LogValuesPerTick", false);
    FleeingEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.FleeingEnabled", true);
    SummonAtInnkeepersEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    RandomBotMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMinLevel", 1);
    RandomBotMaxLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotMaxLevel", 80);
    if (RandomBotMaxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        RandomBotMaxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // Bracket defaults (below) derive from randomBotMaxLevel, so this must run after it is read.
    LoadRandomBotLevelConfig();

    RandomBotTeleLowerLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleLowerLevel", 1);
    RandomBotTeleHigherLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotTeleHigherLevel", 3);
    OpenGoSpell = sConfigMgr->GetOption<int32>("AiPlayerbot.OpenGoSpell", 6477);

    // Zones for NewRpgStrategy teleportation brackets
    std::vector<uint32> zoneIds = {
        // Classic WoW - Low-level zones
        1, 12, 14, 85, 141, 215, 3430, 3524,
        // Classic WoW - Mid-level zones
        17, 38, 40, 130, 148, 3433, 3525,
        // Classic WoW - High-level zones
        10, 11, 44, 267, 331, 400, 406,
        // Classic WoW - Higher-level zones
        3, 8, 15, 16, 33, 45, 47, 51, 357, 405, 440,
        // Classic WoW - Top-level zones
        4, 28, 46, 139, 361, 490, 618, 1377,
        // The Burning Crusade - Zones
        3483, 3518, 3519, 3520, 3521, 3522, 3523, 4080,
        // Wrath of the Lich King - Zones
        65, 66, 67, 210, 394, 495, 2817, 3537, 3711, 4197
    };

    for (uint32 zoneId : zoneIds)
    {
        std::string setting = "AiPlayerbot.ZoneBracket." + std::to_string(zoneId);
        std::string value = sConfigMgr->GetOption<std::string>(setting, "");

        if (!value.empty())
        {
            size_t commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
                uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
                ZoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
            }
        }
    }

    RandomChangeMultiplier = sConfigMgr->GetOption<float>("AiPlayerbot.RandomChangeMultiplier", 1.0);

    RandomBotCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotCombatStrategies", "");
    RandomBotNonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotNonCombatStrategies", "");
    CombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.CombatStrategies", "");
    NonCombatStrategies = sConfigMgr->GetOption<std::string>("AiPlayerbot.NonCombatStrategies", "");
    ApplyInstanceStrategies = sConfigMgr->GetOption<bool>("AiPlayerbot.ApplyInstanceStrategies", true);

    CommandPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandPrefix", "");
    CommandSeparator = sConfigMgr->GetOption<std::string>("AiPlayerbot.CommandSeparator", "\\\\");

    CommandServerPort = sConfigMgr->GetOption<int32>("AiPlayerbot.CommandServerPort", 8888);
    PerfMonEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.PerfMonEnabled", false);

    UseGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseGroundMountAtMinLevel", 20);
    UseFastGroundMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastGroundMountAtMinLevel", 40);
    UseFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFlyMountAtMinLevel", 60);
    UseFastFlyMountAtMinLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.UseFastFlyMountAtMinLevel", 70);

    // stagger bot flightpath takeoff
    BotTaxiDelayMin = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMinMs", 350);
    BotTaxiDelayMax = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiDelayMaxMs", 5000);
    BotTaxiGapMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapMs", 200);
    BotTaxiGapJitterMs = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotTaxiGapJitterMs", 100);

    LOG_INFO("server.loading", "Loading TalentSpecs...");

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (cls == 10)
        {
            continue;
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            PremadeSpecName[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecGlyph." << cls << "." << spec;
            PremadeSpecGlyph[cls][spec] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
            std::vector<std::string> splitSpecGlyph = split(PremadeSpecGlyph[cls][spec], ',');
            for (std::string& split : splitSpecGlyph)
            {
                if (split.size() != 0)
                {
                    ParsedSpecGlyph[cls][spec].push_back(atoi(split.c_str()));
                }
            }
            for (uint32 level = 0; level < MAX_LEVEL; ++level)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                PremadeSpecLink[cls][spec][level] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                ParsedSpecLinkOrder[cls][spec][level] = ParseTempTalentsOrder(cls, PremadeSpecLink[cls][spec][level]);
            }
        }
        for (uint32 spec = 0; spec < 3; ++spec)
        {
            for (uint32 points = 0; points < 21; ++points)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeHunterPetLink." << spec << "." << points;
                PremadeHunterPetLink[spec][points] = sConfigMgr->GetOption<std::string>(os.str().c_str(), "", false);
                ParsedHunterPetLinkOrder[spec][points] =
                    ParseTempPetTalentsOrder(spec, PremadeHunterPetLink[spec][points]);
            }
        }
        for (uint32 spec = 0; spec < MAX_SPECNO; ++spec)
        {
            std::ostringstream os;
            os << "AiPlayerbot.RandomClassSpecProb." << cls << "." << spec;
            uint32 def;
            if (spec <= 1)
                def = 33;
            else if (spec == 2)
                def = 34;
            else
                def = 0;
            RandomClassSpecProb[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), def, false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.RandomClassSpecIndex." << cls << "." << spec;
            RandomClassSpecIndex[cls][spec] = sConfigMgr->GetOption<uint32>(os.str().c_str(), spec, false);
        }
    }

    BotCheats.clear();
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.BotCheats", "food,taxi,raid"),
                                             BotCheats);

    BotCheatMask = 0;

    if (std::find(BotCheats.begin(), BotCheats.end(), "food") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::food;
    if (std::find(BotCheats.begin(), BotCheats.end(), "taxi") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(BotCheats.begin(), BotCheats.end(), "gold") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(BotCheats.begin(), BotCheats.end(), "health") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(BotCheats.begin(), BotCheats.end(), "mana") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(BotCheats.begin(), BotCheats.end(), "power") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(BotCheats.begin(), BotCheats.end(), "raid") != BotCheats.end())
        BotCheatMask |= (uint32)BotCheatMask::raid;

    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.AllowedLogFiles", ""),
                                             AllowedLogFiles);
    EnableAutoTradeOnItemMention = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableAutoTradeOnItemMention", true);
    LoadListString<std::vector<std::string>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.TradeActionExcludedPrefixes", ""),
                                             TradeActionExcludedPrefixes);

    WorldBuffs.clear();
    LoadWorldBuff();
    LOG_INFO("playerbots", "Loading World Buff Feature...");

    RandomBotAccountPrefix = sConfigMgr->GetOption<std::string>("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    RandomBotAccountCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAccountCount", 0);
    DeleteRandomBotAccounts = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotAccounts", false);
    RandomBotGuildCount = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotGuildCount", 20);
    RandomBotGuildSizeMax = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotGuildSizeMax", 15);
    DeleteRandomBotGuilds = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotGuilds", false);

    BotSendMailEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.BotSendMailEnabled", true);

    GuildTaskEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGuildTasks", false);
    MinGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    MaxGuildTaskChangeTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    MinGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskAdvertisementTime", 300);
    MaxGuildTaskAdvertisementTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    MinGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MinGuildTaskRewardTime", 300);
    MaxGuildTaskRewardTime = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxGuildTaskRewardTime", 3600);
    GuildTaskAdvertCleanupTime = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);
    GuildTaskKillTaskDistance = sConfigMgr->GetOption<int32>("AiPlayerbot.GuildTaskKillTaskDistance", 2000);
    TargetPosRecalcDistance = sConfigMgr->GetOption<float>("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    //cosmetics
    switch (sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotShowHelmet", 1))
    {
        case 0:
            RandomBotShowHelmet = ShowHideCosmetic::ALWAYS_HIDE;
            break;
        case 2:
            RandomBotShowHelmet = ShowHideCosmetic::RANDOMIZE;
            break;
        case 1:
        default:
            RandomBotShowHelmet = ShowHideCosmetic::ALWAYS_SHOW;
            break;
    }
    switch (sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotShowCloak", 1))
    {
        case 0:
            RandomBotShowCloak = ShowHideCosmetic::ALWAYS_HIDE;
            break;
        case 2:
            RandomBotShowCloak = ShowHideCosmetic::RANDOMIZE;
            break;
        case 1:
        default:
            RandomBotShowCloak = ShowHideCosmetic::ALWAYS_SHOW;
            break;
    }

    // SPP switches
    EnableGreet = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableGreet", true);
    SummonWhenGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.SummonWhenGroup", true);
    RandomBotFixedLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotFixedLevel", false);
    DisableRandomLevels = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableRandomLevels", false);
    RandomBotRandomPassword = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotRandomPassword", true);
    DowngradeMaxLevelBot = sConfigMgr->GetOption<bool>("AiPlayerbot.DowngradeMaxLevelBot", true);
    EquipAndSpecPersistence = sConfigMgr->GetOption<bool>("AiPlayerbot.EquipAndSpecPersistence", true);
    EquipAndSpecPersistenceLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.EquipAndSpecPersistenceLevel", 1);
    GroupInvitationPermission = sConfigMgr->GetOption<int32>("AiPlayerbot.GroupInvitationPermission", 1);
    KeepAltsInGroup = sConfigMgr->GetOption<bool>("AiPlayerbot.KeepAltsInGroup", false);
    AllowSummonInCombat = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonInCombat", true);
    AllowSummonWhenMasterIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenMasterIsDead", true);
    AllowSummonWhenBotIsDead = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowSummonWhenBotIsDead", true);
    ReviveBotWhenSummoned = sConfigMgr->GetOption<int32>("AiPlayerbot.ReviveBotWhenSummoned", 1);
    BotRepairWhenSummon = sConfigMgr->GetOption<bool>("AiPlayerbot.BotRepairWhenSummon", true);
    AutoInitOnly = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoInitOnly", false);
    ResetInstanceIdForAltBots = sConfigMgr->GetOption<bool>("AiPlayerbot.ResetInstanceIdForAltBots", false);
    AutoInitEquipLevelLimitRatio = sConfigMgr->GetOption<float>("AiPlayerbot.AutoInitEquipLevelLimitRatio", 1.0);

    MaxAddedBots = sConfigMgr->GetOption<int32>("AiPlayerbot.MaxAddedBots", 40);
    AddClassCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassCommand", 1);
    AddClassAccountPoolSize = sConfigMgr->GetOption<int32>("AiPlayerbot.AddClassAccountPoolSize", 50);
    AddClassRandomCharacter = sConfigMgr->GetOption<bool>("AiPlayerbot.AddClassRandomCharacter", false);
    MaintenanceCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.MaintenanceCommand", 1);

    AltMaintenanceAttunementQs = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceAttunementQuests", true);
    AltMaintenanceBags = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceBags", true);
    AltMaintenanceAmmo = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceAmmo", true);
    AltMaintenanceFood = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceFood", true);
    AltMaintenanceReagents = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceReagents", true);
    AltMaintenanceConsumables = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceConsumables", true);
    AltMaintenancePotions = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenancePotions", true);
    AltMaintenanceTalentTree = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceTalentTree", true);
    AltMaintenancePet = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenancePet", true);
    AltMaintenancePetTalents = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenancePetTalents", true);
    AltMaintenanceClassSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceClassSpells", true);
    AltMaintenanceAvailableSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceAvailableSpells", true);
    AltMaintenanceSkills = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceSkills", true);
    AltMaintenanceReputation = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceReputation", true);
    AltMaintenanceSpecialSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceSpecialSpells", true);
    AltMaintenanceMounts = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceMounts", true);
    AltMaintenanceGlyphs = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceGlyphs", true);
    AltMaintenanceKeyring = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceKeyring", true);
    AltMaintenanceGemsEnchants = sConfigMgr->GetOption<bool>("AiPlayerbot.AltMaintenanceGemsEnchants", true);

    AutoGearCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommand", 1);
    AutoGearCommandAltBots = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearCommandAltBots", 1);
    AutoGearBisCommand = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearBisCommand", 0);
    AutoGearQualityLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearQualityLimit", 3);
    AutoGearScoreLimit = sConfigMgr->GetOption<int32>("AiPlayerbot.AutoGearScoreLimit", 0);

    RandomBotXPRate = sConfigMgr->GetOption<float>("AiPlayerbot.RandomBotXPRate", 1.0);
    RandomBotAllianceRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotAllianceRatio", 50);
    RandomBotHordeRatio = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotHordeRatio", 50);
    DisableDeathKnightLogin = sConfigMgr->GetOption<bool>("AiPlayerbot.DisableDeathKnightLogin", 0);
    LimitTalentsExpansion = sConfigMgr->GetOption<bool>("AiPlayerbot.LimitTalentsExpansion", 0);
    BotActiveAlone = sConfigMgr->GetOption<int32>("AiPlayerbot.BotActiveAlone", 10);
    BotActiveAloneDurationSeconds = sConfigMgr->GetOption<int32>("AiPlayerbot.BotActiveAloneDurationSeconds", 30);
    BotActiveAloneForceWhenInRadius = sConfigMgr->GetOption<uint32>("AiPlayerbot.BotActiveAloneForceWhenInRadius", 150);
    BotActiveAloneForceWhenInZone = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInZone", 1);
    BotActiveAloneForceWhenInMap = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInMap", 0);
    BotActiveAloneForceWhenIsFriend = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenIsFriend", 0);
    BotActiveAloneForceWhenInGuild = sConfigMgr->GetOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInGuild", 1);
    BotActiveAloneSmartScale = sConfigMgr->GetOption<bool>("AiPlayerbot.botActiveAloneSmartScale", 1);
    BotActiveAloneSmartScaleDiffLimitFloor = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitfloor", 50);
    BotActiveAloneSmartScaleDiffLimitCeiling = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitCeiling", 200);
    BotActiveAloneSmartScaleWhenMinLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMinLevel", 1);
    BotActiveAloneSmartScaleWhenMaxLevel = sConfigMgr->GetOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMaxLevel", 80);

    RandomBotsWalkingRPG = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG", false);
    RandomBotsWalkingRPGInDoors = sConfigMgr->GetOption<bool>("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    MinEnchantingBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.MinEnchantingBotLevel", 60);
    LimitEnchantExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitEnchantExpansion", 1);
    LimitGearExpansion = sConfigMgr->GetOption<int32>("AiPlayerbot.LimitGearExpansion", 1);
    RandomBotStartingLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.RandombotStartingLevel", 1);
    EnablePeriodicOnlineOffline = sConfigMgr->GetOption<bool>("AiPlayerbot.EnablePeriodicOnlineOffline", false);
    EnableRandomBotTrading = sConfigMgr->GetOption<int32>("AiPlayerbot.EnableRandomBotTrading", 1);
    PeriodicOnlineOfflineRatio = sConfigMgr->GetOption<float>("AiPlayerbot.PeriodicOnlineOfflineRatio", 2.0);
    GearScoreCheck = sConfigMgr->GetOption<bool>("AiPlayerbot.GearScoreCheck", false);
    RandomBotPreQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.PreQuests", false);

    // SPP automation
    FreeMethodLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.FreeMethodLoot", false);
    LootNeedRollLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.LootNeedRollLevel", 1);
    LootRollRecipe = sConfigMgr->GetOption<bool>("AiPlayerbot.LootRollRecipe", false);
    LootRollDisenchant = sConfigMgr->GetOption<bool>("AiPlayerbot.LootRollDisenchant", false);
    LootGreedRollLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.LootGreedRollLevel", false);
    AutoPickReward = sConfigMgr->GetOption<std::string>("AiPlayerbot.AutoPickReward", "yes");
    AutoEquipUpgradeLoot = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoEquipUpgradeLoot", true);
    EquipUpgradeThreshold = sConfigMgr->GetOption<float>("AiPlayerbot.EquipUpgradeThreshold", 1.1f);
    TwoRoundsGearInit = sConfigMgr->GetOption<bool>("AiPlayerbot.TwoRoundsGearInit", false);
    SyncQuestWithPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestWithPlayer", true);
    SyncQuestForPlayer = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncQuestForPlayer", false);
    DropObsoleteQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.DropObsoleteQuests", true);
    AllowLearnTrainerSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AllowLearnTrainerSpells", true);
    AutoPickTalents = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoPickTalents", true);
    AutoUpgradeEquip = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoUpgradeEquip", true);
    HunterWolfPet = sConfigMgr->GetOption<int32>("AiPlayerbot.HunterWolfPet", 0);
    DefaultPetStance = sConfigMgr->GetOption<int32>("AiPlayerbot.DefaultPetStance", 1);
    PetChatCommandDebug = sConfigMgr->GetOption<bool>("AiPlayerbot.PetChatCommandDebug", 0);
    AutoLearnTrainerSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnTrainerSpells", true);
    AutoLearnQuestSpells = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoLearnQuestSpells", true);
    AutoTeleportForLevel = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoTeleportForLevel", false);
    AutoDoQuests = sConfigMgr->GetOption<bool>("AiPlayerbot.AutoDoQuests", true);
    EnableNewRpgStrategy = sConfigMgr->GetOption<bool>("AiPlayerbot.EnableNewRpgStrategy", true);

    RpgStatusProbWeight[RPG_WANDER_RANDOM] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderRandom", 15);
    RpgStatusProbWeight[RPG_WANDER_NPC] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderNpc", 20);
    RpgStatusProbWeight[RPG_GO_GRIND] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoGrind", 15);
    RpgStatusProbWeight[RPG_GO_CAMP] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoCamp", 10);
    RpgStatusProbWeight[RPG_DO_QUEST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.DoQuest", 60);
    RpgStatusProbWeight[RPG_TRAVEL_FLIGHT] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.TravelFlight", 15);
    RpgStatusProbWeight[RPG_REST] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.Rest", 5);
    RpgStatusProbWeight[RPG_OUTDOOR_PVP] = sConfigMgr->GetOption<int32>("AiPlayerbot.RpgStatusProbWeight.OutdoorPvp", 10);

    SyncLevelWithPlayers = sConfigMgr->GetOption<bool>("AiPlayerbot.SyncLevelWithPlayers", false);
    RandomBotConcentrateInPlayerZone =
        sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotConcentrateInPlayerZone", false);
    RandomBotGroupNearby = sConfigMgr->GetOption<bool>("AiPlayerbot.RandomBotGroupNearby", false);

    // arena
    RandomBotArenaTeam2v2Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam2v2Count", 10);
    RandomBotArenaTeam3v3Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam3v3Count", 10);
    RandomBotArenaTeam5v5Count = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeam5v5Count", 5);
    DeleteRandomBotArenaTeams = sConfigMgr->GetOption<bool>("AiPlayerbot.DeleteRandomBotArenaTeams", false);
    RandomBotArenaTeamMaxRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMaxRating", 2000);
    RandomBotArenaTeamMinRating = sConfigMgr->GetOption<int32>("AiPlayerbot.RandomBotArenaTeamMinRating", 1000);

    SelfBotLevel = sConfigMgr->GetOption<int32>("AiPlayerbot.SelfBotLevel", 1);

    RandomPlayerbotFactory::CreateRandomBots();
    if (World::IsStopped())
    {
        return true;
    }

    // Assign account types after accounts are created
    sRandomPlayerbotMgr.AssignAccountTypes();

    if (sPlayerbotAIConfig.Enabled)
    {
        sRandomPlayerbotMgr.Init();
    }

    PlayerbotGuildMgr::instance().Init();
    sRandomPlayerbotMgr.InitArenaTeams();
    sRandomItemMgr.Init();
    sRandomItemMgr.InitAfterAhBot();
    sBisListMgr->LoadAll();
    PlayerbotTextMgr::instance().LoadBotTexts();
    PlayerbotTextMgr::instance().LoadBotTextChance();
    PlayerbotFactory::Init();

    AiObjectContext::BuildAllSharedContexts();

    if (sPlayerbotAIConfig.RandomBotSuggestDungeons)
    {
        PlayerbotDungeonRepository::instance().LoadDungeonSuggestions();
    }
    sTravelMgr.Init();

    ExcludedHunterPetFamilies.clear();
    LoadList<std::vector<uint32>>(sConfigMgr->GetOption<std::string>("AiPlayerbot.ExcludedHunterPetFamilies", ""), ExcludedHunterPetFamilies);

    LOG_INFO("server.loading", "---------------------------------------");
    LOG_INFO("server.loading", "       mod-playerbots initialized      ");
    LOG_INFO("server.loading", "---------------------------------------");

    return true;
}

// Loads AiPlayerbot.LevelBrackets.* and AiPlayerbot.ResetBotLevel.* (see RandomBotLevelMgr). Also
// re-run on ".reload config" via RandomBotLevelWorldScript::OnAfterConfigLoad. Bracket
// bounds/percentages are only the as-configured values here; RandomBotLevelMgr::LoadConfig()
// copies them into its own working state, since dynamic distribution and clamp/rebalance mutate
// percentages at runtime.
void PlayerbotAIConfig::LoadRandomBotLevelConfig()
{
    // ---- Level brackets ----
    LevelBracketsEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.Enabled", false);
    LevelBracketsIgnoreGuildWithRealPlayers =
        sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.IgnoreGuildBotsWithRealPlayers", true);
    LevelBracketsIgnoreArenaTeamBots =
        sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.IgnoreArenaTeamBots", true);

    LevelBracketsCheckFrequency = sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.CheckFrequency", 300);
    LevelBracketsFlaggedCheckFrequency =
        sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.CheckFlaggedFrequency", 15);
    LevelBracketsDynamicDistribution =
        sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.Dynamic.UseDynamicDistribution", false);
    LevelBracketsRealPlayerWeight =
        sConfigMgr->GetOption<float>("AiPlayerbot.LevelBrackets.Dynamic.RealPlayerWeight", 1.0f);
    LevelBracketsSyncFactions = sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.Dynamic.SyncFactions", false);
    LevelBracketsIgnoreFriendListed = sConfigMgr->GetOption<bool>("AiPlayerbot.LevelBrackets.IgnoreFriendListed", true);
    LevelBracketsFlaggedProcessLimit =
        sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.FlaggedProcessLimit", 5);

    ParseLevelMgrExcludeNames(sConfigMgr->GetOption<std::string>("AiPlayerbot.LevelBrackets.ExcludeNames", ""),
        LevelBracketsExcludeNames);

    LevelBracketsNumRanges =
        static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.NumRanges", 9));
    LevelBracketsAlliance.resize(LevelBracketsNumRanges);
    LevelBracketsHorde.resize(LevelBracketsNumRanges);

    for (uint8 i = 0; i < LevelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < LevelBracketsNumRanges - 1 ? i * 10 + 9 : RandomBotMaxLevel);
        LevelBracketsAlliance[i].Lower = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Lower", defaultLower));
        LevelBracketsAlliance[i].Upper = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Upper", defaultUpper));
        LevelBracketsAlliance[i].Pct = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Pct", 11));
    }

    for (uint8 i = 0; i < LevelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < LevelBracketsNumRanges - 1 ? i * 10 + 9 : RandomBotMaxLevel);
        LevelBracketsHorde[i].Lower = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Lower", defaultLower));
        LevelBracketsHorde[i].Upper = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Upper", defaultUpper));
        LevelBracketsHorde[i].Pct = static_cast<uint8>(
            sConfigMgr->GetOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Pct", 11));
    }

    // A mismatch forcibly disables SyncFactions and logs an error; it never brings the server down.
    if (LevelBracketsSyncFactions)
    {
        for (uint8 i = 0; i < LevelBracketsNumRanges; ++i)
        {
            if (LevelBracketsAlliance[i].Lower != LevelBracketsHorde[i].Lower ||
                LevelBracketsAlliance[i].Upper != LevelBracketsHorde[i].Upper)
            {
                LOG_ERROR("server.loading",
                    "[RandomBotLevelMgr] Bracket mismatch detected between factions at index {}. Alliance: {}-{}, "
                    "Horde: {}-{}. SyncFactions requires both bracket count and min/max levels to match exactly; "
                    "forcibly disabling SyncFactions for this session. Check your configuration.",
                    i, LevelBracketsAlliance[i].Lower, LevelBracketsAlliance[i].Upper,
                    LevelBracketsHorde[i].Lower, LevelBracketsHorde[i].Upper);
                LevelBracketsSyncFactions = false;
                break;
            }
        }
    }

    // ---- Level reset ----
    ResetBotLevelEnabled = sConfigMgr->GetOption<bool>("AiPlayerbot.ResetBotLevel.Enabled", false);

    ResetBotLevelMaxLevel =
        static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.MaxLevel", 80));
    if ((ResetBotLevelMaxLevel < 2 || ResetBotLevelMaxLevel > 80) && ResetBotLevelMaxLevel != 0)
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.MaxLevel value: {}. Using default value 80.",
            ResetBotLevelMaxLevel);
        ResetBotLevelMaxLevel = 80;
    }

    ResetBotLevelResetTo =
        static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.ResetToLevel", 1));
    if (ResetBotLevelResetTo < 1 || (ResetBotLevelMaxLevel > 0 && ResetBotLevelResetTo >= ResetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetToLevel value: {}. Using default value 1.",
            ResetBotLevelResetTo);
        ResetBotLevelResetTo = 1;
    }

    ResetBotLevelSkipFrom =
        static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.SkipFromLevel", 0));
    if (ResetBotLevelSkipFrom > 80 || (ResetBotLevelMaxLevel > 0 && ResetBotLevelSkipFrom >= ResetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipFromLevel value: {}. Using default value 0 "
            "(disabled).",
            ResetBotLevelSkipFrom);
        ResetBotLevelSkipFrom = 0;
    }

    ResetBotLevelSkipTo = static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.SkipToLevel", 1));
    if (ResetBotLevelSkipTo < 1 || ResetBotLevelSkipTo > 80 ||
        (ResetBotLevelMaxLevel > 0 && ResetBotLevelSkipTo > ResetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipToLevel value: {}. Using default value 1.",
            ResetBotLevelSkipTo);
        ResetBotLevelSkipTo = 1;
    }

    ResetBotLevelChance =
        static_cast<uint8>(sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.ResetChance", 100));
    if (ResetBotLevelChance > 100)
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetChance value: {}. Using default value 100.",
            ResetBotLevelChance);
        ResetBotLevelChance = 100;
    }

    ResetBotLevelScaledChance = sConfigMgr->GetOption<bool>("AiPlayerbot.ResetBotLevel.ScaledChance", false);

    ResetBotLevelRestrictTimePlayed =
        sConfigMgr->GetOption<bool>("AiPlayerbot.ResetBotLevel.RestrictTimePlayed", false);
    ResetBotLevelMinTimePlayed = sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.MinTimePlayed", 86400);
    ResetBotLevelPlayedTimeCheckFrequency =
        sConfigMgr->GetOption<uint32>("AiPlayerbot.ResetBotLevel.PlayedTimeCheckFrequency", 864);

    ResetBotLevelIgnoreGuildWithRealPlayers =
        sConfigMgr->GetOption<bool>("AiPlayerbot.ResetBotLevel.IgnoreGuildBotsWithRealPlayers", false);

    ParseLevelMgrExcludeNames(sConfigMgr->GetOption<std::string>("AiPlayerbot.ResetBotLevel.ExcludeNames", ""),
        ResetBotLevelExcludeNames);
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(RandomBotAccounts.begin(), RandomBotAccounts.end(), id) != RandomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(RandomBotQuestItems.begin(), RandomBotQuestItems.end(), id) != RandomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsPvpProhibited(uint32 zoneId, uint32 areaId)
{
    return IsInPvpProhibitedZone(zoneId) || IsInPvpProhibitedArea(areaId) || IsInPvpProhibitedZone(areaId);
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return find(PvpProhibitedZoneIds.begin(), PvpProhibitedZoneIds.end(), id) != PvpProhibitedZoneIds.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedArea(uint32 id)
{
    return find(PvpProhibitedAreaIds.begin(), PvpProhibitedAreaIds.end(), id) != PvpProhibitedAreaIds.end();
}

bool PlayerbotAIConfig::IsRestrictedHealerDPSMap(uint32 mapId) const
{
    return RestrictHealerDPS &&
            std::find(RestrictedHealerDPSMaps.begin(), RestrictedHealerDPSMaps.end(), mapId) != RestrictedHealerDPSMaps.end();
}

std::string const PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    // Sized for the widest output snprintf can produce for these int conversions, so the
    // result is never truncated.
    char buf[128];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour,
             aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::OpenLog(std::string const fileName, char const* mode)
{
    if (!HasLog(fileName))
        return false;

    auto logFileIt = LogFiles.find(fileName);
    if (logFileIt == LogFiles.end())
    {
        LogFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = LogFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen)  // close log file
        fclose(file);

    std::string m_logsDir = sConfigMgr->GetOption<std::string>("LogsDir", "", false);
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }

    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;

    return true;
}

void PlayerbotAIConfig::Log(std::string const fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(LogMutex);

    if (!IsLogOpen(fileName) && !OpenLog(fileName, "a"))
        return;

    FILE* file = LogFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::LoadWorldBuff()
{
    std::string matrix = sConfigMgr->GetOption<std::string>("AiPlayerbot.WorldBuffMatrix", "", true);
    if (matrix.empty())
        return;

    std::istringstream entryStream(matrix);
    std::string entry;

    while (std::getline(entryStream, entry, ';'))
    {

        entry.erase(0, entry.find_first_not_of(" \t\r\n"));
        entry.erase(entry.find_last_not_of(" \t\r\n") + 1);

        size_t firstColon = entry.find(':');
        size_t secondColon = entry.find(':', firstColon + 1);

        if (firstColon == std::string::npos || secondColon == std::string::npos)
        {
            LOG_ERROR("playerbots", "Malformed entry: [{}]", entry);
            continue;
        }

        std::string metaPart = entry.substr(firstColon + 1, secondColon - firstColon - 1);
        std::string spellPart = entry.substr(secondColon + 1);

        std::vector<uint32> ids;
        std::istringstream metaStream(metaPart);
        std::string token;
        while (std::getline(metaStream, token, ','))
        {
            try {
                ids.push_back(static_cast<uint32>(std::stoi(token)));
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid meta token in [{}]", entry);
                break;
            }
        }

        if (ids.size() != 5)
        {
            LOG_ERROR("playerbots", "Entry [{}] has incomplete meta block", entry);
            continue;
        }

        std::istringstream spellStream(spellPart);
        while (std::getline(spellStream, token, ','))
        {
            try {
                uint32 spellId = static_cast<uint32>(std::stoi(token));
                WorldBuff wb = { spellId, ids[0], ids[1], ids[2], ids[3], ids[4] };
                WorldBuffs.push_back(wb);
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid spell ID in [{}]", entry);
            }
        }
    }
}

static std::vector<std::string> split(std::string const& str, std::string const& pattern)
{
    std::vector<std::string> res;
    if (str == "")
        return res;
    // Also add separators to string connections to facilitate intercepting the last paragraph.
    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        // Remove the split string and split the remaining string
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempTalentsOrder(uint32 cls, std::string tab_link)
{
    // check bad link
    uint32 classMask = 1 << (cls - 1);
    std::vector<std::vector<uint32>> res;
    std::vector<std::string> tab_links = split(tab_link, "-");
    std::map<uint32, std::vector<TalentEntry const*>> spells;
    std::vector<std::vector<std::vector<uint32>>> orders(3);
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        spells[talentTabInfo->tabpage].push_back(talentInfo);
    }
    for (int tab = 0; tab < 3; tab++)
    {
        if (tab_links.size() <= (size_t)tab)
        {
            break;
        }
        std::sort(spells[tab].begin(), spells[tab].end(),
                  [&](TalentEntry const* lhs, TalentEntry const* rhs)
                  { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
        for (uint32 i = 0; i < tab_links[tab].size(); i++)
        {
            if (i >= spells[tab].size())
            {
                break;
            }
            int lvl = tab_links[tab][i] - '0';
            if (lvl == 0)
                continue;
            orders[tab].push_back({(uint32)tab, spells[tab][i]->Row, spells[tab][i]->Col, (uint32)lvl});
        }
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });
    for (auto& order : orders)
    {
        res.insert(res.end(), order.begin(), order.end());
    }
    return res;
}

std::vector<std::vector<uint32>> PlayerbotAIConfig::ParseTempPetTalentsOrder(uint32 spec, std::string tab_link)
{
    // check bad link
    // uint32 classMask = 1 << (cls - 1);
    std::vector<TalentEntry const*> spells;
    std::vector<std::vector<uint32>> orders;
    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if (!((1 << spec) & talentTabInfo->petTalentMask))
            continue;
        // skip some duplicate spells like dash/dive
        if (talentInfo->TalentID == 2201 || talentInfo->TalentID == 2208 || talentInfo->TalentID == 2219 ||
            talentInfo->TalentID == 2203)
            continue;

        spells.push_back(talentInfo);
    }
    std::sort(spells.begin(), spells.end(),
              [&](TalentEntry const* lhs, TalentEntry const* rhs)
              { return lhs->Row != rhs->Row ? lhs->Row < rhs->Row : lhs->Col < rhs->Col; });
    for (uint32 i = 0; i < tab_link.size(); i++)
    {
        if (i >= spells.size())
        {
            break;
        }
        int lvl = tab_link[i] - '0';
        if (lvl == 0)
            continue;
        orders.push_back({spells[i]->Row, spells[i]->Col, (uint32)lvl});
    }
    // sort by talent tab size
    std::sort(orders.begin(), orders.end(), [&](auto& lhs, auto& rhs) { return lhs.size() > rhs.size(); });

    return orders;
}
