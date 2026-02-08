#include "RaidSSCHelpers.h"
#include "AiFactory.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

namespace SerpentShrineCavernHelpers
{
    // Hydross the Unstable <Duke of Currents>

    const Position HYDROSS_FROST_TANK_POSITION = { -236.669f, -358.352f, -0.828f };
    const Position HYDROSS_NATURE_TANK_POSITION = { -225.471f, -327.790f, -3.682f };

    std::unordered_map<uint32, time_t> hydrossFrostDpsWaitTimer;
    std::unordered_map<uint32, time_t> hydrossNatureDpsWaitTimer;
    std::unordered_map<uint32, time_t> hydrossChangeToFrostPhaseTimer;
    std::unordered_map<uint32, time_t> hydrossChangeToNaturePhaseTimer;

    bool HasMarkOfHydrossAt100Percent(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_HYDROSS_100) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_250) ||
               bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
    }

    bool HasNoMarkOfHydross(Player* bot)
    {
        return !bot->HasAura(SPELL_MARK_OF_HYDROSS_10) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_25) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_50) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_100) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_250) &&
               !bot->HasAura(SPELL_MARK_OF_HYDROSS_500);
    }

    bool HasMarkOfCorruptionAt100Percent(Player* bot)
    {
        return bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) ||
               bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
    }

    bool HasNoMarkOfCorruption(Player* bot)
    {
        return !bot->HasAura(SPELL_MARK_OF_CORRUPTION_10) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_25) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_50) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_100) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_250) &&
               !bot->HasAura(SPELL_MARK_OF_CORRUPTION_500);
    }

    // The Lurker Below

    const Position LURKER_MAIN_TANK_POSITION = { 23.706f, -406.038f, -19.686f };

    std::unordered_map<uint32, time_t> lurkerSpoutTimer;
    std::unordered_map<ObjectGuid, Position> lurkerRangedPositions;

    bool IsLurkerCastingSpout(Unit* lurker)
    {
        if (!lurker || !lurker->HasUnitState(UNIT_STATE_CASTING))
            return false;

        Spell* currentSpell = lurker->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!currentSpell)
            return false;

        uint32 spellId = currentSpell->m_spellInfo->Id;
        bool isSpout = spellId == SPELL_SPOUT_VISUAL;

        return isSpout;
    }

    // Leotheras the Blind

    std::unordered_map<uint32, time_t> leotherasHumanFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasDemonFormDpsWaitTimer;
    std::unordered_map<uint32, time_t> leotherasFinalPhaseDpsWaitTimer;

    Unit* GetLeotherasHuman(PlayerbotAI* botAI)
    {
        auto const& npcs =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_LEOTHERAS_THE_BLIND &&
                unit->IsInCombat() && !unit->HasAura(SPELL_METAMORPHOSIS))
                return unit;
        }
        return nullptr;
    }

    Unit* GetPhase2LeotherasDemon(PlayerbotAI* botAI)
    {
        auto const& npcs =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_LEOTHERAS_THE_BLIND &&
                unit->HasAura(SPELL_METAMORPHOSIS))
                return unit;
        }
        return nullptr;
    }

    Unit* GetPhase3LeotherasDemon(PlayerbotAI* botAI)
    {
        auto const& npcs =
            botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs")->Get();
        for (auto const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->GetEntry() == NPC_SHADOW_OF_LEOTHERAS)
                return unit;
        }
        return nullptr;
    }

    Unit* GetActiveLeotherasDemon(PlayerbotAI* botAI)
    {
        Unit* phase2 = GetPhase2LeotherasDemon(botAI);
        Unit* phase3 = GetPhase3LeotherasDemon(botAI);
        return phase2 ? phase2 : phase3;
    }

    Player* GetLeotherasDemonFormTank(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        // (1) First loop: Return the first assistant Warlock (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_WARLOCK)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member;
        }

        // (2) Fall back to first found bot Warlock
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_WARLOCK)
                continue;

            return member;
        }

        // (3) Return nullptr if none found
        return nullptr;
    }

    // Fathom-Lord Karathress

    const Position KARATHRESS_TANK_POSITION = { 474.403f, -531.118f, -7.548f };
    const Position TIDALVESS_TANK_POSITION = { 511.282f, -501.162f, -13.158f };
    const Position SHARKKIS_TANK_POSITION = { 508.057f, -541.109f, -10.133f };
    const Position CARIBDIS_TANK_POSITION = { 464.462f, -475.820f, -13.158f };
    const Position CARIBDIS_HEALER_POSITION = { 466.203f, -503.201f, -13.158f };
    const Position CARIBDIS_RANGED_DPS_POSITION = { 463.197f, -501.190f, -13.158f };

    std::unordered_map<uint32, time_t> karathressDpsWaitTimer;

    // Morogrim Tidewalker

    const Position TIDEWALKER_PHASE_1_TANK_POSITION = { 410.925f, -741.916f, -7.146f };
    const Position TIDEWALKER_PHASE_TRANSITION_WAYPOINT = { 407.035f, -759.479f, -7.168f };
    const Position TIDEWALKER_PHASE_2_TANK_POSITION = { 446.571f, -767.155f, -7.144f };
    const Position TIDEWALKER_PHASE_2_RANGED_POSITION = { 432.595f, -766.288f, -7.145f };

    std::unordered_map<ObjectGuid, uint8> tidewalkerTankStep;
    std::unordered_map<ObjectGuid, uint8> tidewalkerRangedStep;

    // Lady Vashj <Coilfang Matron>

    const Position VASHJ_PLATFORM_CENTER_POSITION = { 29.634f, -923.541f, 42.985f };

    std::unordered_map<ObjectGuid, Position> vashjRangedPositions;
    std::unordered_map<ObjectGuid, bool> hasReachedVashjRangedPosition;
    std::unordered_map<uint32, ObjectGuid> nearestTriggerGuid;
    std::unordered_map<ObjectGuid, Position> intendedLineup;
    std::unordered_map<uint32, time_t> lastImbueAttempt;
    std::unordered_map<uint32, time_t> lastCoreInInventoryTime;

    bool IsMainTankInSameSubgroup(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group || !group->isRaidGroup())
            return false;

        uint8 botSubGroup = group->GetMemberGroup(bot->GetGUID());
        if (botSubGroup >= MAX_RAID_SUBGROUPS)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            if (group->GetMemberGroup(member->GetGUID()) != botSubGroup)
                continue;

            if (PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member))
            {
                if (memberAI->IsMainTank(member))
                    return true;
            }
        }

        return false;
    }

    bool IsLadyVashjInPhase1(PlayerbotAI* botAI)
    {
        Unit* vashj =
            botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetHealthPct() > 70.0f &&
               vashjCreature->GetReactState() != REACT_PASSIVE;
    }

    bool IsLadyVashjInPhase2(PlayerbotAI* botAI)
    {
        Unit* vashj =
            botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetReactState() == REACT_PASSIVE;
    }

    bool IsLadyVashjInPhase3(PlayerbotAI* botAI)
    {
        Unit* vashj =
            botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        Creature* vashjCreature = vashj->ToCreature();
        return vashjCreature && vashjCreature->GetHealthPct() <= 50.0f &&
               vashjCreature->GetReactState() != REACT_PASSIVE;
    }

    bool IsValidLadyVashjCombatNpc(Unit* unit, PlayerbotAI* botAI)
    {
        if (!unit || !unit->IsAlive())
            return false;

        uint32 entry = unit->GetEntry();

        if (IsLadyVashjInPhase2(botAI))
        {
            return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
                   entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER;
        }
        else if (IsLadyVashjInPhase3(botAI))
        {
            return entry == NPC_TAINTED_ELEMENTAL || entry == NPC_ENCHANTED_ELEMENTAL ||
                   entry == NPC_COILFANG_ELITE || entry == NPC_COILFANG_STRIDER ||
                   entry == NPC_TOXIC_SPOREBAT || entry == NPC_LADY_VASHJ;
        }

        return false;
    }

    bool AnyRecentCoreInInventory(Group* group, PlayerbotAI* botAI, uint32 graceSeconds)
    {
        Unit* vashj =
            botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "lady vashj")->Get();
        if (!vashj)
            return false;

        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->HasItemCount(ITEM_TAINTED_CORE, 1, false))
                    return true;
            }
        }

        const uint32 instanceId = vashj->GetMap()->GetInstanceId();
        const time_t now = std::time(nullptr);

        auto it = lastCoreInInventoryTime.find(instanceId);
        if (it != lastCoreInInventoryTime.end())
        {
            if ((now - it->second) <= static_cast<time_t>(graceSeconds))
                return true;
        }

        return false;
    }

    Player* GetDesignatedCoreLooter(Group* group, PlayerbotAI* botAI)
    {
        if (!group)
            return nullptr;

        Player* leader = nullptr;
        ObjectGuid leaderGuid = group->GetLeaderGUID();
        if (!leaderGuid.IsEmpty())
            leader = ObjectAccessor::FindPlayer(leaderGuid);

        if (!botAI->HasCheat(BotCheatMask::raid))
            return leader;

        Player* fallback = leader;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == leader)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsMelee(member) && memberAI->IsDps(member))
                return member;

            if (!fallback && memberAI->IsRangedDps(member))
                fallback = member;
        }

        return fallback ? fallback : leader;
    }

    Player* GetFirstTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        if (!group)
            return nullptr;

        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsAssistHealOfIndex(member, 0, true))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                botAI->IsTank(member) || member == designatedLooter)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetSecondTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        if (!group)
            return nullptr;

        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsAssistHealOfIndex(member, 1, true))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                botAI->IsTank(member) || member == designatedLooter ||
                member == firstCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetThirdTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        if (!group)
            return nullptr;

        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
        Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsAssistHealOfIndex(member, 2, true))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                botAI->IsTank(member) || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    Player* GetFourthTaintedCorePasser(Group* group, PlayerbotAI* botAI)
    {
        if (!group)
            return nullptr;

        Player* designatedLooter = GetDesignatedCoreLooter(group, botAI);
        Player* firstCorePasser = GetFirstTaintedCorePasser(group, botAI);
        Player* secondCorePasser = GetSecondTaintedCorePasser(group, botAI);
        Player* thirdCorePasser = GetThirdTaintedCorePasser(group, botAI);

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser ||
                member == thirdCorePasser)
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI)
                continue;

            if (memberAI->IsAssistRangedDpsOfIndex(member, 0, true))
                return member;
        }

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                botAI->IsTank(member) || member == designatedLooter ||
                member == firstCorePasser || member == secondCorePasser ||
                member == thirdCorePasser)
                continue;
            return member;
        }

        return nullptr;
    }

    const std::vector<uint32> SHIELD_GENERATOR_DB_GUIDS =
    {
        47482, // NW
        47483, // NE
        47484, // SE
        47485  // SW
    };

    // Get the positions of all active Shield Generators by their database GUIDs
    std::vector<GeneratorInfo> GetAllGeneratorInfosByDbGuids(
        Map* map, const std::vector<uint32>& generatorDbGuids)
    {
        std::vector<GeneratorInfo> generators;
        if (!map)
            return generators;

        for (uint32 dbGuid : generatorDbGuids)
        {
            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
            if (bounds.first == bounds.second)
                continue;

            GameObject* go = bounds.first->second;
            if (!go)
                continue;

            if (go->GetGoState() != GO_STATE_READY)
                continue;

            GeneratorInfo info;
            info.guid = go->GetGUID();
            info.x = go->GetPositionX();
            info.y = go->GetPositionY();
            info.z = go->GetPositionZ();
            generators.push_back(info);
        }

        return generators;
    }

    // Returns the nearest active Shield Generator to the bot
    // Active generators are powered by NPC_WORLD_INVISIBLE_TRIGGER creatures,
    // which depawn after use
    Unit* GetNearestActiveShieldGeneratorTriggerByEntry(Unit* reference)
    {
        if (!reference)
            return nullptr;

        std::list<Creature*> triggers;
        constexpr float searchRange = 150.0f;
        reference->GetCreatureListWithEntryInGrid(
            triggers, NPC_WORLD_INVISIBLE_TRIGGER, searchRange);

        Creature* nearest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (Creature* creature : triggers)
        {
            if (!creature->IsAlive())
                continue;

            float dist = reference->GetDistance(creature);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = creature;
            }
        }

        return nearest;
    }

    const GeneratorInfo* GetNearestGeneratorToBot(
        Player* bot, const std::vector<GeneratorInfo>& generators)
    {
        if (!bot || generators.empty())
            return nullptr;

        const GeneratorInfo* nearest = nullptr;
        float minDist = std::numeric_limits<float>::max();

        for (auto const& gen : generators)
        {
            float dist = bot->GetExactDist(gen.x, gen.y, gen.z);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = &gen;
            }
        }

        return nearest;
    }
}
