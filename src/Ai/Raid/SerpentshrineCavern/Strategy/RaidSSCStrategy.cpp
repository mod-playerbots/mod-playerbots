#include "RaidSSCStrategy.h"
#include "RaidSSCMultipliers.h"

void RaidSSCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General
    triggers.push_back(new TriggerNode("serpent shrine cavern bot is not in combat", {
        NextAction("serpent shrine cavern erase timers and trackers", ACTION_EMERGENCY + 11) }));

    // Trash Mobs
    triggers.push_back(new TriggerNode("underbog colossus spawned toxic pool after death", {
        NextAction("underbog colossus escape toxic pool", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("greyheart tidecaller water elemental totem spawned", {
        NextAction("greyheart tidecaller mark water elemental totem", ACTION_RAID + 1) }));

    // Hydross the Unstable <Duke of Currents>
    triggers.push_back(new TriggerNode("hydross the unstable bot is frost tank", {
        NextAction("hydross the unstable position frost tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable bot is nature tank", {
        NextAction("hydross the unstable position nature tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable elementals spawned", {
        NextAction("hydross the unstable prioritize elemental adds", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable danger from water tombs", {
        NextAction("hydross the unstable frost phase spread out", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("hydross the unstable tank needs aggro upon phase change", {
        NextAction("hydross the unstable misdirect boss to tank", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("hydross the unstable aggro resets upon phase change", {
        NextAction("hydross the unstable stop dps upon phase change", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("hydross the unstable need to manage timers", {
        NextAction("hydross the unstable manage timers", ACTION_EMERGENCY + 10) }));

    // The Lurker Below
    triggers.push_back(new TriggerNode("the lurker below spout is active", {
        NextAction("the lurker below run around behind boss", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("the lurker below boss is active for main tank", {
        NextAction("the lurker below position main tank", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("the lurker below boss casts geyser", {
        NextAction("the lurker below spread ranged in arc", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("the lurker below boss is submerged", {
        NextAction("the lurker below tanks pick up adds", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("the lurker below need to prepare timer for spout", {
        NextAction("the lurker below manage spout timer", ACTION_EMERGENCY + 10) }));

    // Leotheras the Blind
    triggers.push_back(new TriggerNode("leotheras the blind boss is inactive", {
        NextAction("leotheras the blind target spellbinders", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind boss transformed into demon form", {
        NextAction("leotheras the blind demon form tank attack boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind only warlock should tank demon form", {
        NextAction("leotheras the blind melee tanks don't attack demon form", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind boss engaged by ranged", {
        NextAction("leotheras the blind position ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind boss channeling whirlwind", {
        NextAction("leotheras the blind run away from whirlwind", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind bot has too many chaos blast stacks", {
        NextAction("leotheras the blind melee dps run away from boss", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("leotheras the blind inner demon has awakened", {
        NextAction("leotheras the blind destroy inner demon", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("leotheras the blind entered final phase", {
        NextAction("leotheras the blind final phase assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("leotheras the blind demon form tank needs aggro", {
        NextAction("leotheras the blind misdirect boss to demon form tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("leotheras the blind boss wipes aggro upon phase change", {
        NextAction("leotheras the blind manage dps wait timers", ACTION_EMERGENCY + 10) }));

    // Fathom-Lord Karathress
    triggers.push_back(new TriggerNode("fathom-lord karathress boss engaged by main tank", {
        NextAction("fathom-lord karathress main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress caribdis engaged by first assist tank", {
        NextAction("fathom-lord karathress first assist tank position caribdis", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress sharkkis engaged by second assist tank", {
        NextAction("fathom-lord karathress second assist tank position sharkkis", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress tidalvess engaged by third assist tank", {
        NextAction("fathom-lord karathress third assist tank position tidalvess", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress caribdis tank needs dedicated healer", {
        NextAction("fathom-lord karathress position caribdis tank healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress pulling bosses", {
        NextAction("fathom-lord karathress misdirect bosses to tanks", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress determining kill order", {
        NextAction("fathom-lord karathress assign dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("fathom-lord karathress tanks need to establish aggro", {
        NextAction("fathom-lord karathress manage dps timer", ACTION_EMERGENCY + 10) }));

    // Morogrim Tidewalker
    triggers.push_back(new TriggerNode("morogrim tidewalker boss engaged by main tank", {
        NextAction("morogrim tidewalker move boss to tank position", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker water globules are incoming", {
        NextAction("morogrim tidewalker phase 2 reposition ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("morogrim tidewalker pulling boss", {
        NextAction("morogrim tidewalker misdirect boss to main tank", ACTION_RAID + 1) }));

    // Lady Vashj <Coilfang Matron>
    triggers.push_back(new TriggerNode("lady vashj boss engaged by main tank", {
        NextAction("lady vashj main tank position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj boss engaged by ranged in phase 1", {
        NextAction("lady vashj phase 1 spread ranged in arc", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj casts shock blast on highest aggro", {
        NextAction("lady vashj set grounding totem in main tank group", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj bot has static charge", {
        NextAction("lady vashj static charge move away from group", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("lady vashj pulling boss in phase 1 and phase 3", {
        NextAction("lady vashj misdirect boss to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("lady vashj tainted elemental cheat", {
        NextAction("lady vashj teleport to tainted elemental", ACTION_EMERGENCY + 10),
        NextAction("lady vashj loot tainted core", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("lady vashj tainted core was looted", {
        NextAction("lady vashj pass the tainted core", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("lady vashj tainted core is unusable", {
        NextAction("lady vashj destroy tainted core", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj need to reset core passing trackers", {
        NextAction("lady vashj erase core passing trackers", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("lady vashj adds spawn in phase 2 and phase 3", {
        NextAction("lady vashj assign phase 2 and phase 3 dps priority", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("lady vashj coilfang strider is approaching", {
        NextAction("lady vashj misdirect strider to first assist tank", ACTION_EMERGENCY + 2),
        NextAction("lady vashj tank attack and move away strider", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("lady vashj toxic sporebats are spewing poison clouds", {
        NextAction("lady vashj avoid toxic spores", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("lady vashj bot is entangled in toxic spores or static charge", {
        NextAction("lady vashj use free action abilities", ACTION_EMERGENCY + 7) }));
}

void RaidSSCStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Trash Mobs
    multipliers.push_back(new UnderbogColossusEscapeToxicPoolMultiplier(botAI));

    // Hydross the Unstable <Duke of Currents>
    multipliers.push_back(new HydrossTheUnstableDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableWaitForDpsMultiplier(botAI));
    multipliers.push_back(new HydrossTheUnstableControlMisdirectionMultiplier(botAI));

    // The Lurker Below
    multipliers.push_back(new TheLurkerBelowStayAwayFromSpoutMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowMaintainRangedSpreadMultiplier(botAI));
    multipliers.push_back(new TheLurkerBelowDisableTankAssistMultiplier(botAI));

    // Leotheras the Blind
    multipliers.push_back(new LeotherasTheBlindAvoidWhirlwindMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindMeleeDpsAvoidChaosBlastMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindFocusOnInnerDemonMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindWaitForDpsMultiplier(botAI));
    multipliers.push_back(new LeotherasTheBlindDelayBloodlustAndHeroismMultiplier(botAI));

    // Fathom-Lord Karathress
    multipliers.push_back(new FathomLordKarathressDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressDisableAoeMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressControlMisdirectionMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressWaitForDpsMultiplier(botAI));
    multipliers.push_back(new FathomLordKarathressCaribdisTankHealerMaintainPositionMultiplier(botAI));

    // Morogrim Tidewalker
    multipliers.push_back(new MorogrimTidewalkerDelayBloodlustAndHeroismMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerDisableTankActionsMultiplier(botAI));
    multipliers.push_back(new MorogrimTidewalkerMaintainPhase2StackingMultiplier(botAI));

    // Lady Vashj <Coilfang Matron>
    multipliers.push_back(new LadyVashjDelayCooldownsMultiplier(botAI));
    multipliers.push_back(new LadyVashjMaintainPhase1RangedSpreadMultiplier(botAI));
    multipliers.push_back(new LadyVashjStaticChargeStayAwayFromGroupMultiplier(botAI));
    multipliers.push_back(new LadyVashjDoNotLootTheTaintedCoreMultiplier(botAI));
    multipliers.push_back(new LadyVashjCorePassersPrioritizePositioningMultiplier(botAI));
    multipliers.push_back(new LadyVashjDisableAutomaticTargetingAndMovementModifier(botAI));
}
