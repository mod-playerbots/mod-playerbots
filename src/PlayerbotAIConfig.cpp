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
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>

// Checker for absent configs in the user's .conf file. If GetOption returns the fallback control
// character (x01) string, that means the config isn't in the .conf file.
static constexpr char const* ConfigNotInConfFile = "\x01<not in conf file>";
static bool IsConfigSet(std::string const& name)
{
    return sConfigMgr->GetOption<std::string>(name, ConfigNotInConfFile, false) != ConfigNotInConfFile;
}

// Reads a config's value, and if it has previous names, it accepts those too if the user's .conf file
// still uses an old name. The list of previous names is from newest to oldest, following this structure:
// member = GetPlayerbotsOption<type>("current name", default value,
//                                    { "previous name",
//                                      "older name",
//                                      "oldest name" });
//
// NEVER RENAME A CONFIG TO ANOTHER CONFIG'S OLD NAME, as that can conflict with the user's .conf file.
// You may rename to a brand new name, or revert to an old name that belongs to the same config.
template <class T>
static T GetPlayerbotsOption(std::string const& name, T const& def,
                             std::initializer_list<char const*> previousNames = {}, bool showLogs = true)
{
    if (previousNames.size() && !IsConfigSet(name))
    {
        for (char const* previousName : previousNames)
        {
            if (!IsConfigSet(previousName))
                continue;

            LOG_WARN("server.loading",
                     "Playerbots config '{}' was renamed to '{}'. The old name is still honoured, update playerbots.conf.",
                     previousName, name);

            return sConfigMgr->GetOption<T>(previousName, def, showLogs);
        }
    }

    return sConfigMgr->GetOption<T>(name, def, showLogs);
}

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

    enabled = GetPlayerbotsOption<bool>("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        LOG_INFO("server.loading", "Playerbots Module is disabled in playerbots.conf");
        return false;
    }

    globalCoolDown = GetPlayerbotsOption<int32>("AiPlayerbot.GlobalCooldown", 500);
    maxWaitForMove = GetPlayerbotsOption<int32>("AiPlayerbot.MaxWaitForMove", 5000);
    disableMoveSplinePath = GetPlayerbotsOption<int32>("AiPlayerbot.DisableMoveSplinePath", 0);
    maxMovementSearchTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxMovementSearchTime", 3);
    expireActionTime = GetPlayerbotsOption<int32>("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = GetPlayerbotsOption<int32>("AiPlayerbot.DispelAuraDuration", 700);
    reactDelay = GetPlayerbotsOption<int32>("AiPlayerbot.ReactDelay", 100);
    dynamicReactDelay = GetPlayerbotsOption<bool>("AiPlayerbot.DynamicReactDelay", true);
    passiveDelay = GetPlayerbotsOption<int32>("AiPlayerbot.PassiveDelay", 10000);
    repeatDelay = GetPlayerbotsOption<int32>("AiPlayerbot.RepeatDelay", 2000);
    errorDelay = GetPlayerbotsOption<int32>("AiPlayerbot.ErrorDelay", 100);
    rpgDelay = GetPlayerbotsOption<int32>("AiPlayerbot.RpgDelay", 10000);
    sitDelay = GetPlayerbotsOption<int32>("AiPlayerbot.SitDelay", 20000);
    returnDelay = GetPlayerbotsOption<int32>("AiPlayerbot.ReturnDelay", 2000);
    lootDelay = GetPlayerbotsOption<int32>("AiPlayerbot.LootDelay", 1000);
    disabledWithoutRealPlayerLoginDelay = GetPlayerbotsOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLoginDelay", 30);
    disabledWithoutRealPlayerLogoutDelay = GetPlayerbotsOption<int32>("AiPlayerbot.DisabledWithoutRealPlayerLogoutDelay", 300);

    farDistance = GetPlayerbotsOption<float>("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = GetPlayerbotsOption<float>("AiPlayerbot.SightDistance", 100.0f);
    spellDistance = GetPlayerbotsOption<float>("AiPlayerbot.SpellDistance", 28.5f);
    shootDistance = GetPlayerbotsOption<float>("AiPlayerbot.ShootDistance", 5.0f);
    healDistance = GetPlayerbotsOption<float>("AiPlayerbot.HealDistance", 38.5f);
    lootDistance = GetPlayerbotsOption<float>("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = GetPlayerbotsOption<float>("AiPlayerbot.FleeDistance", 5.0f);
    aggroDistance = GetPlayerbotsOption<float>("AiPlayerbot.AggroDistance", 22.0f);
    tooCloseDistance = GetPlayerbotsOption<float>("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = GetPlayerbotsOption<float>("AiPlayerbot.MeleeDistance", 0.75f);
    followDistance = GetPlayerbotsOption<float>("AiPlayerbot.FollowDistance", 1.5f);
    whisperDistance = GetPlayerbotsOption<float>("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = GetPlayerbotsOption<float>("AiPlayerbot.ContactDistance", 0.45f);
    aoeRadius = GetPlayerbotsOption<float>("AiPlayerbot.AoeRadius", 10.0f);
    rpgDistance = GetPlayerbotsOption<float>("AiPlayerbot.RpgDistance", 200.0f);
    grindDistance = GetPlayerbotsOption<float>("AiPlayerbot.GrindDistance", 75.0f);
    reactDistance = GetPlayerbotsOption<float>("AiPlayerbot.ReactDistance", 150.0f);

    criticalHealth = GetPlayerbotsOption<int32>("AiPlayerbot.CriticalHealth", 25);
    lowHealth = GetPlayerbotsOption<int32>("AiPlayerbot.LowHealth", 45);
    mediumHealth = GetPlayerbotsOption<int32>("AiPlayerbot.MediumHealth", 65);
    almostFullHealth = GetPlayerbotsOption<int32>("AiPlayerbot.AlmostFullHealth", 85);
    lowMana = GetPlayerbotsOption<int32>("AiPlayerbot.LowMana", 15);
    mediumMana = GetPlayerbotsOption<int32>("AiPlayerbot.MediumMana", 40);
    highMana = GetPlayerbotsOption<int32>("AiPlayerbot.HighMana", 65);
    autoSaveMana = GetPlayerbotsOption<bool>("AiPlayerbot.AutoSaveMana", true);
    saveManaThreshold = GetPlayerbotsOption<int32>("AiPlayerbot.SaveManaThreshold", 60);
    switch (GetPlayerbotsOption<uint32>("AiPlayerbot.AutoGreaterBlessings", 1))
    {
        case 0:
            autoGreaterBlessings = AutoPartyBuffMode::DISABLED;
            break;
        case 2:
            autoGreaterBlessings = AutoPartyBuffMode::GROUP_OR_RAID;
            break;
        case 1:
        default:
            autoGreaterBlessings = AutoPartyBuffMode::RAID_ONLY;
            break;
    }
    switch (GetPlayerbotsOption<uint32>("AiPlayerbot.AutoPartyBuffs", 2))
    {
        case 0:
            autoPartyBuffs = AutoPartyBuffMode::DISABLED;
            break;
        case 1:
            autoPartyBuffs = AutoPartyBuffMode::RAID_ONLY;
            break;
        case 2:
        default:
            autoPartyBuffs = AutoPartyBuffMode::GROUP_OR_RAID;
            break;
    }
    tellWhenMissingBuffReagents = GetPlayerbotsOption<bool>("AiPlayerbot.TellWhenMissingBuffReagents", true);
    missingBuffReagentMessageCooldown = GetPlayerbotsOption<uint32>(
        "AiPlayerbot.MissingBuffReagentMessageCooldown", 300);
    forceRebuffOnReadyCheck = GetPlayerbotsOption<bool>("AiPlayerbot.ForceRebuffOnReadyCheck", false);
    forceRebuffMarginSecs = std::min(GetPlayerbotsOption<uint32>("AiPlayerbot.ForceRebuffMarginSecs", 60), 3600u);
    autoAvoidAoe = GetPlayerbotsOption<bool>("AiPlayerbot.AutoAvoidAoe", true);
    maxAoeAvoidRadius = GetPlayerbotsOption<float>("AiPlayerbot.MaxAoeAvoidRadius", 15.0f);
    LoadSet<std::set<uint32>>(GetPlayerbotsOption<std::string>("AiPlayerbot.AoeAvoidSpellWhitelist", "50759,57491,13810,29946"),
                              aoeAvoidSpellWhitelist);
    tellWhenAvoidAoe = GetPlayerbotsOption<bool>("AiPlayerbot.TellWhenAvoidAoe", false);

    gearLoweringChance = GetPlayerbotsOption<float>("AiPlayerbot.GearLoweringChance", 0.0f,
                                                    { "AiPlayerbot.RandomGearLoweringChance" });
    gearQualityLimit = GetPlayerbotsOption<int32>("AiPlayerbot.GearQualityLimit", 3,
                                                  { "AiPlayerbot.RandomGearQualityLimit" });
    gearScoreLimit = GetPlayerbotsOption<int32>("AiPlayerbot.GearScoreLimit", 0,
                                                { "AiPlayerbot.RandomGearScoreLimit" });
    preferClassArmorType  = GetPlayerbotsOption<bool>("AiPlayerbot.PreferClassArmorType", false);
    preferredSpecWeapons  = GetPlayerbotsOption<bool>("AiPlayerbot.PreferredSpecWeapons", false);

    randomBotMinLevelChance = GetPlayerbotsOption<float>("AiPlayerbot.RandomBotMinLevelChance", 0.1f);
    randomBotMaxLevelChance = GetPlayerbotsOption<float>("AiPlayerbot.RandomBotMaxLevelChance", 0.1f);
    randomBotRpgChance = GetPlayerbotsOption<float>("AiPlayerbot.RandomBotRpgChance", 0.20f);

    iterationsPerTick = GetPlayerbotsOption<int32>("AiPlayerbot.IterationsPerTick", 10);

    allowAccountBots = GetPlayerbotsOption<bool>("AiPlayerbot.AllowAccountBots", true);
    allowGuildBots = GetPlayerbotsOption<bool>("AiPlayerbot.AllowGuildBots", true);
    allowTrustedAccountBots = GetPlayerbotsOption<bool>("AiPlayerbot.AllowTrustedAccountBots", true);
    disabledWithoutRealPlayer = GetPlayerbotsOption<bool>("AiPlayerbot.DisabledWithoutRealPlayer", false);
    randomBotGuildNearby = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotGuildNearby", false);
    randomBotInvitePlayer = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotInvitePlayer", false);
    inviteChat = GetPlayerbotsOption<bool>("AiPlayerbot.InviteChat", false);

    randomBotMapsAsString = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<std::vector<uint32>>(randomBotMapsAsString, randomBotMaps);
    probTeleToBankers = GetPlayerbotsOption<float>("AiPlayerbot.ProbTeleToBankers", 0.25f);
    enableWeightTeleToCityBankers = GetPlayerbotsOption<bool>("AiPlayerbot.EnableWeightTeleToCityBankers", false);
    weightTeleToStormwind = GetPlayerbotsOption<int>("AiPlayerbot.TeleToStormwindWeight", 2);
    weightTeleToIronforge = GetPlayerbotsOption<int>("AiPlayerbot.TeleToIronforgeWeight", 1);
    weightTeleToDarnassus = GetPlayerbotsOption<int>("AiPlayerbot.TeleToDarnassusWeight", 1);
    weightTeleToExodar = GetPlayerbotsOption<int>("AiPlayerbot.TeleToExodarWeight", 1);
    weightTeleToOrgrimmar = GetPlayerbotsOption<int>("AiPlayerbot.TeleToOrgrimmarWeight", 2);
    weightTeleToUndercity = GetPlayerbotsOption<int>("AiPlayerbot.TeleToUndercityWeight", 1);
    weightTeleToThunderBluff = GetPlayerbotsOption<int>("AiPlayerbot.TeleToThunderBluffWeight", 1);
    weightTeleToSilvermoonCity = GetPlayerbotsOption<int>("AiPlayerbot.TeleToSilvermoonCityWeight", 1);
    weightTeleToShattrathCity = GetPlayerbotsOption<int>("AiPlayerbot.TeleToShattrathCityWeight", 1);
    weightTeleToDalaran = GetPlayerbotsOption<int>("AiPlayerbot.TeleToDalaranWeight", 1);
    LoadList<std::vector<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.BotQuestItems",
                                           "5175,5176,5177,5178,6948,11000,12382,13704,16309",
                                         { "AiPlayerbot.RandomBotQuestItems" }),
        botQuestItems);
    LoadList<std::vector<uint32>>(GetPlayerbotsOption<std::string>("AiPlayerbot.BotSpellIds", "54197",
                                                                   { "AiPlayerbot.RandomBotSpellIds" }),
                                  botSpellIds);
    LoadList<std::vector<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.PvpProhibitedZoneIds",
                                           "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,"
                                           "3539,3623,4152,3988,4658,4284,4418,4436,4275,4323,4395,3703,4298,3951"),
        pvpProhibitedZoneIds);
    LoadList<std::vector<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.PvpProhibitedAreaIds",
                                           "976,35,392,2268,4161,4010,4317,4312,3649,3887,3958,3724,4080,3938,3754,3786,"
                                           "3973,4085,4086,4087,4088"),
        pvpProhibitedAreaIds);
    fastReactInBG = GetPlayerbotsOption<bool>("AiPlayerbot.FastReactInBG", true);
    LoadList<std::vector<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.BotQuestIds",
                                           "3802,5505,6502,7761,7848,10277,10285,11492,"
                                           "13188,13189,24499,24511,24710,24712",
                                         { "AiPlayerbot.RandomBotQuestIds" }),
        botQuestIds);

    LoadSet<std::set<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.DisallowedGameObjects",
                                           "176213,17155,2656,74448,19020,3719,3658,3705,3706,105579,75293,2857,"
                                           "179490,141596,160836,160845,179516,176224,181085,176112,128308,128403,"
                                           "165739,165738,175245,175970,176325,176327,123329,2560"),
        disallowedGameObjects);
    LoadSet<std::set<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.AttunementQuests", "10279,10277,10282,10283,10284,10285,10296,"
                                           "10297,10298,11481,11482,11488,11490,11492,10901,10888,10445,10985"),
        attunementQuests);

    LoadSet<std::set<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.UnobtainableItems", "12468,44869,44870,46978"),
        unobtainableItems);

    botAutologin = GetPlayerbotsOption<bool>("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBots", 500);
    maxRandomBots = GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBots", 500);
    randomBotUpdateInterval = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotUpdateInterval", 20);
    randomBotCountChangeMinInterval =
        GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotCountChangeMinInterval", 30 * MINUTE);
    randomBotCountChangeMaxInterval =
        GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * HOUR);
    minRandomBotInWorldTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotInWorldTime", 2 * HOUR);
    maxRandomBotInWorldTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotInWorldTime", 14 * 24 * HOUR);
    minRandomBotRandomizeTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotRandomizeTime", 2 * HOUR);
    maxRandomBotRandomizeTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotRandomizeTime", 14 * 24 * HOUR);
    minRandomBotChangeStrategyTime =
        GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotChangeStrategyTime", 30 * MINUTE);
    maxRandomBotChangeStrategyTime =
        GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * HOUR);
    minRandomBotReviveTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotReviveTime", MINUTE);
    maxRandomBotReviveTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotReviveTime", 5 * MINUTE);
    minRandomBotTeleportInterval = GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotTeleportInterval", 1 * HOUR);
    maxRandomBotTeleportInterval = GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotTeleportInterval", 5 * HOUR);
    permanentlyInWorldTime =
        GetPlayerbotsOption<int32>("AiPlayerbot.PermanentlyInWorldTime", 1 * YEAR);
    randomBotTeleportDistance = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotTeleportDistance", 100);
    randomBotsPerInterval = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotsPerInterval", 60);
    randomBotPrintStatsInterval = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotPrintStatsInterval", 300);
    minRandomBotsPriceChangeInterval =
        GetPlayerbotsOption<int32>("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * HOUR);
    maxRandomBotsPriceChangeInterval =
        GetPlayerbotsOption<int32>("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * HOUR);
    randomBotJoinLfg = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotJoinLfg", true);

    restrictHealerDPS = GetPlayerbotsOption<bool>("AiPlayerbot.HealerDPSMapRestriction", false);
    LoadList<std::vector<uint32>>(
        GetPlayerbotsOption<std::string>("AiPlayerbot.RestrictedHealerDPSMaps",
                                             "33,34,36,43,47,48,70,90,109,129,209,229,230,329,349,389,429,1001,1004,"
                                             "1007,269,540,542,543,545,546,547,552,553,554,555,556,557,558,560,585,574,"
                                             "575,576,578,595,599,600,601,602,604,608,619,632,650,658,668,409,469,509,"
                                             "531,532,534,544,548,550,564,565,580,249,533,603,615,616,624,631,649,724"),
        restrictedHealerDPSMaps);

    //////////////////////////// ICC

    EnableICCBuffs = GetPlayerbotsOption<bool>("AiPlayerbot.EnableICCBuffs", true);

    //////////////////////////// Professions
    classMatchingProfessionChance =
        std::min<uint32>(100, GetPlayerbotsOption<uint32>("AiPlayerbot.ClassMatchingProfessionChance", 30));
    fishingDistanceFromMaster = GetPlayerbotsOption<float>("AiPlayerbot.FishingDistanceFromMaster", 10.0f);
    endFishingWithMaster = GetPlayerbotsOption<float>("AiPlayerbot.EndFishingWithMaster", 30.0f);
    fishingDistance = GetPlayerbotsOption<float>("AiPlayerbot.FishingDistance", 40.0f);
    enableFishingWithMaster = GetPlayerbotsOption<bool>("AiPlayerbot.EnableFishingWithMaster", true);
    //////////////////////////// CHAT
    enableBroadcasts = GetPlayerbotsOption<bool>("AiPlayerbot.EnableBroadcasts", true);
    randomBotTalk = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotTalk", false);
    randomBotEmote = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotEmote", false);
    randomBotSuggestDungeons = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotSuggestDungeons", true);
    randomBotSayWithoutMaster = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotSayWithoutMaster", false);

    // broadcastChanceMaxValue is used in urand(1, broadcastChanceMaxValue) for broadcasts,
    // lowering it will increase the chance, setting it to 0 will disable broadcasts
    // for internal use, not intended to be change by the user
    broadcastChanceMaxValue = enableBroadcasts ? 30000 : 0;

    // all broadcast chances should be in range 1-broadcastChanceMaxValue, value of 0 will disable this particular
    // broadcast setting value to max does not guarantee the broadcast, as there are some internal randoms as well
    broadcastToGuildGlobalChance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
    broadcastToWorldGlobalChance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
    broadcastToGeneralGlobalChance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
    broadcastToTradeGlobalChance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
    broadcastToLFGGlobalChance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
    broadcastToLocalDefenseGlobalChance =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
    broadcastToWorldDefenseGlobalChance =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
    broadcastToGuildRecruitmentGlobalChance =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);

    broadcastChanceLootingItemPoor = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
    broadcastChanceLootingItemNormal =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemNormal", 300);
    broadcastChanceLootingItemUncommon =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
    broadcastChanceLootingItemRare = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
    broadcastChanceLootingItemEpic = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
    broadcastChanceLootingItemLegendary =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
    broadcastChanceLootingItemArtifact =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);

    broadcastChanceQuestAccepted = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
    broadcastChanceQuestUpdateObjectiveCompleted =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
    broadcastChanceQuestUpdateObjectiveProgress =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
    broadcastChanceQuestUpdateFailedTimer =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
    broadcastChanceQuestUpdateComplete =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
    broadcastChanceQuestTurnedIn = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);

    broadcastChanceKillNormal = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillNormal", 30);
    broadcastChanceKillElite = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillElite", 300);
    broadcastChanceKillRareelite = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
    broadcastChanceKillWorldboss = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
    broadcastChanceKillRare = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillRare", 10000);
    broadcastChanceKillUnknown = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillUnknown", 100);
    broadcastChanceKillPet = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillPet", 10);
    broadcastChanceKillPlayer = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceKillPlayer", 30);

    broadcastChanceLevelupGeneric = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
    broadcastChanceLevelupTenX = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
    broadcastChanceLevelupMaxLevel = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);

    broadcastChanceSuggestInstance = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
    broadcastChanceSuggestQuest = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
    broadcastChanceSuggestGrindMaterials =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
    broadcastChanceSuggestGrindReputation =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
    broadcastChanceSuggestSell = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestSell", 300);
    broadcastChanceSuggestSomething =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);

    broadcastChanceSuggestSomethingToxic =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);

    broadcastChanceSuggestToxicLinks = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
    toxicLinksPrefix = GetPlayerbotsOption<std::string>("AiPlayerbot.ToxicLinksPrefix", "gnomes");

    broadcastChanceSuggestThunderfury =
        GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);

    // does not depend on global chance
    broadcastChanceGuildManagement = GetPlayerbotsOption<int32>("AiPlayerbot.BroadcastChanceGuildManagement", 30000);

    toxicLinksRepliesChance = GetPlayerbotsOption<int32>("AiPlayerbot.ToxicLinksRepliesChance", 30);    // 0-100
    thunderfuryRepliesChance = GetPlayerbotsOption<int32>("AiPlayerbot.ThunderfuryRepliesChance", 40);  // 0-100
    guildRepliesRate = GetPlayerbotsOption<int32>("AiPlayerbot.GuildRepliesRate", 100);                 // 0-100

    randomBotJoinBG = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotAutoJoinBG", false);

    randomBotAutoJoinArenaBracket = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinArenaBracket", 14);

    randomBotAutoJoinWSBrackets = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotAutoJoinWSBrackets", "7");
    randomBotAutoJoinABBrackets = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotAutoJoinABBrackets", "6");
    randomBotAutoJoinAVBrackets = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotAutoJoinAVBrackets", "3");
    randomBotAutoJoinEYBrackets = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotAutoJoinEYBrackets", "2");
    randomBotAutoJoinICBrackets = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotAutoJoinICBrackets", "1");

    randomBotAutoJoinBGWSCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGWSCount", 1);
    randomBotAutoJoinBGABCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGABCount", 1);
    randomBotAutoJoinBGAVCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGAVCount", 0);
    randomBotAutoJoinBGEYCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGEYCount", 1);
    randomBotAutoJoinBGICCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGICCount", 0);

    randomBotAutoJoinBGRatedArena2v2Count =
        GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena2v2Count", 0);
    randomBotAutoJoinBGRatedArena3v3Count =
        GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena3v3Count", 0);
    randomBotAutoJoinBGRatedArena5v5Count =
        GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAutoJoinBGRatedArena5v5Count", 0);
    logInGroupOnly = GetPlayerbotsOption<bool>("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = GetPlayerbotsOption<bool>("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotMaxLevel", 80);
    if (randomBotMaxLevel > sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL))
        randomBotMaxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);

    // Bracket defaults (below) derive from randomBotMaxLevel, so this must run after it is read.
    LoadRandomBotLevelConfig();

    randomBotTeleLowerLevel = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotTeleLowerLevel", 1);
    randomBotTeleHigherLevel = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotTeleHigherLevel", 3);
    openGoSpell = GetPlayerbotsOption<int32>("AiPlayerbot.OpenGoSpell", 6477);

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
        std::string value = GetPlayerbotsOption<std::string>(setting, "");

        if (!value.empty())
        {
            size_t commaPos = value.find(',');
            if (commaPos != std::string::npos)
            {
                uint32 minLevel = atoi(value.substr(0, commaPos).c_str());
                uint32 maxLevel = atoi(value.substr(commaPos + 1).c_str());
                zoneBrackets[zoneId] = std::make_pair(minLevel, maxLevel);
            }
        }
    }

    randomChangeMultiplier = GetPlayerbotsOption<float>("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotCombatStrategies", "");
    randomBotNonCombatStrategies = GetPlayerbotsOption<std::string>("AiPlayerbot.RandomBotNonCombatStrategies", "");
    combatStrategies = GetPlayerbotsOption<std::string>("AiPlayerbot.CombatStrategies", "");
    nonCombatStrategies = GetPlayerbotsOption<std::string>("AiPlayerbot.NonCombatStrategies", "");
    applyInstanceStrategies = GetPlayerbotsOption<bool>("AiPlayerbot.ApplyInstanceStrategies", true);

    commandPrefix = GetPlayerbotsOption<std::string>("AiPlayerbot.CommandPrefix", "");
    commandSeparator = GetPlayerbotsOption<std::string>("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = GetPlayerbotsOption<int32>("AiPlayerbot.CommandServerPort", 8888);
    perfMonEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.PerfMonEnabled", false);

    useGroundMountAtMinLevel = GetPlayerbotsOption<int32>("AiPlayerbot.UseGroundMountAtMinLevel", 20);
    useFastGroundMountAtMinLevel = GetPlayerbotsOption<int32>("AiPlayerbot.UseFastGroundMountAtMinLevel", 40);
    useFlyMountAtMinLevel = GetPlayerbotsOption<int32>("AiPlayerbot.UseFlyMountAtMinLevel", 60);
    useFastFlyMountAtMinLevel = GetPlayerbotsOption<int32>("AiPlayerbot.UseFastFlyMountAtMinLevel", 70);

    // stagger bot flightpath takeoff
    botTaxiDelayMin = GetPlayerbotsOption<uint32>("AiPlayerbot.BotTaxiDelayMinMs", 350);
    botTaxiDelayMax = GetPlayerbotsOption<uint32>("AiPlayerbot.BotTaxiDelayMaxMs", 5000);
    botTaxiGapMs = GetPlayerbotsOption<uint32>("AiPlayerbot.BotTaxiGapMs", 200);
    botTaxiGapJitterMs = GetPlayerbotsOption<uint32>("AiPlayerbot.BotTaxiGapJitterMs", 100);

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
            premadeSpecName[cls][spec] = GetPlayerbotsOption<std::string>(os.str().c_str(), "", {}, false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.PremadeSpecGlyph." << cls << "." << spec;
            premadeSpecGlyph[cls][spec] = GetPlayerbotsOption<std::string>(os.str().c_str(), "", {}, false);
            std::vector<std::string> splitSpecGlyph = split(premadeSpecGlyph[cls][spec], ',');
            for (std::string& split : splitSpecGlyph)
            {
                if (split.size() != 0)
                {
                    parsedSpecGlyph[cls][spec].push_back(atoi(split.c_str()));
                }
            }
            for (uint32 level = 0; level < MAX_LEVEL; ++level)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                premadeSpecLink[cls][spec][level] = GetPlayerbotsOption<std::string>(os.str().c_str(), "", {}, false);
                parsedSpecLinkOrder[cls][spec][level] = ParseTempTalentsOrder(cls, premadeSpecLink[cls][spec][level]);
            }
        }
        for (uint32 spec = 0; spec < 3; ++spec)
        {
            for (uint32 points = 0; points < 21; ++points)
            {
                std::ostringstream os;
                os << "AiPlayerbot.PremadeHunterPetLink." << spec << "." << points;
                premadeHunterPetLink[spec][points] = GetPlayerbotsOption<std::string>(os.str().c_str(), "", {}, false);
                parsedHunterPetLinkOrder[spec][points] =
                    ParseTempPetTalentsOrder(spec, premadeHunterPetLink[spec][points]);
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
            randomClassSpecProb[cls][spec] = GetPlayerbotsOption<uint32>(os.str().c_str(), def, {}, false);
            os.str("");
            os.clear();
            os << "AiPlayerbot.RandomClassSpecIndex." << cls << "." << spec;
            randomClassSpecIndex[cls][spec] = GetPlayerbotsOption<uint32>(os.str().c_str(), spec, {}, false);
        }
    }

    botCheats.clear();
    LoadListString<std::vector<std::string>>(GetPlayerbotsOption<std::string>("AiPlayerbot.BotCheats", "food,taxi,raid"),
                                             botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "food") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::food;
    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(botCheats.begin(), botCheats.end(), "raid") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::raid;

    LoadListString<std::vector<std::string>>(GetPlayerbotsOption<std::string>("AiPlayerbot.AllowedLogFiles", ""),
                                             allowedLogFiles);
    enableAutoTradeOnItemMention = GetPlayerbotsOption<bool>("AiPlayerbot.EnableAutoTradeOnItemMention", true);
    LoadListString<std::vector<std::string>>(GetPlayerbotsOption<std::string>("AiPlayerbot.TradeActionExcludedPrefixes", ""),
                                             tradeActionExcludedPrefixes);

    worldBuffs.clear();
    loadWorldBuff();
    LOG_INFO("playerbots", "Loading World Buff Feature...");

    botAccountPrefix = GetPlayerbotsOption<std::string>("AiPlayerbot.BotAccountPrefix", "rndbot",
                                                        { "AiPlayerbot.RandomBotAccountPrefix" });
    botAccountCount = GetPlayerbotsOption<int32>("AiPlayerbot.BotAccountCount", 0,
                                                 { "AiPlayerbot.RandomBotAccountCount" });
    deleteBotAccounts = GetPlayerbotsOption<bool>("AiPlayerbot.DeleteBotAccounts", false,
                                                  { "AiPlayerbot.DeleteRandomBotAccounts" });
    botRandomPassword = GetPlayerbotsOption<bool>("AiPlayerbot.BotRandomPassword", false,
                                                  { "AiPlayerbot.RandomBotRandomPassword" });
    randomBotGuildCount = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotGuildCount", 20);
    randomBotGuildSizeMax = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotGuildSizeMax", 15);
    deleteRandomBotGuilds = GetPlayerbotsOption<bool>("AiPlayerbot.DeleteRandomBotGuilds", false);

    botSendMailEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.BotSendMailEnabled", true);

    guildTaskEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.EnableGuildTasks", false);
    minGuildTaskChangeTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinGuildTaskChangeTime", 3 * 24 * 3600);
    maxGuildTaskChangeTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxGuildTaskChangeTime", 4 * 24 * 3600);
    minGuildTaskAdvertisementTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinGuildTaskAdvertisementTime", 300);
    maxGuildTaskAdvertisementTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxGuildTaskAdvertisementTime", 12 * 3600);
    minGuildTaskRewardTime = GetPlayerbotsOption<int32>("AiPlayerbot.MinGuildTaskRewardTime", 300);
    maxGuildTaskRewardTime = GetPlayerbotsOption<int32>("AiPlayerbot.MaxGuildTaskRewardTime", 3600);
    guildTaskAdvertCleanupTime = GetPlayerbotsOption<int32>("AiPlayerbot.GuildTaskAdvertCleanupTime", 300);
    guildTaskKillTaskDistance = GetPlayerbotsOption<int32>("AiPlayerbot.GuildTaskKillTaskDistance", 2000);
    targetPosRecalcDistance = GetPlayerbotsOption<float>("AiPlayerbot.TargetPosRecalcDistance", 0.1f);

    //cosmetics
    switch (GetPlayerbotsOption<int32>("AiPlayerbot.BotShowHelmet", 1,
                                       { "AiPlayerbot.RandomBotShowHelmet" }))
    {
        case 0:
            botShowHelmet = ShowHideCosmetic::ALWAYS_HIDE;
            break;
        case 2:
            botShowHelmet = ShowHideCosmetic::RANDOMIZE;
            break;
        case 1:
        default:
            botShowHelmet = ShowHideCosmetic::ALWAYS_SHOW;
            break;
    }
    switch (GetPlayerbotsOption<int32>("AiPlayerbot.BotShowCloak", 1,
                                       { "AiPlayerbot.RandomBotShowCloak" }))
    {
        case 0:
            botShowCloak = ShowHideCosmetic::ALWAYS_HIDE;
            break;
        case 2:
            botShowCloak = ShowHideCosmetic::RANDOMIZE;
            break;
        case 1:
        default:
            botShowCloak = ShowHideCosmetic::ALWAYS_SHOW;
            break;
    }

    // SPP switches
    enableGreet = GetPlayerbotsOption<bool>("AiPlayerbot.EnableGreet", true);
    summonWhenGroup = GetPlayerbotsOption<bool>("AiPlayerbot.SummonWhenGroup", true);
    randomBotFixedLevel = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotFixedLevel", false);
    disableRandomLevels = GetPlayerbotsOption<bool>("AiPlayerbot.DisableRandomLevels", false);
    downgradeMaxLevelBot = GetPlayerbotsOption<bool>("AiPlayerbot.DowngradeMaxLevelBot", true);
    equipAndSpecPersistence = GetPlayerbotsOption<bool>("AiPlayerbot.EquipAndSpecPersistence", true);
    equipAndSpecPersistenceLevel = GetPlayerbotsOption<int32>("AiPlayerbot.EquipAndSpecPersistenceLevel", 1);
    groupInvitationPermission = GetPlayerbotsOption<int32>("AiPlayerbot.GroupInvitationPermission", 1);
    keepAltsInGroup = GetPlayerbotsOption<bool>("AiPlayerbot.KeepAltsInGroup", false);
    allowSummonInCombat = GetPlayerbotsOption<bool>("AiPlayerbot.AllowSummonInCombat", true);
    allowSummonWhenMasterIsDead = GetPlayerbotsOption<bool>("AiPlayerbot.AllowSummonWhenMasterIsDead", true);
    allowSummonWhenBotIsDead = GetPlayerbotsOption<bool>("AiPlayerbot.AllowSummonWhenBotIsDead", true);
    reviveBotWhenSummoned = GetPlayerbotsOption<int32>("AiPlayerbot.ReviveBotWhenSummoned", 1);
    botRepairWhenSummon = GetPlayerbotsOption<bool>("AiPlayerbot.BotRepairWhenSummon", true);
    autoInitOnly = GetPlayerbotsOption<bool>("AiPlayerbot.AutoInitOnly", false);
    resetInstanceIdForAltBots = GetPlayerbotsOption<bool>("AiPlayerbot.ResetInstanceIdForAltBots", false);
    autoInitEquipLevelLimitRatio = GetPlayerbotsOption<float>("AiPlayerbot.AutoInitEquipLevelLimitRatio", 1.0);

    maxAddedBots = GetPlayerbotsOption<int32>("AiPlayerbot.MaxAddedBots", 40);
    addClassCommand = GetPlayerbotsOption<int32>("AiPlayerbot.AddClassCommand", 1);
    addClassAccountPoolSize = GetPlayerbotsOption<int32>("AiPlayerbot.AddClassAccountPoolSize", 50);
    maintenanceCommand = GetPlayerbotsOption<int32>("AiPlayerbot.MaintenanceCommand", 1);

    altMaintenanceAttunementQs = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceAttunementQuests", true);
    altMaintenanceBags = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceBags", true);
    altMaintenanceAmmo = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceAmmo", true);
    altMaintenanceFood = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceFood", true);
    altMaintenanceReagents = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceReagents", true);
    altMaintenanceConsumables = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceConsumables", true);
    altMaintenancePotions = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenancePotions", true);
    altMaintenanceTalentTree = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceTalentTree", true);
    altMaintenancePet = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenancePet", true);
    altMaintenancePetTalents = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenancePetTalents", true);
    altMaintenanceClassSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceClassSpells", true);
    altMaintenanceAvailableSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceAvailableSpells", true);
    altMaintenanceSkills = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceSkills", true);
    altMaintenanceReputation = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceReputation", true);
    altMaintenanceSpecialSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceSpecialSpells", true);
    altMaintenanceMounts = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceMounts", true);
    altMaintenanceGlyphs = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceGlyphs", true);
    altMaintenanceKeyring = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceKeyring", true);
    altMaintenanceGemsEnchants = GetPlayerbotsOption<bool>("AiPlayerbot.AltMaintenanceGemsEnchants", true);

    autoGearCommand = GetPlayerbotsOption<int32>("AiPlayerbot.AutoGearCommand", 1);
    autoGearCommandAltBots = GetPlayerbotsOption<int32>("AiPlayerbot.AutoGearCommandAltBots", 1);
    autoGearBisCommand = GetPlayerbotsOption<int32>("AiPlayerbot.AutoGearBisCommand", 0);
    autoGearQualityLimit = GetPlayerbotsOption<int32>("AiPlayerbot.AutoGearQualityLimit", 3);
    autoGearScoreLimit = GetPlayerbotsOption<int32>("AiPlayerbot.AutoGearScoreLimit", 0);

    randomBotXPRate = GetPlayerbotsOption<float>("AiPlayerbot.RandomBotXPRate", 1.0);
    randomBotAllianceRatio = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotAllianceRatio", 50);
    randomBotHordeRatio = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotHordeRatio", 50);
    disableDeathKnightLogin = GetPlayerbotsOption<bool>("AiPlayerbot.DisableDeathKnightLogin", 0);
    limitTalentsExpansion = GetPlayerbotsOption<bool>("AiPlayerbot.LimitTalentsExpansion", 0);
    botActiveAlone = GetPlayerbotsOption<int32>("AiPlayerbot.BotActiveAlone", 10);
    BotActiveAloneDurationSeconds = GetPlayerbotsOption<int32>("AiPlayerbot.BotActiveAloneDurationSeconds", 30);
    BotActiveAloneForceWhenInRadius = GetPlayerbotsOption<uint32>("AiPlayerbot.BotActiveAloneForceWhenInRadius", 150);
    BotActiveAloneForceWhenInZone = GetPlayerbotsOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInZone", 1);
    BotActiveAloneForceWhenInMap = GetPlayerbotsOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInMap", 0);
    BotActiveAloneForceWhenIsFriend = GetPlayerbotsOption<bool>("AiPlayerbot.BotActiveAloneForceWhenIsFriend", 0);
    BotActiveAloneForceWhenInGuild = GetPlayerbotsOption<bool>("AiPlayerbot.BotActiveAloneForceWhenInGuild", 1);
    botActiveAloneSmartScale = GetPlayerbotsOption<bool>("AiPlayerbot.botActiveAloneSmartScale", 1);
    botActiveAloneSmartScaleDiffLimitfloor = GetPlayerbotsOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitfloor", 50);
    botActiveAloneSmartScaleDiffLimitCeiling = GetPlayerbotsOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleDiffLimitCeiling", 200);
    botActiveAloneSmartScaleWhenMinLevel = GetPlayerbotsOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMinLevel", 1);
    botActiveAloneSmartScaleWhenMaxLevel = GetPlayerbotsOption<uint32>("AiPlayerbot.botActiveAloneSmartScaleWhenMaxLevel", 80);

    randombotsWalkingRPG = GetPlayerbotsOption<bool>("AiPlayerbot.RandombotsWalkingRPG", false);
    randombotsWalkingRPGInDoors = GetPlayerbotsOption<bool>("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = GetPlayerbotsOption<int32>("AiPlayerbot.MinEnchantingBotLevel", 60);
    limitEnchantExpansion = GetPlayerbotsOption<int32>("AiPlayerbot.LimitEnchantExpansion", 1);
    limitGearExpansion = GetPlayerbotsOption<int32>("AiPlayerbot.LimitGearExpansion", 1);
    randombotStartingLevel = GetPlayerbotsOption<int32>("AiPlayerbot.RandombotStartingLevel", 1);
    enablePeriodicOnlineOffline = GetPlayerbotsOption<bool>("AiPlayerbot.EnablePeriodicOnlineOffline", false);
    enableRandomBotTrading = GetPlayerbotsOption<int32>("AiPlayerbot.EnableRandomBotTrading", 1);
    periodicOnlineOfflineRatio = GetPlayerbotsOption<float>("AiPlayerbot.PeriodicOnlineOfflineRatio", 2.0);
    gearscorecheck = GetPlayerbotsOption<bool>("AiPlayerbot.GearScoreCheck", false);
    randomBotPreQuests = GetPlayerbotsOption<bool>("AiPlayerbot.PreQuests", false);

    // SPP automation
    freeMethodLoot = GetPlayerbotsOption<bool>("AiPlayerbot.FreeMethodLoot", false);
    lootNeedRollLevel = GetPlayerbotsOption<int32>("AiPlayerbot.LootNeedRollLevel", 1);
    lootRollRecipe = GetPlayerbotsOption<bool>("AiPlayerbot.LootRollRecipe", false);
    lootRollDisenchant = GetPlayerbotsOption<bool>("AiPlayerbot.LootRollDisenchant", false);
    lootGreedRollLevel = GetPlayerbotsOption<bool>("AiPlayerbot.LootGreedRollLevel", false);
    autoPickReward = GetPlayerbotsOption<std::string>("AiPlayerbot.AutoPickReward", "yes");
    autoEquipUpgradeLoot = GetPlayerbotsOption<bool>("AiPlayerbot.AutoEquipUpgradeLoot", true);
    equipUpgradeThreshold = GetPlayerbotsOption<float>("AiPlayerbot.EquipUpgradeThreshold", 1.1f);
    twoRoundsGearInit = GetPlayerbotsOption<bool>("AiPlayerbot.TwoRoundsGearInit", false);
    syncQuestWithPlayer = GetPlayerbotsOption<bool>("AiPlayerbot.SyncQuestWithPlayer", true);
    syncQuestForPlayer = GetPlayerbotsOption<bool>("AiPlayerbot.SyncQuestForPlayer", false);
    dropObsoleteQuests = GetPlayerbotsOption<bool>("AiPlayerbot.DropObsoleteQuests", true);
    allowLearnTrainerSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AllowLearnTrainerSpells", true);
    autoPickTalents = GetPlayerbotsOption<bool>("AiPlayerbot.AutoPickTalents", true);
    autoUpgradeEquip = GetPlayerbotsOption<bool>("AiPlayerbot.AutoUpgradeEquip", true);
    hunterWolfPet = GetPlayerbotsOption<int32>("AiPlayerbot.HunterWolfPet", 0);
    defaultPetStance = GetPlayerbotsOption<int32>("AiPlayerbot.DefaultPetStance", 1);
    petChatCommandDebug = GetPlayerbotsOption<bool>("AiPlayerbot.PetChatCommandDebug", 0);
    autoLearnTrainerSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AutoLearnTrainerSpells", true);
    autoLearnQuestSpells = GetPlayerbotsOption<bool>("AiPlayerbot.AutoLearnQuestSpells", true);
    autoTeleportForLevel = GetPlayerbotsOption<bool>("AiPlayerbot.AutoTeleportForLevel", false);
    autoDoQuests = GetPlayerbotsOption<bool>("AiPlayerbot.AutoDoQuests", true);
    enableNewRpgStrategy = GetPlayerbotsOption<bool>("AiPlayerbot.EnableNewRpgStrategy", true);

    RpgStatusProbWeight[RPG_WANDER_RANDOM] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderRandom", 15);
    RpgStatusProbWeight[RPG_WANDER_NPC] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.WanderNpc", 20);
    RpgStatusProbWeight[RPG_GO_GRIND] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoGrind", 15);
    RpgStatusProbWeight[RPG_GO_CAMP] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.GoCamp", 10);
    RpgStatusProbWeight[RPG_DO_QUEST] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.DoQuest", 60);
    RpgStatusProbWeight[RPG_TRAVEL_FLIGHT] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.TravelFlight", 15);
    RpgStatusProbWeight[RPG_REST] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.Rest", 5);
    RpgStatusProbWeight[RPG_OUTDOOR_PVP] = GetPlayerbotsOption<int32>("AiPlayerbot.RpgStatusProbWeight.OutdoorPvp", 10);

    syncLevelWithPlayers = GetPlayerbotsOption<bool>("AiPlayerbot.SyncLevelWithPlayers", false);
    randomBotGroupNearby = GetPlayerbotsOption<bool>("AiPlayerbot.RandomBotGroupNearby", false);

    // arena
    randomBotArenaTeam2v2Count = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotArenaTeam2v2Count", 10);
    randomBotArenaTeam3v3Count = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotArenaTeam3v3Count", 10);
    randomBotArenaTeam5v5Count = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotArenaTeam5v5Count", 5);
    deleteRandomBotArenaTeams = GetPlayerbotsOption<bool>("AiPlayerbot.DeleteRandomBotArenaTeams", false);
    randomBotArenaTeamMaxRating = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotArenaTeamMaxRating", 2000);
    randomBotArenaTeamMinRating = GetPlayerbotsOption<int32>("AiPlayerbot.RandomBotArenaTeamMinRating", 1000);

    selfBotLevel = GetPlayerbotsOption<int32>("AiPlayerbot.SelfBotLevel", 1);

    RandomPlayerbotFactory::CreateRandomBots();
    if (World::IsStopped())
    {
        return true;
    }

    // Assign account types after accounts are created
    sRandomPlayerbotMgr.AssignAccountTypes();

    if (sPlayerbotAIConfig.enabled)
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

    if (sPlayerbotAIConfig.randomBotSuggestDungeons)
    {
        PlayerbotDungeonRepository::instance().LoadDungeonSuggestions();
    }
    sTravelMgr.Init();

    excludedHunterPetFamilies.clear();
    LoadList<std::vector<uint32>>(GetPlayerbotsOption<std::string>("AiPlayerbot.ExcludedHunterPetFamilies", ""), excludedHunterPetFamilies);

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
    levelBracketsEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.Enabled", false);
    levelBracketsIgnoreGuildWithRealPlayers =
        GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.IgnoreGuildBotsWithRealPlayers", true);
    levelBracketsIgnoreArenaTeamBots =
        GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.IgnoreArenaTeamBots", true);

    levelBracketsCheckFrequency = GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.CheckFrequency", 300);
    levelBracketsFlaggedCheckFrequency =
        GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.CheckFlaggedFrequency", 15);
    levelBracketsDynamicDistribution =
        GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.Dynamic.UseDynamicDistribution", false);
    levelBracketsRealPlayerWeight =
        GetPlayerbotsOption<float>("AiPlayerbot.LevelBrackets.Dynamic.RealPlayerWeight", 1.0f);
    levelBracketsSyncFactions = GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.Dynamic.SyncFactions", false);
    levelBracketsIgnoreFriendListed = GetPlayerbotsOption<bool>("AiPlayerbot.LevelBrackets.IgnoreFriendListed", true);
    levelBracketsFlaggedProcessLimit =
        GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.FlaggedProcessLimit", 5);

    ParseLevelMgrExcludeNames(GetPlayerbotsOption<std::string>("AiPlayerbot.LevelBrackets.ExcludeNames", ""),
        levelBracketsExcludeNames);

    levelBracketsNumRanges =
        static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.NumRanges", 9));
    levelBracketsAlliance.resize(levelBracketsNumRanges);
    levelBracketsHorde.resize(levelBracketsNumRanges);

    for (uint8 i = 0; i < levelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < levelBracketsNumRanges - 1 ? i * 10 + 9 : randomBotMaxLevel);
        levelBracketsAlliance[i].lower = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Lower", defaultLower));
        levelBracketsAlliance[i].upper = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Upper", defaultUpper));
        levelBracketsAlliance[i].pct = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Pct", 11));
    }

    for (uint8 i = 0; i < levelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < levelBracketsNumRanges - 1 ? i * 10 + 9 : randomBotMaxLevel);
        levelBracketsHorde[i].lower = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Lower", defaultLower));
        levelBracketsHorde[i].upper = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Upper", defaultUpper));
        levelBracketsHorde[i].pct = static_cast<uint8>(
            GetPlayerbotsOption<uint32>("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Pct", 11));
    }

    // A mismatch forcibly disables SyncFactions and logs an error; it never brings the server down.
    if (levelBracketsSyncFactions)
    {
        for (uint8 i = 0; i < levelBracketsNumRanges; ++i)
        {
            if (levelBracketsAlliance[i].lower != levelBracketsHorde[i].lower ||
                levelBracketsAlliance[i].upper != levelBracketsHorde[i].upper)
            {
                LOG_ERROR("server.loading",
                    "[RandomBotLevelMgr] Bracket mismatch detected between factions at index {}. Alliance: {}-{}, "
                    "Horde: {}-{}. SyncFactions requires both bracket count and min/max levels to match exactly; "
                    "forcibly disabling SyncFactions for this session. Check your configuration.",
                    i, levelBracketsAlliance[i].lower, levelBracketsAlliance[i].upper,
                    levelBracketsHorde[i].lower, levelBracketsHorde[i].upper);
                levelBracketsSyncFactions = false;
                break;
            }
        }
    }

    // ---- Level reset ----
    resetBotLevelEnabled = GetPlayerbotsOption<bool>("AiPlayerbot.ResetBotLevel.Enabled", false);

    resetBotLevelMaxLevel =
        static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.MaxLevel", 80));
    if ((resetBotLevelMaxLevel < 2 || resetBotLevelMaxLevel > 80) && resetBotLevelMaxLevel != 0)
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.MaxLevel value: {}. Using default value 80.",
            resetBotLevelMaxLevel);
        resetBotLevelMaxLevel = 80;
    }

    resetBotLevelResetTo =
        static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.ResetToLevel", 1));
    if (resetBotLevelResetTo < 1 || (resetBotLevelMaxLevel > 0 && resetBotLevelResetTo >= resetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetToLevel value: {}. Using default value 1.",
            resetBotLevelResetTo);
        resetBotLevelResetTo = 1;
    }

    resetBotLevelSkipFrom =
        static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.SkipFromLevel", 0));
    if (resetBotLevelSkipFrom > 80 || (resetBotLevelMaxLevel > 0 && resetBotLevelSkipFrom >= resetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipFromLevel value: {}. Using default value 0 "
            "(disabled).",
            resetBotLevelSkipFrom);
        resetBotLevelSkipFrom = 0;
    }

    resetBotLevelSkipTo = static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.SkipToLevel", 1));
    if (resetBotLevelSkipTo < 1 || resetBotLevelSkipTo > 80 ||
        (resetBotLevelMaxLevel > 0 && resetBotLevelSkipTo > resetBotLevelMaxLevel))
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipToLevel value: {}. Using default value 1.",
            resetBotLevelSkipTo);
        resetBotLevelSkipTo = 1;
    }

    resetBotLevelChance =
        static_cast<uint8>(GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.ResetChance", 100));
    if (resetBotLevelChance > 100)
    {
        LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetChance value: {}. Using default value 100.",
            resetBotLevelChance);
        resetBotLevelChance = 100;
    }

    resetBotLevelScaledChance = GetPlayerbotsOption<bool>("AiPlayerbot.ResetBotLevel.ScaledChance", false);

    resetBotLevelRestrictTimePlayed =
        GetPlayerbotsOption<bool>("AiPlayerbot.ResetBotLevel.RestrictTimePlayed", false);
    resetBotLevelMinTimePlayed = GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.MinTimePlayed", 86400);
    resetBotLevelPlayedTimeCheckFrequency =
        GetPlayerbotsOption<uint32>("AiPlayerbot.ResetBotLevel.PlayedTimeCheckFrequency", 864);

    resetBotLevelIgnoreGuildWithRealPlayers =
        GetPlayerbotsOption<bool>("AiPlayerbot.ResetBotLevel.IgnoreGuildBotsWithRealPlayers", false);

    ParseLevelMgrExcludeNames(GetPlayerbotsOption<std::string>("AiPlayerbot.ResetBotLevel.ExcludeNames", ""),
        resetBotLevelExcludeNames);
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsInBotQuestItemList(uint32 id)
{
    return find(botQuestItems.begin(), botQuestItems.end(), id) != botQuestItems.end();
}

bool PlayerbotAIConfig::IsPvpProhibited(uint32 zoneId, uint32 areaId)
{
    return IsInPvpProhibitedZone(zoneId) || IsInPvpProhibitedArea(areaId) || IsInPvpProhibitedZone(areaId);
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
    return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedArea(uint32 id)
{
    return find(pvpProhibitedAreaIds.begin(), pvpProhibitedAreaIds.end(), id) != pvpProhibitedAreaIds.end();
}

bool PlayerbotAIConfig::IsRestrictedHealerDPSMap(uint32 mapId) const
{
    return restrictHealerDPS &&
            std::find(restrictedHealerDPSMaps.begin(), restrictedHealerDPSMaps.end(), mapId) != restrictedHealerDPSMaps.end();
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
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d-%02d-%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour,
             aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(std::string const fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;

    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(std::make_pair(fileName, std::make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
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

void PlayerbotAIConfig::log(std::string const fileName, char const* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName) && !openLog(fileName, "a"))
        return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::loadWorldBuff()
{
    std::string matrix = GetPlayerbotsOption<std::string>("AiPlayerbot.WorldBuffMatrix", "");
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
                worldBuff wb = { spellId, ids[0], ids[1], ids[2], ids[3], ids[4] };
                worldBuffs.push_back(wb);
            } catch (...) {
                LOG_ERROR("playerbots", "Invalid spell ID in [{}]", entry);
            }
        }
    }
}

static std::vector<std::string> split(const std::string& str, const std::string& pattern)
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
