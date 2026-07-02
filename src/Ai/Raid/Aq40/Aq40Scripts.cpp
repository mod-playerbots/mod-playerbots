#include "Aq40Scripts.h"

#include <mutex>
#include <unordered_map>

#include "Map.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "Aq40BossHelper.h"
#include "Aq40SpellIds.h"

namespace Aq40Scripts
{
namespace
{
struct TwinScriptState
{
    uint32 lastTeleportAtMs = 0;
    uint32 teleportSequence = 0;
    uint32 lastBlizzardAtMs = 0;
    uint32 lastArcaneBurstAtMs = 0;
    uint32 lastExplodeBugAtMs = 0;
    ObjectGuid explodeBugSourceGuid = ObjectGuid::Empty;
    Position explodeBugSourcePosition;
};

std::mutex sStateMutex;
std::unordered_map<uint32, TwinScriptState> sTwinStateByInstance;
uint32 constexpr kTwinTeleportDedupeMs = 5000;

uint32 ResolveNow(uint32 nowMs)
{
    return nowMs ? nowMs : getMSTime();
}

uint32 GetInstanceId(Player const* player)
{
    if (!player)
        return 0;

    return player->GetMap() ? player->GetMap()->GetInstanceId() : player->GetMapId();
}

uint32 GetInstanceId(Unit const* unit)
{
    if (!unit)
        return 0;

    return unit->GetMap() ? unit->GetMap()->GetInstanceId() : unit->GetMapId();
}

bool IsRecent(uint32 stampedAtMs, uint32 windowMs, uint32 nowMs)
{
    return stampedAtMs && getMSTimeDiff(stampedAtMs, nowMs) <= windowMs;
}

TwinScriptState* GetState(Player const* player)
{
    uint32 const instanceId = GetInstanceId(player);
    if (!instanceId)
        return nullptr;

    auto itr = sTwinStateByInstance.find(instanceId);
    return itr == sTwinStateByInstance.end() ? nullptr : &itr->second;
}

void StampTwinSpell(Unit* caster, SpellInfo const* spellInfo, uint32 nowMs)
{
    if (!caster || !spellInfo || caster->GetMapId() != Aq40BossHelper::MAP_ID)
        return;

    if (!Aq40SpellIds::IsTwinEncounterSpellId(spellInfo->Id))
        return;

    uint32 const instanceId = GetInstanceId(caster);
    if (!instanceId)
        return;

    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState& state = sTwinStateByInstance[instanceId];

    if (Aq40SpellIds::IsTwinTeleportSpellId(spellInfo->Id))
    {
        if (!IsRecent(state.lastTeleportAtMs, kTwinTeleportDedupeMs, nowMs))
            ++state.teleportSequence;

        state.lastTeleportAtMs = nowMs;
        return;
    }

    switch (spellInfo->Id)
    {
        case Aq40SpellIds::TwinBlizzard:
            state.lastBlizzardAtMs = nowMs;
            break;
        case Aq40SpellIds::TwinArcaneBurst:
            state.lastArcaneBurstAtMs = nowMs;
            break;
        case Aq40SpellIds::TwinExplodeBug:
            state.lastExplodeBugAtMs = nowMs;
            state.explodeBugSourceGuid = caster->GetGUID();
            state.explodeBugSourcePosition = caster->GetPosition();
            break;
        default:
            break;
    }
}
}    // namespace

bool IsTwinTeleportPickupWindow(Player const* bot, uint32 windowMs, uint32 nowMs)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    return state && IsRecent(state->lastTeleportAtMs, windowMs, ResolveNow(nowMs));
}

uint32 GetTwinTeleportSequence(Player const* bot)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    return state ? state->teleportSequence : 0;
}

bool IsTwinBlizzardWindow(Player const* bot, uint32 windowMs, uint32 nowMs)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    return state && IsRecent(state->lastBlizzardAtMs, windowMs, ResolveNow(nowMs));
}

bool IsTwinArcaneBurstWindow(Player const* bot, uint32 windowMs, uint32 nowMs)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    return state && IsRecent(state->lastArcaneBurstAtMs, windowMs, ResolveNow(nowMs));
}

bool IsTwinExplodeBugWindow(Player const* bot, uint32 windowMs, uint32 nowMs)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    return state && IsRecent(state->lastExplodeBugAtMs, windowMs, ResolveNow(nowMs));
}

bool GetTwinExplodeBugSource(Player const* bot, ObjectGuid& sourceGuid, Position& sourcePosition,
                             uint32 windowMs, uint32 nowMs)
{
    sourceGuid = ObjectGuid::Empty;
    std::lock_guard<std::mutex> guard(sStateMutex);
    TwinScriptState* state = GetState(bot);
    if (!state || !IsRecent(state->lastExplodeBugAtMs, windowMs, ResolveNow(nowMs)))
        return false;

    sourceGuid = state->explodeBugSourceGuid;
    sourcePosition = state->explodeBugSourcePosition;
    return true;
}

bool HasPersistentTwinState(Player const* bot)
{
    std::lock_guard<std::mutex> guard(sStateMutex);
    return GetState(bot) != nullptr;
}

bool ResetTwinState(Player const* bot)
{
    uint32 const instanceId = GetInstanceId(bot);
    if (!instanceId)
        return false;

    std::lock_guard<std::mutex> guard(sStateMutex);
    return sTwinStateByInstance.erase(instanceId) != 0;
}

void ResetInstance(uint32 instanceId, Map* /*map*/)
{
    if (!instanceId)
        return;

    std::lock_guard<std::mutex> guard(sStateMutex);
    sTwinStateByInstance.erase(instanceId);
}
class Aq40TwinSpellListenerScript : public AllSpellScript
{
public:
    Aq40TwinSpellListenerScript() : AllSpellScript("Aq40TwinSpellListenerScript") {}

    void OnSpellCast(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        StampTwinSpell(caster, spellInfo, getMSTime());
    }

    void OnSpellPrepare(Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo) override
    {
        StampTwinSpell(caster, spellInfo, getMSTime());
    }
};
}    // namespace Aq40Scripts

void AddSC_Aq40BotScripts()
{
    new Aq40Scripts::Aq40TwinSpellListenerScript();
}
