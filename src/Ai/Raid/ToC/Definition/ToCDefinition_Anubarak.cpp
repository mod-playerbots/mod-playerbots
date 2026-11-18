/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ToCDefinitions.h"

#include "AiObjectContext.h"
#include "Creature.h"
#include "Map.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "ToCEncounterGate.h"
#include "ToCHelpers_Anubarak.h"
#include "ToCMultipliers_Anubarak.h"

using namespace TrialOfTheCrusaderHelpers;

namespace Family = RaidEncounterRules::Family;
namespace Role = RaidEncounterRules::Role;

namespace
{
bool AnubarakDpsTargetOwned(PlayerbotAI* botAI) { return GetAnubarakDpsTarget(botAI) != nullptr; }

// The boss hold trigger's test
bool AnubarakPickupTankHolds(PlayerbotAI* botAI)
{
    if (!AnubarakEngaged(botAI) || !IsAnubarakPickupTank(botAI->GetBot()))
        return false;

    AnubarakPhase const phase = GetAnubarakPhase(botAI);
    return phase == AnubarakPhase::Surface || phase == AnubarakPhase::Swarm;
}

// The whole pull: a side tank on him or on a scarab when a wave lands leaves the wave on the healers.
// The ready node holds it on its patch.
bool AnubarakSideTankHolds(PlayerbotAI* botAI)
{
    return AnubarakEngaged(botAI) && GetAnubarakBurrowerTankSide(botAI->GetBot()) >= 0;
}

// A flying sphere is out of combat, so never the dps target, and assist would take the shooter off it
// every tick. A DK waiting on its grip spot keeps assist.
bool AnubarakSphereShooterHolds(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!AnubarakEngaged(botAI))
        return false;

    AnubarakGripDuty grip;
    return (PlayerbotAI::IsRangedDps(bot) && GetAnubarakSphereToShoot(bot)) ||
           (GetAnubarakGripDuty(bot, grip) && grip.step != AnubarakGripStep::Hold);
}

// An untouched sphere is a critter out of combat, which invalid target drops 0.2 s after every Attack()
bool AnubarakShooterOnDutySphere(PlayerbotAI* botAI)
{
    if (!AnubarakEngaged(botAI))
        return false;

    Creature* sphere = GetAnubarakDutySphere(botAI->GetBot());
    return sphere && IsFrostSphereFlying(sphere) &&
           botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get() == sphere;
}

// Walking off the spot to reach him moves where the gripped sphere lands
bool AnubarakGripHolds(PlayerbotAI* botAI)
{
    AnubarakGripDuty grip;
    return AnubarakEngaged(botAI) && GetAnubarakGripDuty(botAI->GetBot(), grip) &&
           grip.step != AnubarakGripStep::Kill;
}

// Tank face walks the tank round him, through whichever patch lies on the way
bool AnubarakPickupTankStands(PlayerbotAI* botAI)
{
    return AnubarakPickupTankHolds(botAI) && AnubarakTankPatchLatched(botAI->GetBot());
}

// The DK rotation spends Death Grip on a loose burrower, and the tank patch waits out its cooldown
bool AnubarakGripOwed(PlayerbotAI* botAI)
{
    AnubarakGripDuty grip;
    return AnubarakEngaged(botAI) && GetAnubarakGripDuty(botAI->GetBot(), grip);
}

// Heroic phase 2: the main tank leaves the scarabs on others, so Acid-Drenched Mandibles spreads and he
// comes back up without a stack on him
bool AnubarakMainTankLeavesScarabs(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return AnubarakEngaged(botAI) && bot->GetMap()->IsHeroic() && IsAnubarakPickupTank(bot) &&
           GetAnubarakPhase(botAI) == AnubarakPhase::Submerged;
}

// A tank bot holding no side: the main tank, or one waiting to take over
bool AnubarakMainTankEngaged(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return AnubarakEngaged(botAI) && IsAnubarakTankPlayer(bot) && GetAnubarakBurrowerTankSide(bot) < 0;
}

// Heroic phase 2: an immunity spent off the mark is gone for the kites, and Forbearance from Divine
// Shield blocks Hand of Protection too. The marked bot's own kite casts its immunity directly.
bool AnubarakKiteImmunitiesKept(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return AnubarakEngaged(botAI) && bot->GetMap()->IsHeroic() && !bot->HasAura(SPELL_MARK) &&
           GetAnubarakPhase(botAI) == AnubarakPhase::Submerged;
}

// Heroic, a submerge still ahead: Sprint's and Dash's cooldowns outlast a phase 1, so one spent closing on
// a target is gone for the next kite
bool AnubarakKiteSprintKept(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return AnubarakEngaged(botAI) && bot->GetMap()->IsHeroic() && !bot->HasAura(SPELL_MARK) &&
           GetAnubarakPhase(botAI) != AnubarakPhase::Swarm;
}

void DefineAnubarak(EncounterBuilder& e)
{
    e.OwnTargeting("anubarak dps target guard", Role::Any, AnubarakDpsTargetOwned, Family::DpsAssist);
    e.OwnTargeting("anubarak pickup tank target guard", Role::Any, AnubarakPickupTankHolds,
                   Family::DpsAssist | Family::TankAssist);
    e.OwnTargeting("anubarak side tank target guard", Role::Any, AnubarakSideTankHolds,
                   Family::DpsAssist | Family::TankAssist);
    e.OwnTargeting("anubarak sphere shooter target guard", Role::Any, AnubarakSphereShooterHolds,
                   Family::DpsAssist);
    e.OwnTargeting("anubarak main tank scarab guard", Role::Any, AnubarakMainTankLeavesScarabs, Family::TankAssist);

    // reach goes by name, so the walk to a battle res isn't blocked
    e.Block("anubarak spike dodge hold", Role::Any, IsAnubarakDodgeHeld,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Flee |
                Family::RunAway | Family::MoveRandom | Family::Charge | Family::Blink | Family::Disengage,
            {"reach melee", "reach spell"});
    // any other walk replaces the leg's, and he stops wherever the tank stops
    e.Block("anubarak boss drag hold", Role::Any, IsAnubarakDragInFlight,
            Family::SetBehind | Family::TankFace | Family::CombatFormationMove | Family::Follow | Family::Charge,
            {"reach melee"});
    e.Block("anubarak grip spot hold", Role::Any, AnubarakGripHolds,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Charge,
            {"reach melee"});
    e.Block("anubarak pickup tank stand guard", Role::Any, AnubarakPickupTankStands, Family::TankFace);
    // the melee stand walks it off the patch, and behind a held burrower is its patch
    e.Block("anubarak melee burrower stand hold", Role::Any, AnubarakMeleeBurrowerStandApplies,
            Family::SetBehind | Family::RearFlank, {"reach melee"});
    // heroic phase 2: the raid holds its gather, so no walk takes anyone across the spike's run. Flee
    // stepped casters off every scarab and the gather walked them back; the scarab kite moves a victim.
    e.Block("anubarak gather hold", Role::Any, AnubarakGathers,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Charge |
                Family::Flee,
            {"reach melee", "reach spell", "reach party member to heal", "anubarak melee burrower stand"});
    // healers hold their stand while he is up; reach still walks one to a heal target out of range, and the
    // burrower lead one a burrower is on
    e.Block("anubarak healer stand hold", Role::Heal, AnubarakHealerStandApplies,
            Family::CombatFormationMove | Family::Follow | Family::Flee);
    // heroic phase 3: the ranged hold theirs behind the tank patches; reach still walks one to its target
    e.Block("anubarak ranged stand hold", Role::Ranged, AnubarakRangedStandApplies,
            Family::CombatFormationMove | Family::Follow | Family::Flee);
    // the ready node places an idle side tank on its rim, and follow walked it off every second
    e.Block("anubarak side tank follow hold", Role::Any, AnubarakSideTankHolds, Family::Follow);
    // phase 3: it waits at its hold for a burrower still far off or already on it, and the pickup taunts
    // one as it comes in
    e.Block("anubarak side tank swarm leash", Role::Any, AnubarakSideTankSwarmLeash,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Charge,
            {"reach melee", "reach spell"});
    // the escort keeps the kiter in Hand of Protection's reach, whatever its target or heal target
    e.Block("anubarak hop escort hold", Role::Any, IsAnubarakEscort,
            Family::SetBehind | Family::RearFlank | Family::CombatFormationMove | Family::Follow | Family::Charge,
            {"reach melee", "reach spell", "reach party member to heal", "anubarak melee burrower stand"});
    // a GCD or a cast under way when the Hand comes due delays it past the catch; the Hand itself goes
    // through the protect kiter action, which no Spell family covers
    e.Block("anubarak hand reserve", Role::Any, AnubarakHandReserved, Family::Spell,
            {"anubarak heal penetrating cold", "anubarak interrupt shadow strike"});
    e.Block("anubarak main tank scarab taunt guard", Role::Any, AnubarakMainTankLeavesScarabs, Family::Taunt);
    // the class triggers send Misdirection and Tricks to the main tank, pulling the burrowers off their side
    // tanks; the encounter's redirect sends both to the side tanks
    e.Block("anubarak redirect guard", Role::Any, AnubarakEngaged, 0,
            {"misdirection on main tank", "tricks of the trade on main tank"});
    // he obeys taunt DR, so two tanks trading taunts soon leave him immune to both
    e.Block("anubarak taunt guard", Role::Any, AnubarakTauntWouldSteal, Family::Taunt);
    e.Block("anubarak sphere shooter drop guard", Role::Any, AnubarakShooterOnDutySphere, Family::DropTarget);
    e.Block("anubarak grip reserve", Role::Any, AnubarakGripOwed, 0, {"death grip"});
    // the pickup casts its grip directly; the rotation's would yank a burrower the tank just taunted
    e.Block("anubarak side tank grip reserve", Role::Any, AnubarakSideTankHolds, 0, {"death grip"});
    // the class chain's Righteous Defense off the main tank taunts him too; the pickup casts its own
    e.Block("anubarak side tank righteous defense reserve", Role::Any, AnubarakSideTankHolds, 0,
            {"righteous defense"});
    // the ghouls taunt burrowers off their side tanks and off the patches, where they submerge to full
    e.Block("anubarak no ghouls", Role::Any, AnubarakEngaged, 0, {"army of the dead"});
    // heroic phase 3: the swarm holds everyone at 10-25%, so the class nodes spent the tank saves on whoever
    // dipped first; the tank rescue casts them on a tank directly
    e.Block("anubarak swarm save hold", Role::Any, AnubarakSwarmHealing, 0,
            {"lay on hands", "lay on hands on party", "pain suppression", "pain suppression on party",
             "guardian spirit on party"});
    // phase 3: the class nodes spend these at the start, leaving Penetrating Cold carriers nothing, and Mana Shield
    // drains a mage dry. Power Word: Shield on itself is a buff, which the swarm heal multiplier misses.
    e.Block("anubarak swarm self save hold", Role::Any, AnubarakSwarmSavesHeld, 0,
            {"cloak of shadows", "evasion", "feint", "ice block", "divine shield", "divine protection",
             "dispersion", "barkskin", "shamanistic rage", "icebound fortitude", "anti magic shell",
             "deterrence", "enraged regeneration", "mana shield", "power word: shield", "healthstone",
             "healing potion", "lifeblood", "gift of the naaru"});
    // phase 3: Light from a second paladin replaces the first, so the holder judges him through the encounter
    e.Block("anubarak swarm light hold", Role::Any, AnubarakSwarmLightHeld, 0, {"judgement of light"});
    // a paladin's judgements replace each other, so one whose Light is on him casts no other
    e.Block("anubarak swarm judgement hold", Role::Any, AnubarakSwarmJudgementHeld, 0,
            {"judgement", "judgement of wisdom", "judgement of justice"});
    // phase 3: the water totem strategy would put Mana Spring back over Healing Stream
    e.Block("anubarak swarm mana spring hold", Role::Any, AnubarakLeechingSwarmActive, 0, {"mana spring totem"});
    e.Block("anubarak kite immunity reserve", Role::Any, AnubarakKiteImmunitiesKept, 0,
            {"divine shield", "ice block", "vanish", "feign death"});
    e.Block("anubarak kite sprint reserve", Role::Any, AnubarakKiteSprintKept, 0, {"sprint", "dash"});
    // heroic: a marked druid shifts to Cat Form for Dash, and its spec's form trigger would shift it straight
    // back and end the Dash. Once Dash is spent the cat drop takes it out.
    e.Block("anubarak kite cat hold", Role::Any, AnubarakKiteCatHeld, 0,
            {"caster form", "bear form", "dire bear form", "tree of life", "moonkin form"});
    e.Block("anubarak emerge bear hold", Role::Any, AnubarakMainTankKeepsBear, 0, {"caster form"});
    // the side tanks' pickups; from the main tank they pull the burrowers walking past onto him
    e.Block("anubarak main tank shout guard", Role::Any, AnubarakMainTankEngaged, 0,
            {"challenging roar", "challenging shout"});
    // heroic phase 3: a group heal tops up the non-tanks, and Leeching Swarm hands it all back to him
    e.Block("anubarak swarm group heal guard", Role::Any, AnubarakSwarmHealing, 0,
            {"chain heal on party", "prayer of healing on party", "circle of healing on party",
             "wild growth on party", "wild growth blanket", "divine hymn", "tranquility"});
    // the druid's own Innervate goes to the healer with the least mana, through the encounter's node
    e.Block("anubarak self innervate guard", Role::Any, AnubarakSelfInnervateHeld, 0, {"innervate"});

    e.Multiplier<AnubarakThreatCapMultiplier>(Family::Spell | Family::Melee);
    e.Multiplier<AnubarakBurrowerThreatCapMultiplier>(Family::Spell | Family::Melee);
    e.Multiplier<AnubarakSphereGuardMultiplier>(Family::Spell | Family::Attack);
    e.Multiplier<AnubarakBurrowerGuardMultiplier>(Family::Spell | Family::Attack);
    e.Multiplier<AnubarakSideTankAreaGuardMultiplier>(Family::Spell);
    e.Multiplier<AnubarakSwarmHealMultiplier>(Family::Spell | Family::ReachHeal);
}
}  // namespace

EncounterDefinition const& ToCAnubarakDefinition()
{
    static EncounterDefinition const definition(static_cast<uint32>(ToCEncounter::Anubarak), &ToCGateOpen,
                                                &DefineAnubarak);
    return definition;
}
