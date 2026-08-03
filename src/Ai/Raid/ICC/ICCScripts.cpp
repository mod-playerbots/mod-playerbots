/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ICCScripts.h"
#include "Creature.h"
#include "Player.h"
#include "ICCTriggers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Timer.h"
#include <algorithm>
#include <mutex>

namespace
{
    std::unordered_map<uint32, IcecrownHelpers::IccInstanceState> g_state;
    std::mutex g_stateMutex;
}

namespace IcecrownHelpers
{
    IccInstanceState& IccState(uint32 instanceId)
    {
        std::lock_guard lock(g_stateMutex);
        return g_state[instanceId];
    }

    void IccResetInstance(uint32 instanceId)
    {
        std::lock_guard lock(g_stateMutex);
        g_state.erase(instanceId);
    }

    std::vector<Position> ActiveGooPositions(uint32 instanceId, uint32 lifetimeMs)
    {
        std::vector<Position> out;
        IccInstanceState& st = IccState(instanceId);

        uint32 const now = getMSTime();
        for (auto const& impact : st.malleableGoo)
            if (getMSTimeDiff(impact.castTime, now) <= lifetimeMs)
                out.push_back(impact.position);

        return out;
    }
}

class IccPutricideListenerScript : public AllSpellScript
{
public:
    IccPutricideListenerScript() : AllSpellScript("IccPutricideListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != SPELL_MALLEABLE_GOO_10N &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_25N &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_10H &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_25H &&
            spellInfo->Id != SPELL_MALLEABLE_GOO_BALCONY)
            return;

        // Malleable Goo is cast triggered, so m_UniqueTargetInfo is not yet
        // populated at this point; read the explicit unit target directly.
        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        uint32 now = getMSTime();

        IcecrownHelpers::MalleableGooImpact impact;
        impact.position = target->GetPosition();
        impact.castTime = now;

        auto& impacts = IcecrownHelpers::IccState(caster->GetMap()->GetInstanceId()).malleableGoo;
        impacts.push_back(impact);

        // Evict stale entries to keep the list bounded. Retention covers the
        // longest consumer window (Festergut avoid: 8s) + slack.
        impacts.erase(
            std::remove_if(impacts.begin(), impacts.end(),
                           [now](IcecrownHelpers::MalleableGooImpact const& i)
                           { return getMSTimeDiff(i.castTime, now) > 9000; }),
            impacts.end());
    }
};

class IccRotfaceListenerScript : public AllSpellScript
{
public:
    IccRotfaceListenerScript() : AllSpellScript("IccRotfaceListenerScript") { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != SPELL_VILE_GAS_H)
            return;

        // Professor Putricide casts vile gas from the balcony during Rotface
        // heroic. Filtering on caster entry keeps this hook scoped to the
        // Rotface encounter only (Festergut also uses 'vile gas' as the gas
        // spore aura name but a different spell ID).
        if (caster->GetEntry() != NPC_PROFESSOR_PUTRICIDE)
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        IcecrownHelpers::VileGasVictim& entry =
            IcecrownHelpers::IccState(caster->GetMap()->GetInstanceId()).rotfaceVileGas;
        entry.victimGuid = target->GetGUID();
        entry.castTime = getMSTime();
    }
};

class IccLichKingListenerScript : public AllSpellScript
{
public:
    IccLichKingListenerScript() : AllSpellScript("IccLichKingListenerScript") { }

    // OnSpellPrepare fires at cast START (Spell::prepare). OnSpellCast fires
    // at cast END, which for Defile (2s cast time) is too late - the puddle
    // is already spawning and bots have no time to move out.
    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || !spellInfo)
            return;

        if (spellInfo->Id != DEFILE_CAST_ID)
            return;

        Unit* target = spell->m_targets.GetUnitTarget();
        if (!target || !target->IsPlayer())
            return;

        IcecrownHelpers::DefileCastInfo& entry =
            IcecrownHelpers::IccState(caster->GetMap()->GetInstanceId()).defileCast;
        entry.targetGuid = target->GetGUID();
        entry.castTime = getMSTime();
    }
};

class IccBossStateResetScript : public AllCreatureScript
{
public:
    IccBossStateResetScript() : AllCreatureScript("IccBossStateResetScript") { }

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature || creature->GetMapId() != ICC_MAP_ID || !creature->IsDungeonBoss())
            return;

        uint32 const instanceId = creature->GetInstanceId();
        uint32 const now = getMSTime();
        IcecrownHelpers::IccInstanceState& st = IcecrownHelpers::IccState(instanceId);

        if (creature->IsInCombat())
        {
            st.lastBossCombatMs = now;
            return;
        }

        if (st.lastBossCombatMs != 0 && getMSTimeDiff(st.lastBossCombatMs, now) > ICC_RESET_GRACE_MS)
            IcecrownHelpers::IccResetInstance(instanceId);
    }
};

class IccMapCleanupScript : public AllMapScript
{
public:
    IccMapCleanupScript() : AllMapScript("IccMapCleanupScript") { }

    void OnDestroyMap(Map* map) override
    {
        if (map->GetId() == ICC_MAP_ID)
            IcecrownHelpers::IccResetInstance(map->GetInstanceId());
    }
};

void AddSC_IcecrownBotScripts()
{
    new IccPutricideListenerScript();
    new IccRotfaceListenerScript();
    new IccLichKingListenerScript();
    new IccBossStateResetScript();
    new IccMapCleanupScript();
}
