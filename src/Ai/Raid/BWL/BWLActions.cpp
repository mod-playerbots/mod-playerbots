#include "BWLActions.h"

#include "Playerbots.h"
#include "BWLHelpers.h"

using namespace BlackwingLairHelpers;

// General

bool BwlOnyxiaScaleCloakAuraCheckAction::Execute(Event /*event*/)
{
    bot->AddAura(SPELL_ONYXIA_SCALE_CLOAK, bot);
    return true;
}

bool BwlOnyxiaScaleCloakAuraCheckAction::isUseful()
{
    return !bot->HasAura(SPELL_ONYXIA_SCALE_CLOAK);
}

bool BwlTurnOffSuppressionDeviceAction::Execute(Event /*event*/)
{
    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (auto i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (IsActiveSuppressionDeviceInRange(go, bot))
        {
            go->SetGoState(GO_STATE_ACTIVE);
        }
    }
    return true;
}

// Chromaggus

bool BwlUseHourglassSandAction::Execute(Event /*event*/)
{
    return botAI->CastSpell(SPELL_HOURGLASS_SAND, bot);
}

bool BwlNefarianFearWardAction::Execute(Event /*event*/)
{
    Unit* nefarian = AI_VALUE2(Unit*, "find target", "nefarian");
    if (!nefarian)
        return false;

    Unit* victim = nefarian->GetVictim();
    if (!victim)
        return false;

    return botAI->CastSpell("fear ward", victim);
}

// Trash

static constexpr float WYRMGUARD_SAFE_DISTANCE = 16.0f;

Unit* BwlDeathTalonWyrmguardTankMoveAwayAction::GetTarget()
{
    // Find a nearby wyrmguard within 16 yards being tanked by someone else
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_DEATH_TALON_WYRMGUARD)
            continue;
        if (bot->GetDistance2d(unit) > WYRMGUARD_SAFE_DISTANCE)
            continue;
        Unit* victim = unit->GetVictim();
        if (victim && victim != bot && victim->IsPlayer() && PlayerbotAI::IsTank(victim->ToPlayer()))
            return unit;
    }
    return nullptr;
}

bool BwlDeathTalonWyrmguardTankMoveAwayAction::isUseful()
{
    if (!GetTarget())
        return false;

    // Must be actively tanking a wyrmguard before moving away from another tank's
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (unit && unit->IsAlive() && unit->GetEntry() == NPC_DEATH_TALON_WYRMGUARD && unit->GetVictim() == bot)
            return true;
    }
    return false;
}

bool BwlDeathTalonWyrmguardTankMoveAwayAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    float distToTravel = WYRMGUARD_SAFE_DISTANCE - bot->GetDistance2d(target);
    if (distToTravel <= 0.0f)
        return false;

    return MoveAway(target, distToTravel);
}

Unit* BwlDeathTalonWyrmguardRangedMoveAwayAction::GetTarget()
{
    // Find the closest wyrmguard within 16 yards
    Unit* closest = nullptr;
    float closestDist = WYRMGUARD_SAFE_DISTANCE;
    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || !unit->IsAlive() || unit->GetEntry() != NPC_DEATH_TALON_WYRMGUARD)
            continue;
        float dist = bot->GetDistance2d(unit);
        if (dist < closestDist)
        {
            closestDist = dist;
            closest = unit;
        }
    }
    return closest;
}

bool BwlDeathTalonWyrmguardRangedMoveAwayAction::Execute(Event /*event*/)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    float distToTravel = WYRMGUARD_SAFE_DISTANCE - bot->GetDistance2d(target);
    if (distToTravel <= 0.0f)
        return false;

    return MoveAway(target, distToTravel);
}
