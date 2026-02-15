#include "RaidIccStrategy.h"

#include "CreateNextAction.h"
#include "RaidIccMultipliers.h"
#include "RaidIccActions.h"
#include "combat/UniversalTauntAction.h"

void RaidIccStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    //Lord Marrogwar
    triggers.push_back(
        new TriggerNode(
            "icc lm",
            {
                CreateNextAction<IccLmTankPositionAction>(ACTION_RAID + 5.0f),
                CreateNextAction<IccSpikeAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    //Lady Deathwhisper
    triggers.push_back(
        new TriggerNode(
            "icc dark reckoning",
            {
                CreateNextAction<IccDarkReckoningAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc lady deathwhisper",
            {
                CreateNextAction<IccRangedPositionLadyDeathwhisperAction>(ACTION_MOVE + 2.0f),
                CreateNextAction<IccAddsLadyDeathwhisperAction>(ACTION_RAID + 3.0f),
                CreateNextAction<IccShadeLadyDeathwhisperAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    //Gunship Battle
    triggers.push_back(
        new TriggerNode(
            "icc rotting frost giant tank position",
            {
                CreateNextAction<IccRottingFrostGiantTankPositionAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc gunship cannon near",
            {
                CreateNextAction<IccGunshipEnterCannonAction>(ACTION_RAID + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc in cannon",
            {
                CreateNextAction<IccCannonFireAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc gunship teleport ally",
            {
                CreateNextAction<IccGunshipTeleportAllyAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc gunship teleport horde",
            {
                CreateNextAction<IccGunshipTeleportHordeAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    //DBS
    triggers.push_back(
        new TriggerNode(
            "icc dbs",
            {
                CreateNextAction<IccDbsTankPositionAction>(ACTION_RAID + 3.0f),
                CreateNextAction<IccAddsDbsAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc dbs main tank rune of blood",
            {
                // This requires a specific action to be created
                CreateNextAction<UniversalTauntAction>(ACTION_EMERGENCY + 4.0f)
            }
        )
    );

    //DOGS
    triggers.push_back(
        new TriggerNode(
            "icc stinky precious main tank mortal wound",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_EMERGENCY + 4.0f)
            }
        )
    );

    //FESTERGUT
    triggers.push_back(
        new TriggerNode(
            "icc festergut group position",
            {
                CreateNextAction<IccFestergutGroupPositionAction>(ACTION_MOVE + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc festergut main tank gastric bloat",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_EMERGENCY + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc festergut spore",
            {
                CreateNextAction<IccFestergutSporeAction>(ACTION_MOVE + 5.0f)
            }
        )
    );

    //ROTFACE
    triggers.push_back(
        new TriggerNode(
            "icc rotface tank position",
            {
                CreateNextAction<IccRotfaceTankPositionAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc rotface group position",
            {
                CreateNextAction<IccRotfaceGroupPositionAction>(ACTION_RAID + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc rotface move away from explosion",
            {
                CreateNextAction<IccRotfaceMoveAwayFromExplosionAction>(ACTION_RAID + 7.0f)
            }
        )
    );

    //PP
    triggers.push_back(
        new TriggerNode(
            "icc putricide volatile ooze",
            {
                CreateNextAction<IccPutricideVolatileOozeAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc putricide gas cloud",
            {
                CreateNextAction<IccPutricideGasCloudAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc putricide growing ooze puddle",
            {
                CreateNextAction<IccPutricideGrowingOozePuddleAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc putricide main tank mutated plague",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_RAID + 10.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc putricide malleable goo",
            {
                CreateNextAction<IccPutricideAvoidMalleableGooAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    //BPC
    triggers.push_back(
        new TriggerNode(
            "icc bpc keleseth tank",
            {
                CreateNextAction<IccBpcKelesethTankAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bpc main tank",
            {
                CreateNextAction<IccBpcMainTankAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bpc empowered vortex",
            {
                CreateNextAction<IccBpcEmpoweredVortexAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bpc kinetic bomb",
            {
                CreateNextAction<IccBpcKineticBombAction>(ACTION_RAID + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bpc ball of flame",
            {
                CreateNextAction<IccBpcBallOfFlameAction>(ACTION_RAID + 7.0f)
            }
        )
    );

    //BQL
    triggers.push_back(
        new TriggerNode(
            "icc bql group position",
            {
                CreateNextAction<IccBqlGroupPositionAction>(ACTION_RAID)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bql pact of darkfallen",
            {
                CreateNextAction<IccBqlPactOfDarkfallenAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc bql vampiric bite",
            {
                CreateNextAction<IccBqlVampiricBiteAction>(ACTION_EMERGENCY + 5.0f)
            }
        )
    );

    //Sister Svalna
    triggers.push_back(
        new TriggerNode(
            "icc valkyre spear",
            {
                CreateNextAction<IccValkyreSpearAction>(ACTION_EMERGENCY + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sister svalna",
            {
                CreateNextAction<IccSisterSvalnaAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    //VDW
    triggers.push_back(
        new TriggerNode(
            "icc valithria group",
            {
                CreateNextAction<IccValithriaGroupAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc valithria portal",
            {
                CreateNextAction<IccValithriaPortalAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc valithria heal",
            {
                CreateNextAction<IccValithriaHealAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc valithria dream cloud",
            {
                CreateNextAction<IccValithriaDreamCloudAction>(ACTION_RAID + 4.0f)
            }
        )
    );

    //SINDRAGOSA
    triggers.push_back(
        new TriggerNode(
            "icc sindragosa group position",
            {
                CreateNextAction<IccSindragosaGroupPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa frost beacon",
            {
                CreateNextAction<IccSindragosaFrostBeaconAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa blistering cold",
            {
                CreateNextAction<IccSindragosaBlisteringColdAction>(ACTION_EMERGENCY + 4.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa unchained magic",
            {
                CreateNextAction<IccSindragosaUnchainedMagicAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa chilled to the bone",
            {
                CreateNextAction<IccSindragosaChilledToTheBoneAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa mystic buffet",
            {
                CreateNextAction<IccSindragosaMysticBuffetAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa main tank mystic buffet",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_EMERGENCY + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa frost bomb",
            {
                CreateNextAction<IccSindragosaFrostBombAction>(ACTION_RAID + 7.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc sindragosa tank swap position",
            {
                CreateNextAction<IccSindragosaTankSwapPositionAction>(ACTION_EMERGENCY + 2.0f)
            }
        )
    );

    //LICH KING
    triggers.push_back(
        new TriggerNode(
            "icc lich king shadow trap",
            {
                CreateNextAction<IccLichKingShadowTrapAction>(ACTION_RAID + 6.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc lich king necrotic plague",
            {
                CreateNextAction<IccLichKingNecroticPlagueAction>(ACTION_RAID + 3.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc lich king winter",
            {
                CreateNextAction<IccLichKingWinterAction>(ACTION_RAID + 5.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "icc lich king adds",
            {
                CreateNextAction<IccLichKingAddsAction>(ACTION_RAID + 2.0f)
            }
        )
    );
}

void RaidIccStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IccLadyDeathwhisperMultiplier(botAI));
    multipliers.push_back(new IccAddsDbsMultiplier(botAI));
    multipliers.push_back(new IccDogsMultiplier(botAI));
    multipliers.push_back(new IccFestergutMultiplier(botAI));
    multipliers.push_back(new IccRotfaceMultiplier(botAI));
    multipliers.push_back(new IccAddsPutricideMultiplier(botAI));
    multipliers.push_back(new IccBpcAssistMultiplier(botAI));
    multipliers.push_back(new IccBqlMultiplier(botAI));
    multipliers.push_back(new IccValithriaDreamCloudMultiplier(botAI));
    multipliers.push_back(new IccSindragosaMultiplier(botAI));
    multipliers.push_back(new IccLichKingAddsMultiplier(botAI));
}
