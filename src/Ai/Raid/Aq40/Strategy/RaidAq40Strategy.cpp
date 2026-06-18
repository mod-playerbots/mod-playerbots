#include "RaidAq40Strategy.h"

#include "../Multiplier/RaidAq40Multipliers.h"

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aq40 bot is not in combat",
        { NextAction("aq40 erase timers and trackers", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("aq40 resistance strategy check",
        { NextAction("aq40 manage resistance strategies", ACTION_NORMAL) }));

    // Skeram baseline strategy:
    // - quickly reacquire target after blink/teleport
    // - prioritize interrupt windows
    // - respond to mind control pressure
    // - execute-phase focus on the real boss
    triggers.push_back(new TriggerNode("aq40 skeram active",
        { NextAction("aq40 skeram acquire platform target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 skeram blinked",
        { NextAction("aq40 skeram acquire platform target", ACTION_RAID + 3) }));
    triggers.push_back(new TriggerNode("aq40 skeram interrupt cast",
        { NextAction("aq40 skeram interrupt", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 skeram mc detected",
        { NextAction("aq40 skeram control mind control", ACTION_RAID + 3) }));
    triggers.push_back(new TriggerNode("aq40 skeram execute phase",
        { NextAction("aq40 skeram focus real boss", ACTION_RAID + 4) }));

    // Sartura baseline strategy:
    // - prioritize royal guards before boss
    // - non-tanks step out during whirlwind windows
    triggers.push_back(new TriggerNode("aq40 sartura active",
        { NextAction("aq40 sartura choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 sartura whirlwind",
        { NextAction("aq40 sartura avoid whirlwind", ACTION_RAID + 4) }));

    // Bug Trio baseline strategy:
    // - follow stable kill order and prioritize Yauj heal interruptions
    // - move out from Kri poison cloud death zone
    triggers.push_back(new TriggerNode("aq40 bug trio active",
        { NextAction("aq40 bug trio choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 bug trio heal cast",
        {
            NextAction("aq40 bug trio interrupt heal", ACTION_RAID + 5),
            NextAction("aq40 bug trio choose target", ACTION_RAID + 4),
        }));
    triggers.push_back(new TriggerNode("aq40 bug trio fear risk",
        { NextAction("tremor totem", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 bug trio poison cloud",
        { NextAction("aq40 bug trio avoid poison cloud", ACTION_RAID + 5) }));

    // Fankriss baseline strategy:
    // - kill Spawn of Fankriss immediately
    // - fall back to boss pressure between spawn waves
    // - tank swap when Mortal Wound stacks become dangerous
    triggers.push_back(new TriggerNode("aq40 fankriss active",
        { NextAction("aq40 fankriss choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 fankriss spawn active",
        { NextAction("aq40 fankriss choose target", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 fankriss mortal wound",
        { NextAction("aq40 fankriss tank swap", ACTION_RAID + 5) }));

    // AQ40 trash strategy (Anubisath Defenders only):
    // - choose correct target among defenders
    // - spread out of Thunderclap and Plague AoE
    triggers.push_back(new TriggerNode("aq40 trash active",
        { NextAction("aq40 trash choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 trash dangerous aoe",
        { NextAction("aq40 trash avoid dangerous aoe", ACTION_RAID + 5) }));

    // Huhuran baseline strategy:
    // - maintain boss focus
    // - move ranged non-tanks outward during poison/enrage phase
    // - AQ40 resistance manager enables hunter rnature and shaman Nature Resistance Totem
    //   during Huhuran/Viscidus
    triggers.push_back(new TriggerNode("aq40 huhuran active",
        {
            NextAction("aq40 huhuran choose target", ACTION_RAID + 2),
        }));
    triggers.push_back(new TriggerNode("aq40 huhuran poison phase",
        { NextAction("aq40 huhuran poison spread", ACTION_RAID + 4) }));

    // Twin Emperors registration surface:
    // - use an explicit approach stage before strict ready-to-open staging
    // - route target selection through encounter-specific target bias
    // - use request-first movement wrappers for script-armed hazards
    // - keep post-teleport hold logic under the Twin multiplier instead of generic follow/assist churn
    triggers.push_back(new TriggerNode("aq40 twin approach",
        {
            NextAction("aq40 twin approach stage", ACTION_RAID + 1),
        }));
    triggers.push_back(new TriggerNode("aq40 twin prepull ready",
        {
            NextAction("aq40 twin prepull stage", ACTION_RAID + 2),
        }));
    triggers.push_back(new TriggerNode("aq40 twin dual pull",
        {
            NextAction("aq40 twin dual pull engage", ACTION_RAID + 5),
            NextAction("aq40 twin choose target", ACTION_RAID + 4),
        }));
    triggers.push_back(new TriggerNode("aq40 twin swap prep",
        {
            NextAction("aq40 twin swap prep stage", ACTION_RAID + 6),
        }));
    triggers.push_back(new TriggerNode("aq40 twin active",
        {
            NextAction("aq40 twin choose target", ACTION_RAID + 2),
            NextAction("aq40 twin healer support", ACTION_RAID + 2),
            NextAction("aq40 twin warlock tank", ACTION_RAID + 3),
            NextAction("aq40 twin avoid veklor", ACTION_RAID + 4),
        }));
    triggers.push_back(new TriggerNode("aq40 twin blizzard",
        { NextAction("aq40 twin dodge blizzard", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 twin explode bug",
        { NextAction("aq40 twin dodge explode bug", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("aq40 twin arcane burst risk",
        { NextAction("aq40 twin avoid veklor", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 twin split risk",
        { NextAction("aq40 twin hold split", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 twin post swap hold",
        {
            NextAction("aq40 twin post swap hold", ACTION_RAID + 5),
            NextAction("aq40 twin hold split", ACTION_RAID + 4),
        }));

    // Ouro baseline strategy:
    // - keep melee contact to reduce avoidable submerge windows
    // - non-tanks avoid sweep range and dirt mound submerge hazards
    // - prioritize spawned scarabs immediately
    triggers.push_back(new TriggerNode("aq40 ouro active",
        {
            NextAction("aq40 ouro choose target", ACTION_RAID + 2),
            NextAction("aq40 ouro hold melee contact", ACTION_RAID + 3),
        }));
    triggers.push_back(new TriggerNode("aq40 ouro scarabs present",
        { NextAction("aq40 ouro choose target", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 ouro sweep risk",
        { NextAction("aq40 ouro avoid sweep", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 ouro sand blast risk",
        { NextAction("aq40 ouro avoid sand blast", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 ouro submerge hazard",
        { NextAction("aq40 ouro avoid submerge", ACTION_RAID + 5) }));

    // Viscidus baseline strategy:
    // - maintain boss/glob priority targeting
    // - ranged applies frost pressure in non-frozen phases
    // - melee commits during frozen shatter windows
    triggers.push_back(new TriggerNode("aq40 viscidus active",
        {
            NextAction("aq40 viscidus choose target", ACTION_RAID + 2),
            NextAction("aq40 viscidus use frost", ACTION_RAID + 3),
        }));
    triggers.push_back(new TriggerNode("aq40 viscidus frozen",
        { NextAction("aq40 viscidus shatter", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 viscidus globs present",
        { NextAction("aq40 viscidus choose target", ACTION_RAID + 5) }));

    // C'Thun pass 1 strategy:
    // - maintain spread and add kill priority
    // - react to Dark Glare with lateral movement
    // - stomach team kills Flesh Tentacle and exits on high acid stacks
    triggers.push_back(new TriggerNode("aq40 cthun active",
        {
            NextAction("aq40 cthun choose target", ACTION_RAID + 2),
            NextAction("aq40 cthun maintain spread", ACTION_RAID + 3),
        }));
    triggers.push_back(new TriggerNode("aq40 cthun phase2",
        { NextAction("aq40 cthun phase2 add priority", ACTION_RAID + 3) }));
    triggers.push_back(new TriggerNode("aq40 cthun adds present",
        { NextAction("aq40 cthun phase2 add priority", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 cthun dark glare",
        { NextAction("aq40 cthun avoid dark glare", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 cthun eye cast",
        { NextAction("aq40 cthun interrupt eye", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 cthun in stomach",
        {
            NextAction("aq40 cthun stomach dps", ACTION_RAID + 4),
            NextAction("aq40 cthun stomach exit", ACTION_RAID + 5),
        }));
    triggers.push_back(new TriggerNode("aq40 cthun vulnerable",
        { NextAction("aq40 cthun vulnerable burst", ACTION_RAID + 4) }));
}

void RaidAq40Strategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new Aq40GenericMultiplier(botAI));
    multipliers.push_back(new Aq40SkeramMultiplier(botAI));
    multipliers.push_back(new Aq40BugTrioMultiplier(botAI));
    multipliers.push_back(new Aq40SarturaMultiplier(botAI));
    multipliers.push_back(new Aq40FankrissMultiplier(botAI));
    multipliers.push_back(new Aq40HuhuranMultiplier(botAI));
    multipliers.push_back(new Aq40TwinMultiplier(botAI));
    multipliers.push_back(new Aq40OuroMultiplier(botAI));
    multipliers.push_back(new Aq40ViscidusMultiplier(botAI));
    multipliers.push_back(new Aq40CthunMultiplier(botAI));
}
