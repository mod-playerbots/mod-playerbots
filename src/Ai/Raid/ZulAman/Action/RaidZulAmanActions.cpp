/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "RaidZulAmanActions.h"
#include "RaidZulAmanHelpers.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace ZulAmanHelpers;

// Trash

bool AmanishiMedicineManMarkWardAction::Execute(Event /*event*/)
{
    if (Unit* protectiveWard = GetFirstAliveUnitByEntry(
            botAI, static_cast<uint32>(ZulAmanNPCs::NPC_AMANI_PROTECTIVE_WARD)))
    {
        MarkTargetWithSkull(bot, protectiveWard);
    }
    else if (Unit* healingWard = GetFirstAliveUnitByEntry(
                botAI, static_cast<uint32>(ZulAmanNPCs::NPC_AMANI_HEALING_WARD)))
    {
        MarkTargetWithSkull(bot, healingWard);
    }

    return false;
}

// Akil'zon <Eagle Avatar>

bool AkilzonMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", akilzon))
        return botAI->CastSpell("steady shot", akilzon);

    return false;
}

bool AkilzonTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (!akilzon)
        return false;

    if (bot->GetVictim() != akilzon)
        return Attack(akilzon);

    if (akilzon->GetVictim() == bot)
    {
        const Position& position = AKILZON_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool AkilzonSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 13.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}

bool AkilzonMoveToEyeOfTheStormAction::Execute(Event /*event*/)
{
    Player* target = GetElectricalStormTarget(bot);
    if (!target && !botAI->IsMainTank(bot))
        target = GetGroupMainTank(botAI, bot);

    if (target && bot->GetExactDist2d(target) > 2.0f)
    {
        botAI->Reset();
        return MoveTo(ZULAMAN_MAP_ID, target->GetPositionX(), target->GetPositionY(),
                      bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool AkilzonManageElectricalStormTimerAction::Execute(Event /*event*/)
{
    const time_t now = std::time(nullptr);
    const uint32 instanceId = bot->GetMap()->GetInstanceId();

    Unit* akilzon = AI_VALUE2(Unit*, "find target", "akil'zon");
    if (akilzon)
    {
        auto [it, inserted] = akilzonStormTimer.try_emplace(instanceId, now);
        return inserted;
    }
    else if (!bot->IsInCombat() && !akilzon && akilzonStormTimer.erase(instanceId) > 0)
    {
        return true;
    }

    return false;
}

// Nalorakk <Bear Avatar>

bool NalorakkMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", nalorakk))
        return botAI->CastSpell("steady shot", nalorakk);

    return false;
}

bool NalorakkTanksPositionBossAction::Execute(Event /*event*/)
{
    if (!botAI->IsMainTank(bot) && !botAI->IsAssistTankOfIndex(bot, 0, true))
        return false;

    Unit* nalorakk = AI_VALUE2(Unit*, "find target", "nalorakk");
    if (!nalorakk)
        return false;

    if (botAI->IsMainTank(bot))
        return MainTankPositionTrollForm(nalorakk);
    else
        return FirstAssistTankPositionBearForm(nalorakk);
}

bool NalorakkTanksPositionBossAction::MainTankPositionTrollForm(Unit* nalorakk)
{
    if (!nalorakk->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_BEARFORM)))
    {
        if (bot->GetVictim() != nalorakk)
            return Attack(nalorakk);

        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), true);
    }

    const Position& position = NALORAKK_TANK_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool NalorakkTanksPositionBossAction::FirstAssistTankPositionBearForm(Unit* nalorakk)
{
    if (nalorakk->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_BEARFORM)))
    {
        if (bot->GetVictim() != nalorakk)
            return Attack(nalorakk);

        if (nalorakk->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), true);
    }

    const Position& position = NALORAKK_TANK_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                        false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool NalorakkSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 11.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}

// Jan'alai <Dragonhawk Avatar>

bool JanalaiMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", janalai))
        return botAI->CastSpell("steady shot", janalai);

    return false;
}

bool JanalaiTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* janalai = AI_VALUE2(Unit*, "find target", "jan'alai");
    if (!janalai)
        return false;

    if (bot->GetVictim() != janalai)
        return Attack(janalai);

    if (janalai->GetVictim() == bot)
    {
        const Position& position = JANALAI_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool JanalaiSpreadRangedInCircleAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> rangedMembers;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !botAI->IsRanged(member))
            continue;

        rangedMembers.push_back(member);
    }

    if (rangedMembers.empty())
        return false;

    auto findIt = std::find(rangedMembers.begin(), rangedMembers.end(), bot);
    size_t botIndex =
        (findIt != rangedMembers.end()) ? std::distance(rangedMembers.begin(), findIt) : 0;
    size_t count = rangedMembers.size();
    if (count == 0)
        return false;

    constexpr float radius = 15.0f;
    float angle = (count == 1) ? 0.0f :
            (2.0f * M_PI * static_cast<float>(botIndex) / static_cast<float>(count));

    float targetX = JANALAI_TANK_POSITION.GetPositionX() + radius * std::cos(angle);
    float targetY = JANALAI_TANK_POSITION.GetPositionY() + radius * std::sin(angle);

    if (bot->GetExactDist2d(targetX, targetY) > 2.0f)
    {
        return MoveTo(ZULAMAN_MAP_ID, targetX, targetY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool JanalaiAvoidFireBombsAction::Execute(Event /*event*/)
{
    auto const& bombs = GetAllHazardTriggers(
        bot, static_cast<uint32>(ZulAmanNPCs::NPC_FIRE_BOMB), 50.0f);

    if (bombs.empty())
        return false;

    constexpr float hazardRadius = 5.0f;
    bool inDanger = false;
    for (Unit* bomb : bombs)
    {
        if (bot->GetDistance2d(bomb) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& janalaiCenter = JANALAI_TANK_POSITION;
    constexpr float safeZoneRadius = 17.0f;

    Position safestPos =
        FindSafestNearbyPosition(bot, bombs, janalaiCenter, safeZoneRadius, hazardRadius, false);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(ZULAMAN_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool JanalaiMarkAmanishiHatchersAction::Execute(Event /*event*/)
{
    auto [hatcherLow, hatcherHigh] = GetAmanishiHatcherPair(botAI);

    if (hatcherLow && hatcherHigh && hatcherHigh != hatcherLow)
    {
        MarkTargetWithSkull(bot, hatcherLow);
        MarkTargetWithMoon(bot, hatcherHigh);
        SetRtiTarget(botAI, "skull", hatcherLow);
    }

    return false;
}

// Halazzi <Lynx Avatar>

bool HalazziMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", halazzi))
        return botAI->CastSpell("steady shot", halazzi);

    return false;
}

bool HalazziMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi");
    if (!halazzi)
        return false;

    MarkTargetWithStar(bot, halazzi);
    SetRtiTarget(botAI, "star", halazzi);

    if (bot->GetVictim() != halazzi)
        return Attack(halazzi);

    if (halazzi->GetVictim() == bot)
    {
        const Position& position = HALAZZI_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

bool HalazziFirstAssistTankAttackSpiritLynxAction::Execute(Event /*event*/)
{
    bool targetFound = false;

    if (Unit* lynx = AI_VALUE2(Unit*, "find target", "spirit of the lynx"))
    {
        MarkTargetWithCircle(bot, lynx);
        SetRtiTarget(botAI, "circle", lynx);

        if (bot->GetVictim() != lynx)
            return Attack(lynx);

        if (lynx->GetVictim() != bot)
            return botAI->DoSpecificAction("taunt spell", Event(), true);

        targetFound = true;
    }
    else if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetVictim() != halazzi)
            return Attack(halazzi);

        targetFound = true;
    }

    if (!targetFound)
        return false;

    const Position& position = HALAZZI_TANK_POSITION;
    float distToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distToPosition > 2.0f)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveDist = std::min(10.0f, distToPosition);
        float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
        float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

        return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                      false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool HalazziAssignDpsPriorityAction::Execute(Event /*event*/)
{
    // Target priority 1: Corrupted Lightning Totems
    if (Unit* totem = GetFirstAliveUnitByEntry(
            botAI, static_cast<uint32>(ZulAmanNPCs::NPC_CORRUPTED_LIGHTNING_TOTEM)))
    {
        MarkTargetWithSkull(bot, totem);
        SetRtiTarget(botAI, "skull", totem);

        if (bot->GetTarget() != totem->GetGUID())
            return Attack(totem);

        return false;
    }

    // Target priority 2: Halazzi
    if (Unit* halazzi = AI_VALUE2(Unit*, "find target", "halazzi"))
    {
        SetRtiTarget(botAI, "star", halazzi);

        if (bot->GetTarget() != halazzi->GetGUID())
            return Attack(halazzi);
    }

    // Don't attack the Lynx
    return false;
}

// Hex Lord Malacrass

bool HexLordMalacrassMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", malacrass))
        return botAI->CastSpell("steady shot", malacrass);

    return false;
}

bool HexLordMalacrassAssignDpsPriorityAction::Execute(Event /*event*/)
{
    static constexpr uint32 priorityEntries[] =
    {
        static_cast<uint32>(ZulAmanNPCs::NPC_LORD_RAADAN),
        static_cast<uint32>(ZulAmanNPCs::NPC_ALYSON_ANTILLE),
        static_cast<uint32>(ZulAmanNPCs::NPC_KORAGG),
        static_cast<uint32>(ZulAmanNPCs::NPC_DARKHEART),
        static_cast<uint32>(ZulAmanNPCs::NPC_FENSTALKER),
        static_cast<uint32>(ZulAmanNPCs::NPC_GAZAKROTH),
        static_cast<uint32>(ZulAmanNPCs::NPC_THURG),
        static_cast<uint32>(ZulAmanNPCs::NPC_SLITHER),
        static_cast<uint32>(ZulAmanNPCs::NPC_HEX_LORD_MALACRASS)
    };

    auto const& targets =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get();

    Unit* priorityTarget = nullptr;

    for (uint32 entry : priorityEntries)
    {
        for (auto const& guid : targets)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == entry)
            {
                priorityTarget = unit;
                break;
            }
        }

        if (priorityTarget)
            break;
    }

    if (priorityTarget)
    {
        MarkTargetWithSkull(bot, priorityTarget);
        SetRtiTarget(botAI, "skull", priorityTarget);
    }

    return false;
}

bool HexLordMalacrassRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    if (Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass"))
    {
        float currentDistance = bot->GetDistance2d(malacrass);
        constexpr float safeDistance = 9.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(malacrass, safeDistance - currentDistance);
        }
    }

    return false;
}

bool HexLordMalacrassCastersStopAttackingAction::Execute(Event /*event*/)
{
    Unit* malacrass = AI_VALUE2(Unit*, "find target", "hex lord malacrass");
    if (!malacrass ||
        !malacrass->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_HEX_LORD_SPELL_REFLECTION)))
        return false;

    if (bot->GetVictim() == malacrass)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return true;
    }

    return false;
}

bool HexLordMalacrassMoveAwayFromFreezingTrapAction::Execute(Event /*event*/)
{
    GameObject* trapGo = bot->FindNearestGameObject(
        static_cast<uint32>(ZulAmanObjects::GO_FREEZING_TRAP), 20.0f, true);

    if (!trapGo)
        return false;

    float currentDistance = bot->GetDistance2d(trapGo);
    constexpr float safeDistance = 6.0f;
    constexpr uint32 minInterval = 0;
    if (currentDistance < safeDistance)
        return FleePosition(trapGo->GetPosition(), safeDistance, minInterval);

    return false;
}

// Zul'jin

bool ZuljinMisdirectBossToMainTankAction::Execute(Event /*event*/)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    Player* mainTank = GetGroupMainTank(botAI, bot);
    if (!mainTank)
        return false;

    if (botAI->CanCastSpell("misdirection", mainTank))
        return botAI->CastSpell("misdirection", mainTank);

    if (bot->HasAura(static_cast<uint32>(ZulAmanSpells::SPELL_MISDIRECTION)) &&
        botAI->CanCastSpell("steady shot", zuljin))
        return botAI->CastSpell("steady shot", zuljin);

    return false;
}

bool ZuljinTanksPositionBossAction::Execute(Event /*event*/)
{
    Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin");
    if (!zuljin)
        return false;

    if (bot->GetVictim() != zuljin)
        return Attack(zuljin);

    if (zuljin->GetVictim() == bot)
    {
        const Position& position = ZULJIN_TANK_POSITION;
        float distToPosition =
            bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveDist = std::min(10.0f, distToPosition);
            float moveX = bot->GetPositionX() + (dX / distToPosition) * moveDist;
            float moveY = bot->GetPositionY() + (dY / distToPosition) * moveDist;

            return MoveTo(ZULAMAN_MAP_ID, moveX, moveY, bot->GetPositionZ(), false, false,
                          false, false, MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

bool ZuljinRunAwayFromWhirlwindAction::Execute(Event /*event*/)
{
    if (Unit* zuljin = AI_VALUE2(Unit*, "find target", "zul'jin"))
    {
        float currentDistance = bot->GetExactDist2d(zuljin);
        constexpr float safeDistance = 10.0f;
        if (currentDistance < safeDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveAway(zuljin, safeDistance - currentDistance);
        }
    }

    return false;
}

bool ZuljinAvoidCyclonesAction::Execute(Event /*event*/)
{
    auto const& cyclones = GetAllHazardTriggers(
        bot, static_cast<uint32>(ZulAmanNPCs::NPC_FEATHER_VORTEX), 50.0f);

    if (cyclones.empty())
        return false;

    constexpr float hazardRadius = 6.0f;
    bool inDanger = false;
    for (Unit* cyclone : cyclones)
    {
        if (bot->GetDistance2d(cyclone) < hazardRadius)
        {
            inDanger = true;
            break;
        }
    }

    if (!inDanger)
        return false;

    const Position& zuljinCenter = ZULJIN_TANK_POSITION;
    constexpr float safeZoneRadius = 30.0f;

    Position safestPos =
        FindSafestNearbyPosition(bot, cyclones, zuljinCenter, safeZoneRadius, hazardRadius, true);

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveTo(ZULAMAN_MAP_ID, safestPos.GetPositionX(), safestPos.GetPositionY(),
                  bot->GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_FORCED, true, false);
}

bool ZuljinSpreadRangedAction::Execute(Event /*event*/)
{
    constexpr float minDistance = 6.0f;
    constexpr uint32 minInterval = 1000;
    if (Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance))
        return FleePosition(nearestPlayer->GetPosition(), minDistance, minInterval);

    return false;
}
