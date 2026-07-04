#ifndef _PLAYERBOT_RAIDAQ40SPELLIDS_H
#define _PLAYERBOT_RAIDAQ40SPELLIDS_H

#include <initializer_list>

#include "PlayerbotAI.h"

namespace Aq40SpellIds
{
// Sourced from AzerothCore Temple of Ahn'Qiraj scripts.
static constexpr uint32 SkeramArcaneExplosion = 26192;
static constexpr uint32 SkeramTrueFulfillment = 785;
static constexpr uint32 BugTrioToxicVolley = 25812;
static constexpr uint32 BugTrioPoisonCloud = 26590;
static constexpr uint32 BugTrioYaujHeal = 25807;
static constexpr uint32 BugTrioYaujFear = 26580;
static constexpr uint32 SarturaWhirlwind = 26083;
static constexpr uint32 SarturaGuardWhirlwind = 26038;
static constexpr uint32 Aq40DefenderMeteor = 26558;
static constexpr uint32 Aq40DefenderPlague = 26556;
static constexpr uint32 Aq40DefenderShadowStorm = 26555;
static constexpr uint32 Aq40DefenderThunderclap = 26554;
static constexpr uint32 FankrissMortalWound = 25646;
static constexpr uint32 FankrissEntangle = 720;
static constexpr uint32 HuhuranFrenzy = 26051;
static constexpr uint32 OuroSweep = 26103;
static constexpr uint32 OuroSandBlast = 26102;
static constexpr uint32 OuroGroundRupture = 26100;
static constexpr uint32 OuroBerserk = 26615;
static constexpr uint32 CthunDarkGlare = 26029;
static constexpr uint32 CthunMindFlay = 26143;
static constexpr uint32 CthunDigestiveAcid = 26476;
static constexpr uint32 ViscidusPoisonShock = 25993;
static constexpr uint32 ViscidusPoisonboltVolley = 25991;
static constexpr uint32 ViscidusSlowed = 26034;
static constexpr uint32 ViscidusSlowedMore = 26036;
static constexpr uint32 ViscidusFreeze = 25937;

// Twin Emperors identifiers are sourced from AzerothCore Temple of Ahn'Qiraj
// boss_twinemperors.cpp and temple_of_ahnqiraj.h.
// AzerothCore emits Twin Teleport as a paired 799/800 sequence.
static constexpr uint32 TwinTeleportPrimary = 799;
static constexpr uint32 TwinTeleportSecondary = 800;
static constexpr uint32 TwinShadowBolt = 26006;
static constexpr uint32 TwinBlizzard = 26607;
static constexpr uint32 TwinArcaneBurst = 568;
static constexpr uint32 TwinHealBrother = 7393;
static constexpr uint32 TwinExplodeBug = 804;
static constexpr uint32 TwinMutateBug = 802;
static constexpr uint32 TwinVirulentPoisonProc = 22413;
static constexpr uint32 TwinUppercut = 26007;
static constexpr uint32 TwinUnbalancingStrike = 26613;

static constexpr uint32 TwinVeklorNpcEntry = 15276;
static constexpr uint32 TwinVeknilashNpcEntry = 15275;
static constexpr uint32 TwinQirajiScarabNpcEntry = 15316;
static constexpr uint32 TwinQirajiScorpionNpcEntry = 15317;
static constexpr uint32 TwinEntranceDoorGameObjectEntry = 180634;
static constexpr uint32 TwinExitDoorGameObjectEntry = 180635;

inline bool IsTwinTeleportSpellId(uint32 spellId)
{
    return spellId == TwinTeleportPrimary || spellId == TwinTeleportSecondary;
}

inline bool IsTwinEncounterSpellId(uint32 spellId)
{
    return IsTwinTeleportSpellId(spellId) || spellId == TwinShadowBolt || spellId == TwinBlizzard ||
           spellId == TwinArcaneBurst || spellId == TwinHealBrother || spellId == TwinExplodeBug ||
           spellId == TwinMutateBug || spellId == TwinUppercut || spellId == TwinUnbalancingStrike;
}

inline bool IsTwinEmperorEntry(uint32 entry)
{
    return entry == TwinVeklorNpcEntry || entry == TwinVeknilashNpcEntry;
}

inline bool IsTwinBugEntry(uint32 entry)
{
    return entry == TwinQirajiScarabNpcEntry || entry == TwinQirajiScorpionNpcEntry;
}

inline bool IsTwinEncounterNpcEntry(uint32 entry)
{
    return IsTwinEmperorEntry(entry) || IsTwinBugEntry(entry);
}

inline bool IsTwinRoomDoorEntry(uint32 entry)
{
    return entry == TwinEntranceDoorGameObjectEntry || entry == TwinExitDoorGameObjectEntry;
}

inline bool HasAnyAura(PlayerbotAI* botAI, Unit* unit, std::initializer_list<uint32> spellIds)
{
    if (!botAI || !unit)
        return false;

    for (uint32 spellId : spellIds)
    {
        if (unit->HasAura(spellId))
            return true;
    }

    return false;
}

inline Aura* GetAnyAura(Unit* unit, std::initializer_list<uint32> spellIds)
{
    if (!unit)
        return nullptr;

    for (uint32 spellId : spellIds)
    {
        if (Aura* aura = unit->GetAura(spellId))
            return aura;
    }

    return nullptr;
}

inline bool MatchesAnySpellId(SpellInfo const* info, std::initializer_list<uint32> spellIds)
{
    if (!info)
        return false;

    for (uint32 spellId : spellIds)
    {
        if (info->Id == spellId)
            return true;
    }

    return false;
}

inline bool IsTwinTeleportSpell(SpellInfo const* info)
{
    return info && IsTwinTeleportSpellId(info->Id);
}

inline bool IsTwinEncounterSpell(SpellInfo const* info)
{
    return info && IsTwinEncounterSpellId(info->Id);
}
}    // namespace Aq40SpellIds

#endif
