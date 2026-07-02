#include "ICCActions.h"
#include <limits>
#include "NearestNpcsValue.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "Vehicle.h"
#include "RtiValue.h"
#include "GenericSpellActions.h"
#include "GenericActions.h"
#include "ICCTriggers.h"
#include "Multiplier.h"

bool IccSindragosaGroupPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Block positioning only during the pre-combat intro fly-in.
    // Once Sindragosa is in combat the air phase is handled separately,
    // so ground bots must still be allowed to pre-position.
    if (boss->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY) && !boss->IsInCombat())
        return false;

    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (aura && aura->GetStackAmount() >= 6 && botAI->IsMainTank(bot))
        return false;

    // Route tanks to tank positioning.
    // At pull GetVictim() may still be nullptr, so also check IsMainTank so the
    // tank is not incorrectly sent to the melee stack before aggro is established.
    bool const isTankingBoss = botAI->IsTank(bot) && (boss->GetVictim() == bot || botAI->IsMainTank(bot));
    if (isTankingBoss)
        return HandleTankPositioning(boss);

    // Everyone else: boss is not targeting this bot
    if (boss->GetVictim() != bot)
        return HandleNonTankPositioning();

    return false;
}

bool IccSindragosaGroupPositionAction::HandleTankPositioning(Unit* boss)
{
    float const distBossToCenter = boss->GetExactDist2d(ICC_SINDRAGOSA_CENTER_POSITION);
    float const distToTankPos = bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION);

    // Compute how far the boss orientation deviates from east (PI/2)
    float const targetOrientation = fmod(float(M_PI) / 2.0f + 2.0f * float(M_PI), 2.0f * float(M_PI));
    float const currentOrientation = fmod(boss->GetOrientation() + 2.0f * float(M_PI), 2.0f * float(M_PI));
    float orientationDiff = currentOrientation - targetOrientation;

    // Clamp difference to [-PI, PI]
    if (orientationDiff > float(M_PI))
        orientationDiff -= 2.0f * float(M_PI);
    else if (orientationDiff < -float(M_PI))
        orientationDiff += 2.0f * float(M_PI);

    // Stage 1: Drag boss toward the arena centre when it has drifted too far
    if (distBossToCenter > 16.0f && distToTankPos <= 20.0f)
    {
        float const dirX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() - boss->GetPositionX();
        float const dirY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() - boss->GetPositionY();

        // Step 4 yards past centre to keep the boss moving through it
        float const moveX = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionX() + (dirX / distBossToCenter) * 8.0f;
        float const moveY = ICC_SINDRAGOSA_CENTER_POSITION.GetPositionY() + (dirY / distBossToCenter) * 8.0f;

        return MoveTo(bot->GetMapId(), moveX, moveY, boss->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Stage 2: Walk toward the designated tank position
    if (distToTankPos > 10.0f)
    {
        Position const& botPos = bot->GetPosition();
        Position const& tankPos = ICC_SINDRAGOSA_TANK_POSITION;

        float const dx = tankPos.GetPositionX() - botPos.GetPositionX();
        float const dy = tankPos.GetPositionY() - botPos.GetPositionY();
        float const distance = std::hypot(dx, dy);

        // Advance one yard at a time for smooth pathing
        float const scale = 1.0f / distance;
        float const targetX = botPos.GetPositionX() + dx * scale;
        float const targetY = botPos.GetPositionY() + dy * scale;

        return MoveTo(bot->GetMapId(), targetX, targetY, bot->GetPositionZ(), false, false, false, true,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    // Stage 3: Arc around the boss to correct its facing toward east
    if (std::abs(orientationDiff) > 0.15f)
    {
        float const centerX = boss->GetPositionX();
        float const centerY = boss->GetPositionY();
        float const radius = std::max(2.0f, bot->GetExactDist2d(centerX, centerY));

        float angle = atan2(bot->GetPositionY() - centerY, bot->GetPositionX() - centerX);

        // Negative diff → step counterclockwise (north); positive → clockwise (south)
        static constexpr float ARC_STEP = 0.125f;
        angle += (orientationDiff < 0) ? ARC_STEP : -ARC_STEP;

        float const moveX = centerX + radius * cos(angle);
        float const moveY = centerY + radius * sin(angle);

        return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    // Stage 4: Fine-tune Y-axis alignment with the tank position
    float const yDiff = std::abs(bot->GetPositionY() - ICC_SINDRAGOSA_TANK_POSITION.GetPositionY());
    if (yDiff > 2.0f)
    {
        Position const& botPos = bot->GetPosition();
        Position const& tankPos = ICC_SINDRAGOSA_TANK_POSITION;

        float const newY = botPos.GetPositionY() + (tankPos.GetPositionY() > botPos.GetPositionY() ? 1.0f : -1.0f);

        return MoveTo(bot->GetMapId(), botPos.GetPositionX(), newY, botPos.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool IccSindragosaGroupPositionAction::HandleNonTankPositioning()
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    // Collect all alive raid members
    std::vector<Player*> raidMembers;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;

        raidMembers.push_back(member);
    }

    uint32 const totalMembers = static_cast<uint32>(raidMembers.size());
    if (totalMembers == 0)
        return false;

    // Count members currently free of Mystic Buffet
    uint32 membersWithoutAura = 0;
    for (Player* member : raidMembers)
    {
        if (!botAI->GetAura("mystic buffet", member))
            ++membersWithoutAura;
    }

    // Raid is considered "clear" when 60 % or more lack the debuff stack
    float const percentageWithoutAura = static_cast<float>(membersWithoutAura) / static_cast<float>(totalMembers);
    bool const raidClear = (percentageWithoutAura >= 0.6f);

    static constexpr std::array<uint32, 4> TombEntries = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};
    static constexpr uint8 SKULL_ICON_INDEX = 7;

    // Priority: if a tank is ice-tombed (ground phase), mark that tomb skull
    // immediately so the raid DPSes it and frees the tank. Any bot can issue
    // the mark — redundant SetTargetIcon calls are idempotent.
    Unit* const bossForFlyCheck = AI_VALUE2(Unit*, "find target", "sindragosa");
    bool const bossGrounded = bossForFlyCheck &&
        bossForFlyCheck->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(),
                                         ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) >= 30.0f;

    Player* entombedTank = nullptr;
    if (bossGrounded)
    {
        for (Player* member : raidMembers)
        {
            if (botAI->IsTank(member) && member->HasAura(SPELL_ICE_TOMB))
            {
                entombedTank = member;
                break;
            }
        }
    }

    Unit* tankTomb = nullptr;
    if (entombedTank)
    {
        GuidVector const tombGuids = AI_VALUE(GuidVector, "possible targets no los");
        float minDist = 4.0f;
        for (uint32 const entry : TombEntries)
        {
            for (auto const& guid : tombGuids)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || unit->GetEntry() != entry || !unit->IsAlive())
                    continue;
                float const d = unit->GetDistance(entombedTank);
                if (d < minDist)
                {
                    minDist = d;
                    tankTomb = unit;
                }
            }
        }
    }

    if (tankTomb)
    {
        ObjectGuid const currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
        if (currentSkull != tankTomb->GetGUID())
            group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), tankTomb->GetGUID());
    }
    else if (raidClear && botAI->IsTank(bot))
    {
        GuidVector const tombGuids = AI_VALUE(GuidVector, "possible targets no los");

        Unit* nearestTomb = nullptr;
        float minDist = 150.0f;

        for (uint32 const entry : TombEntries)
        {
            for (auto const& guid : tombGuids)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || unit->GetEntry() != entry || !unit->IsAlive())
                    continue;

                float const dist = bot->GetDistance(unit);
                if (dist < minDist)
                {
                    minDist = dist;
                    nearestTomb = unit;
                }
            }
        }

        // Prefer the nearest tomb; fall back to marking the boss itself
        Unit* targetToMark = nearestTomb;
        if (!targetToMark)
        {
            Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
            if (boss && boss->IsAlive())
                targetToMark = boss;
        }

        if (targetToMark)
        {
            ObjectGuid const currentSkull = group->GetTargetIcon(SKULL_ICON_INDEX);
            Unit* const currentSkullUnit = botAI->GetUnit(currentSkull);
            bool const needsUpdate =
                !currentSkullUnit || !currentSkullUnit->IsAlive() || currentSkullUnit != targetToMark;

            if (needsUpdate)
                group->SetTargetIcon(SKULL_ICON_INDEX, bot->GetGUID(), targetToMark->GetGUID());
        }
    }

    context->GetValue<std::string>("rti")->Set("skull");

    if (botAI->IsRanged(bot))
    {
        static constexpr float RANGED_TOLERANCE = 9.0f;
        static constexpr float RANGED_MAX_STEP = 5.0f;

        if (bot->GetExactDist2d(ICC_SINDRAGOSA_RANGED_POSITION) > RANGED_TOLERANCE)
            return MoveIncrementallyToPosition(ICC_SINDRAGOSA_RANGED_POSITION, RANGED_MAX_STEP);

        return false;
    }

    static constexpr float MELEE_TOLERANCE = 10.0f;
    static constexpr float MELEE_MAX_STEP = 5.0f;

    if (bot->GetExactDist2d(ICC_SINDRAGOSA_MELEE_POSITION) > MELEE_TOLERANCE)
        return MoveIncrementallyToPosition(ICC_SINDRAGOSA_MELEE_POSITION, MELEE_MAX_STEP);

    return false;
}

bool IccSindragosaGroupPositionAction::MoveIncrementallyToPosition(Position const& targetPos, float maxStep)
{
    float const dirX = targetPos.GetPositionX() - bot->GetPositionX();
    float const dirY = targetPos.GetPositionY() - bot->GetPositionY();
    float const length = std::hypot(dirX, dirY);

    float const stepSize = std::min(maxStep, bot->GetExactDist2d(targetPos));
    float const moveX = bot->GetPositionX() + (dirX / length) * stepSize;
    float const moveY = bot->GetPositionY() + (dirY / length) * stepSize;

    return MoveTo(bot->GetMapId(), moveX, moveY, targetPos.GetPositionZ(), false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT);
}

bool IccSindragosaFrostBeaconAction::TryDropTombFlares(Unit const* boss)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool anyBeacon = false;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
        {
            anyBeacon = true;
            break;
        }
    }
    if (!anyBeacon)
        return false;

    // Phase tracking: clear flared sets on phase boundary so each phase gets fresh markers.
    // Keyed per-instance so concurrent ICC raids don't share phase state.
    uint32 const instanceId = bot->GetInstanceId();
    bool const phase3 = boss->HealthBelowPct(35);
    bool& lastPhase3 = s_lastPhase3[instanceId];
    if (phase3 != lastPhase3)
    {
        s_flaredRedThisPhase[instanceId].clear();
        s_flaredBluePhase3[instanceId] = false;
        lastPhase3 = phase3;
    }

    // Build position list for current phase.
    std::vector<std::pair<int, Position const*>> targets;
    if (phase3)
    {
        targets.emplace_back(3, &ICC_SINDRAGOSA_THOMBMB2_POSITION);
    }
    else
    {
        Difficulty const diff = bot->GetRaidDifficulty();
        bool const is25Man = (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC);
        targets.emplace_back(0, &ICC_SINDRAGOSA_THOMB1_POSITION);
        if (is25Man)
            targets.emplace_back(1, &ICC_SINDRAGOSA_THOMB2_POSITION);
        targets.emplace_back(2, &ICC_SINDRAGOSA_THOMB3_POSITION);
    }

    // Build marker pool: alive non-tank bots (skip real players). One bot per position
    // sidesteps the item cooldown — each marker only casts once per phase.
    std::vector<Player*> preferred;  // non-beaconed
    std::vector<Player*> fallback;   // beaconed allowed
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !member->IsAlive())
            continue;
        PlayerbotAI* memberAI = sPlayerbotsMgr.GetPlayerbotAI(member);
        if (!memberAI)
            continue;  // real player
        if (memberAI->IsTank(member))
            continue;
        fallback.push_back(member);
        if (!member->HasAura(SPELL_FROST_BEACON))
            preferred.push_back(member);
    }

    auto byGuid = [](Player* a, Player* b) { return a->GetGUID() < b->GetGUID(); };
    std::sort(preferred.begin(), preferred.end(), byGuid);
    std::sort(fallback.begin(), fallback.end(), byGuid);

    // Assign N markers (one per position) from preferred first, then top up from fallback.
    size_t const needed = targets.size();
    std::vector<Player*> markers;
    markers.reserve(needed);
    for (Player* p : preferred)
    {
        if (markers.size() >= needed)
            break;
        markers.push_back(p);
    }
    if (markers.size() < needed)
    {
        for (Player* p : fallback)
        {
            if (markers.size() >= needed)
                break;
            if (std::find(markers.begin(), markers.end(), p) != markers.end())
                continue;
            markers.push_back(p);
        }
    }

    // Find this bot's slot in the marker list.
    auto myIt = std::find(markers.begin(), markers.end(), bot);
    if (myIt == markers.end())
        return false;
    size_t const mySlot = std::distance(markers.begin(), myIt);
    if (mySlot >= targets.size())
        return false;

    auto const& [chosenIdx, chosenPos] = targets[mySlot];

    // Already flared by some bot — don't duplicate.
    if (phase3 && s_flaredBluePhase3[instanceId])
        return false;
    if (!phase3 && s_flaredRedThisPhase[instanceId].count(chosenIdx))
        return false;

    // Skip if a tomb is already standing on this position.
    static constexpr std::array<uint32, 4> tombEntries = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};
    GuidVector const tombGuids = AI_VALUE(GuidVector, "possible targets no los");
    for (uint32 const entry : tombEntries)
    {
        for (auto const& guid : tombGuids)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
                continue;
            if (unit->GetExactDist2d(chosenPos->GetPositionX(), chosenPos->GetPositionY()) <= 5.0f)
                return false;
        }
    }

    uint32 const itemId = phase3 ? ITEM_BLUE_SMOKE_FLARE : ITEM_RED_SMOKE_FLARE;
    if (bot->GetItemCount(itemId, false) == 0)
        bot->StoreNewItemInBestSlots(itemId, 10);

    Item* flare = bot->GetItemByEntry(itemId);
    if (!flare)
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;
    if (bot->CanUseItem(flare) != EQUIP_ERR_OK)
        return false;

    uint8 const bagIndex = flare->GetBagSlot();
    uint8 const slot = flare->GetSlot();
    uint8 const castCount = 1;
    uint32 const spellId = flare->GetTemplate()->Spells[0].SpellId;
    ObjectGuid const itemGuid = flare->GetGUID();
    uint32 const glyphIndex = 0;
    uint8 const castFlags = 0;

    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << castCount << spellId << itemGuid << glyphIndex << castFlags;
    packet << uint32(TARGET_FLAG_DEST_LOCATION);
    packet.appendPackGUID(0);
    packet << chosenPos->GetPositionX() << chosenPos->GetPositionY() << chosenPos->GetPositionZ();

    bot->GetSession()->HandleUseItemOpcode(packet);
    if (phase3)
        s_flaredBluePhase3[instanceId] = true;
    else
        s_flaredRedThisPhase[instanceId].insert(chosenIdx);
    return false;
}

// Todo not really used since tigger is bypassed
bool IccSindragosaTankSwapPositionAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    if (!botAI->IsAssistTank(bot))
        return false;

    // Keep the assist tank on the swap position with a tight tolerance
    if (bot->GetExactDist2d(ICC_SINDRAGOSA_TANK_POSITION) > 3.0f)
    {
        return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_TANK_POSITION.GetPositionX(),
                      ICC_SINDRAGOSA_TANK_POSITION.GetPositionY(), ICC_SINDRAGOSA_TANK_POSITION.GetPositionZ(), false,
                      false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

std::map<uint32, std::set<int>> IccSindragosaFrostBeaconAction::s_flaredRedThisPhase;
std::map<uint32, bool> IccSindragosaFrostBeaconAction::s_flaredBluePhase3;
std::map<uint32, bool> IccSindragosaFrostBeaconAction::s_lastPhase3;
uint32 IccSindragosaFrostBeaconAction::s_nextFlareMs = 0;  // deprecated, kept for ABI of header

bool IccSindragosaFrostBeaconAction::Execute(Event /*event*/)
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return false;

    TryDropTombFlares(boss);

    HandleSupportActions();

    if (bot->HasAura(SPELL_FROST_BEACON))
    {
        return HandleBeaconedPlayer(boss);
    }

    return HandleNonBeaconedPlayer(boss);
}

bool IccSindragosaFrostBeaconAction::HandleSupportActions()
{
    Group* group = bot->GetGroup();

    // Tank support - Paladin Hand of Freedom
    if (group && bot->getClass() == CLASS_PALADIN)
    {
        for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsTank(member))
            {
                continue;
            }

            if (botAI->GetAura("Frost Breath", member) && !member->HasAura(SPELL_HAND_OF_FREEDOM))
            {
                botAI->CastSpell(SPELL_HAND_OF_FREEDOM, member);
                break;
            }
        }
    }

    return false;
}

bool IccSindragosaHotAction::Execute(Event /*event*/)
{
    if (!botAI->IsHeal(bot) || bot->HasAura(SPELL_FROST_BEACON))
        return false;

    auto const members = AI_VALUE(GuidVector, "group members");
    for (auto const& memberGuid : members)
    {
        Unit* member = botAI->GetUnit(memberGuid);
        if (!member || !member->IsAlive() || !member->HasAura(SPELL_FROST_BEACON))
            continue;

        if (member->HasAura(SPELL_ICE_TOMB))
            continue;

        uint32 spellId = 0;
        switch (bot->getClass())
        {
            case CLASS_PRIEST:
                spellId = 48068;  // Renew
                break;
            case CLASS_SHAMAN:
                spellId = 61301;  // Riptide
                break;
            case CLASS_DRUID:
                spellId = 48441;  // Rejuvenation
                break;
            default:
                return false;
        }

        if (!member->HasAura(spellId) && botAI->CanCastSpell(spellId, member))
            botAI->CastSpell(spellId, member);
    }

    return false;
}

bool IccSindragosaFrostBeaconAction::HandleBeaconedPlayer(const Unit* boss)
{
    // Phase 3 positioning (below 35% health, not flying)
    if (boss->HealthBelowPct(35) && !IsBossFlying(boss))
    {
        if (!bot->HasAura(SPELL_NITRO_BOOSTS))
            bot->AddAura(SPELL_NITRO_BOOSTS, bot);
        botAI->Reset();
        return MoveToPositionIfNeeded(ICC_SINDRAGOSA_THOMBMB2_POSITION, POSITION_TOLERANCE);
    }

    // Regular beacon positioning using tomb spots
    Group* group = bot->GetGroup();
    if (!group)
    {
        return false;
    }

    // Collect and sort beaconed players by GUID for deterministic assignment
    std::vector<Player*> beaconedPlayers;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
            beaconedPlayers.push_back(member);
    }

    std::sort(beaconedPlayers.begin(), beaconedPlayers.end(),
              [](const Player* a, const Player* b) { return a->GetGUID() < b->GetGUID(); });

    // Find this bot's index
    auto const it = std::find(beaconedPlayers.begin(), beaconedPlayers.end(), bot);
    if (it == beaconedPlayers.end())
        return false;

    size_t const myIndex = std::distance(beaconedPlayers.begin(), it);
    size_t const beaconCount = beaconedPlayers.size();

    // Calculate tomb spot based on beacon count
    size_t spot = 0;
    switch (beaconCount)
    {
        case 2:
            spot = (myIndex == 0) ? 0 : 2;
            break;
        case 5:
            spot = (myIndex < 2) ? 0 : ((myIndex == 2) ? 1 : 2);
            break;
        case 6:
            spot = myIndex / 2;
            break;
        default:
            spot = myIndex % 3;
            break;
    }

    // Get tomb position and move if needed
    static constexpr std::array<const Position*, 3> tombPositions = {
        &ICC_SINDRAGOSA_THOMB1_POSITION, &ICC_SINDRAGOSA_THOMB2_POSITION, &ICC_SINDRAGOSA_THOMB3_POSITION};

    const Position& tombPosition = *tombPositions[std::min(spot, tombPositions.size() - 1)];
    return MoveToPositionIfNeeded(tombPosition, TOMB_POSITION_TOLERANCE);
}

bool IccSindragosaFrostBeaconAction::HandleNonBeaconedPlayer(const Unit* boss)
{
    // Collect beaconed players
    std::vector<Unit*> beaconedPlayers;
    auto const members = AI_VALUE(GuidVector, "group members");
    for (auto const& memberGuid : members)
    {
        Unit* player = botAI->GetUnit(memberGuid);
        if (player && player->GetGUID() != bot->GetGUID() && player->HasAura(SPELL_FROST_BEACON))
        {
            beaconedPlayers.push_back(player);
        }
    }

    if (beaconedPlayers.empty())
    {
        return false;
    }

    // Air phase positioning
    if (IsBossFlying(boss))
    {
        if (!bot->HasAura(SPELL_FROST_BEACON))
        {
            const Difficulty diff = bot->GetRaidDifficulty();
            bool is25Man = false;
            if (diff && (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC))
                is25Man = true;

            const Position& safePosition = is25Man ? ICC_SINDRAGOSA_FBOMB_POSITION : ICC_SINDRAGOSA_FBOMB10_POSITION;

            float const dist = bot->GetExactDist2d(safePosition.GetPositionX(), safePosition.GetPositionY());
            if (dist > MOVE_TOLERANCE)
            {
                return MoveToPosition(safePosition);
            }

        }
        return botAI->IsHeal(bot);  // Continue for healers, wait for others
    }

    // Ground phase - position based on role and avoid beaconed players
    bool const isRanged = botAI->IsRanged(bot) && !botAI->IsHeal(bot) /*(bot->GetExactDist2d(ICC_SINDRAGOSA_RANGED_POSITION.GetPositionX(),ICC_SINDRAGOSA_RANGED_POSITION.GetPositionY()) <
                          bot->GetExactDist2d(ICC_SINDRAGOSA_MELEE_POSITION.GetPositionX(),ICC_SINDRAGOSA_MELEE_POSITION.GetPositionY()))*/;

    const Position& targetPosition = isRanged ? ICC_SINDRAGOSA_RANGED_POSITION : ICC_SINDRAGOSA_MELEE_POSITION;

    float const deltaX = std::abs(targetPosition.GetPositionX() - bot->GetPositionX());
    float const deltaY = std::abs(targetPosition.GetPositionY() - bot->GetPositionY());
    if (boss && boss->GetVictim() != bot)
    {
        if ((deltaX > MOVE_TOLERANCE) || (deltaY > MOVE_TOLERANCE))
        {
            if (bot->HasUnitState(UNIT_STATE_CASTING))
            {
                botAI->Reset();
            }
            return MoveToPosition(targetPosition);
        }
    }
    return false;
}

bool IccSindragosaFrostBeaconAction::MoveToPositionIfNeeded(const Position& position, float tolerance)
{
    float const distance = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());
    if (distance > tolerance)
    {
        return MoveToPosition(position);
    }
    return distance <= tolerance;
}

bool IccSindragosaFrostBeaconAction::MoveToPosition(const Position& position)
{
    float posX = position.GetPositionX();
    float posY = position.GetPositionY();
    float posZ = position.GetPositionZ();

    bot->UpdateAllowedPositionZ(posX, posY, posZ);

    return MoveTo(bot->GetMapId(), posX, posY, posZ, false, false, false, false, MovementPriority::MOVEMENT_FORCED,
                  true, false);
}

bool IccSindragosaFrostBeaconAction::IsBossFlying(const Unit* boss)
{
    return boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(),
                                ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f;
}

bool IccSindragosaBlisteringColdAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    // Only non-tanks should move out
    if (botAI->IsMainTank(bot))
        return false;

   float dist = bot->GetExactDist2d(boss->GetPositionX(), boss->GetPositionY());

    if (dist >= 33.0f)
        return false;

    Position const& targetPos = ICC_SINDRAGOSA_BLISTERING_COLD_POSITION;

    // Only move if we're too close to the boss (< 30 yards)
    if (dist < 33.0f)
    {

        float const STEP_SIZE = 15.0f;
        float distToTarget = bot->GetDistance2d(targetPos.GetPositionX(), targetPos.GetPositionY());

        if (distToTarget > 0.1f)  // Avoid division by zero
        {
            if (!bot->HasAura(SPELL_NITRO_BOOSTS))
                bot->AddAura(SPELL_NITRO_BOOSTS, bot);
            // Calculate direction vector
            float dirX = targetPos.GetPositionX() - bot->GetPositionX();
            float dirY = targetPos.GetPositionY() - bot->GetPositionY();

            // Normalize direction vector
            float length = sqrt(dirX * dirX + dirY * dirY);
            dirX /= length;
            dirY /= length;

            // Move STEP_SIZE yards in that direction
            float moveX = bot->GetPositionX() + dirX * STEP_SIZE;
            float moveY = bot->GetPositionY() + dirY * STEP_SIZE;

            return MoveTo(bot->GetMapId(), moveX, moveY, bot->GetPositionZ(),
                         false, false, false, true, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }
    return false;
}

bool IccSindragosaUnchainedMagicAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Unchained Magic", bot, false, true);
    if (!aura)
        return false;

    Aura* aura1 = botAI->GetAura("Instability", bot, false, true);

    Difficulty diff = bot->GetRaidDifficulty();
    if (aura && (diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_NORMAL))
    {
        if (aura1 && aura1->GetStackAmount() >= 6)
            return true;  // Stop casting spells
    }

    return false;
}

bool IccSindragosaChilledToTheBoneAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss)
        return false;

    Aura* aura = botAI->GetAura("Chilled to the Bone", bot, false, true);
    if (!aura)
        return false;

    if (aura) // Chilled to the Bone
    {
        if (aura->GetStackAmount() >= 6)
        {
            botAI->Reset();
            bot->AttackStop();
            return true;
        }
    }

    return false;
}

bool IccSindragosaMysticBuffetAction::Execute(Event /*event*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "sindragosa");
    if (!boss || !bot || !bot->IsAlive())
        return false;

    // Check if we have Mystic Buffet
    Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
    if (!aura)
        return false;

    if (boss->GetVictim() == bot)
        return false;

    // Skip if we have Frost Beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    static const std::array<uint32, 4> tombEntries = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};
    const GuidVector tombGuids = AI_VALUE(GuidVector, "possible targets no los");

    Unit* nearestTomb = nullptr;
    float minDist = 150.0f;

    for (auto const entry : tombEntries)
    {
        for (auto const& guid : tombGuids)
        {
            if (Unit* unit = botAI->GetUnit(guid))
            {
                if (unit->GetEntry() == entry && unit->IsAlive())
                {
                    float dist = bot->GetDistance(unit);
                    if (dist < minDist)
                    {
                        minDist = dist;
                        nearestTomb = unit;
                    }
                }
            }
        }
    }

    // Check if anyone in group has Frost Beacon (SPELL_FROST_BEACON)
    bool anyoneHasFrostBeacon = false;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
        {
            anyoneHasFrostBeacon = true;
            break;
        }
    }

    bool tombPresent = nearestTomb != nullptr;
    bool atLOS2 = bot->GetExactDist2d(ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(),
                                      ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY()) <= 2.0f;

    // Move to LOS2 position if: tomb is present and no one has Frost Beacon
    bool shouldMoveLOS2 = tombPresent && !anyoneHasFrostBeacon;

    if (shouldMoveLOS2)
    {
        // If already at LOS2: instead of idling while stacks drop, DPS the LOS
        // tomb down to MYSTIC_BUFFET_TOMB_STOP_HP_PCT so the wait isn't wasted.
        // Single skull icon for the whole raid — pick the tomb with lowest GUID
        // so every bot converges deterministically on the same target.
        if (atLOS2 && aura && !botAI->IsHeal(bot))
        {
            constexpr uint8 SKULL_ICON = 7;
            float MYSTIC_BUFFET_TOMB_STOP_HP_PCT = 50.0f;
            Difficulty const diff = bot->GetRaidDifficulty();

            if (diff && (diff == RAID_DIFFICULTY_10MAN_HEROIC))
                MYSTIC_BUFFET_TOMB_STOP_HP_PCT = 90.0f;

            Unit* tombToMark = nullptr;
            ObjectGuid bestGuid;
            for (auto const entry : tombEntries)
            {
                for (auto const& guid : tombGuids)
                {
                    Unit* unit = botAI->GetUnit(guid);
                    if (!unit || !unit->IsAlive() || unit->GetEntry() != entry)
                        continue;
                    if (unit->GetHealthPct() <= MYSTIC_BUFFET_TOMB_STOP_HP_PCT)
                        continue;
                    if (!tombToMark || unit->GetGUID() < bestGuid)
                    {
                        tombToMark = unit;
                        bestGuid = unit->GetGUID();
                    }
                }
            }

            if (!tombToMark)
            {
                ObjectGuid const currentIcon = group->GetTargetIcon(SKULL_ICON);
                if (!currentIcon.IsEmpty())
                    group->SetTargetIcon(SKULL_ICON, bot->GetGUID(), ObjectGuid::Empty);
                return true;
            }

            context->GetValue<std::string>("rti")->Set("skull");

            Unit* currentIconUnit = botAI->GetUnit(group->GetTargetIcon(SKULL_ICON));
            if (!currentIconUnit || !currentIconUnit->IsAlive() || currentIconUnit != tombToMark)
                group->SetTargetIcon(SKULL_ICON, bot->GetGUID(), tombToMark->GetGUID());

            return false;
        }

        botAI->Reset();
        // Move to LOS2 position
        return MoveTo(bot->GetMapId(), ICC_SINDRAGOSA_LOS2_POSITION.GetPositionX(),
                      ICC_SINDRAGOSA_LOS2_POSITION.GetPositionY(), ICC_SINDRAGOSA_LOS2_POSITION.GetPositionZ(), false,
                      false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    return false;
}

std::map<std::pair<uint32, ObjectGuid>, int> IccSindragosaFrostBombAction::s_groupAssignments;
std::map<std::pair<uint32, ObjectGuid>, ObjectGuid> IccSindragosaFrostBombAction::s_tombAssignments;
std::set<std::pair<uint32, ObjectGuid>> IccSindragosaFrostBombAction::s_freedFallback;
std::map<std::pair<uint32, ObjectGuid>, IccSindragosaFrostBombAction::LastLosMove>
    IccSindragosaFrostBombAction::s_lastLosMove;

bool IccSindragosaFrostBombAction::Execute(Event /*event*/)
{
    if (!bot || !bot->IsAlive())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    if (bot->HasAura(SPELL_ICE_TOMB))
    {
        PinGroupToCurrentZone();
        s_freedFallback.insert(std::make_pair(bot->GetInstanceId(), bot->GetGUID()));
        return false;
    }

    FrostBombContext ctx;
    if (!CollectContext(ctx))
    {
        bot->AttackStop();
        return true;
    }

    int const groupIndex = ResolveGroupIndex(group);
    if (groupIndex < 0)
        return false;

    Difficulty const diff = bot->GetRaidDifficulty();
    int const groupCount = (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC) ? 3 : 2;

    // Fixed tomb zone positions per group:
    //   25-man (3 groups): group 0→THOMB1, group 1→THOMB2, group 2→THOMB3
    //   10-man (2 groups): group 0→THOMB1, group 1→THOMB3  (beacons skip THOMB2 in 2-beacon case)
    static constexpr std::array<Position const*, 3> tombZones = {
        &ICC_SINDRAGOSA_THOMB1_POSITION,
        &ICC_SINDRAGOSA_THOMB2_POSITION,
        &ICC_SINDRAGOSA_THOMB3_POSITION
    };
    int const zoneIdx = (groupCount == 2) ? (groupIndex == 0 ? 0 : 2) : groupIndex;
    Position const& myZone = *tombZones[zoneIdx];

    std::vector<Unit*> myTombs = SelectTombs(ctx.tombs, groupIndex, groupCount);

    bool myZoneAllProtected = false;
    {
        static constexpr std::array<uint8, 3> raidIcons = {7, 6, 0};
        static constexpr float STRIP_HP_PCT = 30.0f;
        bool const is10Man =
            (diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_10MAN_HEROIC);
        float const tombStopHpPct = is10Man ? 60.0f : 40.0f;

        auto isMarked = [&](Unit* tomb) -> bool
        {
            for (uint8 const icon : raidIcons)
                if (group->GetTargetIcon(icon) == tomb->GetGUID())
                    return true;
            return false;
        };

        bool anyAlive = false;
        bool anyKillable = false;
        for (Unit* tomb : myTombs)
        {
            if (!tomb || !tomb->IsAlive())
                continue;
            anyAlive = true;
            bool const marked = isMarked(tomb);

            if (!marked && tomb->GetHealthPct() < STRIP_HP_PCT)
            {
                Unit::AuraMap& auras = tomb->GetOwnedAuras();
                for (Unit::AuraMap::iterator it = auras.begin(); it != auras.end();)
                {
                    Aura* aura = it->second;
                    if (aura && aura->GetDuration() != -1)
                    {
                        tomb->RemoveOwnedAura(it);
                        continue;
                    }
                    ++it;
                }
            }
            (void)marked;
            if (tomb->GetHealthPct() > tombStopHpPct)
                anyKillable = true;
        }

        myZoneAllProtected = anyAlive && !anyKillable;

        std::vector<Creature*> pets;
        if (Pet* mainPet = bot->GetPet())
            pets.push_back(mainPet);
        for (Unit* controlled : bot->m_Controlled)
        {
            if (Creature* c = dynamic_cast<Creature*>(controlled))
            {
                if (std::find(pets.begin(), pets.end(), c) == pets.end())
                    pets.push_back(c);
            }
        }

        if (myZoneAllProtected)
        {
            for (Creature* pet : pets)
            {
                if (!pet || !pet->IsAlive())
                    continue;
                pet->SetReactState(REACT_PASSIVE);
                pet->AttackStop();
                pet->InterruptNonMeleeSpells(true);
                pet->CombatStop();
                pet->SetTarget(ObjectGuid::Empty);
                if (CharmInfo* ci = pet->GetCharmInfo())
                {
                    ci->SetPlayerReactState(REACT_PASSIVE);
                    pet->GetMotionMaster()->MoveFollow(bot, PET_FOLLOW_DIST,
                                                       pet->GetFollowAngle());
                    ci->SetCommandState(COMMAND_FOLLOW);
                    ci->SetIsCommandAttack(false);
                    ci->SetIsAtStay(false);
                    ci->SetIsReturning(true);
                    ci->SetIsFollowing(true);
                }
            }
        }
        else
        {
            for (Creature* pet : pets)
            {
                if (pet && pet->IsAlive() && pet->GetReactState() == REACT_PASSIVE)
                {
                    pet->SetReactState(REACT_DEFENSIVE);
                    if (CharmInfo* ci = pet->GetCharmInfo())
                        ci->SetPlayerReactState(REACT_DEFENSIVE);
                }
            }
        }
    }

    // No tomb in zone
    if (myTombs.empty())
    {
        // Freed-from-tomb fallback: hide behind the nearest alive tomb anywhere
        // in the arena rather than running across to our pinned zone anchor.
        if (s_freedFallback.count(std::make_pair(bot->GetInstanceId(), bot->GetGUID())))
        {
            Unit* nearest = nullptr;
            float minDist = std::numeric_limits<float>::max();
            for (Unit* tomb : ctx.tombs)
            {
                if (!tomb || !tomb->IsAlive())
                    continue;
                float const d = bot->GetExactDist2d(tomb);
                if (d < minDist)
                {
                    minDist = d;
                    nearest = tomb;
                }
            }

            if (nearest)
            {
                float const fbAngle = ctx.marker->GetAngle(nearest);
                float const fbX = nearest->GetPositionX() + std::cos(fbAngle) * 6.5f;
                float const fbY = nearest->GetPositionY() + std::sin(fbAngle) * 6.5f;
                float const fbZ = nearest->GetPositionZ();

                if (bot->GetDistance2d(fbX, fbY) > 0.1f)
                {
                    botAI->Reset();
                    bot->AttackStop();
                    return MoveTo(bot->GetMapId(), fbX, fbY, fbZ, false, false, false, true,
                                  MovementPriority::MOVEMENT_FORCED);
                }
                return false;
            }
        }

        // Default: pre-position behind the zone's anchor point
        float const preAngle = ctx.marker->GetAngle(myZone.GetPositionX(), myZone.GetPositionY());
        float const preX = myZone.GetPositionX() + std::cos(preAngle) * 6.5f;
        float const preY = myZone.GetPositionY() + std::sin(preAngle) * 6.5f;

        if (bot->GetDistance2d(preX, preY) > 3.0f)
        {
            botAI->Reset();
            bot->AttackStop();
            return MoveTo(bot->GetMapId(), preX, preY, myZone.GetPositionZ(), false, false, false, true,
                          MovementPriority::MOVEMENT_FORCED);
        }
        return false;
    }

    // Pinned zone has tombs again — exit fallback mode
    auto const losKey = std::make_pair(bot->GetInstanceId(), bot->GetGUID());
    s_freedFallback.erase(losKey);

    Unit* losTomb = ResolveStickyTomb(myTombs);
    if (!losTomb)
    {
        // LOS tomb died / lost mark mid-walk. If we recently issued an LOS
        // move, replay it for up to 2 seconds so the bot finishes its path
        // instead of freezing in the open until the next valid sticky tomb.
        auto it = s_lastLosMove.find(losKey);
        if (it != s_lastLosMove.end())
        {
            uint32 const now = getMSTime();
            if (getMSTimeDiff(it->second.timestampMs, now) <= 2000 &&
                bot->GetDistance2d(it->second.x, it->second.y) > 0.1f)
            {
                botAI->Reset();
                bot->AttackStop();
                return MoveTo(bot->GetMapId(), it->second.x, it->second.y, it->second.z,
                              false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            s_lastLosMove.erase(it);
        }
        return false;
    }

    float const angle = ctx.marker->GetAngle(losTomb);
    float const posX = losTomb->GetPositionX() + std::cos(angle) * 6.5f;
    float const posY = losTomb->GetPositionY() + std::sin(angle) * 6.5f;
    float const posZ = losTomb->GetPositionZ();

    float const losDist = bot->GetDistance2d(posX, posY);
    if (losDist > 0.1f)
    {
        botAI->Reset();
        bot->AttackStop();
        // Mark the tomb early (within 5yd) so the raid converges on the kill
        // target while the bot is still walking the last few yards into LOS.
        if (losDist <= 10.0f)
            HandleRtiMarking(group, groupIndex, myTombs, losTomb);

        // Stamp this LOS move so we can replay it for up to 2 seconds if the
        // tomb dies/loses mark before we arrive.
        LastLosMove& stamp = s_lastLosMove[losKey];
        stamp.timestampMs = getMSTime();
        stamp.x = posX;
        stamp.y = posY;
        stamp.z = posZ;

        return MoveTo(bot->GetMapId(), posX, posY, posZ, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }

    // Reached LOS spot — clear the replay stamp.
    s_lastLosMove.erase(losKey);

    // Bot is parked at LOS spot. Face away from the LOS tomb only when our
    // zone has no kill-candidates left (every remaining tomb is protected).
    // While extras are still up, the bot must keep facing them so it can DPS.
    if (myZoneAllProtected)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        bot->SetTarget(ObjectGuid::Empty);
        bot->SetFacingTo(losTomb->GetAngle(bot));
    }

    return HandleRtiMarking(group, groupIndex, myTombs, losTomb);
}

bool IccSindragosaFrostBombAction::CollectContext(FrostBombContext& ctx) const
{
    constexpr uint32 tombEntries[] = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};

    std::list<Unit*> units;
    float const range = 200.0f;
    Acore::AnyUnitInObjectRangeCheck check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, range);

    for (Unit* unit : units)
    {
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->HasAura(SPELL_FROST_BOMB_VISUAL))
            ctx.marker = unit;

        for (uint32 entry : tombEntries)
        {
            if (unit->GetEntry() == entry)
            {
                ctx.tombs.push_back(unit);
                break;
            }
        }
    }

    return ctx.marker && !ctx.tombs.empty();
}

int IccSindragosaFrostBombAction::ResolveGroupIndex(Group* group) const
{
    // Collect bot-only GUIDs from the current group snapshot.
    // Real players are excluded so they are never assigned a group index and
    // never become the designated marker; only bots manage icons.
    uint32 const instanceId = bot->GetInstanceId();
    std::vector<ObjectGuid> currentGuids;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (!member || !sPlayerbotsMgr.GetPlayerbotAI(member))
            continue;
        currentGuids.push_back(member->GetGUID());
    }
    std::sort(currentGuids.begin(), currentGuids.end());

    Difficulty const diff = bot->GetRaidDifficulty();
    int const groupCount = (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC) ? 3 : 2;

    // Assign any GUIDs not yet seen, preserving all existing assignments.
    // Never clear s_groupAssignments — this ensures bots that temporarily
    // drop from the group (die, get entombed) keep their original group index
    // and don't trigger a full reshuffle that migrates other bots.
    for (ObjectGuid const& guid : currentGuids)
    {
        auto const key = std::make_pair(instanceId, guid);
        if (s_groupAssignments.find(key) == s_groupAssignments.end())
        {
            // Assign to the group with the fewest members so far (this instance only)
            std::array<int, 3> counts = {};
            for (auto const& [k, idx] : s_groupAssignments)
                if (k.first == instanceId && idx < groupCount)
                    ++counts[idx];

            int minGroup = 0;
            for (int g = 1; g < groupCount; ++g)
                if (counts[g] < counts[minGroup])
                    minGroup = g;

            s_groupAssignments[key] = minGroup;
        }
    }

    auto it = s_groupAssignments.find(std::make_pair(instanceId, bot->GetGUID()));
    return it != s_groupAssignments.end() ? it->second : -1;
}

void IccSindragosaFrostBombAction::PinGroupToCurrentZone()
{
    static constexpr std::array<Position const*, 3> tombZones = {
        &ICC_SINDRAGOSA_THOMB1_POSITION,
        &ICC_SINDRAGOSA_THOMB2_POSITION,
        &ICC_SINDRAGOSA_THOMB3_POSITION
    };

    Difficulty const diff = bot->GetRaidDifficulty();
    int const groupCount =
        (diff == RAID_DIFFICULTY_25MAN_NORMAL || diff == RAID_DIFFICULTY_25MAN_HEROIC) ? 3 : 2;

    // Find the tomb the bot is inside (ice tomb spawns on top of the frozen bot).
    // Mapping the bot's group via the actual tomb position guarantees consistency
    // with SelectTombs, which classifies tombs by nearest anchor. Falling back to
    // raw bot position vs anchor would mis-pin when the boss/beacon dragged the
    // bot into a different zone than where the tomb was assigned.
    constexpr uint32 tombEntries[] = {NPC_TOMB1, NPC_TOMB2, NPC_TOMB3, NPC_TOMB4};

    std::list<Unit*> units;
    float const range = 15.0f;
    Acore::AnyUnitInObjectRangeCheck check(bot, range);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, units, check);
    Cell::VisitObjects(bot, searcher, range);

    Unit* myTomb = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (Unit* unit : units)
    {
        if (!unit || !unit->IsAlive())
            continue;
        bool isTomb = false;
        for (uint32 entry : tombEntries)
        {
            if (unit->GetEntry() == entry)
            {
                isTomb = true;
                break;
            }
        }
        if (!isTomb)
            continue;
        float const d = bot->GetExactDist2d(unit);
        if (d < minDist)
        {
            minDist = d;
            myTomb = unit;
        }
    }

    int bestGroup = 0;
    float bestDist = std::numeric_limits<float>::max();

    // Anchor-from-tomb path (preferred): map nearest tomb -> nearest anchor -> group.
    if (myTomb)
    {
        for (int g = 0; g < groupCount; ++g)
        {
            int const zoneIdx = (groupCount == 2) ? (g == 0 ? 0 : 2) : g;
            float const d = myTomb->GetExactDist2d(*tombZones[zoneIdx]);
            if (d < bestDist)
            {
                bestDist = d;
                bestGroup = g;
            }
        }
    }
    else
    {
        // Fallback: use bot position if no tomb visible (shouldn't happen while tombed,
        // but guard against odd states).
        for (int g = 0; g < groupCount; ++g)
        {
            int const zoneIdx = (groupCount == 2) ? (g == 0 ? 0 : 2) : g;
            float const d = bot->GetExactDist2d(*tombZones[zoneIdx]);
            if (d < bestDist)
            {
                bestDist = d;
                bestGroup = g;
            }
        }
    }

    s_groupAssignments[std::make_pair(bot->GetInstanceId(), bot->GetGUID())] = bestGroup;
}

std::vector<Unit*> IccSindragosaFrostBombAction::SelectTombs(std::vector<Unit*> const& tombs, int groupIndex, int groupCount) const
{
    if (tombs.empty())
        return {};

    // Map group index to its intended zone anchor
    static constexpr std::array<Position const*, 3> tombZones = {
        &ICC_SINDRAGOSA_THOMB1_POSITION,
        &ICC_SINDRAGOSA_THOMB2_POSITION,
        &ICC_SINDRAGOSA_THOMB3_POSITION
    };
    int const zoneIdx = (groupCount == 2) ? (groupIndex == 0 ? 0 : 2) : groupIndex;

    static constexpr float MAX_ZONE_RADIUS = 3.0f;
    std::vector<Unit*> zoneTombs;
    for (Unit* tomb : tombs)
    {
        int closestZone = 0;
        float closestDist = tomb->GetExactDist2d(*tombZones[0]);
        for (int z = 1; z < 3; ++z)
        {
            // Skip zone 1 (THOMB2) in 2-group mode — it is never assigned
            if (groupCount == 2 && z == 1)
                continue;
            float d = tomb->GetExactDist2d(*tombZones[z]);
            if (d < closestDist)
            {
                closestDist = d;
                closestZone = z;
            }
        }
        if (closestZone != zoneIdx)
            continue;
        if (closestDist > MAX_ZONE_RADIUS)
            continue;
        zoneTombs.push_back(tomb);
    }
    return zoneTombs;
}

Unit* IccSindragosaFrostBombAction::ResolveStickyTomb(std::vector<Unit*> const& myTombs)
{
    // Keep the previously assigned tomb while it is still a valid member of
    // this group's zone. This avoids the cascading reassignment where a tomb's
    // HP fluctuation causes every bot to flip to a different LOS spot each tick.
    auto const key = std::make_pair(bot->GetInstanceId(), bot->GetGUID());
    auto it = s_tombAssignments.find(key);
    if (it != s_tombAssignments.end())
    {
        for (Unit* tomb : myTombs)
        {
            if (tomb->GetGUID() == it->second && tomb->IsAlive())
                return tomb;
        }
    }

    // No valid sticky — pick the tomb nearest to the bot so the first
    // assignment snaps to where the bot already stands.
    Unit* nearest = nullptr;
    float minDist = std::numeric_limits<float>::max();
    for (Unit* tomb : myTombs)
    {
        if (!tomb->IsAlive())
            continue;
        float const d = bot->GetExactDist2d(tomb);
        if (d < minDist)
        {
            minDist = d;
            nearest = tomb;
        }
    }

    if (nearest)
        s_tombAssignments[key] = nearest->GetGUID();
    else
        s_tombAssignments.erase(key);

    return nearest;
}

bool IccSindragosaFrostBombAction::HandleRtiMarking(Group* group, int groupIndex, std::vector<Unit*> const& myTombs, Unit* losTomb)
{
    constexpr uint8 SKULL_ICON = 7;
    constexpr uint8 CROSS_ICON = 6;
    constexpr uint8 STAR_ICON = 0;
    constexpr float TOMB_STOP_HP_PCT = 40.0f;
    constexpr float TOMB_STOP_HP_PCT_10_MAN = 60.0f;

    Difficulty const diff = bot->GetRaidDifficulty();
    bool const is10Man = (diff == RAID_DIFFICULTY_10MAN_NORMAL || diff == RAID_DIFFICULTY_10MAN_HEROIC);

    uint8 iconIndex = 0;
    std::string rtiValue;

    switch (groupIndex)
    {
        case 0: iconIndex = SKULL_ICON; rtiValue = "skull"; break;
        case 1: iconIndex = CROSS_ICON; rtiValue = "cross"; break;
        case 2: iconIndex = STAR_ICON;  rtiValue = "star";  break;
        default: return false;
    }

    context->GetValue<std::string>("rti")->Set(rtiValue);

    Unit* currentIconUnit = botAI->GetUnit(group->GetTargetIcon(iconIndex));

    Unit* tombToMark = nullptr;

    // Prefer to keep the current icon if it is still a valid extra in our zone.
    if (currentIconUnit && currentIconUnit->IsAlive() && currentIconUnit != losTomb &&
        !(is10Man && currentIconUnit->GetHealthPct() <= TOMB_STOP_HP_PCT_10_MAN))
    {
        for (Unit* tomb : myTombs)
        {
            if (tomb == currentIconUnit)
            {
                tombToMark = currentIconUnit;
                break;
            }
        }
    }

    // No valid current icon — pick a new extra deterministically (lowest GUID)
    // so every bot in the group agrees on the same choice.
    if (!tombToMark)
    {
        ObjectGuid bestGuid;
        for (Unit* tomb : myTombs)
        {
            if (!tomb || !tomb->IsAlive() || tomb == losTomb)
                continue;
            if (is10Man && tomb->GetHealthPct() <= TOMB_STOP_HP_PCT_10_MAN)
                continue;
            if (!tombToMark || tomb->GetGUID() < bestGuid)
            {
                tombToMark = tomb;
                bestGuid = tomb->GetGUID();
            }
        }
    }

    // No extras left — DPS the sticky down to the stop threshold
    float const stickyStopPct = is10Man ? TOMB_STOP_HP_PCT_10_MAN : TOMB_STOP_HP_PCT;
    if (!tombToMark && losTomb && losTomb->IsAlive() && losTomb->GetHealthPct() > stickyStopPct)
        tombToMark = losTomb;

    if (!tombToMark)
    {
        // All tombs at/below threshold — clear icon and stand idle
        ObjectGuid const currentIcon = group->GetTargetIcon(iconIndex);
        if (!currentIcon.IsEmpty())
            group->SetTargetIcon(iconIndex, bot->GetGUID(), ObjectGuid::Empty);

        bot->AttackStop();
        return true;
    }

    if (!currentIconUnit || !currentIconUnit->IsAlive() || currentIconUnit != tombToMark)
        group->SetTargetIcon(iconIndex, bot->GetGUID(), tombToMark->GetGUID());

    // Let combat actions fire so this bot DPSes the marked tomb
    return false;
}