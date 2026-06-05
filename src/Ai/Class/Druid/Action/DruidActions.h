/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_DRUIDACTIONS_H
#define _PLAYERBOT_DRUIDACTIONS_H

#include "GenericSpellActions.h"
#include "SharedDefines.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class CastFaerieFireAction : public CastDebuffSpellAction
{
public:
    CastFaerieFireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "faerie fire") {}
};

class CastFaerieFireFeralAction : public CastSpellAction
{
public:
    CastFaerieFireFeralAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "faerie fire (feral)") {}
};

class CastRejuvenationAction : public CastHealingSpellAction
{
public:
    CastRejuvenationAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "rejuvenation") {}
};

class CastRegrowthAction : public CastHealingSpellAction
{
public:
    CastRegrowthAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "regrowth") {}
};

class CastHealingTouchAction : public CastHealingSpellAction
{
public:
    CastHealingTouchAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "healing touch") {}
};

class CastRejuvenationOnPartyAction : public HealPartyMemberAction
{
public:
    CastRejuvenationOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "rejuvenation", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastRegrowthOnPartyAction : public HealPartyMemberAction
{
public:
    CastRegrowthOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "regrowth", 35.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastHealingTouchOnPartyAction : public HealPartyMemberAction
{
public:
    CastHealingTouchOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "healing touch", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastReviveAction : public ResurrectPartyMemberAction
{
public:
    CastReviveAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "revive") {}

    std::vector<NextAction> getPrerequisites() override;
};

class CastRebirthAction : public ResurrectPartyMemberAction
{
public:
    CastRebirthAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "rebirth") {}

    std::vector<NextAction> getPrerequisites() override;
    bool isUseful() override;
};

class CastMarkOfTheWildAction : public CastBuffSpellAction
{
public:
    CastMarkOfTheWildAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "mark of the wild") {}
};

class CastMarkOfTheWildOnPartyAction : public BuffOnPartyAction
{
public:
    CastMarkOfTheWildOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "mark of the wild") {}
};

class CastSurvivalInstinctsAction : public CastBuffSpellAction
{
public:
    CastSurvivalInstinctsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "survival instincts") {}
};

class CastFrenziedRegenerationAction : public CastBuffSpellAction
{
public:
    CastFrenziedRegenerationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "frenzied regeneration") {}
};

class CastThornsAction : public CastBuffSpellAction
{
public:
    CastThornsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "thorns") {}

    bool Execute(Event event) override;
};

class CastThornsOnPartyAction : public BuffOnPartyAction
{
public:
    CastThornsOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "thorns") {}

    bool Execute(Event event) override;
};

class CastThornsOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastThornsOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "thorns", false) {}

    bool Execute(Event event) override;
};

class CastLifebloomOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastLifebloomOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "lifebloom", true) {}

    bool isUseful() override;
};

class CastWrathAction : public CastSpellAction
{
public:
    CastWrathAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "wrath") {}
    bool isUseful() override;
};

class CastStarfallAction : public CastSpellAction
{
public:
    CastStarfallAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "starfall") {}

    bool isUseful() override;
};

class CastHurricaneAction : public CastSpellAction
{
public:
    CastHurricaneAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hurricane") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastTyphoonAction : public CastSpellAction
{
public:
    CastTyphoonAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "typhoon") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
    bool isUseful() override;
};

class CastMoonfireAction : public CastDebuffSpellAction
{
public:
    CastMoonfireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "moonfire", true) {}
};

class CastInsectSwarmAction : public CastDebuffSpellAction
{
public:
    CastInsectSwarmAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "insect swarm", true) {}
};

class CastStarfireAction : public CastSpellAction
{
public:
    CastStarfireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "starfire") {}
    bool isUseful() override;
};

class CastEntanglingRootsAction : public CastSpellAction
{
public:
    CastEntanglingRootsAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "entangling roots") {}
};

class CastEntanglingRootsCcAction : public CastCrowdControlSpellAction
{
public:
    CastEntanglingRootsCcAction(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, "entangling roots") {}
    Value<Unit*>* GetTargetValue() override;
};

class CastHibernateAction : public CastSpellAction
{
public:
    CastHibernateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hibernate") {}
};

class CastHibernateCcAction : public CastCrowdControlSpellAction
{
public:
    CastHibernateCcAction(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, "hibernate") {}
    Value<Unit*>* GetTargetValue() override;
};

class CastCycloneCcAction : public CastCrowdControlSpellAction
{
public:
    CastCycloneCcAction(PlayerbotAI* botAI) : CastCrowdControlSpellAction(botAI, "cyclone") {}
    Value<Unit*>* GetTargetValue() override;
};

class CastNaturesGraspAction : public CastBuffSpellAction
{
public:
    CastNaturesGraspAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "nature's grasp") {}
};

class CastCurePoisonAction : public CastCureSpellAction
{
public:
    CastCurePoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cure poison") {}
};

class CastCurePoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCurePoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cure poison", DISPEL_POISON) {}
};

class CastAbolishPoisonAction : public CastCureSpellAction
{
public:
    CastAbolishPoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "abolish poison") {}
    std::vector<NextAction> getAlternatives() override;
};

class CastAbolishPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastAbolishPoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "abolish poison", DISPEL_POISON)
    {
    }

    std::vector<NextAction> getAlternatives() override;
};

class CastBarkskinAction : public CastBuffSpellAction
{
public:
    CastBarkskinAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "barkskin") {}
};

class CastInnervateAction : public CastSpellAction
{
public:
    CastInnervateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "innervate") {}

    std::string const GetTargetName() override { return "self target"; }
};

class CastInnervateOnHealerAction : public CastSpellAction
{
public:
    CastInnervateOnHealerAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "innervate") {}

    std::string const GetTargetName() override { return "healer low mana"; }
    bool isPossible() override;
    std::vector<NextAction> getPrerequisites() override;
};

class CastTranquilityAction : public CastAoeHealSpellAction
{
public:
    CastTranquilityAction(PlayerbotAI* botAI)
        : CastAoeHealSpellAction(botAI, "tranquility", 15.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastNaturesSwiftnessAction : public CastBuffSpellAction
{
public:
    CastNaturesSwiftnessAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "nature's swiftness") {}
};

class CastWildGrowthOnPartyAction : public HealPartyMemberAction
{
public:
    CastWildGrowthOnPartyAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "wild growth", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastPartySwiftmendAction : public HealPartyMemberAction
{
public:
    CastPartySwiftmendAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "swiftmend", 15.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastPartyNourishAction : public HealPartyMemberAction
{
public:
    CastPartyNourishAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "nourish", 25.0f, HealingManaEfficiency::LOW) {}
};

class CastDruidRemoveCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastDruidRemoveCurseOnPartyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "remove curse", DISPEL_CURSE) {}
};

class CastInsectSwarmOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastInsectSwarmOnAttackerAction(PlayerbotAI* ai) : CastDebuffSpellOnAttackerAction(ai, "insect swarm", true, 0.0f) {}
    bool isUseful() override { return CastAuraSpellAction::isUseful(); }
};

class CastMoonfireOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastMoonfireOnAttackerAction(PlayerbotAI* ai) : CastDebuffSpellOnAttackerAction(ai, "moonfire", true, 0.0f) {}
    bool isUseful() override { return CastAuraSpellAction::isUseful(); }
};

class CastEnrageAction : public CastBuffSpellAction
{
public:
    CastEnrageAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "enrage") {}
};

class CastRejuvenationOnNotFullAction : public HealPartyMemberAction
{
public:
    CastRejuvenationOnNotFullAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "rejuvenation", 5.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastForceOfNatureAction : public CastSpellAction
{
public:
    CastForceOfNatureAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "force of nature") {}
};

// Base for blanket HoT actions. Provides GetBlanketTarget() as a member so
// subclasses can use AI_VALUE and the standard context machinery.
class CastBlanketHotAction : public CastSpellAction
{
public:
    CastBlanketHotAction(PlayerbotAI* ai, std::string const& spell) : CastSpellAction(ai, spell)
    {
        range = botAI->GetRange("heal");
    }

protected:
    Unit* GetBlanketTarget(std::string const& auraName);
};

class CastRejuvenationBlanketAction : public CastBlanketHotAction
{
public:
    CastRejuvenationBlanketAction(PlayerbotAI* ai) : CastBlanketHotAction(ai, "rejuvenation") {}
    bool isUseful() override;
    Unit* GetTarget() override;
    std::string const getName() override { return "rejuvenation blanket"; }
};

class CastWildGrowthBlanketAction : public CastBlanketHotAction
{
public:
    CastWildGrowthBlanketAction(PlayerbotAI* ai) : CastBlanketHotAction(ai, "wild growth") {}
    bool isUseful() override;
    Unit* GetTarget() override;
    std::string const getName() override { return "wild growth blanket"; }
};

class EclipseSolarProcTimeValue : public ManualSetValue<time_t>
{
public:
    EclipseSolarProcTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0) {}
};

class EclipseLunarProcTimeValue : public ManualSetValue<time_t>
{
public:
    EclipseLunarProcTimeValue(PlayerbotAI* botAI) : ManualSetValue<time_t>(botAI, 0) {}
};

#endif
