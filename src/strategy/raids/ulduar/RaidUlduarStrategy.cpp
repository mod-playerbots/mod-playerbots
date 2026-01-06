#include "RaidUlduarStrategy.h"

#include "RaidUlduarMultipliers.h"

void RaidUlduarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Flame Leviathan
    //
    triggers.push_back(new TriggerNode(
        "flame leviathan vehicle near",
        { NextAction("flame leviathan enter vehicle", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "flame leviathan on vehicle",
        { NextAction("flame leviathan vehicle", ACTION_RAID + 1) }));

    //
    // Razorscale
    //
    triggers.push_back(new TriggerNode(
        "razorscale avoid devouring flames",
        { NextAction("razorscale avoid devouring flames", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "razorscale avoid sentinel",
        { NextAction("razorscale avoid sentinel", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "razorscale flying alone",
    { NextAction("razorscale ignore flying alone", ACTION_MOVE + 5) }));

    triggers.push_back(new TriggerNode(
        "razorscale avoid whirlwind",
        { NextAction("razorscale avoid whirlwind", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode(
        "razorscale grounded",
        { NextAction("razorscale grounded", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "razorscale harpoon trigger",
        { NextAction("razorscale harpoon action", ACTION_MOVE) }));

    triggers.push_back(new TriggerNode(
        "razorscale fuse armor trigger",
        { NextAction("razorscale fuse armor action", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "razorscale fire resistance trigger",
        { NextAction("razorscale fire resistance action", ACTION_RAID) }));

    //
    // Ignis
    //
    triggers.push_back(new TriggerNode(
        "ignis fire resistance trigger",
        { NextAction("ignis fire resistance action", ACTION_RAID) }));

    //
    // Iron Assembly
    //
    triggers.push_back(new TriggerNode(
        "iron assembly lightning tendrils trigger",
        { NextAction("iron assembly lightning tendrils action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "iron assembly overload trigger",
        { NextAction("iron assembly overload action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "iron assembly rune of power trigger",
        { NextAction("iron assembly rune of power action", ACTION_RAID) }));

    //
    // Kologarn
    //
    triggers.push_back(new TriggerNode(
        "kologarn fall from floor trigger",
        { NextAction("kologarn fall from floor action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn rti target trigger",
        { NextAction("kologarn rti target action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn eyebeam trigger",
        { NextAction("kologarn eyebeam action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn attack dps target trigger",
        { NextAction("attack rti target", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn mark dps target trigger",
        { NextAction("kologarn mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn nature resistance trigger",
        { NextAction("kologarn nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn rubble slowdown trigger",
        { NextAction("kologarn rubble slowdown action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn crunch armor trigger",
        { NextAction("kologarn crunch armor action", ACTION_RAID) }));

    //
    // Auriaya
    //
    triggers.push_back(new TriggerNode(
        "auriaya fall from floor trigger",
        { NextAction("auriaya fall from floor action", ACTION_RAID) }));

    //
    // Hodir
    //
    triggers.push_back(new TriggerNode(
        "hodir near snowpacked icicle",
        { NextAction("hodir move snowpacked icicle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "hodir biting cold",
        { NextAction("hodir biting cold jump", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "hodir frost resistance trigger",
        { NextAction("hodir frost resistance action", ACTION_RAID) }));

    //
    // Freya
    //
    triggers.push_back(new TriggerNode(
        "freya near nature bomb",
        { NextAction("freya move away nature bomb", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya nature resistance trigger",
        { NextAction("freya nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya fire resistance trigger",
        { NextAction("freya fire resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya mark dps target trigger",
        { NextAction("freya mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya move to healing spore trigger",
        { NextAction("freya move to healing spore action", ACTION_RAID) }));

    //
    // Thorim
    //
    triggers.push_back(new TriggerNode(
        "thorim nature resistance trigger",
        { NextAction("thorim nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim frost resistance trigger",
        { NextAction("thorim frost resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim unbalancing strike trigger",
        { NextAction("thorim unbalancing strike action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim mark dps target trigger",
        { NextAction("thorim mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim gauntlet positioning trigger",
        { NextAction("thorim gauntlet positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim arena positioning trigger",
        { NextAction("thorim arena positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim fall from floor trigger",
        { NextAction("thorim fall from floor action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "thorim phase 2 positioning trigger",
        { NextAction("thorim phase 2 positioning action", ACTION_RAID) }));

    //
    // Mimiron
    //
    triggers.push_back(new TriggerNode(
        "mimiron p3wx2 laser barrage trigger",
        { NextAction("mimiron p3wx2 laser barrage action", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "mimiron shock blast trigger",
        { NextAction("mimiron shock blast action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "mimiron fire resistance trigger",
        { NextAction("mimiron fire resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron phase 1 positioning trigger",
        { NextAction("mimiron phase 1 positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron rapid burst trigger",
        { NextAction("mimiron rapid burst action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron aerial command unit trigger",
        { NextAction("mimiron aerial command unit action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron rocket strike trigger",
        { NextAction("mimiron rocket strike action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron phase 4 mark dps trigger",
        { NextAction("mimiron phase 4 mark dps action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron cheat trigger",
        { NextAction("mimiron cheat action", ACTION_RAID) }));

    //
    // General Vezax
    //
    triggers.push_back(new TriggerNode(
        "vezax cheat trigger",
        { NextAction("vezax cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax shadow crash trigger",
        { NextAction("vezax shadow crash action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax mark of the faceless trigger",
        { NextAction("vezax mark of the faceless action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax shadow resistance trigger",
        { NextAction("vezax shadow resistance action", ACTION_RAID) }));

    //
    // Yogg-Saron
    //
    triggers.push_back(new TriggerNode(
        "sara shadow resistance trigger",
        { NextAction("sara shadow resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron shadow resistance trigger",
        { NextAction("yogg-saron shadow resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron ominous cloud cheat trigger",
        { NextAction("yogg-saron ominous cloud cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron guardian positioning trigger",
        { NextAction("yogg-saron guardian positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron sanity trigger",
        { NextAction("yogg-saron sanity action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron death orb trigger",
        { NextAction("yogg-saron death orb action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron malady of the mind trigger",
        { NextAction("yogg-saron malady of the mind action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron mark target trigger",
        { NextAction("yogg-saron mark target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron brain link trigger",
        { NextAction("yogg-saron brain link action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron move to enter portal trigger",
        { NextAction("yogg-saron move to enter portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron use portal trigger",
        { NextAction("yogg-saron use portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron fall from floor trigger",
        { NextAction("yogg-saron fall from floor action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron boss room movement cheat trigger",
        { NextAction("yogg-saron boss room movement cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron illusion room trigger",
        { NextAction("yogg-saron illusion room action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron move to exit portal trigger",
        { NextAction("yogg-saron move to exit portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron lunatic gaze trigger",
        { NextAction("yogg-saron lunatic gaze action", ACTION_EMERGENCY) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron phase 3 positioning trigger",
        { NextAction("yogg-saron phase 3 positioning action", ACTION_RAID) }));
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FlameLeviathanMultiplier(botAI));
}
