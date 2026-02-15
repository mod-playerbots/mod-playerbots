#include "RaidMcStrategy.h"

#include "CreateNextAction.h"
#include "RaidMcMultipliers.h"
#include "Strategy.h"
#include "RaidMcActions.h"

void RaidMcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Lucifron
    triggers.push_back(
        new TriggerNode(
            "mc lucifron shadow resistance",
            {
                CreateNextAction<LucifronShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Magmadar
    // TODO: Fear ward / tremor totem, or general anti-fear strat development. Same as King Dred (Drak'Tharon) and faction commander (Nexus).
    triggers.push_back(
        new TriggerNode(
            "mc magmadar fire resistance",
            {
                CreateNextAction<MagmadarFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Gehennas
    triggers.push_back(
        new TriggerNode(
            "mc gehennas shadow resistance",
            {
                CreateNextAction<GehennasShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Garr
    triggers.push_back(
        new TriggerNode(
            "mc garr fire resistance",
            {
                CreateNextAction<GarrFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Baron Geddon
    triggers.push_back(
        new TriggerNode(
            "mc baron geddon fire resistance",
            {
                CreateNextAction<BaronGeddonFireResistanceAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mc living bomb debuff",
            {
                CreateNextAction<McMoveFromGroupAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mc baron geddon inferno",
            {
                CreateNextAction<McMoveFromBaronGeddonAction>(ACTION_RAID)
            }
        )
    );

    // Shazzrah
    triggers.push_back(
        new TriggerNode(
            "mc shazzrah ranged",
            {
                CreateNextAction<McShazzrahMoveAwayAction>(ACTION_RAID)
            }
        )
    );

    // Sulfuron Harbinger
    // Alternatively, shadow resistance is also possible.
    triggers.push_back(
        new TriggerNode(
            "mc sulfuron harbinger fire resistance",
            {
                CreateNextAction<SulfuronHarbingerFireResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Golemagg the Incinerator
    triggers.push_back(
        new TriggerNode(
            "mc golemagg fire resistance",
            {
                CreateNextAction<GolemaggTheIncineratorFireResistanceAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mc golemagg mark boss",
            {
                CreateNextAction<McGolemaggMarkBossAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mc golemagg is main tank",
            {
                CreateNextAction<McGolemaggMainTankAttackGolemaggAction>(ACTION_RAID)
            }
        )
    );
    triggers.push_back(
        new TriggerNode(
            "mc golemagg is assist tank",
            {
                CreateNextAction<McGolemaggAssistTankAttackCoreRagerAction>(ACTION_RAID)
            }
        )
    );

    // Majordomo Executus
    triggers.push_back(
        new TriggerNode(
            "mc majordomo shadow resistance",
            {
                CreateNextAction<MajordomoExecutusShadowResistanceAction>(ACTION_RAID)
            }
        )
    );

    // Ragnaros
    triggers.push_back(
        new TriggerNode(
            "mc ragnaros fire resistance",
            {
                CreateNextAction<RagnarosFireResistanceAction>(ACTION_RAID)
            }
        )
    );
}

void RaidMcStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GarrDisableDpsAoeMultiplier(botAI));
    multipliers.push_back(new BaronGeddonAbilityMultiplier(botAI));
    multipliers.push_back(new GolemaggMultiplier(botAI));
}
