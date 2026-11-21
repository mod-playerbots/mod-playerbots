/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "EncounterHelpers.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SWPEncounter_Felmyst.h"
#include "SWPEncounter_Kalec.h"
#include "SWPEncounter_KJ.h"
#include "SWPEncounter_Muru.h"
#include "SWPEncounter_Twins.h"
#include "SWPShared.h"
#include "SWPTriggers.h"
#include <list>
#include <vector>

using namespace EncounterHelpers;
using namespace SwpHelpers;

namespace
{

PlayerbotAI* FindFirstSunwellCombatBotInGroup(Player* referencePlayer)
{
    if (!referencePlayer)
        return nullptr;

    if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(referencePlayer);
        botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
    {
        return botAI;
    }

    Group* group = referencePlayer->GetGroup();
    if (!group)
        return nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == referencePlayer || member->GetMapId() != SWP_MAP_ID)
            continue;

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(member);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            return botAI;
        }
    }

    return nullptr;
}

bool IsCastingOrChanneling(Player* player)
{
    return player && (player->GetCurrentSpell(CURRENT_GENERIC_SPELL) ||
        player->GetCurrentSpell(CURRENT_CHANNELED_SPELL));
}

// The bot's AI, if the player is a living Sunwell bot mid-cast or channel; otherwise nullptr.
PlayerbotAI* GetCastingSunwellBotAI(Player* player)
{
    if (!player || !player->IsAlive() || !IsCastingOrChanneling(player))
        return nullptr;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    return botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT) ? botAI : nullptr;
}

Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
{
    if (!spell || !caster)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
    for (TargetInfo const& targetInfo : targets)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*caster, targetInfo.targetGUID))
            return target;
    }

    return nullptr;
}

// Each test is the one the move's trigger uses: the fog move, the group move (the early move
// included), and the sidestep.
bool IsFelmystMoveDueForBot(Player* player, PlayerbotAI* botAI, Creature* felmyst)
{
    FogOfCorruptionState fogState;
    if (TryGetActiveFogOfCorruptionState(player, felmyst, fogState))
        return true;

    Position groupPosition;
    if ((IsFelmystAirGroupMoveActive(player, felmyst) || HasFelmystEarlyMoveStarted(botAI) ||
         IsFelmystEarlyMoveDue(player, felmyst)) &&
        TryGetFelmystAirGroupPosition(felmyst, groupPosition) &&
        player->GetExactDist2d(groupPosition) > FELMYST_GROUP_ARRIVAL_DISTANCE)
    {
        return true;
    }

    return felmyst->IsFlying() && !GetFelmystOwnDemonicVapor(player) &&
        IsNearFelmystDemonicVaporHazard(player);
}

void RequestInterruptForBotsNeedingFelmystMovement(Creature* felmyst)
{
    // No state before the bots' triggers first see the encounter in progress
    if (!felmyst || !FindFelmystState(felmyst->GetInstanceId()))
        return;

    Map::PlayerList const& players = felmyst->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
        if (botAI && IsFelmystMoveDueForBot(player, botAI, felmyst))
            botAI->RequestSpellInterrupt();
    }
}

void RequestInterruptForBotsWithFelmystEncapsulate(Creature* felmyst)
{
    if (!felmyst || felmyst->IsFlying())
        return;

    Map::PlayerList const& players = felmyst->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
        if (!botAI)
            continue;

        Player* encapsulateTarget = GetFelmystEncapsulateTarget(player);
        if (ShouldMoveAwayFromFelmystEncapsulateTarget(player, felmyst, encapsulateTarget))
            botAI->RequestSpellInterrupt();
    }
}

// The Conflagration target moves away from the raid, and a bot that pulls aggro on Alythess
// steps aside from its stack.
void RequestInterruptForEredarTwinsAlythessTargets(Creature* alythess)
{
    if (!alythess)
        return;

    ObjectGuid const conflagTargetGuid = GetEredarTwinsConflagrationTargetGuid(alythess, nullptr);
    if (!conflagTargetGuid.IsEmpty())
    {
        Player* conflagTarget = ObjectAccessor::GetPlayer(*alythess, conflagTargetGuid);
        if (PlayerbotAI* botAI = GetCastingSunwellBotAI(conflagTarget))
            botAI->RequestSpellInterrupt();
    }

    Unit* victim = alythess->GetVictim();
    Player* player = victim ? victim->ToPlayer() : nullptr;
    if (!IsHoldingAlythessAggro(player, alythess))
        return;

    Position const aside = GetEredarTwinsStepAsidePosition(
        alythess, player->GetPositionZ() > EREDAR_TWINS_BALCONY_Z);
    PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
    if (botAI && player->GetExactDist2d(aside) > EREDAR_TWINS_STEP_ASIDE_ARRIVAL_DISTANCE)
        botAI->RequestSpellInterrupt();
}

// A bot in a long cast can't move until it ends, so a new void zone (about to summon a fiend) stops
// the casts of bots within the fiend's clearance.
void RequestInterruptForBotsNearNewMuruVoidZone(Creature* voidZone)
{
    if (!voidZone || !voidZone->HasAura(Id(SwpSpells::SPELL_VOID_ZONE_VISUAL)))
        return;

    Map::PlayerList const& players = voidZone->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
        if (botAI && player->GetExactDist(voidZone) < DARK_FIEND_CLEARANCE)
            botAI->RequestSpellInterrupt();
    }
}

// In phase 2, where bots keep clear of fiends, a fiend's victim in a long cast stops it to move.
void RequestInterruptForMuruDarkFiendVictim(Creature* darkFiend)
{
    if (!darkFiend)
        return;

    Unit* victimUnit = darkFiend->GetVictim();
    Player* victim = victimUnit ? victimUnit->ToPlayer() : nullptr;
    PlayerbotAI* botAI = GetCastingSunwellBotAI(victim);
    if (!botAI || victim->GetExactDist(darkFiend) >= DARK_FIEND_CLEARANCE)
        return;

    InstanceScript* instance = darkFiend->GetInstanceScript();
    Creature* muru = instance ? instance->GetCreature(Id(SwpBosses::DATA_MURU)) : nullptr;
    if (IsMuruPhaseActive(muru))
        return;

    botAI->RequestSpellInterrupt();
}

void RecordKiljaedenArmageddonTarget(Creature* armageddonTarget)
{
    uint32 const instanceId = armageddonTarget->GetInstanceId();
    KiljaedenEncounterState const* state = FindKiljaedenState(instanceId);
    if (state && state->trackedArmageddonTargets.contains(armageddonTarget->GetGUID()))
        return;

    bool hasSunwellStrategy = false;
    std::vector<PlayerbotAI*> botsToInterrupt;
    Map::PlayerList const& players = armageddonTarget->GetMap()->GetPlayers();
    for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
    {
        Player* player = it->GetSource();
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            botAI && botAI->HasStrategy("sunwell", BOT_STATE_COMBAT))
        {
            hasSunwellStrategy = true;

            if (!player->IsAlive() || !IsCastingOrChanneling(player) ||
                HasKiljaedenDragonAura(player))
            {
                continue;
            }

            if (armageddonTarget->GetExactDist2d(player) > ARMAGEDDON_CLEARANCE)
                continue;

            botsToInterrupt.push_back(botAI);
        }
    }

    if (!hasSunwellStrategy)
        return;

    KiljaedenState(instanceId).trackedArmageddonTargets.insert(armageddonTarget->GetGUID());

    AddKiljaedenArmageddon(
        instanceId, armageddonTarget->GetPosition(), ARMAGEDDON_HAZARD_DURATION_MS,
        ARMAGEDDON_CLEARANCE);

    for (PlayerbotAI* botAI : botsToInterrupt)
        botAI->RequestSpellInterrupt();
}

} // end anonymous namespace

class KalecgosPortalSpellListenerScript : public AllSpellScript
{
public:
    KalecgosPortalSpellListenerScript() : AllSpellScript("KalecgosPortalSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        uint32 const spellId = spellInfo->Id;
        if (spellId != Id(SwpSpells::SPELL_SPECTRAL_BLAST_PORTAL) &&
            spellId != Id(SwpSpells::SPELL_TELEPORT_SPECTRAL))
        {
            return;
        }

        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player)
            return;

        PlayerbotAI* botAI = FindFirstSunwellCombatBotInGroup(player);
        if (!botAI)
            return;

        if (spellId == Id(SwpSpells::SPELL_SPECTRAL_BLAST_PORTAL))
            RecordSpectralBlastTarget(player, botAI);
        else
            RecordSpectralRealmEnter(player);
    }
};

class FelmystSpellListenerScript : public AllSpellScript
{
public:
    FelmystSpellListenerScript() : AllSpellScript("FelmystSpellListenerScript") {}

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* spellInfo) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_ENCAPSULATE))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (FindFirstSunwellCombatBotInGroup(target))
            RecordFelmystIncomingEncapsulateTarget(target);
    }

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_SUMMON_DEMONIC_VAPOR))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (FindFirstSunwellCombatBotInGroup(target))
            RecordFelmystDemonicVaporSummon(target);

        if (PlayerbotAI* botAI = GetCastingSunwellBotAI(target))
            botAI->RequestSpellInterrupt();
    }
};

// Records a completed Conflagration cast, so its target holds position while the missile flies.
class EredarTwinsConflagrationSpellListenerScript : public AllSpellScript
{
public:
    EredarTwinsConflagrationSpellListenerScript()
        : AllSpellScript("EredarTwinsConflagrationSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_CONFLAGRATION))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (target && FindFirstSunwellCombatBotInGroup(target))
            RecordEredarTwinsConflagrationImpact(target);
    }
};

// A mage mid-cast when a Fury Mage gains Spell Fury stops the cast to steal it.
class MuruSpellFurySpellListenerScript : public AllSpellScript
{
public:
    MuruSpellFurySpellListenerScript() : AllSpellScript("MuruSpellFurySpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_SPELL_FURY))
            return;

        uint32 const spellsteal = Id(SwpSpells::SPELL_SPELLSTEAL);
        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
            if (!botAI || player->getClass() != CLASS_MAGE ||
                !player->IsWithinCombatRange(caster, RANGED_ABILITY_REACH))
            {
                continue;
            }

            if (!player->HasSpell(spellsteal) || player->HasSpellCooldown(spellsteal) ||
                player->HasAura(Id(SwpSpells::SPELL_SPELL_FURY)))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

class KiljaedenDarknessSpellListenerScript : public AllSpellScript
{
public:
    KiljaedenDarknessSpellListenerScript()
        : AllSpellScript("KiljaedenDarknessSpellListenerScript") {}

    void OnSpellCast(
        Spell* /*spell*/, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!caster || spellInfo->Id != Id(SwpSpells::SPELL_DARKNESS_OF_A_THOUSAND_SOULS))
            return;

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            PlayerbotAI* botAI = GetCastingSunwellBotAI(player);
            if (botAI && !HasKiljaedenDragonAura(player))
                botAI->RequestSpellInterrupt();
        }
    }
};

class SunwellCreatureUpdateScript : public AllCreatureScript
{
public:
    SunwellCreatureUpdateScript() : AllCreatureScript("SunwellCreatureUpdateScript") {}

    void OnAllCreatureUpdate(Creature* creature, uint32 /*diff*/) override
    {
        if (!creature)
            return;

        switch (creature->GetEntry())
        {
            case Id(SwpNpcs::NPC_FELMYST):
            {
                if (IsBossInProgress(creature, SWP_MAP_ID, Id(SwpBosses::DATA_FELMYST)))
                {
                    RequestInterruptForBotsNeedingFelmystMovement(creature);
                    RequestInterruptForBotsWithFelmystEncapsulate(creature);
                }
                break;
            }
            case Id(SwpNpcs::NPC_GRAND_WARLOCK_ALYTHESS):
            {
                if (IsBossInProgress(creature, SWP_MAP_ID, Id(SwpBosses::DATA_EREDAR_TWINS)))
                    RequestInterruptForEredarTwinsAlythessTargets(creature);
                break;
            }
            case Id(SwpNpcs::NPC_DARKNESS):
            {
                if (IsBossInProgress(creature, SWP_MAP_ID, Id(SwpBosses::DATA_MURU)))
                    RequestInterruptForBotsNearNewMuruVoidZone(creature);
                break;
            }
            case Id(SwpNpcs::NPC_DARK_FIEND):
            {
                if (IsBossInProgress(creature, SWP_MAP_ID, Id(SwpBosses::DATA_MURU)))
                    RequestInterruptForMuruDarkFiendVictim(creature);
                break;
            }
            case Id(SwpNpcs::NPC_ARMAGEDDON_TARGET):
            {
                if (IsBossInProgress(creature, SWP_MAP_ID, Id(SwpBosses::DATA_KILJAEDEN)))
                    RecordKiljaedenArmageddonTarget(creature);
                break;
            }
            default:
                break;
        }
    }

    // Ungated: cleanup runs whatever the encounter state.
    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (!creature || creature->GetEntry() != Id(SwpNpcs::NPC_ARMAGEDDON_TARGET))
            return;

        if (KiljaedenEncounterState* state = FindKiljaedenState(creature->GetInstanceId()))
            state->trackedArmageddonTargets.erase(creature->GetGUID());
    }
};

void AddSC_SunwellBotScripts()
{
    // AllSpellScript
    new KalecgosPortalSpellListenerScript();
    new FelmystSpellListenerScript();
    new EredarTwinsConflagrationSpellListenerScript();
    new MuruSpellFurySpellListenerScript();
    new KiljaedenDarknessSpellListenerScript();
    // AllCreatureScript
    new SunwellCreatureUpdateScript();
}
