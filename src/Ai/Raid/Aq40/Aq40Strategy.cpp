#include "Aq40Strategy.h"

#include "Aq40Multipliers.h"

void RaidAq40Strategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aq40 bot is not in combat",
        { NextAction("aq40 erase timers and trackers", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("aq40 resistance strategy check",
        { NextAction("aq40 manage resistance strategies", ACTION_NORMAL) }));

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

    triggers.push_back(new TriggerNode("aq40 sartura active",
        { NextAction("aq40 sartura choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 sartura whirlwind",
        { NextAction("aq40 sartura avoid whirlwind", ACTION_RAID + 4) }));

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

    triggers.push_back(new TriggerNode("aq40 fankriss active",
        { NextAction("aq40 fankriss choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 fankriss spawn active",
        { NextAction("aq40 fankriss choose target", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 fankriss mortal wound",
        { NextAction("aq40 fankriss tank swap", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("aq40 trash active",
        { NextAction("aq40 trash choose target", ACTION_RAID + 2) }));
    triggers.push_back(new TriggerNode("aq40 trash dangerous aoe",
        { NextAction("aq40 trash avoid dangerous aoe", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("aq40 huhuran active",
        {
            NextAction("aq40 huhuran choose target", ACTION_RAID + 2),
        }));
    triggers.push_back(new TriggerNode("aq40 huhuran poison phase",
        { NextAction("aq40 huhuran poison spread", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("aq40 twin active",
        {
            NextAction("aq40 twin tank", ACTION_RAID + 3),
            NextAction("aq40 twin warlock tank", ACTION_RAID + 3),
            NextAction("aq40 twin healer anchor", ACTION_RAID + 3),
            NextAction("aq40 twin choose target", ACTION_RAID + 2),
            NextAction("aq40 twin avoid veklor", ACTION_RAID + 2),
        }));
    triggers.push_back(new TriggerNode("aq40 twin teleport",
        {
            NextAction("aq40 twin tank", ACTION_RAID + 5),
            NextAction("aq40 twin warlock tank", ACTION_RAID + 5),
            NextAction("aq40 twin healer anchor", ACTION_RAID + 5),
            NextAction("aq40 twin avoid veklor", ACTION_RAID + 5),
            NextAction("aq40 twin choose target", ACTION_RAID + 4),
        }));
    triggers.push_back(new TriggerNode("aq40 twin hazard",
        { NextAction("aq40 twin avoid hazard", ACTION_RAID + 6) }));
    triggers.push_back(new TriggerNode("aq40 twin veklor range",
        { NextAction("aq40 twin avoid veklor", ACTION_RAID + 5) }));
    triggers.push_back(new TriggerNode("aq40 twin bug",
        { NextAction("aq40 twin choose target", ACTION_RAID + 4) }));

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

    triggers.push_back(new TriggerNode("aq40 viscidus active",
        {
            NextAction("aq40 viscidus choose target", ACTION_RAID + 2),
            NextAction("aq40 viscidus use frost", ACTION_RAID + 3),
        }));
    triggers.push_back(new TriggerNode("aq40 viscidus frozen",
        { NextAction("aq40 viscidus shatter", ACTION_RAID + 4) }));
    triggers.push_back(new TriggerNode("aq40 viscidus globs present",
        { NextAction("aq40 viscidus choose target", ACTION_RAID + 5) }));

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
