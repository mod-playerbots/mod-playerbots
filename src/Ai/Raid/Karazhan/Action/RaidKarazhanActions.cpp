#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"

using namespace KarazhanHelpers;

// Trash

// Mana Warps blow up when they die for massive raid damage
// But they cannot cast the ability if they are stunned
bool ManaWarpStunCreatureBeforeWarpBreachAction::Execute(Event /*event*/)
{
    Unit* manaWarp = GetFirstAliveUnitByEntry(botAI, NPC_MANA_WARP);
    if (!manaWarp)
        return false;

    static const std::array<const char*, 8> spells =
    {
        "bash",
        "concussion blow",
        "hammer of justice",
        "kidney shot",
        "maim",
        "revenge stun",
        "shadowfury",
        "shockwave"
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, manaWarp))
            return botAI->CastSpell(spell, manaWarp);
    }

    return false;
}

// Attumen the Huntsman

// Prioritize Midnight until Attumen is mounted
bool AttumenTheHuntsmanMarkTargetAction::Execute(Event /*event*/)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (attumenMounted)
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithStar(bot, attumenMounted);

        SetRtiTarget(botAI, "star", attumenMounted);

        if (bot->GetTarget() != attumenMounted->GetGUID())
        {
            bot->SetTarget(attumenMounted->GetGUID());
            return Attack(attumenMounted);
        }
    }
    else if (Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight"))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithStar(bot, midnight);

        if (!botAI->IsAssistTankOfIndex(bot, 0))
        {
            SetRtiTarget(botAI, "star", midnight);

            if (bot->GetTarget() != midnight->GetGUID())
            {
                bot->SetTarget(midnight->GetGUID());
                return Attack(midnight);
            }
        }
    }

    return false;
}

// Off tank should move Attumen out of the way so he doesn't cleave bots
bool AttumenTheHuntsmanSplitBossesAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    if (!attumen)
        return false;

    MarkTargetWithSquare(bot, attumen);
    SetRtiTarget(botAI, "square", attumen);

    if (bot->GetVictim() != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() == bot && midnight->GetVictim() != bot)
    {
        const float safeDistance = 6.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (nearestPlayer && attumen->GetExactDist2d(nearestPlayer) < safeDistance)
            return MoveFromGroup(safeDistance + 2.0f);
    }

    return false;
}

// Stack behind mounted Attumen (inside minimum range of Berserker Charge)
bool AttumenTheHuntsmanStackBehindAction::Execute(Event /*event*/)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    const float distanceBehind = botAI->IsRanged(bot) ? 6.0f : 2.0f;
    float orientation = attumenMounted->GetOrientation() + M_PI;
    float rearX = attumenMounted->GetPositionX() + std::cos(orientation) * distanceBehind;
    float rearY = attumenMounted->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (bot->GetExactDist2d(rearX, rearY) > 1.0f)
    {
        return MoveTo(KARAZHAN_MAP_ID, rearX, rearY, attumenMounted->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Reset timer for bots to pause DPS when Attumen mounts Midnight
bool AttumenTheHuntsmanManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    const uint32 instanceId = midnight->GetMap()->GetInstanceId();

    if (midnight && midnight->GetHealth() == midnight->GetMaxHealth())
        attumenDpsWaitTimer.erase(instanceId);

    // Midnight is still present as a separate (invisible) unit after Attumen mounts
    // So this block can be reached
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    const time_t now = std::time(nullptr);

    if (attumenMounted)
        attumenDpsWaitTimer.try_emplace(instanceId, now);

    return false;
}

// Moroes

bool MoroesMainTankAttackBossAction::Execute(Event /*event*/)
{
    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    if (!moroes)
        return false;

    MarkTargetWithCircle(bot, moroes);
    SetRtiTarget(botAI, "circle", moroes);

    if (bot->GetVictim() != moroes)
        return Attack(moroes);

    return false;
}

// Mark targets with skull in the recommended kill order
bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = GetFirstAliveUnit({dorothea, catriona, keira, rafe, robin, crispin});

    if (target)
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, target);

        SetRtiTarget(botAI, "skull", target);
    }

    return false;
}

// Maiden of Virtue

// Tank the boss in the center of the room
// Move to healers after Repentenace to break the stun
bool MaidenOfVirtueMoveBossToHealerAction::Execute(Event /*event*/)
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    if (bot->GetVictim() != maiden)
        return Attack(maiden);

    Unit* healer = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsHeal(member) ||
                !member->HasAura(SPELL_REPENTANCE))
                continue;

            healer = member;
            break;
        }
    }

    if (healer)
    {
        float angle = healer->GetOrientation();
        float targetX = healer->GetPositionX() + std::cos(angle) * 6.0f;
        float targetY = healer->GetPositionY() + std::sin(angle) * 6.0f;
        {
            return MoveTo(KARAZHAN_MAP_ID, targetX, targetY, healer->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    const Position& position = MAIDEN_OF_VIRTUE_BOSS_POSITION;
    const float maxDistance = 2.0f;
    float distanceToPosition = maiden->GetExactDist2d(position);
    if (distanceToPosition > maxDistance)
    {
        float dX = position.GetPositionX() - maiden->GetPositionX();
        float dY = position.GetPositionY() - maiden->GetPositionY();
        float mX = position.GetPositionX() + (dX / distanceToPosition) * maxDistance;
        float mY = position.GetPositionY() + (dY / distanceToPosition) * maxDistance;
        {
            return MoveTo(KARAZHAN_MAP_ID, mX, mY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS between the pillars
bool MaidenOfVirtuePositionRangedAction::Execute(Event /*event*/)
{
    const uint8 maxIndex = 7;
    uint8 index = 0;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (member == bot)
                break;

            if (index >= maxIndex)
            {
                index = 0;
                continue;
            }
            index++;
        }
    }

    const Position& position = MAIDEN_OF_VIRTUE_RANGED_POSITION[index];
    if (bot->GetExactDist2d(position) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// The Big Bad Wolf

// Tank the boss at the front left corner of the stage
bool BigBadWolfPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    if (bot->GetVictim() != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() == bot)
    {
        const Position& position = BIG_BAD_WOLF_BOSS_POSITION;
        float distanceToPosition = wolf->GetExactDist2d(position);

        if (distanceToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - wolf->GetPositionX();
            float dY = position.GetPositionY() - wolf->GetPositionY();
            float moveDist = std::min(5.0f, distanceToPosition);
            float moveX = wolf->GetPositionX() + (dX / distanceToPosition) * moveDist;
            float moveY = wolf->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(KARAZHAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Run away, little girl, run away
bool BigBadWolfRunAwayFromBossAction::Execute(Event /*event*/)
{
    const ObjectGuid botGuid = bot->GetGUID();
    uint8 index = bigBadWolfRunIndex.count(botGuid) ? bigBadWolfRunIndex[botGuid] : 0;

    while (bot->GetExactDist2d(BIG_BAD_WOLF_RUN_POSITION[index].GetPositionX(),
                               BIG_BAD_WOLF_RUN_POSITION[index].GetPositionY()) < 1.0f)
    {
        index = (index + 1) % 4;
    }

    bigBadWolfRunIndex[botGuid] = index;
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    const Position& position = BIG_BAD_WOLF_RUN_POSITION[index];
    return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Romulo and Julianne

// Keep the couple within 10% HP of each other
bool RomuloAndJulianneMarkTargetAction::Execute(Event /*event*/)
{
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!romulo)
        return false;

    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!julianne)
        return false;

    Unit* target = nullptr;
    const float maxPctDifference = 10.0f;

    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() || julianne->GetHealthPct() < 1.0f)
        target = romulo;
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() || romulo->GetHealthPct() < 1.0f)
        target = julianne;
    else
        target = (romulo->GetHealthPct() >= julianne->GetHealthPct()) ? romulo : julianne;

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// The Wizard of Oz

// Mark targets with skull in the recommended kill order
bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = GetFirstAliveUnit({dorothee, tito, roar, strawman, tinhead, crone});

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Mages spam Scorch on Strawman to disorient him
bool WizardOfOzScorchStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    if (strawman && botAI->CanCastSpell("scorch", strawman))
        return botAI->CastSpell("scorch", strawman);

    return false;
}

// The Curator

// Prioritize destroying Astral Flares
bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = AI_VALUE2(Unit*, "find target", "astral flare");
    if (!flare)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
        MarkTargetWithSkull(bot, flare);

    SetRtiTarget(botAI, "skull", flare);

    return false;
}

// Tank the boss in the center of the hallway near the Guardian's Library
// Main tank and off tank will attack the boss; others will focus on Astral Flares
bool TheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    MarkTargetWithCircle(bot, curator);
    SetRtiTarget(botAI, "circle", curator);

    if (bot->GetVictim() != curator)
        return Attack(curator);

    if (curator->GetVictim() == bot)
    {
        const Position& position = THE_CURATOR_BOSS_POSITION;
        float distanceToPosition = curator->GetExactDist2d(position);

        if (distanceToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - curator->GetPositionX();
            float dY = position.GetPositionY() - curator->GetPositionY();
            float moveDist = std::min(10.0f, distanceToPosition);
            float moveX = position.GetPositionX() + (dX / distanceToPosition) * moveDist;
            float moveY = position.GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(KARAZHAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS to avoid Arcing Sear damage
bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    const float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

// Terestian Illhoof

// Prioritize (1) Demon Chains, (2) Kil'rek, (3) Illhoof
bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    Unit* demonChains = GetFirstAliveUnitByEntry(botAI, NPC_DEMON_CHAINS);
    Unit* kilrek = GetFirstAliveUnitByEntry(botAI, NPC_KILREK);
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    Unit* target = GetFirstAliveUnit({demonChains, kilrek, illhoof});
    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Shade of Aran

// Run to the edge of the room to avoid Arcane Explosion
bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    const float safeDistance = 20.0f;
    float distance = bot->GetDistance2d(aran);
    if (distance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(aran, safeDistance - distance);
    }

    return false;
}

// I will not move when Flame Wreath is cast or the raid blows up
bool ShadeOfAranStopMovingDuringFlameWreathAction::Execute(Event /*event*/)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);

    if (bot->isMoving())
    {
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        return true;
    }

    return false;
}

// Mark Conjured Elementals with skull so DPS can burn them down
bool ShadeOfAranMarkConjuredElementalAction::Execute(Event /*event*/)
{
    Unit* elemental = GetFirstAliveUnitByEntry(botAI, NPC_CONJURED_ELEMENTAL);

    if (elemental)
        MarkTargetWithSkull(bot, elemental);

    return false;
}

// Don't get closer than 11 yards to Aran to avoid counterspell
// Don't get farther than 15 yards from Aran to avoid getting stuck in alcoves
bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const float minDist = 11.0f;
    const float maxDist = 15.0f;
    const float ringIncrement = M_PI / 8;
    const float distIncrement = 0.5f;

    float bestX = 0, bestY = 0, bestMoveDist = std::numeric_limits<float>::max();
    bool found = false;

    for (float dist = minDist; dist <= maxDist; dist += distIncrement)
    {
        for (float angle = 0; angle < 2 * M_PI; angle += ringIncrement)
        {
            float x = aran->GetPositionX() + std::cos(angle) * dist;
            float y = aran->GetPositionY() + std::sin(angle) * dist;

            bool tooClose = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member == bot || !member->IsAlive())
                    continue;
                if (member->GetExactDist2d(x, y) < 3.0f)
                {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose)
                continue;

            float moveDist = bot->GetExactDist2d(x, y);
            if (moveDist < bestMoveDist)
            {
                bestMoveDist = moveDist;
                bestX = x;
                bestY = y;
                found = true;
            }
        }
    }

    if (found && bestMoveDist > 0.5f)
    {
        return MoveTo(KARAZHAN_MAP_ID, bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// Netherspite

// One tank bot per phase will dance in and out of the red beam (5 seconds in, 5 seconds out)
// Tank bots will ignore void zones--their positioning is too important to risk losing beam control
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    if (!redPortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == redBlocker);

    auto it = _wasBlockingRedBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingRedBeam.end()) ? it->second : false;

    Position beamPos = GetPositionOnBeam(netherspite, redPortal, 18.0f);

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_red", "%player is moving to block the red beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingRedBeam[botGuid] = true;

        const uint8 intervalSecs = 5;
        if (std::time(nullptr) - redBeamMoveTimer[botGuid] >= intervalSecs)
        {
            lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
            redBeamMoveTimer[botGuid] = std::time(nullptr);
        }
        if (!lastBeamMoveSideways[botGuid])
            return MoveTo(KARAZHAN_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        else
        {
            float length = netherspite->GetExactDist2d(redPortal);
            if (length == 0.0f)
                return false;

            float dx = (redPortal->GetPositionX() - netherspite->GetPositionX()) / length;
            float dy = (redPortal->GetPositionY() - netherspite->GetPositionY()) / length;
            float perpDx = -dy;
            float perpDy = dx;
            float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
            float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
            float sideZ = beamPos.GetPositionZ();

            return MoveTo(KARAZHAN_MAP_ID, sideX, sideY, sideZ, false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    _wasBlockingRedBeam[botGuid] = false;
    return false;
}

Position NetherspiteBlockRedBeamAction::GetPositionOnBeam(Unit* netherspite, Unit* portal, float distanceFromBoss)
{
    float bx = netherspite->GetPositionX();
    float by = netherspite->GetPositionY();
    float bz = netherspite->GetPositionZ();
    float px = portal->GetPositionX();
    float py = portal->GetPositionY();

    float dx = px - bx;
    float dy = py - by;
    float length = netherspite->GetExactDist2d(px, py);
    if (length == 0.0f)
        return Position(bx, by, bz);

    dx /= length;
    dy /= length;
    float targetX = bx + dx * distanceFromBoss;
    float targetY = by + dy * distanceFromBoss;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
}

// Two non-Rogue/Warrior DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockBlueBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    if (!bluePortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == blueBlocker);

    auto it = _wasBlockingBlueBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingBlueBeam.end()) ? it->second : false;

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_leaving_blue", "%player is leaving the blue beam--next blocker up!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
        _wasBlockingBlueBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_blue", "%player is moving to block the blue beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingBlueBeam[botGuid] = true;

        float idealDistance = botAI->IsRanged(bot) ? 25.0f : 18.0f;
        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = bluePortal->GetPositionX();
        float py = bluePortal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = netherspite->GetExactDist2d(bluePortal);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f)
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            if (!IsSafePosition(candidateX, candidateY, candidateZ, voidZones, 4.0f))
                continue;

            float distToIdeal = fabs(dist - idealDistance);
            if (!found || distToIdeal < bestDist)
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    _wasBlockingBlueBeam[botGuid] = false;
    return false;
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one rogue or DPS warrior bot will block the green beam for an entire phase (if they begin the phase as the blocker)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockGreenBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
    if (!greenPortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == greenBlocker);

    auto it = _wasBlockingGreenBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingGreenBeam.end()) ? it->second : false;

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_leaving_green", "%player is leaving the green beam--next blocker up!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
        _wasBlockingGreenBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_green", "%player is moving to block the green beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingGreenBeam[botGuid] = true;

        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = greenPortal->GetPositionX();
        float py = greenPortal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = netherspite->GetExactDist2d(greenPortal);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f)
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            if (!IsSafePosition(candidateX, candidateY, candidateZ, voidZones, 4.0f))
                continue;

            float distToIdeal = fabs(dist - 18.0f);
            if (!found || distToIdeal < bestDist)
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    _wasBlockingGreenBeam[botGuid] = false;
    return false;
}

// All bots not currently blocking a beam will avoid beams and void zones
bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

    bool nearVoidZone = !IsSafePosition(bot->GetPositionX(), bot->GetPositionY(),
                                        bot->GetPositionZ(), voidZones, 4.0f);

    std::vector<BeamAvoid> beams;
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    if (redPortal)
    {
        float length = netherspite->GetExactDist2d(redPortal);
        beams.push_back({redPortal, 0.0f, length});
    }

    if (bluePortal)
    {
        float length = netherspite->GetExactDist2d(bluePortal);
        beams.push_back({bluePortal, 0.0f, length});
    }

    if (greenPortal)
    {
        float length = netherspite->GetExactDist2d(greenPortal);
        beams.push_back({greenPortal, 0.0f, length});
    }

    bool nearBeam = !IsAwayFromBeams(bot->GetPositionX(), bot->GetPositionY(), beams, netherspite);

    if (!nearVoidZone && !nearBeam)
        return false;

    const float minMoveDist = 2.0f;
    const float minMoveDistSq = minMoveDist * minMoveDist;
    const float maxSearchDist = 30.0f, stepAngle = M_PI/18.0f, stepDist = 0.5f;
    float netherspiteZ = netherspite->GetPositionZ();
    Position bestCandidate;
    float bestDistSq = std::numeric_limits<float>::max();
    bool found = false;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();

    for (float angle = 0; angle < 2 * M_PI; angle += stepAngle)
    {
        for (float dist = 2.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = botX + std::cos(angle) * dist;
            float cy = botY + std::sin(angle) * dist;
            float cz = netherspiteZ;

            if (!IsSafePosition(cx, cy, cz, voidZones, 4.0f) ||
                !IsAwayFromBeams(cx, cy, beams, netherspite))
                continue;

            float dx = cx - botX;
            float dy = cy - botY;
            float moveDistSq = dx*dx + dy*dy;
            if (moveDistSq < minMoveDistSq)
                continue;

            if (!found || moveDistSq < bestDistSq)
            {
                bestCandidate = Position(cx, cy, cz);
                bestDistSq = moveDistSq;
                found = true;
            }
        }
    }

    if (found)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
                      bestCandidate.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool NetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(
     float x, float y, const std::vector<BeamAvoid>& beams, Unit* netherspite)
{
    for (auto const& beam : beams)
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = netherspite->GetExactDist2d(beam.portal);

        if (length == 0.0f)
            continue;

        dx /= length; dy /= length;
        float botdx = x - bx, botdy = y - by;
        float distanceAlongBeam = (botdx * dx + botdy * dy);
        float beamX = bx + dx * distanceAlongBeam, beamY = by + dy * distanceAlongBeam;
        float distToBeam = sqrt((x - beamX) * (x - beamX) + (y - beamY) * (y - beamY));

        if (distToBeam < 5.0f && distanceAlongBeam > beam.minDist && distanceAlongBeam < beam.maxDist)
            return false;
    }

    return true;
}

bool NetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event /*event*/)
{
    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

    for (Unit* vz : voidZones)
    {
        if (vz->GetEntry() == NPC_VOID_ZONE && bot->GetExactDist2d(vz) < 4.0f)
            return FleePosition(vz->GetPosition(), 4.0f);
    }

    return false;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    const uint32 instanceId = netherspite->GetMap()->GetInstanceId();
    const ObjectGuid botGuid = bot->GetGUID();
    const time_t now = std::time(nullptr);

    // DpsWaitTimer is for pausing DPS during phase transitions
    // redBeamMoveTimer and lastBeamMoveSideways are for tank dancing in/out of the red beam
    if (netherspite->GetHealth() == netherspite->GetMaxHealth() &&
        !netherspite->HasAura(SPELL_GREEN_BEAM_HEAL))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.insert_or_assign(instanceId, now);

        if (botAI->IsTank(bot) && !bot->HasAura(SPELL_RED_BEAM_DEBUFF))
        {
            redBeamMoveTimer.erase(botGuid);
            lastBeamMoveSideways.erase(botGuid);
        }
    }
    else if (netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.erase(instanceId);

        if (botAI->IsTank(bot))
        {
            redBeamMoveTimer.erase(botGuid);
            lastBeamMoveSideways.erase(botGuid);
        }
    }
    else if (!netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.try_emplace(instanceId, now);

        if (botAI->IsTank(bot) && bot->HasAura(SPELL_RED_BEAM_DEBUFF))
        {
            redBeamMoveTimer.try_emplace(botGuid, now);
            lastBeamMoveSideways.try_emplace(botGuid, false);
        }
    }

    return false;
}

// Move away from the boss to avoid Shadow Nova when Enfeebled
// Do not cross within Infernal Hellfire radius while doing so
bool PrinceMalchezaarEnfeebledAvoidHazardAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float minSafeBossDistance = 34.0f;
    const float minSafeBossDistanceSq = minSafeBossDistance * minSafeBossDistance;
    const float maxSafeBossDistance = 60.0f;
    const float safeInfernalDistance = 23.0f;
    const float distIncrement = 0.5f;
    const uint8 numAngles = 64;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();
    float bestMoveDistSq = std::numeric_limits<float>::max();
    float bestDestX = 0.0f, bestDestY = 0.0f, bestDestZ = bz;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2 * M_PI * i) / numAngles;
        float dx = std::cos(angle);
        float dy = std::sin(angle);

        for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += distIncrement)
        {
            float x = malchezaarX + dx * dist;
            float y = malchezaarY + dy * dist;
            float destZ = malchezaarZ;
            float destX = x, destY = y;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                continue;

            float ddx = destX - malchezaarX;
            float ddy = destY - malchezaarY;
            float distFromBossSq = ddx*ddx + ddy*ddy;
            if (distFromBossSq < minSafeBossDistanceSq)
                continue;

            bool pathSafe = IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ),
                                               infernals, safeInfernalDistance, distIncrement);
            float mdx = destX - bx;
            float mdy = destY - by;
            float moveDistSq = mdx*mdx + mdy*mdy;

            if (pathSafe && moveDistSq < bestMoveDistSq)
            {
                bestMoveDistSq = moveDistSq;
                bestDestX = destX;
                bestDestY = destY;
                bestDestZ = destZ;
                found = true;
            }
        }
    }

    if (found)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Move away from infernals while staying within range of the boss
// Prioritize finding a safe path to the new location, but will fallback to just finding a safe location if needed
bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 23.0f;
    const float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    const float maxSafeBossDistance = 35.0f;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float dx = bx - infernal->GetPositionX();
        float dy = by - infernal->GetPositionY();
        float infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        const float distIncrement = 0.5f;
        const uint8 numAngles = 64;

        // 1. Try to find a safe position with a safe path
        found = TryFindSafePositionWithSafePath(bot, bx, by, bz, malchezaarX, malchezaarY, malchezaarZ,
                infernals, safeInfernalDistance, distIncrement, numAngles, maxSafeBossDistance,
                true, bestDestX, bestDestY, bestDestZ);

        // 2. Fallback: try to find a safe position (ignore path safety)
        if (!found)
        {
            found = TryFindSafePositionWithSafePath(bot, bx, by, bz, malchezaarX, malchezaarY, malchezaarZ,
                    infernals, safeInfernalDistance, distIncrement, numAngles, maxSafeBossDistance,
                    false, bestDestX, bestDestY, bestDestZ);
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

// This is similar to the non-tank avoid infernal action, but the movement is based on the bot's location
// And the safe distance from infernals is larger to give melee more room to maneuver
bool PrinceMalchezaarMainTankMovementAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    if (bot->GetVictim() != malchezaar)
        return Attack(malchezaar);

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 30.0f;
    const float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    const float maxSampleDist = 75.0f;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float dx = bx - infernal->GetPositionX();
        float dy = by - infernal->GetPositionY();
        float infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        const float distIncrement = 0.5f;
        const uint8 numAngles = 64;

        // 1. Try to find a safe position with a safe path
        found = TryFindSafePositionWithSafePath( bot, bx, by, bz, bx, by, bz,
                infernals, safeInfernalDistance, distIncrement, numAngles, maxSampleDist,
                true, bestDestX, bestDestY, bestDestZ);

        // 2. Fallback: try to find a safe position (ignore path safety)
        if (!found)
        {
            found = TryFindSafePositionWithSafePath( bot, bx, by, bz, bx, by, bz,
                    infernals, safeInfernalDistance, distIncrement, numAngles, maxSampleDist,
                    false, bestDestX, bestDestY, bestDestZ);
        }

        if (found)
        {
            bot->AttackStop();
            return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    return false;
}

// The tank position is near the Southeastern area of the Master's Terrace
// The tank moves Nightbane into position in two steps to try to get Nightbane to face sideways to the raid
bool NightbaneGroundPhasePositionBossAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    MarkTargetWithSkull(bot, nightbane);

    if (bot->GetVictim() != nightbane)
        return Attack(nightbane);

    const ObjectGuid botGuid = bot->GetGUID();
    uint8 step = nightbaneTankStep.count(botGuid) ? nightbaneTankStep[botGuid] : 0;

    if (nightbane->GetVictim() == bot)
    {
        const Position tankPositions[2] =
        {
            NIGHTBANE_TRANSITION_BOSS_POSITION,
            NIGHTBANE_FINAL_BOSS_POSITION
        };
        const Position& position = tankPositions[step];
        const float maxDistance = 0.5f;
        float distanceToTarget = bot->GetExactDist2d(position);

        if ((distanceToTarget > maxDistance) && bot->IsWithinMeleeRange(nightbane))
            return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, true);

        if (step == 0 && distanceToTarget <= maxDistance)
            nightbaneTankStep[botGuid] = 1;

        if (step == 1 && distanceToTarget <= maxDistance)
        {
            float orientation = atan2(nightbane->GetPositionY() - bot->GetPositionY(),
                                      nightbane->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
    }

    return false;
}

// Ranged bots rotate between 3 positions to avoid standing in Charred Earth, which lasts for
// 30s and has a minimum cooldown of 18s (so there can be 2 active at once)
// Ranged positions are near the Northeastern door to the tower
bool NightbaneGroundPhaseRotateRangedPositionsAction::Execute(Event /*event*/)
{
    const ObjectGuid botGuid = bot->GetGUID();
    uint8 index = nightbaneRangedStep.count(botGuid) ? nightbaneRangedStep[botGuid] : 0;

    const Position rangedPositions[3] =
    {
        NIGHTBANE_RANGED_POSITION1,
        NIGHTBANE_RANGED_POSITION2,
        NIGHTBANE_RANGED_POSITION3
    };
    const Position& position = rangedPositions[index];
    const float maxDistance = 2.0f;
    float distanceToTarget = bot->GetExactDist2d(position);

    if (distanceToTarget <= maxDistance &&
        bot->HasAura(SPELL_CHARRED_EARTH) && !bot->HasAura(SPELL_BELLOWING_ROAR))
    {
        index = (index + 1) % 3;
        nightbaneRangedStep[botGuid] = index;
        const Position& newPosition = rangedPositions[index];
        float newDistanceToTarget = bot->GetExactDist2d(newPosition);
        if (newDistanceToTarget > maxDistance)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, newPosition.GetPositionX(), newPosition.GetPositionY(), newPosition.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        return false;
    }

    if (distanceToTarget > maxDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// For countering Bellowing Roars during the ground phase
bool NightbaneCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
        return botAI->CastSpell("fear ward", mainTank);

    return false;
}

// Put pets on passive during the flight phase so they don't try to chase Nightbane off the map
bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z && pet->GetReactState() == REACT_PASSIVE)
        pet->SetReactState(REACT_DEFENSIVE);

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z && pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }

    return false;
}

// 1. Stack at the "Flight Stack Position" near Nightbane so he doesn't use Fireball Barrage
// 2. Once Rain of Bones hits, the whole party moves to a new stack position
// This action lasts for the first 35 seconds of the flight phase, after which Nightbane gets
// ready to land, and the player will need to lead the bots over near the ground phase position
bool NightbaneFlightPhaseMovementAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
        return false;

    MarkTargetWithMoon(bot, nightbane);

    Unit* botTarget = botAI->GetUnit(bot->GetTarget());
    if (botTarget && botTarget == nightbane)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
    }

    const ObjectGuid botGuid = bot->GetGUID();
    bool hasRainOfBones = bot->HasAura(SPELL_RAIN_OF_BONES);

    if (hasRainOfBones)
        nightbaneRainOfBonesHit[botGuid] = true;

    float destX, destY, destZ;
    if (nightbaneRainOfBonesHit[botGuid])
    {
        destX = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionX();
        destY = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionY();
        destZ = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionZ();
    }
    else
    {
        destX = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionX();
        destY = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionY();
        destZ = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionZ();
    }

    if (bot->GetExactDist2d(destX, destY) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const ObjectGuid botGuid = bot->GetGUID();
    const time_t now = std::time(nullptr);

    // Erase DPS wait timer and tank and ranged position tracking on encounter reset
    if (nightbane->GetHealth() == nightbane->GetMaxHealth())
    {
        if (botAI->IsMainTank(bot))
            nightbaneTankStep.erase(botGuid);

        if (botAI->IsRanged(bot))
            nightbaneRangedStep.erase(botGuid);

        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            nightbaneDpsWaitTimer.erase(instanceId);
    }
    // Erase flight phase timer and Rain of Bones tracker on ground phase and start DPS wait timer
    else if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
    {
        nightbaneRainOfBonesHit.erase(botGuid);

        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
        {
            nightbaneFlightPhaseStartTimer.erase(instanceId);
            nightbaneDpsWaitTimer.try_emplace(instanceId, now);
        }
    }
    // Erase DPS wait timer and tank and ranged position tracking and start flight phase timer
    // at beginning of flight phase
    else if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
    {
        if (botAI->IsMainTank(bot))
            nightbaneTankStep.erase(botGuid);

        if (botAI->IsRanged(bot))
            nightbaneRangedStep.erase(botGuid);

        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
        {
            nightbaneDpsWaitTimer.erase(instanceId);
            nightbaneFlightPhaseStartTimer.try_emplace(instanceId, now);
        }
    }

    return false;
}
