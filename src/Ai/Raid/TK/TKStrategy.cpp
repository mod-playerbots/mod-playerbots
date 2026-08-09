/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TKStrategy.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "TKHelpers.h"
#include "TKKaelthasBossAI.h"
#include "TKMultipliers.h"

void RaidTempestKeepStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("tempest keep bot is not in combat", {
        NextAction("tempest keep reset encounter states", ACTION_EMERGENCY + 10) }));

    // Trash
    triggers.push_back(new TriggerNode("crimson hand centurion casts arcane volley", {
        NextAction("crimson hand centurion cast polymorph", ACTION_RAID) }));

    // Al'ar <Phoenix God>
    triggers.push_back(new TriggerNode("al'ar pulling boss", {
        NextAction("al'ar misdirect boss to main tank", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("al'ar boss is flying between platforms", {
        NextAction("al'ar boss tanks move between platforms", ACTION_RAID),
        NextAction("al'ar melee dps move between platforms", ACTION_RAID),
        NextAction("al'ar ranged and ember tank move under platforms", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("al'ar embers of al'ar explode upon death", {
        NextAction("al'ar assist tanks pick up embers", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("al'ar killing embers of al'ar damages boss", {
        NextAction("al'ar ranged dps prioritize embers", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("al'ar incoming flame quills", {
        NextAction("al'ar jump from platform", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("al'ar rising from the ashes", {
        NextAction("al'ar move away from rebirth", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("al'ar everything is on fire in phase 2", {
        NextAction("al'ar swap tanks on boss", ACTION_EMERGENCY + 2),
        NextAction("al'ar avoid flame patches and dive bombs", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("al'ar should manage phase tracker", {
        NextAction("al'ar manage phase tracker", ACTION_EMERGENCY + 10) }));

    // Void Reaver
    triggers.push_back(new TriggerNode("void reaver boss casts pounding", {
        NextAction("void reaver tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("void reaver knock away reduces tank aggro", {
        NextAction("void reaver use aggro dump ability", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("void reaver ranged should stand back", {
        NextAction("void reaver keep ranged in goldilocks zone", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("void reaver arcane orb is incoming", {
        NextAction("void reaver avoid arcane orb", ACTION_EMERGENCY + 1) }));

    // High Astromancer Solarian
    triggers.push_back(new TriggerNode("high astromancer solarian engaged by main tank", {
        NextAction("high astromancer solarian main tank pick up boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("high astromancer solarian bot has wrath of the astromancer", {
        NextAction("high astromancer solarian move away from group", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("high astromancer solarian solarium priests spawned", {
        NextAction("high astromancer solarian target solarium priests", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high astromancer solarian boss casts psychic scream", {
        NextAction("high astromancer solarian cast fear ward on main tank", ACTION_RAID + 1) }));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    triggers.push_back(new TriggerNode("kael'thas sunstrider thaladred is fixated on bot", {
        NextAction("kael'thas sunstrider kite thaladred", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider pulling tankable advisors", {
        NextAction("kael'thas sunstrider misdirect advisors to tanks", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider sanguinar engaged by main tank", {
        NextAction("kael'thas sunstrider main tank position sanguinar", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider sanguinar casts bellowing roar", {
        NextAction("kael'thas sunstrider cast fear ward on sanguinar tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider capernian should be tanked by a warlock", {
        NextAction("kael'thas sunstrider warlock tank position capernian", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider capernian casts arcane burst and conflagration", {
        NextAction("kael'thas sunstrider spread and move away from capernian", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider telonicus engaged by first assist tank", {
        NextAction("kael'thas sunstrider first assist tank position telonicus", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider bots have specific roles in phase 3", {
        NextAction("kael'thas sunstrider handle advisor roles in phase 3", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider determining advisor kill order", {
        NextAction("kael'thas sunstrider assign advisor dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider should manage advisor dps timer", {
        NextAction("kael'thas sunstrider manage advisor dps timer", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are alive", {
        NextAction("kael'thas sunstrider assign legendary weapon dps priority", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary axe casts whirlwind", {
        NextAction("kael'thas sunstrider move devastation away", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are dead and lootable", {
        NextAction("kael'thas sunstrider loot legendary weapons", ACTION_NORMAL) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons are equipped", {
        NextAction("kael'thas sunstrider use legendary weapons", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider legendary weapons were lost", {
        NextAction("kael'thas sunstrider reequip gear", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider boss has entered the fight", {
        NextAction("kael'thas sunstrider main tank position boss", ACTION_RAID),
        NextAction("kael'thas sunstrider avoid flame strike", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider phoenixes and eggs are spawning", {
        NextAction("kael'thas sunstrider handle phoenixes and eggs", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider raid member is mind controlled", {
        NextAction("kael'thas sunstrider break mind control", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("kael'thas sunstrider boss is manipulating gravity", {
        NextAction("kael'thas sunstrider spread out in midair", ACTION_RAID + 1) }));
}

void RaidTempestKeepStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Alar <Phoenix God>
    multipliers.push_back(new AlarMoveBetweenPlatformsMultiplier(botAI));
    multipliers.push_back(new AlarControlMovementMultiplier(botAI));
    multipliers.push_back(new AlarDisableAutomaticTargetingMultiplier(botAI));
    multipliers.push_back(new AlarStayAwayFromRebirthMultiplier(botAI));
    multipliers.push_back(new AlarControlTauntingMultiplier(botAI));

    // Void Reaver
    multipliers.push_back(new VoidReaverMaintainPositionsMultiplier(botAI));

    // High Astromancer Solarian
    multipliers.push_back(new HighAstromancerSolarianDisableMeleeTargetingMultiplier(botAI));
    multipliers.push_back(new HighAstromancerSolarianWrathStayAwayMultiplier(botAI));

    // Kael'thas Sunstrider <Lord of the Blood Elves>
    multipliers.push_back(new KaelthasSunstriderWaitForDpsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderKiteThaladredMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderKeepDistanceFromCapernianMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderManageWeaponTankingMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderSuppressEquipUpgradeMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderManageAutomaticTargetingMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDisableDisperseMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderPrepareForPhase3Multiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new KaelthasSunstriderStaySpreadDuringGravityLapseMultiplier(botAI));
}

namespace
{

using namespace TkHelpers;

void AppendKaelthasDevastationExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* kaelthas = AI_VALUE2(Unit*, "find target", "kael'thas sunstrider");
    if (!kaelthas)
        return;

    boss_kaelthas* kaelAI = dynamic_cast<boss_kaelthas*>(kaelthas->GetAI());
    if (kaelAI && (kaelAI->GetPhase() < PHASE_WEAPONS || kaelAI->GetPhase() > PHASE_ALL_ADVISORS))
        return;

    constexpr float searchRadius = 75.0f;
    if (Creature* axe = botAI->GetBot()->FindNearestCreature(
            Id(TkNpcs::NPC_DEVASTATION), searchRadius))
    {
        exclusions.insert(axe->GetGUID());
    }
}

void AppendEmberOfAlarExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (auto const& guid : AI_VALUE(GuidVector, "attackers"))
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->GetEntry() == Id(TkNpcs::NPC_EMBER_OF_ALAR))
            exclusions.insert(unit->GetGUID());
    }
}

} // end anonymous namespace

void RaidTempestKeepStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    Player* bot = botAI->GetBot();
    if (!PlayerbotAI::IsMelee(bot) || !PlayerbotAI::IsDps(bot))
        return;

    AppendKaelthasDevastationExclusions(botAI, exclusions);
    AppendEmberOfAlarExclusions(botAI, exclusions);
}
