#ifndef _PLAYERBOT_ICCSCRIPTS_H
#define _PLAYERBOT_ICCSCRIPTS_H

#include <map>
#include <unordered_map>
#include <vector>
#include "ObjectGuid.h"
#include "Position.h"

namespace IcecrownHelpers
{
    // Putricide - Malleable Goo
    // Each entry records the impact position (target's location at cast time)
    // and the ms timestamp of the cast. IccPutricideAvoidMalleableGooAction
    // reads this list on every tick and makes every bot flee any active
    // impact points, since the core casts the spell triggered (no cast bar)
    // and it is neither a DynamicObject, trap, nor trigger NPC.
    struct MalleableGooImpact
    {
        Position position;
        uint32 castTime;
    };
    extern std::unordered_map<uint32, std::vector<MalleableGooImpact>> malleableGooImpacts;

    // Festergut avoid-malleable-goo wait state. When a bot dodges goo we stamp
    // a wait-until timestamp here so the trigger stays active and movement is
    // held for the full 8s impact window - otherwise the group-position action
    // pulls the bot back the very next tick, producing jitter.
    extern std::map<ObjectGuid, uint32> festergutGooWaitUntil;

    // Lich King - Defile (SPELL_DEFILE = 72762). Stamped at OnSpellCast time
    // because the boss script casts via CastSpell(target, ...) and reading the
    // target later via current-spell APIs is unreliable. Readers treat entries
    // older than ~3s as expired (cast time is 2s).
    struct DefileCastInfo
    {
        ObjectGuid targetGuid;
        uint32 castTime;
    };
    extern std::unordered_map<uint32, DefileCastInfo> defileCast;

    // Rotface - Vile Gas. Stamped at OnSpellCast time so the targeted bot can
    // react before the aura applies. Readers treat entries older than ~5s as
    // expired (covers the dodge window plus the 3s post-arrival hold).
    struct VileGasVictim
    {
        ObjectGuid victimGuid;
        uint32 castTime;
    };
    extern std::unordered_map<uint32, VileGasVictim> rotfaceVileGas;

    // Rotface vile gas hold-at-safe-spot state. When the victim bot reaches
    // its safe spot we stamp now+3000ms so the multiplier blocks any other
    // movement action that would yank the bot back into the raid stack.
    extern std::map<ObjectGuid, uint32> rotfaceVileGasWaitUntil;
}

// Putricide - Mutated Abomination vehicle
constexpr uint32 GO_PUTRICIDE_DRINK_ME = 201584;
constexpr uint32 NPC_MUTATED_ABOMINATION_10 = 37672;
constexpr uint32 NPC_MUTATED_ABOMINATION_25 = 38285;
constexpr uint32 SPELL_MUTATED_TRANSFORMATION = 70311;
constexpr uint32 SPELL_ABO_EAT_OOZE = 70346;
constexpr uint32 SPELL_ABO_REGURGITATED_OOZE = 70539;

void AddSC_IcecrownBotScripts();

#endif
