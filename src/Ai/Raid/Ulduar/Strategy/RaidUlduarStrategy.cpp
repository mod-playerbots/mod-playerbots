#include "RaidUlduarStrategy.h"

#include "BossAuraActions.h"
#include "RaidUlduarMultipliers.h"
#include "RaidUlduarActions.h"
#include "CreateNextAction.h"

void RaidUlduarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //
    // Flame Leviathan
    //
    triggers.push_back(
        new TriggerNode(
            "flame leviathan vehicle near",
            {
                CreateNextAction<FlameLeviathanEnterVehicleAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "flame leviathan on vehicle",
            {
                CreateNextAction<FlameLeviathanVehicleAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    //
    // Razorscale
    //
    triggers.push_back(
        new TriggerNode(
            "razorscale avoid devouring flames",
            {
                CreateNextAction<RazorscaleAvoidDevouringFlameAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale avoid sentinel",
            {
                CreateNextAction<RazorscaleAvoidSentinelAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale flying alone",
            {
                CreateNextAction<RazorscaleIgnoreBossAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale avoid whirlwind",
            {
                CreateNextAction<RazorscaleAvoidWhirlwindAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale grounded",
            {
                CreateNextAction<RazorscaleGroundedAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale harpoon trigger",
            {
                CreateNextAction<RazorscaleHarpoonAction>(ACTION_MOVE)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale fuse armor trigger",
            {
                CreateNextAction<RazorscaleFuseArmorAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razorscale fire resistance trigger",
            {
                CreateNextAction<BossFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    //
    // Ignis
    //
    triggers.push_back(
        new TriggerNode(
            "ignis fire resistance trigger",
            {
                CreateNextAction<BossFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    //
    // Iron Assembly
    //
    triggers.push_back(
        new TriggerNode(
            "iron assembly lightning tendrils trigger",
            {
                CreateNextAction<IronAssemblyLightningTendrilsAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "iron assembly overload trigger",
            {
                CreateNextAction<IronAssemblyOverloadAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "iron assembly rune of power trigger",
            {
                CreateNextAction<IronAssemblyRuneOfPowerAction>(ACTION_RAID)
            }
        )
    );

    //
    // Kologarn
    //
    triggers.push_back(
        new TriggerNode(
            "kologarn fall from floor trigger",
            {
                CreateNextAction<KologarnFallFromFloorAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn rti target trigger",
            {
                CreateNextAction<KologarnRtiTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn eyebeam trigger",
            {
                CreateNextAction<KologarnEyebeamAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn attack dps target trigger",
            {
                CreateNextAction<AttackMyTargetAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn mark dps target trigger",
            {
                CreateNextAction<KologarnMarkDpsTargetAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn nature resistance trigger",
            {
                CreateNextAction<BossNatureResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn rubble slowdown trigger",
            {
                CreateNextAction<KologarnRubbleSlowdownAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "kologarn crunch armor trigger",
            {
                CreateNextAction<KologarnCrunchArmorAction>(ACTION_RAID)
            }
        )
    );

    //
    // Auriaya
    //
    triggers.push_back(
        new TriggerNode(
            "auriaya fall from floor trigger",
            {
                CreateNextAction<AuriayaFallFromFloorAction>(ACTION_RAID)
            }
        )
    );

    //
    // Hodir
    //
    triggers.push_back(
        new TriggerNode(
            "hodir near snowpacked icicle",
            {
                CreateNextAction<HodirMoveSnowpackedIcicleAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
        "hodir biting cold",
        {
            CreateNextAction<HodirBitingColdJumpAction>(ACTION_RAID)
        }
    )
);

    triggers.push_back(
        new TriggerNode(
            "hodir frost resistance trigger",
            {
                CreateNextAction<BossFrostResistanceAction>(ACTION_RAID)
            }
        )
    );

    //
    // Freya
    //
    triggers.push_back(
        new TriggerNode(
            "freya near nature bomb",
            {
                CreateNextAction<FreyaMoveAwayNatureBombAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "freya nature resistance trigger",
            {
                CreateNextAction<BossNatureResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "freya fire resistance trigger",
            {
                CreateNextAction<BossFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "freya mark dps target trigger",
            {
                CreateNextAction<FreyaMarkDpsTargetAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
        "freya move to healing spore trigger",
        {
            CreateNextAction<FreyaMoveToHealingSporeAction>(ACTION_RAID)
        }
    )
);

    //
    // Thorim
    //
    triggers.push_back(
        new TriggerNode(
            "thorim nature resistance trigger",
            {
                CreateNextAction<BossNatureResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim frost resistance trigger",
            {
                CreateNextAction<BossFrostResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim unbalancing strike trigger",
            {
                CreateNextAction<ThorimUnbalancingStrikeAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim mark dps target trigger",
            {
                CreateNextAction<ThorimMarkDpsTargetAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim gauntlet positioning trigger",
            {
                CreateNextAction<ThorimGauntletPositioningAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim arena positioning trigger",
            {
                CreateNextAction<ThorimArenaPositioningAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim fall from floor trigger",
            {
                CreateNextAction<ThorimFallFromFloorAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thorim phase 2 positioning trigger",
            {
                CreateNextAction<ThorimPhase2PositioningAction>(ACTION_RAID)
            }
        )
    );

    //
    // Mimiron
    //
    triggers.push_back(
        new TriggerNode(
            "mimiron p3wx2 laser barrage trigger",
            {
                CreateNextAction<MimironP3Wx2LaserBarrageAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron shock blast trigger",
            {
                CreateNextAction<MimironShockBlastAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron fire resistance trigger",
            {
                CreateNextAction<BossFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron phase 1 positioning trigger",
            {
                CreateNextAction<MimironPhase1PositioningAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron rapid burst trigger",
            {
                CreateNextAction<MimironRapidBurstAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron aerial command unit trigger",
            {
                CreateNextAction<MimironAerialCommandUnitAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron rocket strike trigger",
            {
                CreateNextAction<MimironRocketStrikeAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron phase 4 mark dps trigger",
            {
                CreateNextAction<MimironPhase4MarkDpsAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mimiron cheat trigger",
            {
                CreateNextAction<MimironCheatAction>(ACTION_RAID)
            }
        )
    );

    //
    // General Vezax
    //
    triggers.push_back(
        new TriggerNode(
            "vezax cheat trigger",
            {
                CreateNextAction<VezaxCheatAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "vezax shadow crash trigger",
            {
                CreateNextAction<VezaxShadowCrashAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "vezax mark of the faceless trigger",
            {
                CreateNextAction<VezaxMarkOfTheFacelessAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "vezax shadow resistance trigger",
            {
                CreateNextAction<BossShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    //
    // Yogg-Saron
    //
    triggers.push_back(
        new TriggerNode(
            "sara shadow resistance trigger",
            {
                CreateNextAction<BossShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron shadow resistance trigger",
            {
                CreateNextAction<BossShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron ominous cloud cheat trigger",
            {
                CreateNextAction<YoggSaronOminousCloudCheatAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron guardian positioning trigger",
            {
                CreateNextAction<YoggSaronGuardianPositioningAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron sanity trigger",
            {
                CreateNextAction<YoggSaronSanityAction>(ACTION_RAID + 1)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron death orb trigger",
            {
                CreateNextAction<YoggSaronDeathOrbAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron malady of the mind trigger",
            {
                CreateNextAction<YoggSaronMaladyOfTheMindAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron mark target trigger",
            {
                CreateNextAction<YoggSaronMarkTargetAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron brain link trigger",
            {
                CreateNextAction<YoggSaronBrainLinkAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron move to enter portal trigger",
            {
                CreateNextAction<YoggSaronMoveToEnterPortalAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron use portal trigger",
            {
                CreateNextAction<YoggSaronUsePortalAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron fall from floor trigger",
            {
                CreateNextAction<YoggSaronFallFromFloorAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron boss room movement cheat trigger",
            {
                CreateNextAction<YoggSaronBossRoomMovementCheatAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron illusion room trigger",
            {
                CreateNextAction<YoggSaronIllusionRoomAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron move to exit portal trigger",
            {
                CreateNextAction<YoggSaronMoveToExitPortalAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron lunatic gaze trigger",
            {
                CreateNextAction<YoggSaronLunaticGazeAction>(ACTION_EMERGENCY)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "yogg-saron phase 3 positioning trigger",
            {
                CreateNextAction<YoggSaronPhase3PositioningAction>(ACTION_RAID)
            }
        )
    );
}

void RaidUlduarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(
        new FlameLeviathanMultiplier(botAI));
}
