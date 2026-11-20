/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraStrategy.h"
#include "AiObjectContext.h"
#include "KaraHelpers.h"
#include "KaraMultipliers.h"
#include "Playerbots.h"

void RaidKarazhanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shared
    triggers.push_back(new TriggerNode("karazhan no encounter in progress",
        { NextAction("karazhan reset encounter states", ACTION_EMERGENCY + 10) }));

    triggers.push_back(new TriggerNode("karazhan enemies cast fear",
        { NextAction("karazhan set tremor totem", ACTION_RAID) }));

    // Trash
    triggers.push_back(new TriggerNode("mana warp incoming warp breach",
        { NextAction("mana warp cast stun spell", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("mana warp target at low health",
        { NextAction("mana warp stand by to stun", ACTION_RAID) }));

    // Attumen the Huntsman
    triggers.push_back(new TriggerNode("attumen the huntsman unmounted should be tanked",
        { NextAction("attumen the huntsman assist tank position unmounted", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman midnight should be focused",
        { NextAction("attumen the huntsman attack midnight", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("attumen the huntsman mounted should be tanked",
        { NextAction("attumen the huntsman tank position mounted", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman should stack behind",
        { NextAction("attumen the huntsman stack behind", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("attumen the huntsman should hold dps",
        { NextAction("attumen the huntsman stop attacking", ACTION_RAID + 3),
          NextAction("attumen the huntsman select target for hold", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("attumen the huntsman should manage dps timer",
        { NextAction("attumen the huntsman manage dps timer", ACTION_EMERGENCY + 10) }));

    // Moroes
    triggers.push_back(new TriggerNode("moroes should prioritize guests",
        { NextAction("moroes mark target", ACTION_RAID) }));

    // Maiden of Virtue
    triggers.push_back(new TriggerNode("maiden of virtue should be tanked",
        { NextAction("maiden of virtue tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("maiden of virtue should break repentance",
        { NextAction("maiden of virtue move boss to stunned healer", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("maiden of virtue ranged should spread",
        { NextAction("maiden of virtue position ranged between pillars", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("maiden of virtue shaman should ground holy fire",
        { NextAction("maiden of virtue set grounding totem", ACTION_RAID) }));

    // The Big Bad Wolf
    triggers.push_back(new TriggerNode("big bad wolf should be tanked",
        { NextAction("big bad wolf tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("big bad wolf chases red hood",
        { NextAction("big bad wolf red hood use escape ability", ACTION_EMERGENCY + 7),
          NextAction("big bad wolf red hood run away", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("big bad wolf victim has red hood",
        { NextAction("big bad wolf protect red hood", ACTION_EMERGENCY + 5) }));

    // Romulo and Julianne
    triggers.push_back(new TriggerNode("romulo and julianne should die together",
        { NextAction("romulo and julianne skull higher health", ACTION_RAID) }));

    // The Wizard of Oz
    triggers.push_back(new TriggerNode("wizard of oz should prioritize targets",
        { NextAction("wizard of oz mark target", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("wizard of oz mage should burn strawman",
        { NextAction("wizard of oz cast scorch on strawman", ACTION_RAID) }));

    // The Curator
    triggers.push_back(new TriggerNode("the curator should prioritize astral flares",
        { NextAction("the curator mark astral flare", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("the curator should be tanked",
        { NextAction("the curator tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("the curator ranged should spread",
        { NextAction("the curator spread ranged", ACTION_RAID) }));

    // Terestian Illhoof
    triggers.push_back(new TriggerNode("terestian illhoof should prioritize targets",
        { NextAction("terestian illhoof mark target", ACTION_RAID) }));

    // Shade of Aran
    triggers.push_back(new TriggerNode("shade of aran casting arcane explosion",
        { NextAction("shade of aran run away from arcane explosion", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("shade of aran ranged in blizzard",
        { NextAction("shade of aran move behind blizzard", ACTION_EMERGENCY + 5) }));

    triggers.push_back(new TriggerNode("shade of aran flame wreath active",
        { NextAction("shade of aran stop moving", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("shade of aran should prioritize elementals",
        { NextAction("shade of aran mark elemental", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("shade of aran ranged should maintain distance",
        { NextAction("shade of aran ranged maintain distance", ACTION_RAID + 1) }));

    // Netherspite
    triggers.push_back(new TriggerNode("netherspite should block red beam",
        { NextAction("netherspite block red beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should block blue or green beam",
        { NextAction("netherspite block blue or green beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite next beam blocker should prepare",
        { NextAction("netherspite wait to block beam", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should stand between beams",
        { NextAction("netherspite move between blue and green beams", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("netherspite should spread for netherbreath",
        { NextAction("netherspite move to banish slot", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("netherspite should hold dps",
        { NextAction("netherspite stop attacking", ACTION_EMERGENCY + 8),
          NextAction("netherspite select target for hold", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("netherspite should manage pet aggression",
        { NextAction("netherspite set pet stance", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("netherspite should manage timers and trackers",
        { NextAction("netherspite manage timers and trackers", ACTION_EMERGENCY + 10) }));

    // Prince Malchezaar
    triggers.push_back(new TriggerNode("prince malchezaar bot is enfeebled",
        { NextAction("prince malchezaar enfeebled bot run to safety", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("prince malchezaar near hazard",
        { NextAction("prince malchezaar move clear of hazards", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("prince malchezaar should be tanked",
        { NextAction("prince malchezaar tank position boss", ACTION_EMERGENCY + 6) }));

    // Nightbane
    triggers.push_back(new TriggerNode("nightbane should be tanked",
        { NextAction("nightbane tank position boss", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("nightbane ranged should avoid charred earth",
        { NextAction("nightbane ranged move from charred earth", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nightbane melee should avoid charred earth",
        { NextAction("nightbane melee move from charred earth", ACTION_EMERGENCY + 1) }));

    triggers.push_back(new TriggerNode("nightbane should manage pet aggression",
        { NextAction("nightbane set pet stance", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nightbane should hold dps",
        { NextAction("nightbane stop attacking", ACTION_RAID + 3),
          NextAction("nightbane select target for hold", ACTION_RAID + 2) }));

    triggers.push_back(new TriggerNode("nightbane in flight phase",
        { NextAction("nightbane stack away from rain of bones", ACTION_RAID) }));

    triggers.push_back(new TriggerNode("nightbane in landing phase",
        { NextAction("nightbane leave combat for landing", ACTION_RAID + 1) }));

    triggers.push_back(new TriggerNode("nightbane should manage timers and trackers",
        { NextAction("nightbane manage timers and trackers", ACTION_EMERGENCY + 10) }));
}

void RaidKarazhanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new KarazhanKeepTremorTotemMultiplier(botAI));
    multipliers.push_back(new KarazhanDelayDpsCooldownsMultiplier(botAI));
    multipliers.push_back(new ManaWarpSaveStunsMultiplier(botAI));
    multipliers.push_back(new KarazhanDisableCombatFormationMoveMultiplier(
        botAI, "attumen the huntsman disable combat formation move",
        KaraHelpers::KaraBosses::DATA_ATTUMEN));
    multipliers.push_back(new AttumenTheHuntsmanDisableAutoTargetingMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanStayStackedMultiplier(botAI));
    multipliers.push_back(new AttumenTheHuntsmanHoldDpsMultiplier(botAI));
    multipliers.push_back(new KarazhanDisableCombatFormationMoveMultiplier(
        botAI, "maiden of virtue disable combat formation move",
        KaraHelpers::KaraBosses::DATA_MAIDEN));
    multipliers.push_back(new MaidenOfVirtueKeepGroundingTotemMultiplier(botAI));
    multipliers.push_back(new BigBadWolfRedHoodHunterStayDownMultiplier(botAI));
    multipliers.push_back(new TheCuratorDisableTankAssistMultiplier(botAI));
    multipliers.push_back(new KarazhanDisableCombatFormationMoveMultiplier(
        botAI, "the curator disable combat formation move",
        KaraHelpers::KaraBosses::DATA_CURATOR));
    multipliers.push_back(new TerestianIllhoofDisableDotsOnFiendishImpsMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranArcaneExplosionDisableMovementMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranDisableReactiveMovementMultiplier(botAI));
    multipliers.push_back(new ShadeOfAranFlameWreathDisableMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteDisableReactiveMovementMultiplier(botAI));
    multipliers.push_back(new NetherspiteHoldDpsMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarDisableReactiveMovementMultiplier(botAI));
    multipliers.push_back(new PrinceMalchezaarEnfeebledBotFocusOnEscapeMultiplier(botAI));
    multipliers.push_back(new NightbaneDelaySummonsAndPetAttacksMultiplier(botAI));
    multipliers.push_back(new NightbaneHoldDpsMultiplier(botAI));
    multipliers.push_back(new NightbaneDisableMovementMultiplier(botAI));
}

void AppendNightbaneFlightPhaseExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    Unit* nightbane =
        KaraHelpers::GetKaraBoss(botAI->GetBot(), KaraHelpers::KaraBosses::DATA_NIGHTBANE);
    if (!nightbane)
        return;

    if (KaraHelpers::IsNightbaneFlying(nightbane))
        exclusions.insert(nightbane->GetGUID());

    // Somehow, skeletons can get out of bounds, and bots can chase them out of bounds as a result.
    // So we ensure only in-bounds skeletons are valid targets.
    uint32 const skeletonEntry = KaraHelpers::Id(KaraHelpers::KaraNpcs::NPC_RESTLESS_SKELETON);
    GuidVector const targets =
        botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const& guid : targets)
    {
        if (guid.GetEntry() != skeletonEntry)
            continue;

        Unit* skeleton = botAI->GetUnit(guid);
        if (skeleton && KaraHelpers::IsOutsideNightbaneTerrace(skeleton))
            exclusions.insert(guid);
    }
}

// Midnight is the phase 1 target. Do not count Attumen for AoE or put secondary DoTs on him, or
// DPS him at all.
void AppendAttumenUnmountedNonTankExclusions(PlayerbotAI* botAI, GuidSet& exclusions)
{
    Unit* attumen = KaraHelpers::GetKaraBoss(
        botAI->GetBot(), KaraHelpers::KaraBosses::DATA_ATTUMEN);
    if (!attumen || attumen->GetEntry() ==
            KaraHelpers::Id(KaraHelpers::KaraNpcs::NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED))
    {
        return;
    }

    exclusions.insert(attumen->GetGUID());
}

void RaidKarazhanStrategy::AppendTargetExclusions(
    GuidSet& exclusions, TargetValueExclusionType type)
{
    if (botAI->GetBot()->GetMapId() != KaraHelpers::KARA_MAP_ID)
        return;

    AppendNightbaneFlightPhaseExclusions(botAI, exclusions);

    if (type != TargetValueExclusionType::TankTarget)
        AppendAttumenUnmountedNonTankExclusions(botAI, exclusions);
}
