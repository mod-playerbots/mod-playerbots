#include "Aq40Actions.h"

#include "Pet.h"
#include "RtiTargetValue.h"
#include "Spell.h"
#include "../Aq40SpellIds.h"
#include "../Util/Aq40Helpers_Shared.h"
#include "../Aq40Scripts.h"

namespace
{
float constexpr kTwinExplodeBugDangerRadius = 17.0f;
float constexpr kTwinArcaneBurstDangerRadius = 18.0f;
float constexpr kTwinArcaneBurstLooseRadius = 24.0f;
float constexpr kTwinWarlockMinRange = 19.0f;
float constexpr kTwinWarlockMaxRange = 30.0f;
float constexpr kTwinWarlockPreferredRange = 24.0f;
float constexpr kTwinMeleeContactRange = 5.0f;

void ApplyTwinBossMarkers(Player* bot, Unit* veknilash, Unit* veklor)
{
    if (!bot)
        return;

    if (veknilash)
        Aq40Helpers::SetRaidTargetIcon(bot, veknilash, RtiTargetValue::skullIndex, "twin", "skull");
    if (veklor)
        Aq40Helpers::SetRaidTargetIcon(bot, veklor, RtiTargetValue::crossIndex, "twin", "cross");
}

void ApplyTwinTargetMarker(Player* bot, PlayerbotAI* botAI, Unit* target)
{
    if (!target)
        return;

    switch (target->GetEntry())
    {
        case Aq40SpellIds::TwinVeknilashNpcEntry:
            Aq40Helpers::SetRtiTarget(botAI, "skull", target);
            break;
        case Aq40SpellIds::TwinVeklorNpcEntry:
            Aq40Helpers::SetRtiTarget(botAI, "cross", target);
            break;
        default:
            if (Aq40SpellIds::IsTwinBugEntry(target->GetEntry()))
            {
                Aq40Helpers::SetRaidTargetIcon(bot, target, RtiTargetValue::starIndex, "twin", "star");
                Aq40Helpers::SetRtiTarget(botAI, "star", target);
            }
            break;
    }
}

Unit* ResolveTwinTarget(Player* bot, PlayerbotAI* botAI, GuidVector const& encounterUnits, char const*& reason)
{
    reason = "fallback";
    if (!bot || !botAI)
        return nullptr;

    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);

    if (Unit* explodingBug = Aq40BossHelper::Twin::FindNearestBug(bot, botAI, encounterUnits, 32.0f, true))
    {
        reason = "exploding_bug";
        return explodingBug;
    }

    float const bugRange = bot->getClass() == CLASS_HUNTER ? 30.0f :
        (Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI) ? 18.0f : 26.0f);
    Unit* nearbyBug = Aq40BossHelper::Twin::FindNearestBug(bot, botAI, encounterUnits, bugRange);
    if (nearbyBug && !botAI->IsHeal(bot))
    {
        reason = "bug";
        return nearbyBug;
    }

    if (Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI) ||
        Aq40BossHelper::Twin::IsTrueCasterProfile(bot, botAI))
    {
        reason = "veklor";
        return veklor ? veklor : veknilash;
    }

    if (Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI))
    {
        reason = "veknilash";
        return veknilash ? veknilash : veklor;
    }

    return veklor ? veklor : veknilash;
}

bool CastFirstAvailable(PlayerbotAI* botAI, Unit* target, std::initializer_list<char const*> spells)
{
    if (!botAI || !target)
        return false;

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, target) && botAI->CastSpell(spell, target))
            return true;
    }

    return false;
}

bool CastFirstAvailableSelf(PlayerbotAI* botAI, Player* bot, std::initializer_list<char const*> spells)
{
    if (!botAI || !bot)
        return false;

    for (char const* spell : spells)
    {
        if (botAI->CanCastSpell(spell, bot) && botAI->CastSpell(spell, bot))
            return true;
    }

    return false;
}

void StopPetFromVeklor(Player* bot, Unit* veklor)
{
    if (!bot || !veklor)
        return;

    Pet* pet = bot->GetPet();
    if (pet && pet->GetVictim() == veklor)
        pet->AttackStop();
}

bool HasTrackedExplodeBugHazard(Player* bot, PlayerbotAI* botAI, Unit*& outBug, Position& outPosition)
{
    outBug = nullptr;
    if (!bot || !botAI)
        return false;

    ObjectGuid sourceGuid;
    if (!Aq40Scripts::GetTwinExplodeBugSource(bot, sourceGuid, outPosition))
        return false;

    if (!sourceGuid.IsEmpty())
    {
        Unit* source = botAI->GetUnit(sourceGuid);
        if (source && source->IsAlive() && source->IsInWorld() && Aq40SpellIds::IsTwinBugEntry(source->GetEntry()))
        {
            outBug = source;
            outPosition = source->GetPosition();
        }
    }

    return bot->GetExactDist2d(outPosition.GetPositionX(), outPosition.GetPositionY()) <= kTwinExplodeBugDangerRadius;
}
}    // namespace

bool Aq40TwinChooseTargetAction::Execute(Event /*event*/)
{
    if (!bot || botAI->IsHeal(bot))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    ApplyTwinBossMarkers(bot, Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits),
                         Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits));

    char const* reason = "none";
    Unit* target = ResolveTwinTarget(bot, botAI, encounterUnits, reason);
    if (!target)
        return false;

    ApplyTwinTargetMarker(bot, botAI, target);

    if (target->GetEntry() == Aq40SpellIds::TwinVeklorNpcEntry &&
        Aq40BossHelper::Twin::IsMeleeOrHunterProfile(bot, botAI) &&
        !Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI))
    {
        return false;
    }

    if (AI_VALUE(Unit*, "current target") == target && bot->GetVictim() == target)
        return false;

    Aq40Helpers::LogAq40Target(bot, "twin", reason, target, 1000);
    return Attack(target);
}

bool Aq40TwinTankAction::Execute(Event /*event*/)
{
    if (!bot || !Aq40BossHelper::IsEncounterTank(bot, bot))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veknilash = Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits);
    if (!veknilash)
        return false;

    ApplyTwinBossMarkers(bot, veknilash, Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits));
    Aq40Helpers::SetRtiTarget(botAI, "skull", veknilash);

    if (bot->GetDistance2d(veknilash) > 8.0f)
        return MoveNear(veknilash, kTwinMeleeContactRange, MovementPriority::MOVEMENT_COMBAT);

    if (veknilash->GetVictim() != bot)
        CastFirstAvailable(botAI, veknilash, { "hand of reckoning", "dark command", "growl", "taunt" });

    if (AI_VALUE(Unit*, "current target") == veknilash && bot->GetVictim() == veknilash)
        return false;

    Aq40Helpers::LogAq40Target(bot, "twin", "tank_veknilash", veknilash, 1000);
    return Attack(veknilash);
}

bool Aq40TwinWarlockTankAction::Execute(Event /*event*/)
{
    if (!Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    if (!veklor)
        return false;

    ApplyTwinBossMarkers(bot, Aq40BossHelper::Twin::FindVeknilash(botAI, encounterUnits), veklor);
    Aq40Helpers::SetRtiTarget(botAI, "cross", veklor);

    if (bot->GetTarget() != veklor->GetGUID() || AI_VALUE(Unit*, "current target") != veklor)
    {
        Aq40Helpers::LogAq40Target(bot, "twin", "warlock_veklor", veklor, 1000);
        Attack(veklor);
    }

    float const distance = bot->GetDistance2d(veklor);
    if (distance < kTwinWarlockMinRange)
        return MoveAway(veklor, kTwinWarlockPreferredRange - distance);

    if (distance > kTwinWarlockMaxRange)
        return MoveNear(veklor, kTwinWarlockPreferredRange, MovementPriority::MOVEMENT_COMBAT);

    if (!botAI->HasAura("shadow ward", bot))
        CastFirstAvailableSelf(botAI, bot, { "shadow ward" });

    if (CastFirstAvailable(botAI, veklor, { "searing pain", "shadow bolt" }))
        return true;

    return bot->GetVictim() != veklor ? Attack(veklor) : false;
}

bool Aq40TwinAvoidHazardAction::Execute(Event /*event*/)
{
    if (!bot)
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* explodeBug = Aq40BossHelper::Twin::FindNearestBug(
        bot, botAI, encounterUnits, kTwinExplodeBugDangerRadius, true);
    Position explodePosition;
    if (!explodeBug && HasTrackedExplodeBugHazard(bot, botAI, explodeBug, explodePosition))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:explode_bug:tracked",
            "boss=twin hazard=explode_bug source=tracked_script_source", 1000);
        return FleePosition(explodePosition, kTwinExplodeBugDangerRadius, 250U);
    }

    if (explodeBug)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
            "twin:explode_bug:" + Aq40Helpers::GetAq40LogUnit(explodeBug),
            "boss=twin hazard=explode_bug source=" + Aq40Helpers::GetAq40LogUnit(explodeBug), 1000);
        return FleePosition(explodeBug->GetPosition(), kTwinExplodeBugDangerRadius, 250U) ||
               MoveAway(explodeBug, kTwinExplodeBugDangerRadius - bot->GetDistance2d(explodeBug));
    }

    if (Aq40SpellIds::HasAnyAura(botAI, bot, { Aq40SpellIds::TwinBlizzard }) ||
        Aq40Scripts::IsTwinBlizzardWindow(bot))
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        if (botAI->DoSpecificAction("avoid aoe", Event(), true))
        {
            Aq40Helpers::LogAq40Info(bot, "avoid_hazard", "twin:blizzard",
                "boss=twin hazard=blizzard action=avoid_aoe", 1000);
            return true;
        }
    }

    return false;
}

bool Aq40TwinAvoidVeklorAction::Execute(Event /*event*/)
{
    if (!bot || Aq40BossHelper::Twin::IsWarlockTankProfile(bot, botAI))
        return false;

    GuidVector const encounterUnits = Aq40BossHelper::Twin::GetEncounterUnits(botAI);
    Unit* veklor = Aq40BossHelper::Twin::FindVeklor(botAI, encounterUnits);
    if (!veklor)
        return false;

    StopPetFromVeklor(bot, veklor);

    float const distance = bot->GetDistance2d(veklor);
    bool const arcaneWindow = Aq40Scripts::IsTwinArcaneBurstWindow(bot);
    float const safeRadius = arcaneWindow ? kTwinArcaneBurstLooseRadius : kTwinArcaneBurstDangerRadius;
    if (distance > safeRadius)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    Aq40Helpers::LogAq40Info(bot, "avoid_hazard",
        "twin:veklor_range:" + Aq40Helpers::GetAq40LogUnit(veklor),
        "boss=twin hazard=arcane_burst source=" + Aq40Helpers::GetAq40LogUnit(veklor), 1000);
    return MoveAway(veklor, safeRadius - distance + 2.0f);
}
