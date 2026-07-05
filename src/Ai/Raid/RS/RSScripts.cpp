/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RSScripts.h"
#include "Player.h"
#include "RSActions.h"
#include "RSTriggers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <set>

namespace RubySanctumHelpers
{
    static std::mutex stateMutex;
    static std::unordered_map<uint32, RsInstanceState> instanceStates;

    RsInstanceState& RsState(uint32 instanceId)
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        return instanceStates[instanceId];
    }

    void ResetInstance(uint32 instanceId)
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        instanceStates.erase(instanceId);
    }
}

static uint8 RsHalionReadCorporealityIndex(Creature* creature)
{
    using namespace RubySanctumHelpers;
    for (uint8 i = 0; i <= 10; ++i)
    {
        if (creature->HasAura(HALION_CORPOREALITY_AURAS[i]))
            return i;
    }
    return 5;
}

class RsHalionRootScript : public AllCreatureScript
{
public:
    RsHalionRootScript() : AllCreatureScript("RsHalionRootScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 ) override
    {
        if (!creature || creature->GetMapId() != RS_MAP_RUBY_SANCTUM)
            return;

        if (creature->GetEntry() == NPC_METEOR_STRIKE_MARK)
        {
            HandleMeteorMark(creature);
            return;
        }

        if (creature->GetEntry() == NPC_HALION)
            HandleHalionRoot(creature);

        if (creature->GetEntry() == NPC_TWILIGHT_HALION)
            HandleTwilightHalionRoot(creature);

        if (creature->GetEntry() == NPC_ORB_CARRIER)
            HandleCutterTiming(creature);

        if ((creature->GetEntry() == NPC_LIVING_INFERNO || creature->GetEntry() == NPC_LIVING_EMBER) &&
            creature->IsAlive())
            RubySanctumHelpers::RsState(creature->GetMap()->GetInstanceId())
                .portalAddGate.lastAddAliveTime = getMSTime();
    }

private:
    void HandleCutterTiming(Creature* carrier)
    {
        uint32 const instanceId = carrier->GetMap()->GetInstanceId();
        RubySanctumHelpers::CutterTiming& state = RubySanctumHelpers::RsState(instanceId).cutterTiming;

        bool firing = false;
        Unit* tracked = nullptr;
        if (Vehicle* vehicle = carrier->GetVehicleKit())
        {
            tracked = vehicle->GetPassenger(0);
            for (int8 seat = 0; seat < 4; ++seat)
            {
                if (Unit* orb = vehicle->GetPassenger(seat); orb && orb->HasAura(SPELL_TWILIGHT_PULSE_PERIODIC))
                {
                    firing = true;
                    break;
                }
            }
        }

        if (firing && !state.active)
            state.lastShootTime = getMSTime();

        state.active = firing;

        if (tracked)
        {
            float const angle = std::atan2(tracked->GetPositionY() - carrier->GetPositionY(),
                                           tracked->GetPositionX() - carrier->GetPositionX());
            if (state.hasOrbAngle)
            {
                float const delta = RsAngleDiff(angle, state.lastOrbAngle);
                if (std::fabs(delta) > 0.0005f)
                    state.spinSign = delta > 0.0f ? 1.0f : -1.0f;
            }
            state.lastOrbAngle = angle;
            state.hasOrbAngle = true;
        }
        else
        {
            state.hasOrbAngle = false;
        }
    }

    void HandleTankAuras(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();
        uint32 const now = getMSTime();

        uint32& last = RubySanctumHelpers::RsState(instanceId).tankAuraLastApply;
        if (last != 0 && getMSTimeDiff(last, now) < 2000)
            return;
        last = now;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player) || !PlayerbotAI::IsTank(player))
                continue;
            if (!player->HasAura(RS_SPELL_PAIN_SUPPRESION))
                player->AddAura(RS_SPELL_PAIN_SUPPRESION, player);
        }
    }

    void HandleAddBuff(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();
        uint32 const now = getMSTime();

        uint32& last = RubySanctumHelpers::RsState(instanceId).addBuffLastApply;
        if (last != 0 && getMSTimeDiff(last, now) < 2000)
            return;
        last = now;

        bool addsAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player)
                continue;
            if (Creature* add = player->FindNearestCreature(NPC_LIVING_INFERNO, 200.0f); add && add->IsAlive())
            {
                addsAlive = true;
                break;
            }
            if (Creature* add = player->FindNearestCreature(NPC_LIVING_EMBER, 200.0f); add && add->IsAlive())
            {
                addsAlive = true;
                break;
            }
        }

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player))
                continue;
            bool const wantEmpowered = addsAlive && !player->HasAura(SPELL_TWILIGHT_REALM);
            if (wantEmpowered && !player->HasAura(RS_SPELL_EMPOWERED_BLOOD))
                player->AddAura(RS_SPELL_EMPOWERED_BLOOD, player);
            else if (!wantEmpowered && player->HasAura(RS_SPELL_EMPOWERED_BLOOD))
                player->RemoveAura(RS_SPELL_EMPOWERED_BLOOD);
        }
    }

    void HandleP3TankRescue(Creature* creature)
    {
        uint32 const now = getMSTime();
        uint32 const rescueMs = 5000;
        float const bossHp = creature->GetHealthPct();

        auto& grantStart = RubySanctumHelpers::RsState(creature->GetMap()->GetInstanceId()).p3RescueGrant;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();

        bool physicalHealerAlive = false;
        bool twilightHealerAlive = false;
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !PlayerbotAI::IsHeal(player))
                continue;
            if (RsHalionInTwilight(player))
                twilightHealerAlive = true;
            else
                physicalHealerAlive = true;
        }

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player) || !PlayerbotAI::IsTank(player))
                continue;

            bool const tankTwilight = RsHalionInTwilight(player);
            bool const healerInRealm = tankTwilight ? twilightHealerAlive : physicalHealerAlive;

            ObjectGuid const guid = player->GetGUID();

            if (!tankTwilight && bossHp >= 46.0f)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart.erase(guid);
                continue;
            }

            if (bossHp > 50.0f)
                continue;

            auto granted = grantStart.find(guid);

            if (granted != grantStart.end())
            {
                if (getMSTimeDiff(granted->second, now) >= rescueMs)
                {
                    player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                    grantStart.erase(granted);
                }
                else
                    player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                continue;
            }

            if (!player->HealthAbovePct(50) && healerInRealm)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
            }
        }
    }

    void HandleBreathGodMode(Creature* boss, uint32 const* breathIds, size_t breathCount, bool twilight)
    {
        uint32 const now = getMSTime();
        uint32 const holdMs = 1000;

        bool casting = false;
        for (size_t i = 0; i < breathCount; ++i)
        {
            if (boss->FindCurrentSpellBySpellId(breathIds[i]))
            {
                casting = true;
                break;
            }
        }

        RubySanctumHelpers::RsInstanceState& state = RubySanctumHelpers::RsState(boss->GetMap()->GetInstanceId());
        std::unordered_map<ObjectGuid, uint32>& grantStart =
            twilight ? state.breathTwilightGrant : state.breathPhysicalGrant;

        Map::PlayerList const& players = boss->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player) || PlayerbotAI::IsTank(player) || RsHalionInTwilight(player) != twilight)
                continue;

            ObjectGuid const guid = player->GetGUID();
            if (casting)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
                continue;
            }

            auto granted = grantStart.find(guid);
            if (granted == grantStart.end())
                continue;

            if (getMSTimeDiff(granted->second, now) >= holdMs)
            {
                player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                grantStart.erase(granted);
            }
            else
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
        }
    }

    void HandleCombustionDispel(Creature* physical)
    {
        Map::PlayerList const& players = physical->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || GET_PLAYERBOT_AI(player))
                continue;
            if (!RsHalionHasCombustion(player) || !RsHalionCombustionPlayerSafe(player))
                continue;

            player->RemoveAurasDueToSpell(SPELL_FIERY_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
            player->RemoveAurasDueToSpell(SPELL_MARK_OF_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        }
    }

    void HandleConsumptionGodMode(Creature* twilight)
    {
        if (!twilight->GetMap()->IsHeroic())
            return;

        uint32 const now = getMSTime();
        uint32 const holdMs = 2000;

        auto& grantStart = RubySanctumHelpers::RsState(twilight->GetMap()->GetInstanceId()).consumptionGrant;

        Map::PlayerList const& players = twilight->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player))
                continue;

            ObjectGuid const guid = player->GetGUID();
            if (player->HasAura(SPELL_SOUL_CONSUMPTION))
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
                continue;
            }

            auto granted = grantStart.find(guid);
            if (granted == grantStart.end())
                continue;

            if (getMSTimeDiff(granted->second, now) >= holdMs)
            {
                player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                grantStart.erase(granted);
            }
            else
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
        }
    }

    void HandleConsumptionDispel(Creature* twilight)
    {
        float const bossX = twilight->GetPositionX();
        float const bossY = twilight->GetPositionY();

        Map::PlayerList const& players = twilight->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || GET_PLAYERBOT_AI(player))
                continue;
            if (!RsHalionHasConsumption(player))
                continue;

            float const distToBoss = player->GetExactDist2d(bossX, bossY);
            if (distToBoss < RS_HALION_CONSUMPTION_OUT_DIST - RS_HALION_CONSUMPTION_OUT_REACH)
                continue;

            player->RemoveAurasDueToSpell(SPELL_SOUL_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
            player->RemoveAurasDueToSpell(SPELL_MARK_OF_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        }
    }

    void HandleTwilightHalionRoot(Creature* creature)
    {
        RubySanctumHelpers::RsInstanceState& instState =
            RubySanctumHelpers::RsState(creature->GetMap()->GetInstanceId());

        if (creature->IsAlive() && creature->IsInCombat())
        {
            RubySanctumHelpers::HalionCorporeality& corp = instState.halionCorporeality;
            corp.twilightIndex = RsHalionReadCorporealityIndex(creature);
            corp.twilightStamp = getMSTime();
            instState.bossHealth = { uint8(creature->GetHealthPct()), getMSTime() };
        }

        RubySanctumHelpers::CutterTiming& state = instState.cutterTiming;
        if (state.bossGuid != creature->GetGUID())
        {
            state = RubySanctumHelpers::CutterTiming{};
            state.bossGuid = creature->GetGUID();
        }
        if (state.encounterStart == 0 && creature->IsInCombat())
            state.encounterStart = getMSTime();

        bool twilightTankAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (player && player->IsAlive() && PlayerbotAI::IsTank(player) && RsHalionInTwilight(player))
            {
                twilightTankAlive = true;
                break;
            }
        }

        bool const shouldRoot = creature->IsAlive() && creature->IsInCombat() && twilightTankAlive;
        bool const rooted = creature->HasUnitState(UNIT_STATE_ROOT);

        if (shouldRoot && !rooted)
            creature->SetControlled(true, UNIT_STATE_ROOT);
        else if (!shouldRoot && rooted)
            creature->SetControlled(false, UNIT_STATE_ROOT);

        if (shouldRoot)
        {
            Position const& home = creature->GetHomePosition();
            if (creature->GetExactDist2d(home.GetPositionX(), home.GetPositionY()) > 3.0f)
                creature->NearTeleportTo(home.GetPositionX(), home.GetPositionY(),
                                         home.GetPositionZ(), home.GetOrientation());
        }

        if (creature->IsAlive() && creature->IsInCombat())
        {
            HandleTankAuras(creature);
            HandleAddBuff(creature);
            static uint32 const darkBreathIds[] = {
                SPELL_DARK_BREATH, SPELL_DARK_BREATH_25N, SPELL_DARK_BREATH_10H, SPELL_DARK_BREATH_25H
            };
            HandleBreathGodMode(creature, darkBreathIds, sizeof(darkBreathIds) / sizeof(darkBreathIds[0]), true);
            HandleConsumptionDispel(creature);
            HandleConsumptionGodMode(creature);
        }
    }
    void HandleMeteorMark(Creature* mark)
    {
        if (!mark->IsAlive())
            return;

        RubySanctumHelpers::RsInstanceState& instState = RubySanctumHelpers::RsState(mark->GetMap()->GetInstanceId());

        if (!instState.seenMeteorMarks.insert(mark->GetGUID()).second)
            return;

        RubySanctumHelpers::MeteorPingPong& state = instState.meteorPingPong;
        state.lastCastTime = getMSTime();

        bool const tankMeteor = mark->GetExactDist2d(RS_HALION_TANK_POSITION.GetPositionX(), RS_HALION_TANK_POSITION.GetPositionY()) <= 10.0f;
        bool const tankEscapeMeteor = mark->GetExactDist2d(RS_HALION_TANK_METEOR_SPOT.GetPositionX(), RS_HALION_TANK_METEOR_SPOT.GetPositionY()) <= 10.0f;
        if (tankMeteor)
            state.tankMeteorTime = getMSTime();
        else if (tankEscapeMeteor)
            state.tankReturnTime = getMSTime();
        else
            ++state.count;
    }

    void HandleHalionRoot(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();

        bool const fightActive = creature->IsAlive() && creature->IsInCombat();

        if (!fightActive)
        {
            if (creature->HasUnitState(UNIT_STATE_ROOT))
                creature->SetControlled(false, UNIT_STATE_ROOT);
            RubySanctumHelpers::ResetInstance(instanceId);
            return;
        }

        RubySanctumHelpers::RsInstanceState& instState = RubySanctumHelpers::RsState(instanceId);

        RubySanctumHelpers::HalionCorporeality& corp = instState.halionCorporeality;
        corp.physicalIndex = RsHalionReadCorporealityIndex(creature);
        corp.physicalStamp = getMSTime();
        corp.physicalGuid = creature->GetGUID();
        instState.bossHealth = { uint8(creature->GetHealthPct()), getMSTime() };

        HandleTankAuras(creature);
        HandleAddBuff(creature);

        static uint32 const flameBreathIds[] = {
            SPELL_FLAME_BREATH, SPELL_FLAME_BREATH_ALT1, SPELL_FLAME_BREATH_ALT2, SPELL_FLAME_BREATH_ALT3,
            SPELL_FLAME_BREATH_ALT4, SPELL_FLAME_BREATH_ALT5, SPELL_FLAME_BREATH_ALT6
        };
        HandleBreathGodMode(creature, flameBreathIds, sizeof(flameBreathIds) / sizeof(flameBreathIds[0]), false);
        HandleCombustionDispel(creature);

        bool const physicalActive = creature->GetDisplayId() == creature->GetNativeDisplayId();

        if (!physicalActive)
        {
            if (creature->HasUnitState(UNIT_STATE_ROOT))
                creature->SetControlled(false, UNIT_STATE_ROOT);
            return;
        }

        if (creature->GetMap()->IsHeroic() && !creature->HealthAbovePct(51))
            HandleP3TankRescue(creature);

        uint32 const now = getMSTime();
        uint32& last = instState.halionRootLastScan;
        if (last != 0 && getMSTimeDiff(last, now) < 1000)
            return;
        last = now;

        bool mainTankAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (player && player->IsAlive() && PlayerbotAI::IsMainTank(player))
            {
                mainTankAlive = true;
                break;
            }
        }

        bool const shouldRoot = mainTankAlive;
        bool const rooted = creature->HasUnitState(UNIT_STATE_ROOT);

        if (shouldRoot && !rooted)
            creature->SetControlled(true, UNIT_STATE_ROOT);
        else if (!shouldRoot && rooted)
            creature->SetControlled(false, UNIT_STATE_ROOT);
    }
};

void AddSC_RubySanctumBotScripts()
{
    new RsHalionRootScript();
}
