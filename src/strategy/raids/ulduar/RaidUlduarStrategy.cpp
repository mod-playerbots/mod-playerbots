#include "RaidUlduarStrategy.h"

#include "RaidUlduarMultipliers.h"

void RaidUlduarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Flame Leviathan
    //
    triggers.push_back(new TriggerNode(
        "flame leviathan vehicle near",
        { new NextAction("flame leviathan enter vehicle", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "flame leviathan on vehicle",
        { new NextAction("flame leviathan vehicle", ACTION_RAID + 1) }));

    //
    // Razorscale
    //
    triggers.push_back(new TriggerNode(
        "razorscale avoid devouring flames",
        { new NextAction("razorscale avoid devouring flames", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "razorscale avoid sentinel",
        { new NextAction("razorscale avoid sentinel", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "razorscale flying alone",
    { new NextAction("razorscale ignore flying alone", ACTION_MOVE + 5) }));

    triggers.push_back(new TriggerNode(
        "razorscale avoid whirlwind",
        { new NextAction("razorscale avoid whirlwind", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode(
        "razorscale grounded",
        { new NextAction("razorscale grounded", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "razorscale harpoon trigger",
        { new NextAction("razorscale harpoon action", ACTION_MOVE) }));

    triggers.push_back(new TriggerNode(
        "razorscale fuse armor trigger",
        { new NextAction("razorscale fuse armor action", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "razorscale fire resistance trigger",
        { new NextAction("razorscale fire resistance action", ACTION_RAID) }));

    //
    // Ignis
    //
    triggers.push_back(new TriggerNode(
        "ignis fire resistance trigger",
        { new NextAction("ignis fire resistance action", ACTION_RAID) }));

    //
    // Iron Assembly
    //
    triggers.push_back(new TriggerNode(
        "iron assembly lightning tendrils trigger",
        { new NextAction("iron assembly lightning tendrils action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "iron assembly overload trigger",
        { new NextAction("iron assembly overload action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "iron assembly rune of power trigger",
        { new NextAction("iron assembly rune of power action", ACTION_RAID) }));

    //
    // Kologarn
    //
    triggers.push_back(new TriggerNode(
        "kologarn fall from floor trigger",
        { new NextAction("kologarn fall from floor action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn rti target trigger",
        { new NextAction("kologarn rti target action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn eyebeam trigger",
        { new NextAction("kologarn eyebeam action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "kologarn attack dps target trigger",
        { new NextAction("attack rti target", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn mark dps target trigger",
        { new NextAction("kologarn mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn nature resistance trigger",
        { new NextAction("kologarn nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn rubble slowdown trigger",
        { new NextAction("kologarn rubble slowdown action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "kologarn crunch armor trigger",
        { new NextAction("kologarn crunch armor action", ACTION_RAID) }));

    //
    // Auriaya
    //
    triggers.push_back(new TriggerNode(
        "auriaya fall from floor trigger",
        { new NextAction("auriaya fall from floor action", ACTION_RAID) }));

    //
    // Hodir
    //
    triggers.push_back(new TriggerNode(
        "hodir near snowpacked icicle",
        { new NextAction("hodir move snowpacked icicle", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "hodir biting cold",
        { new NextAction("hodir biting cold jump", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "hodir frost resistance trigger",
        { new NextAction("hodir frost resistance action", ACTION_RAID) }));

    //
    // Freya
    //
    triggers.push_back(new TriggerNode(
        "freya near nature bomb",
        { new NextAction("freya move away nature bomb", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya nature resistance trigger",
        { new NextAction("freya nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya fire resistance trigger",
        { new NextAction("freya fire resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya mark dps target trigger",
        { new NextAction("freya mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "freya move to healing spore trigger",
        { new NextAction("freya move to healing spore action", ACTION_RAID) }));

    //
    // Thorim
    //
    triggers.push_back(new TriggerNode(
        "thorim nature resistance trigger",
        { new NextAction("thorim nature resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim frost resistance trigger",
        { new NextAction("thorim frost resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim unbalancing strike trigger",
        { new NextAction("thorim unbalancing strike action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim mark dps target trigger",
        { new NextAction("thorim mark dps target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim gauntlet positioning trigger",
        { new NextAction("thorim gauntlet positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim arena positioning trigger",
        { new NextAction("thorim arena positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "thorim fall from floor trigger",
        { new NextAction("thorim fall from floor action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "thorim phase 2 positioning trigger",
        { new NextAction("thorim phase 2 positioning action", ACTION_RAID) }));

    //
    // Mimiron
    //
    triggers.push_back(new TriggerNode(
        "mimiron p3wx2 laser barrage trigger",
        { new NextAction("mimiron p3wx2 laser barrage action", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode(
        "mimiron shock blast trigger",
        { new NextAction("mimiron shock blast action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "mimiron fire resistance trigger",
        { new NextAction("mimiron fire resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron phase 1 positioning trigger",
        { new NextAction("mimiron phase 1 positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron rapid burst trigger",
        { new NextAction("mimiron rapid burst action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron aerial command unit trigger",
        { new NextAction("mimiron aerial command unit action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron rocket strike trigger",
        { new NextAction("mimiron rocket strike action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron phase 4 mark dps trigger",
        { new NextAction("mimiron phase 4 mark dps action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "mimiron cheat trigger",
        { new NextAction("mimiron cheat action", ACTION_RAID) }));

    //
    // General Vezax
    //
    triggers.push_back(new TriggerNode(
        "vezax cheat trigger",
        { new NextAction("vezax cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax shadow crash trigger",
        { new NextAction("vezax shadow crash action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax mark of the faceless trigger",
        { new NextAction("vezax mark of the faceless action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "vezax shadow resistance trigger",
        { new NextAction("vezax shadow resistance action", ACTION_RAID) }));

    //
    // Yogg-Saron
    //
    triggers.push_back(new TriggerNode(
        "sara shadow resistance trigger",
        { new NextAction("sara shadow resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron shadow resistance trigger",
        { new NextAction("yogg-saron shadow resistance action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron ominous cloud cheat trigger",
        { new NextAction("yogg-saron ominous cloud cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron guardian positioning trigger",
        { new NextAction("yogg-saron guardian positioning action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron sanity trigger",
        { new NextAction("yogg-saron sanity action", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron death orb trigger",
        { new NextAction("yogg-saron death orb action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron malady of the mind trigger",
        { new NextAction("yogg-saron malady of the mind action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron mark target trigger",
        { new NextAction("yogg-saron mark target action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron brain link trigger",
        { new NextAction("yogg-saron brain link action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron move to enter portal trigger",
        { new NextAction("yogg-saron move to enter portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron use portal trigger",
        { new NextAction("yogg-saron use portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron fall from floor trigger",
        { new NextAction("yogg-saron fall from floor action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron boss room movement cheat trigger",
        { new NextAction("yogg-saron boss room movement cheat action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron illusion room trigger",
        { new NextAction("yogg-saron illusion room action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron move to exit portal trigger",
        { new NextAction("yogg-saron move to exit portal action", ACTION_RAID) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron lunatic gaze trigger",
        { new NextAction("yogg-saron lunatic gaze action", ACTION_EMERGENCY) }));

    triggers.push_back(new TriggerNode(
        "yogg-saron phase 3 positioning trigger",
        { new NextAction("yogg-saron phase 3 positioning action", ACTION_RAID) }));
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new FlameLeviathanMultiplier(botAI));
}
