#include "ICCMultipliers.h"

#include "ChooseTargetActions.h"
#include "DKActions.h"
#include "DruidActions.h"
#include "DruidBearActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PaladinActions.h"
#include "PriestActions.h"
#include "ICCActions.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"
#include "UseMeetingStoneAction.h"
#include "WarriorActions.h"
#include "PlayerbotAI.h"
#include "ICCTriggers.h"
#include "ICCScripts.h"

// LK global variables
namespace
{
std::map<ObjectGuid, uint32> g_plagueTimes;
std::map<ObjectGuid, bool> g_allowCure;
std::mutex g_plagueMutex; // Lock before accessing shared variables
}

// Lady Deathwhisper
float IccLadyDeathwhisperMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "lady deathwhisper");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<FollowAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    static constexpr uint32 VENGEFUL_SHADE_ID = NPC_SHADE;

    // Get the nearest hostile NPCs
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    // Allow the IccShadeLadyDeathwhisperAction to run
    if (dynamic_cast<IccShadeLadyDeathwhisperAction*>(action))
        return 1.0f;

    for (auto const& npcGuid : npcs)
    {
        Unit* shade = botAI->GetUnit(npcGuid);

        if (!shade || shade->GetEntry() != VENGEFUL_SHADE_ID)
            continue;

        if (!shade->GetVictim() || shade->GetVictim()->GetGUID() != bot->GetGUID())
            continue;

        return 0.0f; // Cancel all other actions when we need to handle Vengeful Shade
    }

    return 1.0f;
}

// dbs
float IccAddsDbsMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "deathbringer saurfang");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
        dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
        dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<FleeAction*>(action) || dynamic_cast<CastArmyOfTheDeadAction*>(action))
        return 0.0f;

    if (botAI->IsRanged(bot))
        if (dynamic_cast<ReachSpellAction*>(action))
            return 0.0f;

    if (botAI->IsTank(bot))
    {
        Aura* aura = botAI->GetAura("rune of blood", bot);
        if (aura)
        {
            if (dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action))
                return 0.0f;

            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;

            return 0.0f;
        }
    }

    return 1.0f;
}

// Gunship
float IccGunshipMultiplier::GetValue(Action* action)
{
    // Detect gunship encounter via hostile enemy captain nearby
    Unit* saurfang = bot->FindNearestCreature(NPC_HIGH_OVERLORD_SAURFANG, 200.0f);
    Unit* muradin = bot->FindNearestCreature(NPC_MURADIN_BRONZEBEARD, 200.0f);
    bool const inGunship = (saurfang && saurfang->IsAlive() && saurfang->IsHostileTo(bot)) ||
                           (muradin && muradin->IsAlive() && muradin->IsHostileTo(bot));
    if (!inGunship)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    // Main tank is locked to captain via IccGunshipRocketJumpAction — block RTI targeting
    if (botAI->IsMainTank(bot) && dynamic_cast<AttackRtiTargetAction*>(action))
        return 0.0f;

    // Bot in transit between ships: lock to rocket-jump action only so combat/movement
    // actions don't interfere with jump packet timing.
    bool const isHordeSide = muradin && muradin->IsAlive() && muradin->IsHostileTo(bot);
    bool const isAllySide = saurfang && saurfang->IsAlive() && saurfang->IsHostileTo(bot);
    Position const friendlyPoint = isHordeSide ? ICC_GUNSHIP_ROCKET_JUMP_HORDE_FRIENDLY_POINT
                                               : ICC_GUNSHIP_ROCKET_JUMP_ALLY_FRIENDLY_POINT;
    Position const middlePoint = isHordeSide ? ICC_GUNSHIP_ROCKET_JUMP_HORDE_MIDDLE_POINT
                                             : ICC_GUNSHIP_ROCKET_JUMP_ALLY_MIDDLE_POINT;
    Position const attackPos = isHordeSide ? ICC_GUNSHIP_ROCKET_JUMP_HORDE
                                           : ICC_GUNSHIP_ROCKET_JUMP_ALLY;
    static constexpr float JUMP_GATE = 30.0f;
    bool const nearFriendly = bot->GetExactDist2d(friendlyPoint) <= JUMP_GATE;
    bool const nearMiddle = bot->GetExactDist2d(middlePoint) <= JUMP_GATE;
    bool const nearAttack = bot->GetExactDist2d(attackPos) <= JUMP_GATE;
    if (!nearFriendly && !nearMiddle && !nearAttack)
    {
        if (!dynamic_cast<IccGunshipRocketJumpAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// Dogs
float IccDogsMultiplier::GetValue(Action* action)
{
    if (!AI_VALUE2(Unit*, "find target", "stinky") && !AI_VALUE2(Unit*, "find target", "precious"))
        return 1.0f;

    if (botAI->IsTank(bot))
    {
        Aura* aura = botAI->GetAura("mortal wound", bot, false, true);
        if (aura && aura->GetStackAmount() >= 8)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;

            if (dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action))
                return 0.0f;

            return 0.0f;
        }
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}

// Festergut
float IccFestergutMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "festergut");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action))
        return 0.0f;

    if (dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsTank(bot))
    {
        Aura* aura = botAI->GetAura("gastric bloat", bot, false, true);
        if (aura && aura->GetStackAmount() >= 6)
        {
            if (dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action))
                return 0.0f;

            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;

            return 0.0f;
        }
    }

    if (dynamic_cast<IccFestergutSporeAction*>(action))
        return 1.0f;

    if (bot->HasAura(SPELL_GAS_SPORE))
    {
        if (dynamic_cast<MovementAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<ReachSpellAction*>(action))
            return 0.0f;
    }

    // Hold position during the 8s malleable goo wait window so the bot can
    // keep DPS/heal rotations running without drifting back into the impact.
    // Avoid action itself is whitelisted (may still need to dodge new goos).
    {
        auto const& waitMap = IcecrownHelpers::festergutGooWaitUntil;
        auto it = waitMap.find(bot->GetGUID());
        if (it != waitMap.end() && getMSTime() < it->second)
        {
            if (dynamic_cast<IccFestergutAvoidMalleableGooAction*>(action))
                return 1.0f;

            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }
    }

    return 1.0f;
}

// Rotface
float IccRotfaceMultiplier::GetValue(Action* action)
{
    Unit* boss1 = AI_VALUE2(Unit*, "find target", "rotface");
    if (!boss1)
        return 1.0f;

    {
        uint32 const now = getMSTime();
        auto const& waitMap = IcecrownHelpers::rotfaceVileGasWaitUntil;
        auto it = waitMap.find(bot->GetGUID());
        bool const inWait = it != waitMap.end() && now < it->second;
        auto vgIt = IcecrownHelpers::rotfaceVileGas.find(bot->GetMap()->GetInstanceId());
        bool const isVictim =
            vgIt != IcecrownHelpers::rotfaceVileGas.end() &&
            vgIt->second.victimGuid == bot->GetGUID() &&
            getMSTimeDiff(vgIt->second.castTime, now) < 8000;

        if (isVictim || inWait || botAI->HasAura("Vile Gas", bot))
        {
            if (dynamic_cast<IccRotfaceAvoidVileGasAction*>(action))
                return 1.0f;
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }
    }

    if (botAI->HasAura("Vile Gas", bot))
        return 0.0f;

    if (botAI->IsTank(bot) && dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 0.0f;

    if (dynamic_cast<FleeAction*>(action) && !(bot->getClass() == CLASS_HUNTER))
        return 0.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastArmyOfTheDeadAction*>(action))
        return 0.0f;

    if (botAI->IsAssistTank(bot) &&
        (dynamic_cast<AttackRtiTargetAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
         dynamic_cast<CastTauntAction*>(action) || dynamic_cast<CastDarkCommandAction*>(action) ||
         dynamic_cast<CastHandOfReckoningAction*>(action) || dynamic_cast<CastGrowlAction*>(action)))
        return 0.0f;

    if (botAI->IsAssistTank(bot) && boss1 && bot->GetVictim() == boss1)
    {
        bot->AttackStop();
        bot->SetTarget(ObjectGuid::Empty);
        return 0.0f;
    }

    // Never cure/dispel Mutated Infection — it must expire naturally to spawn a small ooze
    if (dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastCleanseDiseaseAction*>(action) ||
        dynamic_cast<CastCleanseDiseaseOnPartyAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) || dynamic_cast<CastCleanseSpiritAction*>(action))
        return 0.0f;

    {
        Creature* bigOoze = bot->FindNearestCreature(NPC_BIG_OOZE, 100.0f);
        bool castingNow = bigOoze && bigOoze->IsAlive() &&
            bigOoze->HasUnitState(UNIT_STATE_CASTING) && bigOoze->FindCurrentSpellBySpellId(SPELL_UNSTABLE_OOZE_EXPLOSION);

        if (castingNow && (dynamic_cast<MovementAction*>(action) || dynamic_cast<IccRotfaceGroupPositionAction*>(action)) &&
            !dynamic_cast<IccRotfaceMoveAwayFromExplosionAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

// pp
float IccAddsPutricideMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "professor putricide");
    if (!boss)
        return 1.0f;
    Unit* boss1 = AI_VALUE2(Unit*, "find target", "rotface");
    if (boss1)
        return 1.0f;
    Unit* boss2 = AI_VALUE2(Unit*, "find target", "festergut");
    if (boss2)
        return 1.0f;

    if (Unit* vehBase = bot->GetVehicleBase())
    {
        uint32 e = vehBase->GetEntry();
        if (e == NPC_MUTATED_ABOMINATION_10 || e == NPC_MUTATED_ABOMINATION_25)
        {
            if (dynamic_cast<IccPutricideAbominationAction*>(action))
                return 1.0f;
            return 0.0f;
        }
    }

    bool hasGaseousBloat = botAI->HasAura("Gaseous Bloat", bot);
    bool hasUnboundPlague = botAI->HasAura("Unbound Plague", bot);

    if (botAI->IsTank(bot) &&
        bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        if (dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<IccPutricideAvoidMalleableGooAction*>(action))
            return 1.0f;
        return 0.0f;
    }

    if (!(bot->getClass() == CLASS_HUNTER) && dynamic_cast<FleeAction*>(action))
        return 0.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    if (dynamic_cast<CastBlinkBackAction*>(action))
        return 0.0f;

    if (botAI->IsTank(bot))
    {
        auto GetPlagueStacks = [&](Unit* unit) -> uint32
        {
            if (!unit)
                return 0;
            Aura* a = botAI->GetAura("mutated plague", unit, false, true);
            return a ? a->GetStackAmount() : 0;
        };

        uint32 const myStacks = GetPlagueStacks(bot);

        // Another tank has fewer stacks — they should be tanking instead of us.
        // Block generic taunts (so we don't fight for aggro), keep movement, and
        // cancel the rest of the rotation (IccPutricideMutatedPlagueAction owns
        // the AttackStop/taunt handoff).
        bool anotherTankHasFewer = false;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (!member || member == bot || !member->IsAlive() || !member->IsInWorld())
                    continue;
                if (!PlayerbotAI::IsTank(member))
                    continue;

                if (GetPlagueStacks(member) < myStacks)
                {
                    anotherTankHasFewer = true;
                    break;
                }
            }
        }

        if (anotherTankHasFewer)
        {
            if (dynamic_cast<CastTauntAction*>(action) ||
                dynamic_cast<CastDarkCommandAction*>(action) ||
                dynamic_cast<CastHandOfReckoningAction*>(action) ||
                dynamic_cast<CastGrowlAction*>(action))
                return 0.0f;

            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;

            if (dynamic_cast<IccPutricideMutatedPlagueAction*>(action))
                return 1.0f;

            return 0.0f;
        }
    }

    if (hasGaseousBloat)
    {
        if (dynamic_cast<IccPutricideGasCloudAction*>(action))
            return 1.0f;

        if (dynamic_cast<IccPutricideGrowingOozePuddleAction*>(action))
            return 1.0f;

        if (botAI->IsHeal(bot))
            return 1.0f;
        else
            return 0.0f; // Cancel all other actions when we need to handle Gaseous Bloat
    }

    if (hasUnboundPlague && boss && !boss->HealthBelowPct(35))
    {
        if (dynamic_cast<IccPutricideAvoidMalleableGooAction*>(action))
            return 1.0f;
        else
            return 0.0f; // Cancel all other actions when we need to handle Unbound Plague
    }

    if (dynamic_cast<IccPutricideVolatileOozeAction*>(action))
    {
        if (dynamic_cast<IccPutricideAvoidMalleableGooAction*>(action))
            return 1.0f;
        if (dynamic_cast<IccPutricideGrowingOozePuddleAction*>(action) && !botAI->IsMainTank(bot))
            return 0.0f;
    }

    return 1.0f;
}

// bpc
float IccBpcAssistMultiplier::GetValue(Action* action)
{
    Unit* keleseth = AI_VALUE2(Unit*, "find target", "prince keleseth");
    if (!keleseth)
        return 1.0f;

    if (keleseth && (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
        dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
        dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
        dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
        dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastFlamestrikeAction*>(action) ||
        dynamic_cast<CastExplosiveTrapAction*>(action) || dynamic_cast<CastExplosiveShotBaseAction*>(action) ||
        dynamic_cast<CastArmyOfTheDeadAction*>(action)))
        return 0.0f;

    Aura* aura = botAI->GetAura("Shadow Prison", bot, false, true);

    // Bomb assignment check (done early so it can override shadow prison stack limits)
    static const std::array<uint32, 4> bombEntries = {NPC_KINETIC_BOMB1, NPC_KINETIC_BOMB2, NPC_KINETIC_BOMB3,
                                                      NPC_KINETIC_BOMB4};
    GuidVector const bombs = AI_VALUE(GuidVector, "possible targets no los");

    std::vector<Unit*> kineticBombs;
    for (auto const& guid : bombs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        if (std::find(bombEntries.begin(), bombEntries.end(), unit->GetEntry()) == bombEntries.end())
            continue;

        if (unit->GetPositionZ() - bot->GetPositionZ() > 35.0f)
            continue;

        kineticBombs.push_back(unit);
    }

    bool botAssignedToBomb = false;
    if (!kineticBombs.empty() && botAI->IsRangedDps(bot) &&
        !(aura && aura->GetStackAmount() > 18))
    {
        std::sort(kineticBombs.begin(), kineticBombs.end(),
                  [](Unit* a, Unit* b) { return a->GetPositionZ() < b->GetPositionZ(); });

        std::vector<Player*> rangedDps;
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->GetSource();
                if (member && member->IsAlive() && GET_PLAYERBOT_AI(member) && botAI->IsRangedDps(member))
                    rangedDps.push_back(member);
            }
        }

        static float const MAX_ASSIGN_RANGE = 80.0f;
        std::set<Player*> assigned;
        for (Unit* bomb : kineticBombs)
        {
            Player* nearest = nullptr;
            float nearestDist = std::numeric_limits<float>::max();

            // Priority classes: hunter > druid > any
            static constexpr std::array<uint8, 3> classPriority = {CLASS_HUNTER, CLASS_DRUID, 0};
            for (uint8 priorityClass : classPriority)
            {
                nearest = nullptr;
                nearestDist = std::numeric_limits<float>::max();

                for (Player* dps : rangedDps)
                {
                    if (assigned.count(dps))
                        continue;

                    if (priorityClass != 0 && dps->getClass() != priorityClass)
                        continue;

                    float dist = dps->GetDistance(bomb);
                    if (dist < nearestDist && dist < MAX_ASSIGN_RANGE)
                    {
                        nearestDist = dist;
                        nearest = dps;
                    }
                }

                if (nearest)
                    break;
            }

            if (nearest)
            {
                assigned.insert(nearest);
                if (nearest == bot)
                    botAssignedToBomb = true;
            }
        }
    }

    // Bomb-assigned bot: block target switching and non-bomb BPC actions, allow combat rotation
    if (botAssignedToBomb)
    {
        if (dynamic_cast<IccBpcKineticBombAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
            return 1.0f;

        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<AttackRtiTargetAction*>(action) || dynamic_cast<IccBpcEmpoweredVortexAction*>(action) ||
            dynamic_cast<IccBpcBallOfFlameAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action) ||
            dynamic_cast<FollowAction*>(action))
            return 0.0f;
    }

    // Shadow prison movement block (non-bomb bots use normal 12 stack limit)
    if (aura)
    {
        if (aura->GetStackAmount() > 18 && botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }

        if (aura->GetStackAmount() > 12 && !botAI->IsTank(bot))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
        }
    }

    Unit* valanar = AI_VALUE2(Unit*, "find target", "prince valanar");
    if (!valanar)
        return 1.0f;

    if (valanar && valanar->HasUnitState(UNIT_STATE_CASTING) &&
        (valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX1) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX2) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX3) ||
         valanar->FindCurrentSpellBySpellId(SPELL_EMPOWERED_SHOCK_VORTEX4)))
    {
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<IccBpcEmpoweredVortexAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    Unit* flame1 = bot->FindNearestCreature(NPC_BALL_OF_FLAME, 100.0f);
    Unit* flame2 = bot->FindNearestCreature(NPC_BALL_OF_INFERNO_FLAME, 100.0f);
    bool ballOfFlame = flame1 && flame1->GetVictim() == bot;
    bool infernoFlame = flame2 && flame2->GetVictim() == bot;

    if (flame2)
    {
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<IccBpcKineticBombAction*>(action))
            return 0.0f;

        if (dynamic_cast<IccBpcBallOfFlameAction*>(action))
            return 1.0f;
    }

    if (ballOfFlame || infernoFlame)
    {
        if (dynamic_cast<IccBpcBallOfFlameAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    // For assist tank during BPC fight
    if (botAI->IsAssistTank(bot) && !(aura && aura->GetStackAmount() > 18))
    {
        // Allow BPC-specific actions
        if (dynamic_cast<IccBpcKelesethTankAction*>(action))
            return 1.0f;

        // Disable normal assist behavior (allow RTI targeting)
        if (dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CastConsecrationAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

//BQL
float IccBqlMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blood-queen lana'thel");
    if (!boss)
        return 1.0f;

    Aura* aura2 = botAI->GetAura("Swarming Shadows", bot);
    Aura* aura = botAI->GetAura("Frenzied Bloodthirst", bot);

    if (botAI->IsRanged(bot))
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
            return 0.0f;

    // If bot has Pact of Darkfallen aura, return 0 for all other actions
    if (bot->HasAura(SPELL_PACT_OF_THE_DARKFALLEN))
    {
        if (dynamic_cast<IccBqlPactOfDarkfallenAction*>(action))
            return 1.0f; // Allow Pact of Darkfallen action
        else
            return 0.0f; // Cancel all other actions when we need to handle Pact of Darkfallen
    }

    // Air phase: block movement/chase actions, allow combat rotation (attacks/heals)
    if (((boss->GetPositionZ() - ICC_BQL_CENTER_POSITION.GetPositionZ()) > 5.0f) && !aura)
    {
        if (dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<CastDisengageAction*>(action) ||
            dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachTargetAction*>(action))
            return 0.0f;
    }

    // If bot has frenzied bloodthirst, allow highest priority for bite action
    if (aura) // If bot has frenzied bloodthirst
    {
        if (dynamic_cast<IccBqlVampiricBiteAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    if (aura2 && !aura)
    {
        if (dynamic_cast<IccBqlGroupPositionAction*>(action))
            return 1.0f;
        else
            return 0.0f;  // Cancel all other actions when we need to handle Swarming Shadows
    }

    if ((boss->GetExactDist2d(ICC_BQL_TANK_POSITION.GetPositionX(), ICC_BQL_TANK_POSITION.GetPositionY()) > 10.0f) &&
        botAI->IsRanged(bot) && !((boss->GetPositionZ() - bot->GetPositionZ()) > 5.0f))
    {
        if (dynamic_cast<FleeAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

//VDW
float IccValithriaDreamCloudMultiplier::GetValue(Action* action)
{
    Unit* boss = bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f);

    Aura* twistedNightmares = botAI->GetAura("Twisted Nightmares", bot);
    Aura* emeraldVigor = botAI->GetAura("Emerald Vigor", bot);

    if (!boss && !bot->HasAura(SPELL_DREAM_STATE))
        return 1.0f;

    if (dynamic_cast<FollowAction*>(action) || dynamic_cast<CombatFormationMoveAction*>(action))
        return 0.0f;

    // Zombie victim: only the kite action runs. Blocks combat/movement so bot
    // doesn't try to attack/cast/move toward marks while being chased.
    if (boss && !botAI->IsTank(bot))
    {
        Creature* attackingZombie = nullptr;
        std::list<Creature*> zombies;
        bot->GetCreatureListWithEntryInGrid(zombies, NPC_BLISTERING_ZOMBIE, 100.0f);
        for (Creature* z : zombies)
        {
            if (z && z->IsAlive() && z->GetVictim() == bot)
            {
                attackingZombie = z;
                break;
            }
        }
        if (attackingZombie)
        {
            if (dynamic_cast<IccValithriaZombieKiteAction*>(action))
                return 1.0f;
            return 0.0f;
        }
    }

    if (botAI->IsTank(bot))
    {
        if (dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;
    }
    else
    {
        // Non-tanks must strictly follow RTI marks. Block generic assist actions
        // so bots never attack unmarked adds; AttackRtiTargetAction drives them to
        // skull/cross targets set by HandleMarkingLogic.
        if (dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        // Melee bots must not engage Blistering Zombies (one-shot melee swing).
        // Only ranged DPS handle them. If RTI/current target is a zombie, block
        // attack actions so melee falls through to other priorities.
        if (!PlayerbotAI::IsRangedDps(bot) && !botAI->IsHeal(bot))
        {
            Unit* victim = bot->GetVictim();
            bool victimIsZombie = victim && victim->GetEntry() == NPC_BLISTERING_ZOMBIE;

            bool rtiIsZombie = false;
            if (Group* group = bot->GetGroup())
            {
                ObjectGuid rtiGuid = group->GetTargetIcon(7);
                if (!rtiGuid.IsEmpty())
                {
                    Unit* rtiUnit = ObjectAccessor::GetUnit(*bot, rtiGuid);
                    if (rtiUnit && rtiUnit->GetEntry() == NPC_BLISTERING_ZOMBIE)
                        rtiIsZombie = true;
                }
            }

            if (victimIsZombie || rtiIsZombie)
            {
                if (dynamic_cast<AttackRtiTargetAction*>(action) ||
                    dynamic_cast<DpsAssistAction*>(action))
                    return 0.0f;
            }
        }
    }

    if (botAI->IsHeal(bot) && (twistedNightmares || emeraldVigor))
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;

    if (bot->HasAura(SPELL_DREAM_STATE) && !bot->HealthBelowPct(50))
    {
        if (dynamic_cast<IccValithriaDreamCloudAction*>(action))
            return 1.0f; // Allow Dream Cloud action
        else
            return 0.0f; // Cancel all other actions when we need to handle Dream Cloud
    }

    return 1.0f;

}

//SINDRAGOSA

float IccSindragosaMultiplier::GetValue(Action* action)
{
    Unit* boss = bot->FindNearestCreature(NPC_SINDRAGOSA, 200.0f);
    if (!boss)
        return 1.0f;

    // HoT support is an instant cast that never moves the bot. Always allow so
    // beaconed targets stay topped up across air phase, blistering cold cast,
    // phase 3 tank lockdown, and other "everything else 0.0f" branches below.
    if (dynamic_cast<IccSindragosaHotAction*>(action))
        return 1.0f;

    Aura* aura = botAI->GetAura("Unchained Magic", bot, false, true);

    Difficulty diff = bot->GetRaidDifficulty();

    if (boss->HealthBelowPct(95))
    {
        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FollowAction*>(action) || dynamic_cast<CastStarfallAction*>(action))
            return 0.0f;
    }

    if (aura && (diff == RAID_DIFFICULTY_10MAN_HEROIC || diff == RAID_DIFFICULTY_25MAN_HEROIC) &&
        !dynamic_cast<IccSindragosaFrostBombAction*>(action))
    {
        if (dynamic_cast<MovementAction*>(action) || dynamic_cast<IccSindragosaUnchainedMagicAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    // Check if boss is casting blistering cold (using both normal and heroic spell IDs)
    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        (boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD1) || boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD2) ||
         boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD3) || boss->FindCurrentSpellBySpellId(SPELL_BLISTERING_COLD4)))
    {
        // If this is the blistering cold action, give it highest priority
        if (dynamic_cast<IccSindragosaBlisteringColdAction*>(action) ||
            dynamic_cast<HealPartyMemberAction*>(action) ||
            dynamic_cast<ReachPartyMemberToHealAction*>(action) ||
            dynamic_cast<IccSindragosaTankSwapPositionAction*>(action))
            return 1.0f;

        // Ranged / healer already beyond the blast radius: keep DPSing or
        // healing, just block any movement so they don't wander back in.
        bool const safe = bot->GetExactDist2d(boss) >= 33.0f;
        if (safe && (botAI->IsRanged(bot) || botAI->IsHeal(bot)))
        {
            if (dynamic_cast<MovementAction*>(action))
                return 0.0f;
            return 1.0f;
        }

        // Disable all other actions while blistering cold is casting
        return 0.0f;
    }

    // Highest priority if we have beacon
    if (bot->HasAura(SPELL_FROST_BEACON))
    {
        if (dynamic_cast<IccSindragosaFrostBeaconAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    Group* group = bot->GetGroup();
    // Check if anyone in group has Frost Beacon (SPELL_FROST_BEACON)
    bool anyoneHasFrostBeacon = false;

    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->HasAura(SPELL_FROST_BEACON))
            {
                anyoneHasFrostBeacon = true;
                break;
            }
        }
    }

    if (anyoneHasFrostBeacon && boss &&
        boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(),
                             ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f &&
        !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
    {
        if (dynamic_cast<IccSindragosaFrostBeaconAction*>(action))
            return 1.0f;
        else
            return 0.0f;
    }

    if (anyoneHasFrostBeacon && !botAI->IsMainTank(bot))
    {
        if (dynamic_cast<IccSindragosaGroupPositionAction*>(action))
            return 0.0f;
    }

    if (botAI->IsMainTank(bot))
    {
        Aura* aura = botAI->GetAura("mystic buffet", bot, false, true);
        if (aura && aura->GetStackAmount() >= 6)
        {
            if (dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (!botAI->IsTank(bot) && boss && boss->HealthBelowPct(35))
    {
        if (dynamic_cast<IccSindragosaGroupPositionAction*>(action))
            return 0.0f;
    }

    if (boss && botAI->IsTank(bot))
    {
        if (boss->HealthBelowPct(35))
        {
            if (dynamic_cast<IccSindragosaTankSwapPositionAction*>(action) || dynamic_cast<TankFaceAction*>(action) ||
                dynamic_cast<AttackAction*>(action) || dynamic_cast<MovementAction*>(action))
                return 1.0f;
            else
                return 0.0f;
        }
    }

    if (boss && boss->GetExactDist2d(ICC_SINDRAGOSA_FLYING_POSITION.GetPositionX(), ICC_SINDRAGOSA_FLYING_POSITION.GetPositionY()) < 30.0f && !boss->HealthBelowPct(25) && !boss->HealthAbovePct(99))
    {
        if (dynamic_cast<IccSindragosaFrostBombAction*>(action))
            return 1.0f;

        if (dynamic_cast<FollowAction*>(action) || dynamic_cast<IccSindragosaBlisteringColdAction*>(action) ||
            dynamic_cast<IccSindragosaChilledToTheBoneAction*>(action) || dynamic_cast<IccSindragosaMysticBuffetAction*>(action) ||
            dynamic_cast<IccSindragosaFrostBeaconAction*>(action) || dynamic_cast<IccSindragosaUnchainedMagicAction*>(action) ||
            dynamic_cast<FleeAction*>(action) || dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<PetAttackAction*>(action) ||
            dynamic_cast<IccSindragosaGroupPositionAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
            dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
            dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastConsecrationAction*>(action) ||
            dynamic_cast<CastFlamestrikeAction*>(action) || dynamic_cast<CastExplosiveTrapAction*>(action) ||
            dynamic_cast<CastExplosiveShotBaseAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float IccLichKingAddsMultiplier::GetValue(Action* action)
{
    if (bot->FindNearestCreature(NPC_VALITHRIA_DREAMWALKER, 100.0f))
        return 1.0f;

    Unit* terenas = bot->FindNearestCreature(NPC_TERENAS_MENETHIL_HC, 55.0f);
    if (terenas)
    {
        // Warlocks and melee stay functional (movement + adds action only)
        if (botAI->IsMelee(bot) || bot->getClass() == CLASS_WARLOCK)
        {
            if (dynamic_cast<MovementAction*>(action) || dynamic_cast<IccLichKingAddsAction*>(action))
                return 1.0f;
            return 0.0f;
        }

        // Main tank near another tank: suppress movement jitter
        Unit* mainTank = AI_VALUE(Unit*, "main tank");
        if (!botAI->IsMainTank(bot) && mainTank && bot->GetExactDist2d(mainTank) < 2.0f &&
            dynamic_cast<MovementAction*>(action))
            return 0.0f;

        // Suppress all these regardless of role
        if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
            dynamic_cast<FleeAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) ||
            dynamic_cast<CastDisengageAction*>(action) || dynamic_cast<CastChargeAction*>(action) ||
            dynamic_cast<CastFeralChargeBearAction*>(action) || dynamic_cast<CastIceBlockAction*>(action) ||
            dynamic_cast<CastRevivePetAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
            dynamic_cast<CastArmyOfTheDeadAction*>(action))
            return 0.0f;

        return 1.0f;
    }

    Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
    if (!boss)
        return 1.0f;

    // Allow cure actions only after a brief delay so the plague can spread once
    if (dynamic_cast<CurePartyMemberAction*>(action) || dynamic_cast<CastCleanseDiseaseAction*>(action) ||
        dynamic_cast<CastCleanseDiseaseOnPartyAction*>(action) ||
        dynamic_cast<CastCleanseSpiritCurseOnPartyAction*>(action) || dynamic_cast<CastCleanseSpiritAction*>(action))
    {
        Unit* boss = AI_VALUE2(Unit*, "find target", "the lich king");
        if (!boss)
            return 1.0f;

        Group* group = bot->GetGroup();
        if (!group)
            return 1.0f;

        static constexpr float DELIVER_RANGE = 3.0f;
        static constexpr std::array<uint32, 4> HorrorEntries = {NPC_SHAMBLING_HORROR1, NPC_SHAMBLING_HORROR2,
                                                                NPC_SHAMBLING_HORROR3, NPC_SHAMBLING_HORROR4};

        // Check whether any Shambling Horror is alive anywhere in the encounter
        auto const anyHorrorAlive = [&]() -> bool
        {
            GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
            for (ObjectGuid const& guid : npcs)
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive())
                    continue;

                uint32 const entry = unit->GetEntry();
                if (entry == NPC_SHAMBLING_HORROR1 || entry == NPC_SHAMBLING_HORROR2 ||
                    entry == NPC_SHAMBLING_HORROR3 || entry == NPC_SHAMBLING_HORROR4)
                    return true;
            }
            return false;
        };

        bool anyPlagued = false;
        bool allDelivered = true;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive())
                continue;

            if (!botAI->HasAura("Necrotic Plague", member))
                continue;

            anyPlagued = true;

            bool nearHorror = false;
            for (uint32 const entry : HorrorEntries)
            {
                Creature* horror = member->FindNearestCreature(entry, DELIVER_RANGE);
                if (horror && horror->IsAlive())
                {
                    nearHorror = true;
                    break;
                }
            }

            if (!nearHorror)
            {
                allDelivered = false;
                break;
            }
        }

        if (!anyPlagued)
            return 1.0f;

        // No Horror alive at all — allow immediate dispel to prevent
        // uncontrolled spread wiping the raid
        if (!anyHorrorAlive())
            return 1.0f;

        // Horrors exist but not everyone has delivered yet — suppress cures
        return allDelivered ? 1.0f : 0.0f;
    }

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
        dynamic_cast<CastBlinkBackAction*>(action) || dynamic_cast<CastDisengageAction*>(action))
        return 0.0f;

    // Hunters may flee (kite mechanics); everyone else stays put
    if (dynamic_cast<FleeAction*>(action) && bot->getClass() != CLASS_HUNTER)
        return 0.0f;

    if (boss->HealthAbovePct(71))
    {
        // Assist tank targeting is fully managed by HandleAssistTankAddManagement —
        // suppress generic target-switching actions so they don't override it.
        if (botAI->IsAssistTank(bot) &&
            (dynamic_cast<TankAssistAction*>(action) || dynamic_cast<AttackRtiTargetAction*>(action) ||
             dynamic_cast<DpsAssistAction*>(action)))
            return 0.0f;

        if (!botAI->IsTank(bot) && dynamic_cast<CastConsecrationAction*>(action))
            return 0.0f;

        if (dynamic_cast<DpsAoeAction*>(action) || dynamic_cast<CastHurricaneAction*>(action) ||
            dynamic_cast<CastVolleyAction*>(action) || dynamic_cast<CastBlizzardAction*>(action) ||
            dynamic_cast<CastStarfallAction*>(action) || dynamic_cast<FanOfKnivesAction*>(action) ||
            dynamic_cast<CastWhirlwindAction*>(action) || dynamic_cast<CastMindSearAction*>(action) ||
            dynamic_cast<CastMagmaTotemAction*>(action) || dynamic_cast<CastFlamestrikeAction*>(action) ||
            dynamic_cast<CastExplosiveTrapAction*>(action) || dynamic_cast<CastExplosiveShotBaseAction*>(action) ||
            dynamic_cast<CastArmyOfTheDeadAction*>(action))
            return 0.0f;
    }

    auto const hasWinterAura = [&]() -> bool
    {
        return boss->HasAura(SPELL_REMORSELESS_WINTER1) || boss->HasAura(SPELL_REMORSELESS_WINTER2) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER3) || boss->HasAura(SPELL_REMORSELESS_WINTER4) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER5) || boss->HasAura(SPELL_REMORSELESS_WINTER6) ||
               boss->HasAura(SPELL_REMORSELESS_WINTER7) || boss->HasAura(SPELL_REMORSELESS_WINTER8);
    };

    auto const isCastingWinter = [&]() -> bool
    {
        if (!boss->HasUnitState(UNIT_STATE_CASTING))
            return false;

        return boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER1) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER2) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER3) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER4) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER5) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER6) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER7) ||
               boss->FindCurrentSpellBySpellId(SPELL_REMORSELESS_WINTER8);
    };

    if (hasWinterAura() || isCastingWinter())
    {
        // Winter action and facing take priority
        if (dynamic_cast<IccLichKingWinterAction*>(action) || dynamic_cast<SetFacingTargetAction*>(action))
            return 1.0f;

        // Staging window: while boss is casting Winter, non-tanks must commit
        // to the staging move. Only heals are allowed; everything else blocked.
        if (isCastingWinter() && !botAI->IsTank(bot))
        {
            if (dynamic_cast<HealPartyMemberAction*>(action) ||
                dynamic_cast<ReachPartyMemberToHealAction*>(action))
                return 1.0f;
            return 0.0f;
        }

        // Adds action is suppressed during winter
        if (dynamic_cast<IccLichKingAddsAction*>(action))
            return 0.0f;

        if (dynamic_cast<CastArmyOfTheDeadAction*>(action))
            return 0.0f;

        // Assist tank should not pick up adds independently during winter
        if (botAI->IsAssistTank(bot) && dynamic_cast<TankAssistAction*>(action))
            return 0.0f;

        // Suppress movement/attack toward the boss if we are far away
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget && currentTarget == boss && bot->GetDistance2d(boss) > 50.0f)
        {
            if (dynamic_cast<ReachSpellAction*>(action) ||
                dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachTargetAction*>(action) ||
                dynamic_cast<TankAssistAction*>(action) || dynamic_cast<DpsAssistAction*>(action))
                return 0.0f;
        }

        // Suppress movement toward boss/sphere — but allow target-switching actions
        // (DpsAssistAction, AttackRtiTargetAction) so bots can pick up skull-marked adds.
        if (currentTarget &&
            (currentTarget == boss || currentTarget->GetEntry() == NPC_ICE_SPHERE1 ||
             currentTarget->GetEntry() == NPC_ICE_SPHERE2 ||
             currentTarget->GetEntry() == NPC_ICE_SPHERE3 || currentTarget->GetEntry() == NPC_ICE_SPHERE4))
        {
            if (dynamic_cast<ReachMeleeAction*>(action) || dynamic_cast<ReachSpellAction*>(action) ||
                dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<TankAssistAction*>(action))
                return 0.0f;
        }
    }

    if (botAI->IsRanged(bot) && !botAI->GetAura("Harvest Soul", bot, false, false))
    {
        GuidVector const& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        bool defilePresent = false;
        for (ObjectGuid const& guid : npcs)
        {
            Unit* unit = botAI->GetUnit(guid);
            if (unit && unit->IsAlive() && unit->GetEntry() == DEFILE_NPC_ID)
            {
                defilePresent = true;
                break;
            }
        }

        if (defilePresent && (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
                              dynamic_cast<FleeAction*>(action) || dynamic_cast<MoveRandomAction*>(action) ||
                              dynamic_cast<MoveFromGroupAction*>(action)))
            return 0.0f;
    }

    if (botAI->IsAssistTank(bot) && boss->HealthAbovePct(71))
    {
        Unit* currentTarget = AI_VALUE(Unit*, "current target");
        if (currentTarget && currentTarget == boss && dynamic_cast<AttackRtiTargetAction*>(action))
            return 0.0f;
    }

    return 1.0f;
}

float IccLichKingSpiritBombMultiplier::GetValue(Action* action)
{
    if (!IccLichKingSpiritBombAction::IsBombThreatActive(botAI, bot))
        return 1.0f;

    // Allowlist: only the avoidance move and facing run during a bomb threat.
    // Everything else is suppressed so the avoidance move sticks.
    if (dynamic_cast<IccLichKingSpiritBombAction*>(action) ||
        dynamic_cast<SetFacingTargetAction*>(action))
        return 1.0f;

    return 0.0f;
}
