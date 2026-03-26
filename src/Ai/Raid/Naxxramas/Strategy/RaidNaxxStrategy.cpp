#include "RaidNaxxStrategy.h"

#include "CreateNextAction.h"
#include "MovementActions.h"
#include "RaidNaxxActions.h"
#include "RaidNaxxMultipliers.h"
#include "UniversalTauntAction.h"

void RaidNaxxStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Grobbulus
    triggers.push_back(
        new TriggerNode(
            "mutating injection melee",
            {
                CreateNextAction<GrobbulusMoveAwayAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mutating injection ranged",
            {
                CreateNextAction<GrobbulusGoBehindAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "mutating injection removed",
            {
                CreateNextAction<GrobbulusMoveCenterAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "grobbulus cloud",
            {
                CreateNextAction<GrobbulusRotateAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Kel'Thuzad
    triggers.push_back(
        new TriggerNode(
            "kel'thuzad",
            {
                CreateNextAction<KelthuzadPositionAction>(ACTION_RAID + 2.0f),
                CreateNextAction<KelthuzadChooseTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Anub'Rekhan
    triggers.push_back(
        new TriggerNode(
            "anub'rekhan",
            {
                CreateNextAction<AnubrekhanPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

     // Grand Widow Faerlina
    triggers.push_back(
        new TriggerNode(
            "faerlina",
            {
                CreateNextAction<AvoidAoeAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Maexxna
    triggers.push_back(
        new TriggerNode(
            "maexxna",
            {
                CreateNextAction<RearFlankAction>(ACTION_RAID + 1.0f),
                CreateNextAction<AvoidAoeAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Thaddius
    triggers.push_back(
        new TriggerNode(
            "thaddius phase pet",
            {
                CreateNextAction<ThaddiusAttackNearestPetAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thaddius phase pet lose aggro",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_RAID + 2.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thaddius phase transition",
            {
                CreateNextAction<ThaddiusMoveToPlatformAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "thaddius phase thaddius",
            {
                CreateNextAction<ThaddiusMovePolarityAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Instructor Razuvious
    triggers.push_back(
        new TriggerNode(
            "razuvious tank",
            {
                CreateNextAction<RazuviousUseObedienceCrystalAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "razuvious nontank",
            {
                CreateNextAction<RazuviousTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // four horsemen
    triggers.push_back(
        new TriggerNode(
            "horsemen attractors",
            {
                CreateNextAction<FourHorsemenAttractAlternativelyAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "horsemen except attractors",
            {
                CreateNextAction<FourHorsemenAttactInOrderAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // sapphiron
    triggers.push_back(
        new TriggerNode(
            "sapphiron ground",
            {
                CreateNextAction<SapphironGroundPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    triggers.push_back(
        new TriggerNode(
            "sapphiron flight",
            {
                CreateNextAction<SapphironFlightPositionAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Gluth
    triggers.push_back(
        new TriggerNode("gluth",
        {
            CreateNextAction<GluthChooseTargetAction>(ACTION_RAID + 1.0f),
            CreateNextAction<GluthPositionAction>(ACTION_RAID + 1.0f),
            CreateNextAction<GluthSlowdownAction>(ACTION_RAID)
        })
    );

    triggers.push_back(
        new TriggerNode(
            "gluth main tank mortal wound",
            {
                CreateNextAction<UniversalTauntAction>(ACTION_RAID + 1.0f)
            }
        )
    );

    // Loatheb
    triggers.push_back(
        new TriggerNode(
            "loatheb",
            {
                CreateNextAction<LoathebPositionAction>(ACTION_RAID + 1.0f),
                CreateNextAction<LoathebChooseTargetAction>(ACTION_RAID + 1.0f)
            }
        )
    );
}

void RaidNaxxStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(
        new GrobbulusMultiplier(this->botAI)
    );
    multipliers.push_back(
        new LoathebGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new ThaddiusGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new SapphironGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new InstructorRazuviousGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new KelthuzadGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new AnubrekhanGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new FourHorsemenGenericMultiplier(this->botAI)
    );
    multipliers.push_back(
        new GluthGenericMultiplier(this->botAI)
    );
}
