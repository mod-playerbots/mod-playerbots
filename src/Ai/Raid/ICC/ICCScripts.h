/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_ICCSCRIPTS_H
#define PLAYERBOTS_ICCSCRIPTS_H

#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include "ObjectGuid.h"
#include "Position.h"

namespace IcecrownHelpers
{
    struct MalleableGooImpact
    {
        Position position;
        uint32 castTime;
    };

    struct DefileCastInfo
    {
        ObjectGuid targetGuid;
        uint32 castTime = 0;
    };

    struct VileGasVictim
    {
        ObjectGuid victimGuid;
        uint32 castTime = 0;
    };

    struct WinterStageState
    {
        uint32 startMs;
        Position const* pos;
    };

    struct StackChoice
    {
        uint32 evaluatedMs;
        int slotIdx;
    };

    struct LastLosMove
    {
        uint32 timestampMs = 0;
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct ValithriaCloudSync
    {
        ObjectGuid targetCloudGuid;
        uint32 moveOnAfterMs = 0;
    };

    struct SpreadSlotState
    {
        uint32 lastCallMs = 0;
        int currentSlot = 1;
    };

    struct BloatDir
    {
        float x;
        float y;
    };

    struct IccInstanceState
    {
        std::vector<MalleableGooImpact> malleableGoo;
        std::map<ObjectGuid, uint32> festergutGooWaitUntil;
        DefileCastInfo defileCast;
        VileGasVictim rotfaceVileGas;
        std::map<ObjectGuid, uint32> rotfaceVileGasWaitUntil;
        std::map<ObjectGuid, uint32> lastTauntMs;
        std::map<ObjectGuid, uint32> lastCcMs;
        std::map<ObjectGuid, uint32> lastBuffMs;
        uint32 lastBossCombatMs = 0;

        std::map<ObjectGuid, WinterStageState> winterStage;
        std::map<ObjectGuid, bool> winterStageInbound;
        std::map<ObjectGuid, bool> mtAddInbound;
        std::map<ObjectGuid, bool> rangedInbound;
        std::map<ObjectGuid, StackChoice> mtStackChoice;
        float defileLastUnsafeX = 0.0f;
        float defileLastUnsafeY = 0.0f;
        uint32 defileLastUnsafeTime = 0;
        uint32 lkLastYellMs = 0;
        int lkSharedSlot = -1;

        std::set<int> sgFlaredRedThisPhase;
        bool sgFlaredBluePhase3 = false;
        bool sgLastPhase3 = false;
        std::map<ObjectGuid, int> sgGroupAssignments;
        std::map<ObjectGuid, ObjectGuid> sgTombAssignments;
        std::set<ObjectGuid> sgFreedFallback;
        std::map<ObjectGuid, LastLosMove> sgLastLosMove;

        std::map<ObjectGuid, int> bqlBotCurrentCurve;
        bool bqlGroundPhaseEstablished = false;
        bool bqlBossWasAirborne = false;
        bool bqlPostAirLanding = false;
        std::map<ObjectGuid, int> bqlAirSlotMemory;
        std::map<ObjectGuid, int> bqlBotSlotMemory;

        ValithriaCloudSync vtCloudSync;
        std::map<ObjectGuid, ObjectGuid> vtPortalClaim;

        Position gsbLastStarPos;
        std::map<ObjectGuid, uint32> gsbLastRocketPackUse;

        std::map<ObjectGuid, int> dbsBotSlotMemory;

        std::map<ObjectGuid, uint32> bpcSpreadLockTimers;

        SpreadSlotState fgSlotState;

        std::map<ObjectGuid, int32> rfExplosionSlotMemory;

        std::map<ObjectGuid, BloatDir> ppBloatLastDir;
    };

    IccInstanceState& IccState(uint32 instanceId);
    void IccResetInstance(uint32 instanceId);

    std::vector<Position> ActiveGooPositions(uint32 instanceId, uint32 lifetimeMs);
}

constexpr uint32 ICC_MAP_ID = 631;
constexpr uint32 ICC_RESET_GRACE_MS = 5000;

// Putricide - Mutated Abomination vehicle
constexpr uint32 GO_PUTRICIDE_DRINK_ME = 201584;
constexpr uint32 NPC_MUTATED_ABOMINATION_10 = 37672;
constexpr uint32 NPC_MUTATED_ABOMINATION_25 = 38285;
constexpr uint32 SPELL_MUTATED_TRANSFORMATION = 70311;
constexpr uint32 SPELL_ABO_EAT_OOZE = 70346;
constexpr uint32 SPELL_ABO_REGURGITATED_OOZE = 70539;

void AddSC_IcecrownBotScripts();

#endif
