/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BTStrategy.h"
#include "BTHelpers.h"
#include "BTMultipliers.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <algorithm>

using namespace BtHelpers;
using namespace EncounterHelpers;

void RaidBlackTempleStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shared
    triggers.push_back(new TriggerNode("black temple no encounter in progress",
        { NextAction("black temple reset encounter states", ACTION_EMERGENCY + 10),
          NextAction("illidari council announce class roles", ACTION_EMERGENCY + 8),
          NextAction("illidan stormrage announce class roles", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("black temple class roles should be assigned",
        { NextAction("black temple assign class roles", ACTION_EMERGENCY + 9) }));

    // Trash
    triggers.push_back(new TriggerNode("sister of pleasure should be focused",
        { NextAction("sister of pleasure focus dps", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("shadowmoon reaver wand builds charges",
        { NextAction("shadowmoon reaver stop wand", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("shadowmoon reaver should control caster pet",
        { NextAction("shadowmoon reaver control caster pet", ACTION_EMERGENCY + 1) }));

    // High Warlord Naj'entus
    triggers.push_back(new TriggerNode("high warlord naj'entus hunter should misdirect",
        { NextAction("high warlord naj'entus misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus should be tanked",
        { NextAction("high warlord naj'entus tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus ranged should spread",
        { NextAction("high warlord naj'entus spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaled player needs remover",
        { NextAction("high warlord naj'entus assign spine remover", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus impaling spine on group member",
        { NextAction("high warlord naj'entus remove impaling spine", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus needs spine thrower",
        { NextAction("high warlord naj'entus assign spine thrower", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("high warlord naj'entus has tidal shield",
        { NextAction("high warlord naj'entus throw impaling spine", ACTION_RAID + 2) }));

    // Supremus
    triggers.push_back(new TriggerNode("supremus hunter should misdirect",
        { NextAction("supremus misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("supremus should be pulled back by tank",
        { NextAction("supremus tanks position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("supremus ranged should spread",
        { NextAction("supremus spread ranged", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("supremus fixated on bot",
        { NextAction("supremus kite boss", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("supremus near fire",
        { NextAction("supremus move away from fire", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("supremus reach blocked by fire",
        { NextAction("supremus reach around fire", ACTION_RAID - 1) }));

    // Shade of Akama
    triggers.push_back(new TriggerNode("shade of akama should attack channelers and sorcerers",
        { NextAction("shade of akama attack channelers and sorcerers", ACTION_RAID + 1) }));

    // Teron Gorefiend
    triggers.push_back(new TriggerNode("teron gorefiend hunter should misdirect",
        { NextAction("teron gorefiend misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("teron gorefiend should be tanked",
        { NextAction("teron gorefiend tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend ranged should position on balcony",
        { NextAction("teron gorefiend position ranged on balcony", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("teron gorefiend casts shadow of death",
        { NextAction("teron gorefiend avoid shadow of death", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend should position for vengeful spirit",
        { NextAction("teron gorefiend move to corner to die", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("teron gorefiend transformed into vengeful spirit",
        { NextAction("teron gorefiend destroy shadowy constructs", ACTION_EMERGENCY + 10) }));

    // Gurtogg Bloodboil
    triggers.push_back(new TriggerNode("gurtogg bloodboil hunter should misdirect",
        { NextAction("gurtogg bloodboil misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should be tanked",
        { NextAction("gurtogg bloodboil tanks position boss", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil should position for bloodboil",
        { NextAction("gurtogg bloodboil rotate ranged groups", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil melee should stay behind",
        { NextAction("gurtogg bloodboil melee move behind", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil ranged should avoid frontal cone",
        { NextAction("gurtogg bloodboil ranged move out of frontal cone", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("gurtogg bloodboil fel rage on bot",
        { NextAction("gurtogg bloodboil lead boss to tank position", ACTION_RAID + 1) }));

    // Reliquary of Souls
    triggers.push_back(new TriggerNode("reliquary of souls hunter should misdirect",
        { NextAction("reliquary of souls misdirect to main tank", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("reliquary of souls should position for suffering",
        { NextAction("reliquary of souls adjust distance from suffering", ACTION_RAID + 2) }));

    // Priority is below standard party dispels to ensure that Soul Drain is dispelled.
    triggers.push_back(new TriggerNode("reliquary of souls healers should attack suffering",
        { NextAction("reliquary of souls healers dps suffering", ACTION_INTERRUPT - 1) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire has rune shield",
        { NextAction("reliquary of souls spellsteal rune shield", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("reliquary of souls essence of desire casting deaden",
        { NextAction("reliquary of souls spell reflect deaden", ACTION_EMERGENCY + 6) }));

    // Mother Shahraz
    triggers.push_back(new TriggerNode("mother shahraz hunter should misdirect",
        { NextAction("mother shahraz misdirect to main tank", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("mother shahraz should be tanked",
        { NextAction("mother shahraz tanks position boss under pillar", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz being positioned",
        { NextAction("mother shahraz melee dps wait at safe position", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz ranged should position under statue",
        { NextAction("mother shahraz position ranged under statue", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("mother shahraz fatal attraction",
        { NextAction("mother shahraz break fatal attraction", ACTION_EMERGENCY + 10) }));

    // Illidari Council
    triggers.push_back(new TriggerNode("illidari council hunter should misdirect",
        { NextAction("illidari council misdirect to tanks", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidari council gathios should be tanked",
        { NextAction("illidari council main tank position gathios", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council gathios casting judgement of command",
        { NextAction(
            "illidari council main tank reflect judgement of command", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidari council malande casting circle of healing",
        { NextAction("illidari council interrupt circle of healing", ACTION_EMERGENCY + 8) }));

    triggers.push_back(new TriggerNode("illidari council malande should be tanked",
        { NextAction("illidari council first assist tank focus malande", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council veras should be tanked",
        { NextAction("illidari council second assist tank position veras", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council zerevor should be tanked by mage",
        { NextAction("illidari council mage tank position zerevor", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidari council mage tank healer should reposition",
        { NextAction("illidari council position mage tank healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council ranged should spread",
        { NextAction("illidari council spread ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidari council pet should switch target",
        { NextAction("illidari council command pet target", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidari council should assign dps priority",
        { NextAction("illidari council assign dps targets", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidari council should manage dps timer",
        { NextAction("illidari council manage dps timer", ACTION_EMERGENCY + 10) }));

    // Illidan Stormrage <The Betrayer>
    triggers.push_back(new TriggerNode("illidan stormrage should position for pull",
        { NextAction("illidan stormrage position on grate during speech", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage hunter should misdirect",
        { NextAction("illidan stormrage misdirect to tanks", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage should be tanked",
        { NextAction("illidan stormrage main tank reposition boss", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage parasitic shadowfiend on group member",
        { NextAction("illidan stormrage isolate bot with infection", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidan stormrage should track infections",
        { NextAction("illidan stormrage track infections", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("illidan stormrage flames of azzinoth should be tanked",
        { NextAction("illidan stormrage assist tanks handle flames", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage pets die to hazards",
        { NextAction("illidan stormrage control pet aggression", ACTION_RAID + 4) }));

    // A pet still passive after a wipe gets its stance back.
    triggers.push_back(new TriggerNode("black temple no encounter in progress",
        { NextAction("illidan stormrage control pet aggression", ACTION_RAID + 4) }));

    triggers.push_back(new TriggerNode("illidan stormrage pet should attack shadowfiend",
        { NextAction("illidan stormrage command pet attack add", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage pet should attack shadow demon",
        { NextAction("illidan stormrage command pet attack add", ACTION_RAID + 3) }));

    triggers.push_back(new TriggerNode("illidan stormrage should position above grate",
        { NextAction("illidan stormrage position above grate", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage immunity can clear debuff",
        { NextAction("illidan stormrage clear debuff with immunity", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("illidan stormrage immunity no longer needed",
        { NextAction("illidan stormrage cancel immunity", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode(
        "illidan stormrage shaman has totems before tanks have aggro",
        { NextAction("illidan stormrage recall totems", ACTION_RAID + 5) }));

    triggers.push_back(new TriggerNode("illidan stormrage should position for landing",
        { NextAction("illidan stormrage position for landing", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage melee should stop attacking",
        { NextAction("illidan stormrage melee stop attacking", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage should stop shooting",
        { NextAction("illidan stormrage stop shooting", ACTION_EMERGENCY + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage ranged should spread",
        { NextAction("illidan stormrage spread ranged", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage ranged should reach shadow demon",
        { NextAction("illidan stormrage reach shadow demon", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage melee should stay away from demon form",
        { NextAction("illidan stormrage melee stay away from demon form", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("illidan stormrage warlock should tank demon form",
        { NextAction("illidan stormrage warlock tank handle demon form", ACTION_EMERGENCY + 9) }));

    triggers.push_back(new TriggerNode("illidan stormrage warlock should seed shadow demons",
        { NextAction("illidan stormrage seed shadow demons", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("illidan stormrage should assign target priority",
        { NextAction("illidan stormrage assign target priority", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("illidan stormrage should manage dps timers",
        { NextAction("illidan stormrage manage dps timers", ACTION_EMERGENCY + 11) }));

    triggers.push_back(new TriggerNode("illidan stormrage should clear hazards before landing",
        { NextAction("illidan stormrage clear hazards", ACTION_EMERGENCY + 10) }));
}

void RaidBlackTempleStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    // Shared
    multipliers.push_back(new BlackTempleDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new BlackTempleDisableKillingSpreeMultiplier(botAI));
    multipliers.push_back(new BlackTempleNoThreatRedirectsMultiplier(botAI));

    // Trash
    multipliers.push_back(new ShadowmoonReaverHoldChargeBuildingSpellsMultiplier(botAI));

    // High Warlord Naj'entus
    multipliers.push_back(new HighWarlordNajentusDisableCombatFormationMoveMultiplier(botAI));
    multipliers.push_back(new HighWarlordNajentusTidalShieldHoldPositionMultiplier(botAI));

    // Supremus
    multipliers.push_back(new SupremusRunAwayWhenFixatedMultiplier(botAI));
    multipliers.push_back(new SupremusDoNotReachThroughFireMultiplier(botAI));
    multipliers.push_back(new SupremusControlMovementMultiplier(botAI));

    // Shade of Akama
    multipliers.push_back(new ShadeOfAkamaDoNotDropOutOfSightTargetMultiplier(botAI));

    // Teron Gorefiend
    multipliers.push_back(new TeronGorefiendControlMovementMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendMarkedBotOnlyMoveToDieMultiplier(botAI));
    multipliers.push_back(new TeronGorefiendSpiritsAttackShadowyConstructsMultiplier(botAI));

    // Gurtogg Bloodboil
    multipliers.push_back(new GurtoggBloodboilControlMovementMultiplier(botAI));
    multipliers.push_back(new GurtoggBloodboilFelRageTargetHoldsAggroMultiplier(botAI));
    multipliers.push_back(new GurtoggBloodboilHoldThreatMultiplier(botAI));

    // Reliquary of Souls
    multipliers.push_back(new ReliquaryOfSoulsDoNotWasteHealingMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsControlMeleeMovementMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsLetMagesStealRuneShieldMultiplier(botAI));
    multipliers.push_back(new ReliquaryOfSoulsSaveSpellReflectionForDeadenMultiplier(botAI));

    // Mother Shahraz
    multipliers.push_back(new MotherShahrazControlMovementMultiplier(botAI));
    multipliers.push_back(new MotherShahrazFatalAttractionRunAwayMultiplier(botAI));

    // Illidari Council
    multipliers.push_back(new IllidariCouncilControlMovementAndTargetingMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilDisableMageTankActionsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilHoldMisdirectionAndDispelsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilManageInterruptsMultiplier(botAI));
    multipliers.push_back(new IllidariCouncilWaitForDpsMultiplier(botAI));

    // Illidan Stormrage <The Betrayer>
    multipliers.push_back(new IllidanStormrageHoldPullPositionsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageControlMovementAndTargetingMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageHoldNearNewShadowfiendMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageHoldGuardianSummonsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageHoldTotemsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageWaitForDpsMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageDisableTankSoulshatterMultiplier(botAI));
    multipliers.push_back(new IllidanStormrageDisableAoeOnShadowDemonsMultiplier(botAI));
}

namespace
{

void AppendSisterOfPainExclusions(Player* bot, GuidSet& exclusions)
{
    if (PlayerbotAI::IsTank(bot))
        return;

    if (Unit* pain = GetSkulledSisterOfPain(bot))
        exclusions.insert(pain->GetGUID());
}

void AppendShadowmoonReaverExclusions(PlayerbotAI* botAI, Player* bot, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsCaster(bot) && !PlayerbotAI::IsHeal(bot))
        return;

    auto const& reavers =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowmoon reavers")->RefGet();
    for (ObjectGuid const& guid : reavers)
    {
        if (IsShadowmoonReaverUnsafeForMagic(botAI->GetUnit(guid)))
            exclusions.insert(guid);
    }
}

// Tanks and ranged other than shadow priests leave channelers and sorcerers to melee dps while
// another add is up. The exclusion ensures the skull doesn't affect targeting.
void AppendShadeOfAkamaChannelerExclusions(PlayerbotAI* botAI, Player* bot, GuidSet& exclusions)
{
    if (PlayerbotAI::IsDps(bot) && (bot->getClass() == CLASS_PRIEST || PlayerbotAI::IsMelee(bot)))
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& adds = context->GetValue<GuidVector>("shade of akama adds")->RefGet();
    if (adds.empty())
        return;

    for (ObjectGuid const& guid : context->GetValue<GuidVector>("attackers")->RefGet())
    {
        if (std::find(adds.begin(), adds.end(), guid) == adds.end())
        {
            exclusions.insert(adds.begin(), adds.end());
            return;
        }
    }
}

// Shadowy Constructs cannot be damaged by anyone other than the Spirit of Vengeance.
void AppendShadowyConstructExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    if (botAI->GetBot()->HasAura(Id(BtSpells::SPELL_SPIRITUAL_VENGEANCE)))
        return;

    auto const& constructs = botAI->GetAiObjectContext()
        ->GetValue<GuidVector>("teron gorefiend shadowy constructs")->RefGet();
    exclusions.insert(constructs.begin(), constructs.end());
}

void AppendIllidanMeleeExclusions(PlayerbotAI* botAI, Player* bot, GuidSet& exclusions)
{
    if (!PlayerbotAI::IsMelee(bot))
        return;

    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* illidan = GetBtBoss(bot, BtBosses::DATA_ILLIDAN_STORMRAGE);
    if (!illidan)
        return;

    auto const& shadowfiends =
        context->GetValue<GuidVector>("illidan stormrage parasitic shadowfiends")->RefGet();
    exclusions.insert(shadowfiends.begin(), shadowfiends.end());

    if (GetIllidanPhase(illidan) != IllidanPhase::Demon)
        return;

    exclusions.insert(illidan->GetGUID());
    if (Unit* demon = GetShadowDemonMovingToWarlockTank(botAI))
        exclusions.insert(demon->GetGUID());
}

} // end anonymous namespace

void RaidBlackTempleStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType type)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != BT_MAP_ID)
        return;

    if (type != TargetValueExclusionType::TankTarget)
    {
        AppendSisterOfPainExclusions(bot, exclusions);
        AppendShadowmoonReaverExclusions(botAI, bot, exclusions);
    }

    if (!IsEncounterInProgress(bot, BT_MAP_ID))
        return;

    if (type == TargetValueExclusionType::TankTarget || type == TargetValueExclusionType::DpsTarget)
        AppendShadeOfAkamaChannelerExclusions(botAI, bot, exclusions);

    AppendShadowyConstructExclusions(botAI, exclusions);
    AppendIllidanMeleeExclusions(botAI, bot, exclusions);
}
